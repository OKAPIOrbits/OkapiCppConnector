#include "OkapiConnector.h"

// Routine to initialize communications with the OKAPI
OkapiConnector::OkapiResult OkapiConnector::init(string baseUrl, string username, string password)
{
	this->baseUrl = baseUrl;
	this->username = username;
	this->password = password;
	OkapiResult result;
	http_client auth0(U("https://okapi-development.eu.auth0.com/oauth/token/"));
	web::json::value request_token_payload;
	request_token_payload[U("grant_type")] = web::json::value::string("password");
	request_token_payload[U("username")] = web::json::value::string(username);
	request_token_payload[U("password")] = web::json::value::string(password);
	request_token_payload[U("audience")] = web::json::value::string("https://api.okapiorbits.space/picard");
	request_token_payload[U("scope")] = web::json::value::string("('pass_predictions pass_prediction_requests' 'neptune_propagation neptune_propagation_request' 'pass_predictions_long pass_prediction_requests_long')");
	request_token_payload[U("client_id")] = web::json::value::string("jrk0ZTrTuApxUstXcXdu9r71IX5IeKD3");

	auth0.request(methods::POST, "", request_token_payload).then([&](http_response response)
	{
		result.error.code = response.status_code();
		response.headers().set_content_type("application/json");
		result.body = response.extract_json().get();

		if (result.error.code != 200 && result.error.code != 202)
		{
			web::json::object errorObject = result.body.as_object();
			result.error.message = errorObject.at(U("error")).as_string() + " error: " + errorObject.at(U("error_description")).as_string();
			result.error.status = "FATAL";
			return;
		}

		this->accessToken = result.body.as_object().at(U("access_token")).as_string();

	}).wait();
	return result;
}

// Send "add satellite" request to OKAPI
OkapiConnector::OkapiResult OkapiConnector::addSatellite(web::json::value newSatelliteJson)
{
	return sendRequest("/satellites", newSatelliteJson);
}

// Send "update satellite" request to OKAPI
OkapiConnector::OkapiResult OkapiConnector::updateSatellite(web::json::value satelliteJson, string satelliteId)
{
	return updateRequest("/satellites/" + satelliteId, satelliteJson);
}

// Send "get satellites" request to OKAPI
OkapiConnector::OkapiResult OkapiConnector::getSatellites()
{
	return getValues("/satellites");
}

// Send "delete satellite" request to OKAPI
OkapiConnector::OkapiResult OkapiConnector::deleteSatellite(string satelliteId)
{
	return deleteRequest("/satellites/" + satelliteId);
}

// Send "get conjunctions" request to OKAPI
OkapiConnector::OkapiResult OkapiConnector::getConjunctions()
{
	return getValues("/conjunctions");
}

// Send "get cdms" request to OKAPI
OkapiConnector::OkapiResult OkapiConnector::getCdms(string conjunctionId)
{
	return getValues("/conjunctions/" + conjunctionId + "/cdms");
}

// Send "get maneuver evals" request to OKAPI
OkapiConnector::OkapiResult OkapiConnector::getManeuverEvals(string conjunctionId)
{
	return getValues("/conjunctions/" + conjunctionId + "/maneuver-evals");
}

// Get the requestId from the Okapi response
string OkapiConnector::getRequestId(OkapiResult response)
{
	return response.body.as_object().at(U("request_id")).as_string();
}

// Send a request to the OKAPI platform
OkapiConnector::OkapiResult OkapiConnector::sendRequest(string endpoint, web::json::value requestBody)
{
	OkapiResult result;

	// Compile the post request
	http_client okapiRequest(this->baseUrl + (endpoint[0] == '/' ? endpoint.substr(1) : endpoint));
	http_request request(methods::POST);
	request.set_body(requestBody);
	request.headers().add(U("Authorization"), "Bearer " + this->accessToken);

	okapiRequest.request(request).then([&](http_response response)
	{
		result.error.code = response.status_code();
		response.headers().set_content_type("application/json");
		result.body = response.extract_json().get();

		if (result.error.code != 200 && result.error.code != 202)
		{
			web::json::object errorObject = result.body.as_object().at(U("status")).as_object();
			result.error.message = errorObject.at(U("text")).as_string();
			//result.error.message = result.body.serialize();
			result.error.status = errorObject.at(U("type")).as_string();
		}
	}).wait();
	return result;
}

// Send an update request to the OKAPI platform
OkapiConnector::OkapiResult OkapiConnector::updateRequest(string endpoint, web::json::value requestBody)
{
	OkapiResult result;

	// Compile the put request
	http_client okapiRequest(this->baseUrl + (endpoint[0] == '/' ? endpoint.substr(1) : endpoint));
	http_request request(methods::PUT);
	request.set_body(requestBody);
	request.headers().add(U("Authorization"), "Bearer " + this->accessToken);

	okapiRequest.request(request).then([&](http_response response)
	{
		result.error.code = response.status_code();
		response.headers().set_content_type("application/json");
		result.body = response.extract_json().get();

		if (result.error.code != 200 && result.error.code != 202)
		{
			web::json::object errorObject = result.body.as_object().at(U("status")).as_object();
			result.error.message = errorObject.at(U("text")).as_string();
			result.error.status = errorObject.at(U("type")).as_string();
		}
	}).wait();
	return result;
}

// get the result from a service execution request from OKAPI
OkapiConnector::OkapiResult OkapiConnector::getValues(string endpoint)
{
	OkapiResult result;

	// Compile the proper get request
	http_client okapiGet(this->baseUrl + (endpoint[0] == '/' ? endpoint.substr(1) : endpoint));
	http_request request(methods::GET);
	request.headers().add(U("Authorization"), "Bearer " + this->accessToken);

	okapiGet.request(request).then([&](http_response response)
	{
		result.error.code = response.status_code();
		response.headers().set_content_type("application/json");
		result.body = response.extract_json().get();

		if (result.error.code != 200 && result.error.code != 202)
		{
			web::json::object errorObject = result.body.as_object().at(U("status")).as_object();
			result.error.message = errorObject.at(U("text")).as_string();
			result.error.status = errorObject.at(U("type")).as_string();
		}
	}).wait();

	return result;
}

// get the result from a service execution request from OKAPI
OkapiConnector::OkapiResult OkapiConnector::deleteRequest(string endpoint)
{
	OkapiResult result;

	// Compile the proper get request
	http_client okapiGet(this->baseUrl + (endpoint[0] == '/' ? endpoint.substr(1) : endpoint));
	http_request request(methods::DEL);
	request.headers().add(U("Authorization"), "Bearer " + this->accessToken);

	okapiGet.request(request).then([&](http_response response)
	{
		result.error.code = response.status_code();
		response.headers().set_content_type("application/json");
		result.body = response.extract_json().get();

		if (result.error.code != 200 && result.error.code != 202)
		{
			web::json::object errorObject = result.body.as_object().at(U("status")).as_object();
			result.error.message = errorObject.at(U("text")).as_string();
			result.error.status = errorObject.at(U("type")).as_string();
		}
	}).wait();

	return result;
}

/**
 * This is a little helper function. It polls for the result until it is ready.
 */
OkapiConnector::OkapiResult OkapiConnector::waitForProcessingAndGetValues(string endpoint)
{
  OkapiConnector::OkapiResult result;

  do {
    result = getValues(endpoint);
	cout << result.error.code << endl;
	sleep(1);
  } while(result.error.code == 202);

  return result;
}
