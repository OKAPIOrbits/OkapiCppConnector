#ifndef okapi_connector_H
#define okapi_connector_H

#include <cpprest/http_client.h>
#include <cpprest/json.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>
#include <iostream>
#include <string>

using namespace utility;
using namespace web;
using namespace web::http;
using namespace web::http::client;
using namespace std;

class OkapiConnector
{
public:

	string password;
	string username;
	string accessToken;
	string requestId;

	struct completeError
	{
		string message;
		string status;
		int code;
	};

	struct completeResult
	{
		web::json::value body;
		struct completeError error;
	};

	completeResult init(method mtd, string username, string password);

	completeResult sendRequest(http_client & okapiRequest, http_request & request);

	completeResult getResult(http_client & okapiGet, http_request & request2);

};

#endif // okapiConnector
