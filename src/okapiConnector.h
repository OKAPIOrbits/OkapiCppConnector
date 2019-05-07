#ifndef okapi_connector_H
#define okapi_connector_H

#include <cpprest/http_client.h>
#include <cpprest/json.h>
#include <iostream>
#include <string>


using namespace utility;                    // Common utilities like string conversions
using namespace web;                        // Common features like URIs.
using namespace web::http;                  // Common HTTP functionality
using namespace web::http::client;          // HTTP client features


class okapiConnector
{
	public:
		std::string password;
		std::string username;
		int httpResponse;
		std::string	accessTokenTransport;
		std::string requestIdTransport;
		std::string resultsTransport;

		void init(method mtd);

		void sendRequest(http_client & okapiRequest, http_request & request);

		void getResult(http_client & okapiGet, http_request & request2);

};

#endif // okapiConnector
