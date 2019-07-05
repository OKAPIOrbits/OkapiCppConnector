#include "OkapiConnector.h"

using std::cout;
using std::endl;
using std::string;

using namespace utility;
// Common utilities like string conversions
using namespace web;
// Common features like URIs.
using namespace web::http;
// Common HTTP functionality
using namespace web::http::client;
// HTTP client features

int main(int argc, char* argv[])
{
	// initializing communication
	OkapiConnector connector;

	// Authentication with Auth0 to retrieve the access token
	OkapiConnector::completeResult initResult = connector.init(methods::POST,<username>,<password>);
  if (initResult.error.code == 200 || initResult.error.code == 202)
  {
    cout << "Authentication completed" << endl;
  }
  else {
    cout << "Authentication failed with status: " << initResult.error.status << endl;
    cout << initResult.error.message << endl;
    return -1;
  }

	bool passPredictionDemo = true;
  bool neptuneDemo = true;

	std::string base_url = "http://okapi.ddns.net:34569";

	// PASS PREDICTION
	if (passPredictionDemo) {
  
    // user input PASS PREDICTION
    double altitude = 0.048;
    double longitude = 10.645;
    double latitude = 52.328;
    std::string start = "2018-08-06T18:19:44.256628Z";
    std::string end =   "2018-08-07T00:00:00.000Z";
    //  std::string start = "2018-08-07T17:30:00.000Z";
    //  std::string end =   "2018-08-07T17:31:00.000Z";
    std::string tlePassPrediction = "1 25544U 98067A   18218.76369510  .00001449  00000-0  29472-4 0  9993\n2 25544  51.6423 126.6422 0005481  33.3092  62.9075 15.53806849126382";
    
		// preparation for pass prediction
		web::json::value simpleGroundLocation;
		simpleGroundLocation[U("altitude")] = web::json::value::number(altitude);
		simpleGroundLocation[U("longitude")] = web::json::value::number(longitude);
		simpleGroundLocation[U("latitude")] = web::json::value::number(latitude);
		web::json::value timeWindow;
		timeWindow[U("start")] = web::json::value::string(start);
		timeWindow[U("end")] = web::json::value::string(end);
		web::json::value passPredictionRequestBody;
		passPredictionRequestBody[U("simple_ground_location")] = simpleGroundLocation;
		passPredictionRequestBody[U("time_window")] = timeWindow;
		passPredictionRequestBody[U("tle")] = web::json::value::string(tlePassPrediction);

		// send request for SGP4 pass prediction
		std::string sgp4UrlRequest = "/predict-passes/sgp4/requests";
		std::string sgp4UrlRequestCombined = base_url + sgp4UrlRequest;
		http_client okapiRequestSgp4(sgp4UrlRequestCombined);
		http_request requestSgp4(methods::POST);
		requestSgp4.set_body(passPredictionRequestBody);
		requestSgp4.headers().add(U("access_token"), connector.accessToken);
		OkapiConnector::completeResult sgp4Request = connector.sendRequest(okapiRequestSgp4, requestSgp4);
    if (sgp4Request.error.code == 200 || sgp4Request.error.code == 202)
    {
      cout << "Send Pass Prediction completed" << endl;
    }
    else {
      cout << "Pass prediction request failed with status: " << sgp4Request.error.status << endl;
      cout << sgp4Request.error.message << endl;
      return -1;
    }
		std::string requestIdPassPredictionSgp4 = connector.requestId;
    cout << "Request ID: " << requestIdPassPredictionSgp4 << endl;

		// get results for SGP4 and print them in the terminal
		std::string sgp4UrlGet = "/predict-passes/sgp4/simple/results/";
		std::string sgp4UrlGetCombined = base_url + sgp4UrlGet + requestIdPassPredictionSgp4;
		http_client okapiGetSgp4(sgp4UrlGetCombined);
		http_request getSgp4PassPrediction(methods::GET);
		getSgp4PassPrediction.headers().add(U("access_token"), connector.accessToken);
		OkapiConnector::completeResult sgp4Result = connector.getResult(okapiGetSgp4, getSgp4PassPrediction);
    if (sgp4Result.error.code != 200 && sgp4Result.error.code != 202)
    {
      cout << "Retrieving pass prediction response failed with status: " << sgp4Result.error.status << endl;
      cout << sgp4Result.error.message << endl;
      return -1;
    }
		int i = 0;
		while (sgp4Result.error.code == 202) {
			sgp4Result = connector.getResult(okapiGetSgp4, getSgp4PassPrediction);
			cout << "The request was successful. Your result is not ready yet.  Waiting: " << i << " s." << endl;
			i++;
			sleep(1);
			if (i == 10)
			{
				cout << "OKAPI timeout" << endl;
				return 1;
			}
		}
    sgp4Result = connector.getResult(okapiGetSgp4, getSgp4PassPrediction);
    if (sgp4Result.error.code != 200 && sgp4Result.error.code != 202)
    {
      cout << "Retrieving pass prediction summary response failed with status: " << sgp4Result.error.status << endl;
      cout << sgp4Result.error.message << endl;
      return -1;
    }
		if (sgp4Result.error.code == 200) {
			//cout << sgp4Result.body.to_string() << endl;
		}
    else {
      cout << "Pass prediction summary request failed with status: " << sgp4Result.error.status << endl;
      cout << sgp4Result.error.message << endl;
    }

    // get results for SGP4 and print them in the terminal
    std::string sgp4UrlGetSummary = "/predict-passes/sgp4/summary/results/";
    std::string sgp4UrlGetSummaryCombined = base_url + sgp4UrlGetSummary + requestIdPassPredictionSgp4;
    http_client okapiGetSgp4Summary(sgp4UrlGetSummaryCombined);
    http_request getSgp4PassPredictionSummary(methods::GET);
    getSgp4PassPredictionSummary.headers().add(U("access_token"), connector.accessToken);
    OkapiConnector::completeResult sgp4SummaryResult = connector.getResult(okapiGetSgp4Summary, getSgp4PassPredictionSummary);
    if (sgp4SummaryResult.error.code != 200 && sgp4SummaryResult.error.code != 202)
    {
      cout << "Retrieving pass prediction summary response failed with status: " << sgp4SummaryResult.error.status << endl;
      cout << sgp4SummaryResult.error.message << endl;
      return -1;
    }
    i = 0;
    while (sgp4SummaryResult.error.code == 202) {
      sgp4SummaryResult = connector.getResult(okapiGetSgp4Summary, getSgp4PassPredictionSummary);
      cout << "The request was successful. Your result is not ready yet.  Waiting: " << i << " s." << endl;
      i++;
      sleep(1);
      if (i == 10)
      {
        cout << "OKAPI timeout" << endl;
        return 1;
      }
    }
    sgp4SummaryResult = connector.getResult(okapiGetSgp4Summary, getSgp4PassPredictionSummary);
    if (sgp4SummaryResult.error.code != 200 && sgp4SummaryResult.error.code != 202)
    {
      cout << "Retrieving pass prediction summary response failed with status: " << sgp4SummaryResult.error.status << endl;
      cout << sgp4SummaryResult.error.message << endl;
      return -1;
    }
    if (sgp4SummaryResult.error.code == 200) {
      cout << sgp4SummaryResult.body.to_string() << endl;
    }
    else {
      cout << "SGP4 request failed with status: " << sgp4SummaryResult.error.status << endl;
      cout << sgp4SummaryResult.error.message << endl;
    }

		cout << "Get pass prediction summary completed" << endl;
	}
 
 
  // NEPTUNE propagation
  if (neptuneDemo) {
 
    // user input NEPTUNE simple
    double area = 1;
    double mass = 1;
    double x = 615.119526;
    double y = -7095.644839;
    double z = -678.668352;
    double x_dot = 0.390367;
    double y_dot = 0.741902;
    double z_dot = -7.39698;
    string epoch = "2016-07-23T00:31:50.000Z";
    string propagationEndEpochNeptuneSimple = "2016-07-23T03:31:50.000Z";
    double outputStepSizeNeptuneSimple = 30;

    web::json::value simpleState;
    simpleState[U("area")] = area;
    simpleState[U("mass")] = mass;
    simpleState[U("x")] = x;
    simpleState[U("y")] = y;
    simpleState[U("z")] = z;
    simpleState[U("x_dot")] = x_dot;
    simpleState[U("y_dot")] = y_dot;
    simpleState[U("z_dot")] = z_dot;
    simpleState[U("epoch")] = web::json::value::string(epoch);
    web::json::value neptuneConfigSimple;
    neptuneConfigSimple[U("geopotential_degree_order")] = 6;
    neptuneConfigSimple[U("atmospheric_drag")] = 1;
    neptuneConfigSimple[U("horizontal_wind")] = 0;
    neptuneConfigSimple[U("sun_gravity")] = 1;
    neptuneConfigSimple[U("moon_gravity")] = 1;
    neptuneConfigSimple[U("solar_radiation_pressure")] = 1;
    neptuneConfigSimple[U("solid_tides")] = 1;
    neptuneConfigSimple[U("ocean_tides")] = 0;
    web::json::value settingsSimple;
    settingsSimple[U("propagation_end_epoch")] = web::json::value::string(propagationEndEpochNeptuneSimple);
    settingsSimple[U("output_step_size")] = outputStepSizeNeptuneSimple;
    settingsSimple[U("neptune_config")] = neptuneConfigSimple;
    web::json::value propagateNeptuneSimpleRequestBody;
    propagateNeptuneSimpleRequestBody[U("simple_state")] = simpleState;
    propagateNeptuneSimpleRequestBody[U("settings")] = settingsSimple;
    
    
    // send request for SGP4 pass prediction
    std::string neptuneUrlRequest = "/propagate-orbit/neptune/requests";
    std::string neptuneUrlRequestCombined = base_url + neptuneUrlRequest;
    http_client okapiRequestNeptune(neptuneUrlRequestCombined);
    http_request requestNeptune(methods::POST);
    requestNeptune.set_body(propagateNeptuneSimpleRequestBody);
    requestNeptune.headers().add(U("access_token"), connector.accessToken);
    OkapiConnector::completeResult neptuneRequest = connector.sendRequest(okapiRequestNeptune, requestNeptune);
    if (neptuneRequest.error.code == 200 || neptuneRequest.error.code == 202)
    {
      cout << "Send NEPTUNE propagation request completed" << endl;
    }
    else {
      cout << "NEPTUNE propagation request failed with status: " << neptuneRequest.error.status << endl;
      cout << neptuneRequest.error.message << endl;
      return -1;
    }
    std::string requestIdPassPredictionNeptune = connector.requestId;
    cout << "Request ID: " << requestIdPassPredictionNeptune << endl;
    
    // get results for NEPTUNE and print them in the terminal
    std::string neptuneUrlGet = "/propagate-orbit/neptune/simple/results/";
    std::string neptuneUrlGetCombined = base_url + neptuneUrlGet + requestIdPassPredictionNeptune;
    http_client okapiGetNeptune(neptuneUrlGetCombined);
    http_request getNeptunePropagation(methods::GET);
    getNeptunePropagation.headers().add(U("access_token"), connector.accessToken);
    OkapiConnector::completeResult neptuneResult = connector.getResult(okapiGetNeptune, getNeptunePropagation);
    if (neptuneResult.error.code != 200 && neptuneResult.error.code != 202)
    {
      cout << "Retrieving NEPTUNE propagation response failed with status: " << neptuneResult.error.status << endl;
      cout << neptuneResult.error.message << endl;
      return -1;
    }
    int i = 0;
    while (neptuneResult.error.code == 202) {
      neptuneResult = connector.getResult(okapiGetNeptune, getNeptunePropagation);
      cout << "The request was successful. Your result is not ready yet.  Waiting: " << i << " s." << endl;
      i++;
      sleep(1);
      if (i == 10)
      {
        cout << "OKAPI timeout" << endl;
        return 1;
      }
    }
    neptuneResult = connector.getResult(okapiGetNeptune, getNeptunePropagation);
    if (neptuneResult.error.code != 200 && neptuneResult.error.code != 202)
    {
      cout << "Retrieving NEPTUNE propagation simple response failed with status: " << neptuneResult.error.status << endl;
      cout << neptuneResult.error.message << endl;
      return -1;
    }
    if (neptuneResult.error.code == 200) {
      cout << neptuneResult.body.to_string() << endl;
    }
    else {
      cout << "NEPTUNE propagation simple request failed with status: " << neptuneResult.error.status << endl;
      cout << neptuneResult.error.message << endl;
    }
    
    std::string neptuneGenericUrlGet = "/propagate-orbit/neptune/simple/results/";
    std::string neptuneGenericUrlGetCombined = base_url + neptuneGenericUrlGet + requestIdPassPredictionNeptune + "/generic";
    http_client okapiGetNeptuneGeneric(neptuneGenericUrlGetCombined);
    http_request getNeptuneGenericPropagation(methods::GET);
    getNeptuneGenericPropagation.headers().add(U("access_token"), connector.accessToken);
    OkapiConnector::completeResult neptuneGenericResult = connector.getResult(okapiGetNeptuneGeneric, getNeptuneGenericPropagation);
    if (neptuneGenericResult.error.code != 200 && neptuneGenericResult.error.code != 202)
    {
      cout << "Retrieving NEPTUNE propagation response failed with status: " << neptuneGenericResult.error.status << endl;
      cout << neptuneGenericResult.error.message << endl;
      return -1;
    }
    i = 0;
    while (neptuneGenericResult.error.code == 202) {
      neptuneGenericResult = connector.getResult(okapiGetNeptuneGeneric, getNeptuneGenericPropagation);
      cout << "The request was successful. Your result is not ready yet.  Waiting: " << i << " s." << endl;
      i++;
      sleep(1);
      if (i == 10)
      {
      cout << "OKAPI timeout" << endl;
      return 1;
      }
    }
    neptuneGenericResult = connector.getResult(okapiGetNeptuneGeneric, getNeptuneGenericPropagation);
    if (neptuneGenericResult.error.code != 200 && neptuneGenericResult.error.code != 202)
    {
      cout << "Retrieving NEPTUNE propagation simple generic response failed with status: " << neptuneGenericResult.error.status << endl;
      cout << neptuneGenericResult.error.message << endl;
      return -1;
    }
    if (neptuneGenericResult.error.code == 200) {
      cout << neptuneGenericResult.body.to_string() << endl;
    }
    else {
      cout << "NEPTUNE propagation simple generic request failed with status: " << neptuneGenericResult.error.status << endl;
      cout << neptuneGenericResult.error.message << endl;
    }
  }

  return 0;
}
