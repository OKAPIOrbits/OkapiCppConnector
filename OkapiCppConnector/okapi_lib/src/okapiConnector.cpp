#include "okapiConnector.h"

std::string error_auth0;
std::string error_okapi;
std::string error_okapi_receive;
std::string err;
int http_code;


struct Error400
{
public:
	void ReportError()
	{
		web::json::value error;
		error[U("message")] = web::json::value::string(error_auth0);
		error[U("status")] = web::json::value::string("FATAL");
		error[U("web_status")] = web::json::value::number(400);
		err = error.serialize();
		std::cout << err << std::endl;
	}
};

struct Error401
{
public:
	void ReportError()
	{
		web::json::value error;
		error[U("message")] = web::json::value::string(error_auth0);
		error[U("status")] = web::json::value::string("FATAL");
		error[U("web_status")] = web::json::value::number(401);
		err = error.serialize();
		std::cout << err << std::endl;
	}
};

struct Error403
{
public:
	void ReportError()
	{
		web::json::value error;
		error[U("message")] = web::json::value::string(error_auth0);
		error[U("status")] = web::json::value::string("FATAL");
		error[U("web_status")] = web::json::value::number(403);
		err = error.serialize();
		std::cout << err << std::endl;
	}
};

struct Error408
{
public:
	void ReportError()
	{
		web::json::value error;
		error[U("message")] = web::json::value::string("Got timeout when sending request.");
		error[U("status")] = web::json::value::string("FATAL");
		error[U("web_status")] = web::json::value::number(408);
		err = error.serialize();
		std::cout << err << std::endl;
	}
};

struct Error422
{
public:
	void ReportError()
	{
		web::json::value error;
		error[U("message")] = web::json::value::string(error_okapi);
		error[U("status")] = web::json::value::string("FATAL");
		error[U("web_status")] = web::json::value::number(422);
		err = error.serialize();
		std::cout << err << std::endl;
	}
};

struct Error429
{
public:
	void ReportError()
	{
		web::json::value error;
		error[U("message")] = web::json::value::string("Your Auth0 account has been blocked after 10 failed logins, check your e-mail.");
		error[U("status")] = web::json::value::string("FATAL");
		error[U("web_status")] = web::json::value::number(429);
		err = error.serialize();
		std::cout << err << std::endl;
	}
};

struct Error500
{
public:
	void ReportError()
	{
		web::json::value error;
		error[U("message")] = web::json::value::string(error_okapi);
		error[U("status")] = web::json::value::string("FATAL");
		error[U("web_status")] = web::json::value::number(500);
		err = error.serialize();
		std::cout << err << std::endl;
	}
};

struct Error520
{
public:
	void ReportError()
	{
		web::json::value error;
		error[U("message")] = web::json::value::string("Got unknown exception, maybe wrong URL.");
		error[U("status")] = web::json::value::string("FATAL");
		error[U("web_status")] = web::json::value::number(520);
		err = error.serialize();
		std::cout << err << std::endl;
	}
};

struct ErrorUnknown
{
public:
	void ReportError()
	{
		web::json::value error;
		error[U("message")] = web::json::value::string("Got HTTPError when sending request.");
		error[U("status")] = web::json::value::string("FATAL");
		error[U("web_status")] = web::json::value::number(http_code);
		err = error.serialize();
		std::cout << err << std::endl;
	}
};

struct ErrorUnknownReceive:okapiConnector
{
public:
	void ReportError()
	{
		web::json::value error;
		error[U("message")] = web::json::value::string(error_okapi_receive);
		error[U("status")] = web::json::value::string("FATAL");
		error[U("web_status")] = web::json::value::number(httpResponse);
		err = error.serialize();
		std::cout << err << std::endl;
	}
};

