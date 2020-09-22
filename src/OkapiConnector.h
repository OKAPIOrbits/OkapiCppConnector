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

/**
 * @brief This class provides helper methods to send and retrieve information to and from the OKAPI platform.
 *
 * @version v2020-09
 *
 * @author Christopher Kebschull
 */
class OkapiConnector
{
public:

	string accessToken;

	/**
	 * @brief Holds error message, status and the http error code.
	 *
	 */
	struct OkapiError
	{
		string message;
		string status;
		int code;
	};

    /**
     * @brief Wraps the JSON body and error information in a struct.
     *
     */
	struct OkapiResult
	{
		web::json::value body;
		struct OkapiError error;
	};

	/**
	 * @brief Retrieves the access token using the given username and password. The baseurl will be used for every request later one.
	 *
	 * @param baseUrl - points to the OKAPI platform api, e.g. https://platform.okapiorbits.com/api/
	 * @param username - your username you signed up with on the platform
	 * @param password  - the password associated with the account
	 * @return OkapiResult - response from auth0 is stored in the body
	 */
	OkapiResult init(string baseUrl, string username, string password);

	/**
	 * @brief Sends a post request to the Okapi platform.
	 *
	 * @param endpoint - uses the given endpoint
	 * @param requestBody - a string holding informaiton in JSON format
	 * @return OkapiResult - Holds the response of the OKAPI platform. The body may hold a request id
	 */
    OkapiResult sendRequest(string endpoint, web::json::value requestBody);

	/**
	 * @brief Get the values from the platform. May called multiple times when a 202 code is returned. This means
	 * that the results are only partially available. Only when 200 is returned the results are complete. The developer
	 * may need to wait an appropiate amount of time or poll this function until the code is 200.
	 *
	 * @param endpoint - uses the given endpoint
	 * @return OkapiResult - Holds the response of the OKAPI platform. The body holds the requested information
	 */
	OkapiResult getValues(string endpoint);

    /**
     * @brief  Same as the #getValues(string) function but does not return a 202 code. It will block until the
	 * results are complete or an error is returned.
     *
	 * @param endpoint - uses the given endpoint
	 * @return OkapiResult - Holds the response of the OKAPI platform. The body holds the requested information
     */
	OkapiResult waitForProcessingAndGetValues(string endpoint);

    /**
     * @brief Updates the given object in the platform.
     *
     * @param endpoint - uses the given endpoint together with an id of the object to identify it
     * @param requestBody - the new information to update the backend with
     * @return OkapiResult - holds the answer of the platform on the update request. On success it will return the object itself
     */
	OkapiResult updateRequest(string endpoint, web::json::value requestBody);

	/**
	 * @brief  Deletes the given object from the platform
	 *
	 * @param endpoint - uses the given endpoint together with an id of the object to identify it
	 * @return OkapiResult - holds the answer of the platform on the delete request. On success it will return the object itself
	 */
	OkapiResult deleteRequest(string endpoint);

	/**
	 * @brief A convenience function that adds a satellite object to the platform. It uses internally the #sendRequest(string, web::json::value) function.
	 *
	 * @param requestBody - holds the satellite definition as JSON
	 * @return OkapiResult - holds the answer of the platform on the put request. On success it will return the newly added satellite
	 */
	OkapiResult addSatellite(web::json::value requestBody);

	/**
	 * @brief A convenience function that updates a satellite on the platform. It uses internally the #updateRequest(string, web::json::value) function.
	 *
	 * @param requestBody - holds the new satellite definition as JSON
	 * @param satelliteId - identifies the satellite object in the platform
	 * @return OkapiResult - holds the answer of the platform on the update request. On success it will return the changed satellite
	 */
	OkapiResult updateSatellite(web::json::value requestBody, string satelliteId);

    /**
     * @brief A convenience function that gets all satellite objects
     *
     * @return OkapiResult - holds the answer of the platform on the get request
     */
	OkapiResult getSatellites();

    /**
     * @brief A convenience function that deletes a given satellite.
     *
     * @param satelliteId - identifies the satellite object in the platform
     * @return OkapiResult - holds the answer of the platform on the delete request. On success it will return the changed satellite.
     */
	OkapiResult deleteSatellite(string satelliteId);

    /**
     * @brief  A convenience function that gets all conjunction objects
     *
     * @return OkapiResult - holds the answer of the platform on the get request
     */
	OkapiResult getConjunctions();

    /**
     * @brief A convenience function that gets the cdms associated with the given conjunction.
     *
     * @param conjunctionId - identifies the conjunction object in the platform
     * @return OkapiResult - holds the answer of the platform on the get request
     */
	OkapiResult getCdms(string conjunctionId);

    /**
     * @brief A convenience function that gets the maneuver evals object associated with the given conjunction.
     *
     * @param conjunctionId  - identifies the conjunction object in the platform
     * @return OkapiResult - holds the answer of the platform on the get request
     */
	OkapiResult getManeuverEvals(string conjunctionId);

    /**
     * @brief A convenience function that extracts the request id from the OkapiResult response after a call to sendRequest(string, web::json::value)
     *
     * @param response - holds the response of the platform after a call to sendRequest(string, web::json::value)
     * @return string - the request id or an empty string
     */
	string getRequestId(OkapiResult response);

private:

	string password;
	string username;
	string baseUrl;

};

#endif // okapiConnector
