#include "OkapiConnector.h"

// Routine to initialize communications with OKAPI
OkapiConnector::Result OkapiConnector::init(method mtd, string username, string password)
{
  this->username = username;
  this->password = password;
	Result result;
	http_client auth(U("https://okapi-development.eu.auth0.com/oauth/token/"));
	web::json::value request_token_payload;
	request_token_payload[U("grant_type")] = web::json::value::string("password");
	request_token_payload[U("username")] = web::json::value::string(username);
	request_token_payload[U("password")] = web::json::value::string(password);
	request_token_payload[U("audience")] = web::json::value::string("https://api.okapiorbits.space/picard");
	request_token_payload[U("scope")] = web::json::value::string("('pass_predictions pass_prediction_requests' 'neptune_propagation neptune_propagation_request' 'pass_predictions_long pass_prediction_requests_long')");
	request_token_payload[U("client_id")] = web::json::value::string("jrk0ZTrTuApxUstXcXdu9r71IX5IeKD3");

	auth.request(mtd, "", request_token_payload).then([&](http_response response)
	{
		result.error.code = response.status_code();
		response.headers().set_content_type("application/json");
		json::value access_token_response = response.extract_json().get();
		result.body = access_token_response;
		json::object access_token_response_obj = access_token_response.as_object();

		if (result.error.code != 200 && result.error.code != 202)
		{
			if(result.error.code == 400)
			{
				result.error.message = "Audience Error.";
				result.error.status = "FATAL";
			}
			else if(result.error.code == 401)
			{
				result.error.message = "You are unauthorized.";
				result.error.status = "FATAL";
			}
			else if(result.error.code == 403)
			{
				result.error.message = "Your password or email is wrong.";
				result.error.status = "FATAL";
			}
			else if(result.error.code == 404)
			{
				result.error.message = "URL not found.";
				result.error.status = "FATAL";
			}
			else if(result.error.code == 408)
			{
				result.error.message = "Got timeout when sending request.";
				result.error.status = "FATAL";
			}
			else if(result.error.code == 422)
			{
				result.error.message = "Probably wrong format.";
				result.error.status = "FATAL";
			}
			else if(result.error.code == 429)
			{
				result.error.message = "Your Auth0 account has been blocked after 10 failed logins, check your e-mail.";
				result.error.status = "FATAL";
			}
			else if(result.error.code == 520)
			{
				result.error.message = "Got unknown exception, maybe wrong URL.";
				result.error.status = "FATAL";
			}
			else
			{
				result.error.message = "Probably wrong format.";
				result.error.status = "FATAL";
			}
		}
		else
		{
			if (access_token_response.has_field(U("access_token")))
			{
				accessToken = access_token_response_obj.at(U("access_token")).as_string();
				result.error.message = "No message available";
				result.error.status = "OK";
			}
			else
			{
				result.error.message = "access Token missing in response from Auth0.";
			}
		}
	}).wait();
	return result;
}

// Send a request to OKAPI
OkapiConnector::Result OkapiConnector::sendRequest(string baseUrl, string endpoint, web::json::value requestBody)
{
	Result result;
  
  // Compile the proper post request
  http_client okapiRequest(baseUrl + endpoint);
  http_request request(methods::POST);
  request.set_body(requestBody);
  request.headers().add(U("access_token"), this->accessToken);
  
	okapiRequest.request(request).then([&](http_response response)
	{
		result.error.code = response.status_code();
		response.headers().set_content_type("application/json");
		json::value send_request_response = response.extract_json().get();

		result.body = send_request_response;

		std::stringstream stream;
		stream << result.body;

		boost::property_tree::ptree responseTree;
		boost::property_tree::read_json(stream, responseTree);

		if (result.error.code != 200 && result.error.code != 202)
		{
			if(responseTree.get_optional<std::string>("status"))
			{
				result.error.message = responseTree.get<std::string>("status.text");
				result.error.status = responseTree.get<std::string>("status.type");
			}
			else if(result.error.code == 401)
			{
				result.error.message = "You are unauthorized.";
				result.error.status = "FATAL";
			}
			else if(result.error.code == 404)
			{
				result.error.message = "URL not found.";
				result.error.status = "FATAL";
			}
			else if(result.error.code == 408)
			{
				result.error.message = "Got timeout when sending request.";
				result.error.status = "FATAL";
			}
			else if(result.error.code == 422)
			{
				result.error.message = "Probably wrong format.";
				result.error.status = "FATAL";
			}
			else if(result.error.code == 500)
			{
				result.error.message = "Internal Error.";
				result.error.status = "FATAL";
			}
			else if(result.error.code == 520)
			{
				result.error.message = "Got unknown exception, maybe wrong URL.";
				result.error.status = "FATAL";
			}
			else
			{
				result.error.message = "Unknown Error.";
				result.error.status = "FATAL";
			}
		}
		else
		{
			if(send_request_response.has_field(U("request_id")))
			{
				json::object send_request_response_obj = send_request_response.as_object();
				requestId = send_request_response_obj.at(U("request_id")).as_string();
				result.error.message = responseTree.get<std::string>("status.text");
				result.error.status = responseTree.get<std::string>("status.type");
			}
			else
			{
				result.error.message = "request ID missing in response from OKAPI.";
				result.error.status = "FATAL";
			}
		}
	}).wait();
	return result;
}

// get the result from a service execution request from OKAPI
OkapiConnector::Result OkapiConnector::getResult(string baseUrl, string endpoint, string requestId, string resultType)
{
	Result result;
  
  // Compile the proper get request
  string url = baseUrl + endpoint + requestId + resultType;
  http_client okapiGet(url);
  
  http_request request(methods::GET);
  request.headers().add(U("access_token"), this->accessToken);
 
	okapiGet.request(request).then([&](http_response response)
	{
		result.error.code = response.status_code();
		response.headers().set_content_type("application/json");

		result.body = response.extract_json().get();
		std::stringstream stream;
		stream << result.body;

		boost::property_tree::ptree responseTree;
		boost::property_tree::read_json(stream, responseTree);

    if(result.error.code != 200 && result.error.code != 202)
    {
      if(responseTree.get_optional<std::string>("status"))
      {
        result.error.message = responseTree.get<std::string>("status.text");
        result.error.status = responseTree.get<std::string>("status.type");
      }
      else if(result.error.code == 401)
      {
        result.error.message = "You are unauthorized.";
        result.error.status = "FATAL";
      }
      else if(result.error.code == 404)
      {
        result.error.message = "URL not found.";
        result.error.status = "FATAL";
      }
      else if(result.error.code == 408)
      {
        result.error.message = "Got timeout when sending request.";
        result.error.status = "FATAL";
      }
      else if(result.error.code == 422)
      {
        result.error.message = "Probably wrong format.";
        result.error.status = "FATAL";
      }
      else if(result.error.code == 500)
      {
        result.error.message = "Internal Error.";
        result.error.status = "FATAL";
      }
      else if(result.error.code == 520)
      {
        result.error.message = "Got unknown exception, maybe wrong URL.";
        result.error.status = "FATAL";
      }
      else
      {
        result.error.message = "Unknown Error.";
        result.error.status = "FATAL";
      }
    }
    else
    {
      if(responseTree.get_optional<std::string>("status"))
      {
        result.error.message = responseTree.get<std::string>("status.text");
        result.error.status = responseTree.get<std::string>("status.type");
      }
    }
  }).wait();
	return result;
}