// Routine to initialize communicatins with OKAPI
void okapiConnector::init(method mtd)
{
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
		try
		{
			http_code = response.status_code();
			response.headers().set_content_type("application/json");
			json::value access_token_response = response.extract_json().get();
			error_auth0 = access_token_response.serialize();

			if (response.status_code() == 403) throw Error403();
			else if (response.status_code() == 400) throw Error400();
			else if (response.status_code() == 408) throw Error408();
			else if (response.status_code() == 422) throw Error422();
			else if (response.status_code() == 429) throw Error429();
			else if (response.status_code() == 520) throw Error520();
			else if (response.status_code() != 200 && response.status_code() != 202) throw ErrorUnknown();

			if (access_token_response.has_field(U("access_token")))
			{
				json::object access_token_response_obj = access_token_response.as_object();
				accessTokenTransport = access_token_response_obj.at(U("access_token")).as_string();
				std::cout << "Authentication successful" << std::endl;				
			}
			else
			{
				std::cout << "access_token missing in response from Auth0" << std::endl;
			}
		}

		catch(Error400& err) // Probably wrong password
		{
			err.ReportError();
		}
		catch(Error403& err) // Probably wrong password
		{
			err.ReportError();
		}
		catch(Error408& err) // Timeout
		{
			err.ReportError();
		}
		catch(Error422& err) // wrong format
		{
			err.ReportError();
		}
		catch(Error429& err) // too many failed logins
		{
			err.ReportError();
		}
		catch(Error520& err) // Unknown Exception
		{
			err.ReportError();
		}
		catch(ErrorUnknown& err) // any other unknown HTTP Error
		{
			err.ReportError();
		}
    }).wait();
}

// Send a request to OKAPI
void okapiConnector::sendRequest(http_client & okapiRequest, http_request & request)
{
	okapiRequest.request(request).then([&](http_response response)
	{
		try
        {
			http_code = response.status_code();
			response.headers().set_content_type("application/json");
			json::value send_request_response = response.extract_json().get();
			error_okapi = send_request_response.serialize();
			error_auth0 = error_okapi;

			if (response.status_code() == 500) throw Error500();
			else if (response.status_code() == 401) throw Error401();
			else if (response.status_code() == 408) throw Error408();
			else if (response.status_code() == 422) throw Error422();
			else if (response.status_code() == 520) throw Error520();
			else if (response.status_code() != 200 && response.status_code() != 202) throw ErrorUnknown();

			json::object send_request_response_obj = send_request_response.as_object();
			requestIdTransport = send_request_response_obj.at(U("request_id")).as_string();
			std::cout << "send request successful, with request ID:" << requestIdTransport << std::endl;
		}

		catch(Error401& err) // Unauthorized
		{
			err.ReportError();
		}
		catch(Error408& err) // Timeout
		{
			err.ReportError();
		}
		catch(Error422& err) // wrong format
		{
			err.ReportError();
		}
		catch(Error500& err) // Internal Error
		{
			err.ReportError();
		}
		catch(Error520& err) // Unknown Exception
		{
			err.ReportError();
		}
		catch(ErrorUnknown& err)
		{
			err.ReportError();
		}
    }).wait();
}

// get the result from a service execution request from OKAPI
void okapiConnector::getResult(http_client & okapiGet, http_request & request2)
{
	okapiGet.request(request2).then([&](http_response response)
	{
		try
		{
			httpResponse = response.status_code();
			response.headers().set_content_type("application/json");
			json::value get_results_response = response.extract_json().get();
			error_okapi = get_results_response.serialize();

			if (response.status_code() == 500) throw Error500();
			if (response.status_code() == 401) throw Error401();
			if (response.status_code() == 408) throw Error408();
			if (response.status_code() == 422) throw Error422();
			if (response.status_code() == 520) throw Error520();
			if (response.status_code() != 200 && response.status_code() != 202) throw ErrorUnknownReceive();

			resultsTransport = get_results_response.serialize();
		}

		catch(Error401& err) // Unauthorized
		{
			err.ReportError();
		}
		catch(Error408& err) // Timeout
		{
			err.ReportError();
		}
		catch(Error422& err) // Timeout
		{
			err.ReportError();
		}
		catch(Error500& err) // Internal Error
		{
			err.ReportError();
		}
		catch(Error520& err) // Unknown Exception
		{
			err.ReportError();
		}
		catch(ErrorUnknownReceive& err)
		{
			err.ReportError();
		}
	}).wait();
}
