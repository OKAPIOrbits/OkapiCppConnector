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

	struct CompleteError
	{
		string message;
		string status;
		int code;
	};

	struct CompleteResult
	{
		web::json::value body;
		struct CompleteError error;
	};

	CompleteResult init(method mtd, string username, string password);

  CompleteResult sendRequest(string baseUrl, string endpoint, web::json::value requestBody);

	CompleteResult getResult(string baseUrl, string endpoint, string requestId);

};

#endif // okapiConnector
