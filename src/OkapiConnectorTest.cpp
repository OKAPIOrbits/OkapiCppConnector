#include "OkapiConnector.h"

using std::cout;
using std::endl;
using std::string;

using namespace utility;
using namespace web;
using namespace web::http;
using namespace web::http::client;

/**
 * Retrieves the result from the backend. Waits 10 seconds for the backend until the result is ready.
 */
OkapiConnector::completeResult retrieveResult(OkapiConnector connector, http_client fullUrl, http_request method)
{
  // First call to the backend
  OkapiConnector::completeResult result = connector.getResult(fullUrl, method);
  if (result.error.code != 200 && result.error.code != 202)
  {
    cout << "Retrieving response failed with status: " << result.error.status << endl;
    cout << result.error.message << endl;
    return result;
  }
  int i = 0;
  // Poll the backend until the result is ready and can be retrieved
  while (result.error.code == 202) {
    result = connector.getResult(fullUrl, method);
    cout << "The request was successful. Your result is not ready yet.  Waiting: " << i << " s." << endl;
    i++;
    sleep(1);
//    if (i == 10)
//    {
//      cout << "OKAPI timeout" << endl;
//      return result;
//    }
  }
  // Final call to the backend
  return connector.getResult(fullUrl, method);
}

/**
 * Tests the pass prediction end points
 */
void predictPassesTests(OkapiConnector connector, string baseUrl)
{
// user input PASS PREDICTION
  double altitude = 0.048;
  double longitude = 10.645;
  double latitude = 52.328;
  string start = "2018-08-06T18:19:44.256628Z";
  string end =   "2018-08-07T00:00:00.000Z";
  //  std::string start = "2018-08-07T17:30:00.000Z";
  //  std::string end =   "2018-08-07T17:31:00.000Z";
  string tlePassPrediction = "1 25544U 98067A   18218.76369510  .00001449  00000-0  29472-4 0  9993\n2 25544  51.6423 126.6422 0005481  33.3092  62.9075 15.53806849126382";
  
  // user input NEPTUNE simple
  double area = 0.01;
  double mass = 1.3;
  double x = -2915.65441951;
  double y = -3078.17058851;
  double z = 5284.39698421;
  double x_dot = 4.94176934;
  double y_dot = -5.83109248;
  double z_dot = -0.66365683;
  string epoch = "2018-08-06T18:19:43.256628Z";
  double outputTimeStep = 10;

  // preparation for pass prediction using SGP4
  web::json::value simpleGroundLocation;
  simpleGroundLocation[U("altitude")] = web::json::value::number(altitude);
  simpleGroundLocation[U("longitude")] = web::json::value::number(longitude);
  simpleGroundLocation[U("latitude")] = web::json::value::number(latitude);
  web::json::value timeWindow;
  timeWindow[U("start")] = web::json::value::string(start);
  timeWindow[U("end")] = web::json::value::string(end);
  web::json::value predictPassesSettingsSimple;
  predictPassesSettingsSimple[U("predict_passes_settings")] = web::json::value::number(outputTimeStep);
  web::json::value passPredictionRequestBody;
  passPredictionRequestBody[U("simple_ground_location")] = simpleGroundLocation;
  passPredictionRequestBody[U("predict_passes_settings")] = predictPassesSettingsSimple;
  passPredictionRequestBody[U("time_window")] = timeWindow;
  passPredictionRequestBody[U("tle")] = web::json::value::string(tlePassPrediction);
  // preparation for pass prediction using NEPTUNE and OREKIT
  web::json::value simpleState;
  simpleState[U("area")] = web::json::value::number(area);
  simpleState[U("mass")] = web::json::value::number(mass);
  simpleState[U("x")] = web::json::value::number(x);
  simpleState[U("y")] = web::json::value::number(y);
  simpleState[U("z")] = web::json::value::number(z);
  simpleState[U("x_dot")] = web::json::value::number(x_dot);
  simpleState[U("y_dot")] = web::json::value::number(y_dot);
  simpleState[U("z_dot")] = web::json::value::number(z_dot);
  simpleState[U("epoch")] = web::json::value::string(epoch);
  web::json::value neptuneConfigSimple;
  neptuneConfigSimple[U("geopotential_degree_order")] = web::json::value::number(2);
  web::json::value passPredictionNumericalRequestBody;
  passPredictionNumericalRequestBody[U("simple_ground_location")] = simpleGroundLocation;
  passPredictionNumericalRequestBody[U("predict_passes_settings")] = predictPassesSettingsSimple;
  passPredictionNumericalRequestBody[U("time_window")] = timeWindow;
  passPredictionNumericalRequestBody[U("simple_state")] = simpleState;
  passPredictionNumericalRequestBody[U("neptune_config")] = neptuneConfigSimple;
  
  // send request for SGP4 pass prediction
  http_client sendUrlSGP4(baseUrl + "/predict-passes/sgp4/requests");
  http_request requestSGP4(methods::POST);
  requestSGP4.set_body(passPredictionRequestBody);
  requestSGP4.headers().add(U("access_token"), connector.accessToken);
  OkapiConnector::completeResult responseSGP4 = connector.sendRequest(sendUrlSGP4, requestSGP4);
  if (responseSGP4.error.code != 200 && responseSGP4.error.code != 202)
  {
    cout << "SGP4 request failed with status: " << responseSGP4.error.status << endl;
    cout << responseSGP4.error.message << endl;
    return;
  }
  string requestIdPassPredictionSgp4 = connector.requestId;
  cout << "SGP4 request ID: " << requestIdPassPredictionSgp4 << endl;
  
  // send request for SGP4 pass prediction
  http_client sendUrlNeptune(baseUrl + "/predict-passes/neptune/requests");
  http_request requestNeptune(methods::POST);
  requestNeptune.set_body(passPredictionNumericalRequestBody);
  requestNeptune.headers().add(U("access_token"), connector.accessToken);
  OkapiConnector::completeResult responseNeptune = connector.sendRequest(sendUrlNeptune, requestNeptune);
  // Check response
  if (responseNeptune.error.code != 200 && responseNeptune.error.code != 202)
  {
    cout << "Neptune request failed with status: " << responseNeptune.error.status << endl;
    cout << responseNeptune.error.message << endl;
  }
  string requestIdPassPredictionNeptune = connector.requestId;
  cout << "Neptune request ID: " << requestIdPassPredictionNeptune << endl;
  
  // send request for SGP4 pass prediction
  http_client sendURL(baseUrl + "/predict-passes/orekit-numerical/requests");
  http_request requestOrekit(methods::POST);
  requestOrekit.set_body(passPredictionNumericalRequestBody);
  requestOrekit.headers().add(U("access_token"), connector.accessToken);
  OkapiConnector::completeResult responseOrekit = connector.sendRequest(sendURL, requestOrekit);
  // Check response
  if (responseOrekit.error.code != 200 && responseOrekit.error.code != 202)
  {
    cout << "Orekit request failed with status: " << responseOrekit.error.status << endl;
    cout << responseOrekit.error.message << endl;
  }
  string requestIdPassPredictionOrekit = connector.requestId;
  cout << "Orekit request ID: " << requestIdPassPredictionOrekit << endl;
  
  

  // get results for SGP4 and print them in the terminal
  http_client getUrlSgp4Simple(baseUrl + "/predict-passes/sgp4/simple/results/" + requestIdPassPredictionSgp4);
  http_request getSgp4PassPrediction(methods::GET);
  getSgp4PassPrediction.headers().add(U("access_token"), connector.accessToken);
  OkapiConnector::completeResult sgp4SimpleResult = retrieveResult(connector, getUrlSgp4Simple, getSgp4PassPrediction);
  if (sgp4SimpleResult.error.code != 200 && sgp4SimpleResult.error.code != 202)
  {
    cout << "Response failed with status: " << sgp4SimpleResult.error.status << endl;
    cout << sgp4SimpleResult.error.message << endl;
  }
  else
  {
    cout << sgp4SimpleResult.body.serialize() << endl;
  }

  // get results for SGP4 and print them in the terminal
  http_client getUrlSgp4Summary(baseUrl + "/predict-passes/sgp4/summary/results/" + requestIdPassPredictionSgp4);
  http_request getSgp4PassPredictionSummary(methods::GET);
  getSgp4PassPredictionSummary.headers().add(U("access_token"), connector.accessToken);
  OkapiConnector::completeResult sgp4SummaryResult = retrieveResult(connector, getUrlSgp4Summary, getSgp4PassPredictionSummary);
  if (sgp4SummaryResult.error.code != 200 && sgp4SummaryResult.error.code != 202)
  {
    cout << "Response failed with status: " << sgp4SummaryResult.error.status << endl;
    cout << sgp4SummaryResult.error.message << endl;
  }
  else
  {
    cout << sgp4SummaryResult.body.serialize() << endl;
  }
  
  http_client okapiGetNeptuneSimple(baseUrl + "/predict-passes/neptune/simple/results/" + requestIdPassPredictionNeptune);
  http_request getNeptunePassPrediction(methods::GET);
  getNeptunePassPrediction.headers().add(U("access_token"), connector.accessToken);
  OkapiConnector::completeResult neptuneSimpleResult = retrieveResult(connector, okapiGetNeptuneSimple, getNeptunePassPrediction);
  if (neptuneSimpleResult.error.code != 200 && neptuneSimpleResult.error.code != 202)
  {
    cout << "Response failed with status: " << neptuneSimpleResult.error.status << endl;
    cout << neptuneSimpleResult.error.message << endl;
  }
  else
  {
    cout << neptuneSimpleResult.body.serialize() << endl;
  }
  
  // get results for SGP4 and print them in the terminal
  http_client okapiGetNeptuneSummary(baseUrl + "/predict-passes/neptune/summary/results/" + requestIdPassPredictionNeptune);
  http_request getNeptunePassPredictionSummary(methods::GET);
  getNeptunePassPredictionSummary.headers().add(U("access_token"), connector.accessToken);
  OkapiConnector::completeResult neptuneSummaryResult = retrieveResult(connector, okapiGetNeptuneSummary, getNeptunePassPredictionSummary);
  if (neptuneSummaryResult.error.code != 200 && neptuneSummaryResult.error.code != 202)
  {
    cout << "Response failed with status: " << neptuneSummaryResult.error.status << endl;
    cout << neptuneSummaryResult.error.message << endl;
  }
  else
  {
    cout << neptuneSummaryResult.body.serialize() << endl;
  }
}

/**
 * Tests the NEPTUNE end points
 */
void neptuneTest(OkapiConnector connector, string baseUrl)
{
  // user input NEPTUNE simple
  double area = 0.01;
  double mass = 1.3;
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

  // send request for NEPTUNE propagation
  string neptuneUrlRequest = "/propagate-orbit/neptune/requests";
  http_client okapiRequestNeptune(baseUrl + neptuneUrlRequest);
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
  }
  string requestIdNeptune = connector.requestId;
  cout << "Request ID: " << requestIdNeptune << endl;

  // get results for NEPTUNE and print them in the terminal
  string neptuneUrlGet = "/propagate-orbit/neptune/simple/results/";
  http_client okapiGetNeptune(baseUrl + neptuneUrlGet + requestIdNeptune);
  http_request getNeptunePropagation(methods::GET);
  getNeptunePropagation.headers().add(U("access_token"), connector.accessToken);
  OkapiConnector::completeResult neptuneResult = connector.getResult(okapiGetNeptune, getNeptunePropagation);
  if (neptuneResult.error.code != 200 && neptuneResult.error.code != 202)
  {
    cout << "Retrieving NEPTUNE propagation response failed with status: " << neptuneResult.error.status << endl;
    cout << neptuneResult.error.message << endl;
  }
  int i = 0;
  while (neptuneResult.error.code == 202) {
    neptuneResult = connector.getResult(okapiGetNeptune, getNeptunePropagation);
    cout << "The request was successful. Your result is not ready yet.  Waiting: " << i << " s." << endl;
    i++;
    sleep(1);
//    if (i == 10)
//    {
//      cout << "OKAPI timeout" << endl;
//    }
  }
  neptuneResult = connector.getResult(okapiGetNeptune, getNeptunePropagation);
  if (neptuneResult.error.code != 200 && neptuneResult.error.code != 202)
  {
    cout << "Retrieving NEPTUNE propagation simple response failed with status: " << neptuneResult.error.status << endl;
    cout << neptuneResult.error.message << endl;
  }
  if (neptuneResult.error.code == 200) {
    cout << neptuneResult.body.serialize() << endl;
  }
  else {
    cout << "NEPTUNE propagation simple request failed with status: " << neptuneResult.error.status << endl;
    cout << neptuneResult.error.message << endl;
  }

  string neptuneGenericUrlGet = "/propagate-orbit/neptune/simple/results/";
  http_client okapiGetNeptuneGeneric(baseUrl + neptuneGenericUrlGet + requestIdNeptune + "/generic");
  http_request getNeptuneGenericPropagation(methods::GET);
  getNeptuneGenericPropagation.headers().add(U("access_token"), connector.accessToken);
  OkapiConnector::completeResult neptuneGenericResult = connector.getResult(okapiGetNeptuneGeneric, getNeptuneGenericPropagation);
  if (neptuneGenericResult.error.code != 200 && neptuneGenericResult.error.code != 202)
  {
    cout << "Retrieving NEPTUNE propagation response failed with status: " << neptuneGenericResult.error.status << endl;
    cout << neptuneGenericResult.error.message << endl;
  }
  i = 0;
  while (neptuneGenericResult.error.code == 202) {
    neptuneGenericResult = connector.getResult(okapiGetNeptuneGeneric, getNeptuneGenericPropagation);
    cout << "The request was successful. Your result is not ready yet.  Waiting: " << i << " s." << endl;
    i++;
    sleep(1);
//    if (i == 10)
//    {
//    cout << "OKAPI timeout" << endl;
//    }
  }
  neptuneGenericResult = connector.getResult(okapiGetNeptuneGeneric, getNeptuneGenericPropagation);
  if (neptuneGenericResult.error.code != 200 && neptuneGenericResult.error.code != 202)
  {
    cout << "Retrieving NEPTUNE propagation simple generic response failed with status: " << neptuneGenericResult.error.status << endl;
    cout << neptuneGenericResult.error.message << endl;
  }
  if (neptuneGenericResult.error.code == 200) {
    cout << neptuneGenericResult.body.serialize() << endl;
  }
  else {
    cout << "NEPTUNE propagation simple generic request failed with status: " << neptuneGenericResult.error.status << endl;
    cout << neptuneGenericResult.error.message << endl;
  }
}

/**
 * Tests the NEPTUNE OPM end points with covariance propagation
 */
void neptuneOpmCovarianceTest(OkapiConnector connector, string baseUrl)
{
  // user input NEPTUNE OPM
  string OBJECT_NAME = "ISS (ZARYA)";
  string OBJECT_ID = "1998-067-A";
  string CENTER_NAME = "EARTH";
  string REF_FRAME = "GCRF";
  string REF_FRAME_EPOCH = "2000-01-01T00:00:00Z";
  string TIME_SYSTEM = "UTC";
  string EPOCH = "2016-07-23T00:31:50.000Z";
  double X = 615.119526;
  double Y = -7095.644839;
  double Z = -678.668352;
  double X_DOT = 0.390367;
  double Y_DOT = 0.741902;
  double Z_DOT = -7.39698;
  double CX_X = 0.10000000E-04;
  double CY_X = 0.0;
  double CY_Y = 0.10000000E-03;
  double CZ_X = 0.0;
  double CZ_Y = 0.0;
  double CZ_Z = 0.30000000E-04;
  double CX_DOT_X = 0.0;
  double CX_DOT_Y = 0.0;
  double CX_DOT_Z = 0.0;
  double CX_DOT_X_DOT = 0.20000000E-09;
  double CY_DOT_X = 0.0;
  double CY_DOT_Y = 0.0;
  double CY_DOT_Z = 0.0;
  double CY_DOT_X_DOT = 0.0;
  double CY_DOT_Y_DOT = 0.10000000E-09;
  double CZ_DOT_X = 0.0;
  double CZ_DOT_Y = 0.0;
  double CZ_DOT_Z = 0.0;
  double CZ_DOT_X_DOT = 0.0;
  double CZ_DOT_Y_DOT = 0.0;
  double CZ_DOT_Z_DOT = 0.10000000E-09;
  string MAN_EPOCH_IGINITION = "2016-07-23T00:31:50.000Z";
  double MASS = 1.3;
  double SOLAR_RAD_COEFF = 1.3;
  double DRAG_AREA = 0.01;
  double DRAG_COEFF = 2.2;
  string propagationEndEpochNeptuneOpm = "2016-07-23T03:31:50.000Z";
  double outputStepSizeNeptuneOpm = 30;

  web::json::value opmHeader;
  opmHeader[U("CCSDS_OPM_VERS")] = 2;
  web::json::value opmMetaData;
  opmMetaData[U("OBJECT_NAME")] = web::json::value::string(OBJECT_NAME);
  opmMetaData[U("OBJECT_ID")] = web::json::value::string(OBJECT_ID);
  opmMetaData[U("CENTER_NAME")] = web::json::value::string(CENTER_NAME);
  opmMetaData[U("REF_FRAME")] = web::json::value::string(REF_FRAME);
  opmMetaData[U("REF_FRAME_EPOCH")] = web::json::value::string(REF_FRAME_EPOCH);
  opmMetaData[U("TIME_SYSTEM")] = web::json::value::string(TIME_SYSTEM);
  
  web::json::value opmData;
  opmData[U("DRAG_AREA")] = DRAG_AREA;
  opmData[U("DRAG_COEFF")] = DRAG_COEFF;
  opmData[U("X")] = X;
  opmData[U("Y")] = Y;
  opmData[U("Z")] = Z;
  opmData[U("X_DOT")] = X_DOT;
  opmData[U("Y_DOT")] = Y_DOT;
  opmData[U("Z_DOT")] = Z_DOT;
  opmData[U("EPOCH")] = web::json::value::string(EPOCH);
  opmData[U("COV_REF_FRAME")] = web::json::value::string("UVW");
  opmData[U("CX_X")] = CX_X;
  opmData[U("CY_X")] = CY_X;
  opmData[U("CY_Y")] = CY_Y;
  opmData[U("CZ_X")] = CZ_X;
  opmData[U("CZ_Y")] = CZ_Y;
  opmData[U("CZ_Z")] = CZ_Z;
  opmData[U("CX_DOT_X")] = CX_DOT_X;
  opmData[U("CX_DOT_Y")] = CX_DOT_Y;
  opmData[U("CX_DOT_Z")] = CX_DOT_Z;
  opmData[U("CX_DOT_X_DOT")] = CX_DOT_X_DOT;
  opmData[U("CY_DOT_X")] = CY_DOT_X;
  opmData[U("CY_DOT_Y")] = CY_DOT_Y;
  opmData[U("CY_DOT_Z")] = CY_DOT_Z;
  opmData[U("CY_DOT_X_DOT")] = CY_DOT_X_DOT;
  opmData[U("CY_DOT_Y_DOT")] = CY_DOT_Y_DOT;
  opmData[U("CZ_DOT_X")] = CZ_DOT_X;
  opmData[U("CZ_DOT_Y")] = CZ_DOT_Y;
  opmData[U("CZ_DOT_Z")] = CZ_DOT_Z;
  opmData[U("CZ_DOT_X_DOT")] = CZ_DOT_X_DOT;
  opmData[U("CZ_DOT_Y_DOT")] = CZ_DOT_Y_DOT;
  opmData[U("CZ_DOT_Z_DOT")] = CZ_DOT_Z_DOT;
  opmData[U("MASS")] = MASS;
  opmData[U("SOLAR_RAD_COEFF")] = SOLAR_RAD_COEFF;
  opmData[U("DRAG_AREA")] = DRAG_AREA;
  opmData[U("DRAG_COEFF")] = DRAG_COEFF;
  web::json::value ccsdsOpm;
  ccsdsOpm[U("OPM_HEADER")] = opmHeader;
  ccsdsOpm[U("OPM_META_DATA")] = opmMetaData;
  ccsdsOpm[U("OPM_DATA")] = opmData;

  web::json::value neptuneConfigSimple;
  neptuneConfigSimple[U("geopotential_degree_order")] = 6;
  neptuneConfigSimple[U("atmospheric_drag")] = 1;
  neptuneConfigSimple[U("horizontal_wind")] = 0;
  neptuneConfigSimple[U("sun_gravity")] = 1;
  neptuneConfigSimple[U("moon_gravity")] = 1;
  neptuneConfigSimple[U("solar_radiation_pressure")] = 1;
  neptuneConfigSimple[U("solid_tides")] = 1;
  neptuneConfigSimple[U("ocean_tides")] = 0;
  web::json::value settingsOpm;
  settingsOpm[U("propagation_end_epoch")] = web::json::value::string(propagationEndEpochNeptuneOpm);
  settingsOpm[U("output_step_size")] = outputStepSizeNeptuneOpm;
  settingsOpm[U("neptune_config")] = neptuneConfigSimple;
  web::json::value propagateNeptuneSimpleRequestBody;
  propagateNeptuneSimpleRequestBody[U("CCSDS_OPM")] = ccsdsOpm;
  propagateNeptuneSimpleRequestBody[U("settings")] = settingsOpm;

  // send request for NEPTUNE propagation
  string neptuneUrlRequest = "/propagate-orbit/neptune/requests";
  http_client okapiRequestNeptune(baseUrl + neptuneUrlRequest);
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
  }
  string requestIdNeptune = connector.requestId;
  cout << "Request ID: " << requestIdNeptune << endl;

  // get results for NEPTUNE and print them in the terminal
  string neptuneUrlGet = "/propagate-orbit/neptune/opm/results/";
  http_client okapiGetNeptune(baseUrl + neptuneUrlGet + requestIdNeptune);
  http_request getNeptunePropagation(methods::GET);
  getNeptunePropagation.headers().add(U("access_token"), connector.accessToken);
  OkapiConnector::completeResult neptuneResult = connector.getResult(okapiGetNeptune, getNeptunePropagation);
  if (neptuneResult.error.code != 200 && neptuneResult.error.code != 202)
  {
    cout << "Retrieving NEPTUNE propagation response failed with status: " << neptuneResult.error.status << endl;
    cout << neptuneResult.error.message << endl;
  }
  int i = 0;
  while (neptuneResult.error.code == 202) {
    neptuneResult = connector.getResult(okapiGetNeptune, getNeptunePropagation);
    cout << "The request was successful. Your result is not ready yet.  Waiting: " << i << " s." << endl;
    i++;
    sleep(1);
//    if (i == 10)
//    {
//      cout << "OKAPI timeout" << endl;
//    }
  }
  neptuneResult = connector.getResult(okapiGetNeptune, getNeptunePropagation);
  if (neptuneResult.error.code != 200 && neptuneResult.error.code != 202)
  {
    cout << "Retrieving NEPTUNE propagation simple response failed with status: " << neptuneResult.error.status << endl;
    cout << neptuneResult.error.message << endl;
  }
  if (neptuneResult.error.code == 200) {
    cout << neptuneResult.body.serialize() << endl;
  }
  else {
    cout << "NEPTUNE propagation simple request failed with status: " << neptuneResult.error.status << endl;
    cout << neptuneResult.error.message << endl;
  }

  string neptuneGenericUrlGet = "/propagate-orbit/neptune/opm/results/";
  http_client okapiGetNeptuneGeneric(baseUrl + neptuneGenericUrlGet + requestIdNeptune + "/generic");
  http_request getNeptuneGenericPropagation(methods::GET);
  getNeptuneGenericPropagation.headers().add(U("access_token"), connector.accessToken);
  OkapiConnector::completeResult neptuneGenericResult = connector.getResult(okapiGetNeptuneGeneric, getNeptuneGenericPropagation);
  if (neptuneGenericResult.error.code != 200 && neptuneGenericResult.error.code != 202)
  {
    cout << "Retrieving NEPTUNE propagation response failed with status: " << neptuneGenericResult.error.status << endl;
    cout << neptuneGenericResult.error.message << endl;
  }
  i = 0;
  while (neptuneGenericResult.error.code == 202) {
    neptuneGenericResult = connector.getResult(okapiGetNeptuneGeneric, getNeptuneGenericPropagation);
    cout << "The request was successful. Your result is not ready yet.  Waiting: " << i << " s." << endl;
    i++;
    sleep(1);
//    if (i == 10)
//    {
//    cout << "OKAPI timeout" << endl;
//    }
  }
  neptuneGenericResult = connector.getResult(okapiGetNeptuneGeneric, getNeptuneGenericPropagation);
  if (neptuneGenericResult.error.code != 200 && neptuneGenericResult.error.code != 202)
  {
    cout << "Retrieving NEPTUNE propagation simple generic response failed with status: " << neptuneGenericResult.error.status << endl;
    cout << neptuneGenericResult.error.message << endl;
  }
  if (neptuneGenericResult.error.code == 200) {
    cout << neptuneGenericResult.body.serialize() << endl;
  }
  else {
    cout << "NEPTUNE propagation simple generic request failed with status: " << neptuneGenericResult.error.status << endl;
    cout << neptuneGenericResult.error.message << endl;
  }
}

/**
 * Tests the NEPTUNE OPM end points with enabled maneuvre propagation
 */
void neptuneOpmManeuvreTest(OkapiConnector connector, string baseUrl)
{
  // user input NEPTUNE OPM
  string OBJECT_NAME = "ISS (ZARYA)";
  string OBJECT_ID = "1998-067-A";
  string CENTER_NAME = "EARTH";
  string REF_FRAME = "GCRF";
  string REF_FRAME_EPOCH = "2000-01-01T00:00:00Z";
  string TIME_SYSTEM = "UTC";
  string EPOCH = "2016-07-23T00:31:50.000Z";
  double X = 615.119526;
  double Y = -7095.644839;
  double Z = -678.668352;
  double X_DOT = 0.390367;
  double Y_DOT = 0.741902;
  double Z_DOT = -7.39698;
  
  string MAN_EPOCH_IGINITION = "2016-07-23T00:31:50.000Z";
  double MAN_DURATION = 1800;
  double MAN_A_1 = 0.0;
  double MAN_A_2 = 1e-8;
  double MAN_A_3 = 0.0;
  double MASS = 1.3;
  double SOLAR_RAD_COEFF = 1.3;
  double DRAG_AREA = 0.01;
  double DRAG_COEFF = 2.2;
  string propagationEndEpochNeptuneOpm = "2016-07-23T03:31:50.000Z";
  double outputStepSizeNeptuneOpm = 30;

  web::json::value opmHeader;
  opmHeader[U("CCSDS_OPM_VERS")] = 2;
  web::json::value opmMetaData;
  opmMetaData[U("OBJECT_NAME")] = web::json::value::string(OBJECT_NAME);
  opmMetaData[U("OBJECT_ID")] = web::json::value::string(OBJECT_ID);
  opmMetaData[U("CENTER_NAME")] = web::json::value::string(CENTER_NAME);
  opmMetaData[U("REF_FRAME")] = web::json::value::string(REF_FRAME);
  opmMetaData[U("REF_FRAME_EPOCH")] = web::json::value::string(REF_FRAME_EPOCH);
  opmMetaData[U("TIME_SYSTEM")] = web::json::value::string(TIME_SYSTEM);
  
  web::json::value opmData;
  opmData[U("DRAG_AREA")] = DRAG_AREA;
  opmData[U("DRAG_COEFF")] = DRAG_COEFF;
  opmData[U("X")] = X;
  opmData[U("Y")] = Y;
  opmData[U("Z")] = Z;
  opmData[U("X_DOT")] = X_DOT;
  opmData[U("Y_DOT")] = Y_DOT;
  opmData[U("Z_DOT")] = Z_DOT;
  opmData[U("EPOCH")] = web::json::value::string(EPOCH);
  opmData[U("COV_REF_FRAME")] = web::json::value::string("UVW");
  opmData[U("MASS")] = MASS;
  opmData[U("SOLAR_RAD_COEFF")] = SOLAR_RAD_COEFF;
  opmData[U("DRAG_AREA")] = DRAG_AREA;
  opmData[U("DRAG_COEFF")] = DRAG_COEFF;
  std::vector<web::json::value> maneuvres;
  web::json::value maneuvre;
  maneuvre[U("MAN_EPOCH_IGNITION")] = web::json::value::string(MAN_EPOCH_IGINITION);
  maneuvre[U("MAN_REF_FRAME")] = web::json::value::string("UVW");
  maneuvre[U("MAN_DURATION")] = MAN_DURATION;
  maneuvre[U("MAN_A_1")] = MAN_A_1;
  maneuvre[U("MAN_A_2")] = MAN_A_2;
  maneuvre[U("MAN_A_3")] = MAN_A_3;
  maneuvres.push_back(maneuvre);
  opmData[U("MANEUVERS")] = web::json::value::array(maneuvres);
  web::json::value ccsdsOpm;
  ccsdsOpm[U("OPM_HEADER")] = opmHeader;
  ccsdsOpm[U("OPM_META_DATA")] = opmMetaData;
  ccsdsOpm[U("OPM_DATA")] = opmData;

  web::json::value neptuneConfigSimple;
  neptuneConfigSimple[U("geopotential_degree_order")] = 6;
  neptuneConfigSimple[U("atmospheric_drag")] = 1;
  neptuneConfigSimple[U("horizontal_wind")] = 0;
  neptuneConfigSimple[U("sun_gravity")] = 1;
  neptuneConfigSimple[U("moon_gravity")] = 1;
  neptuneConfigSimple[U("solar_radiation_pressure")] = 1;
  neptuneConfigSimple[U("solid_tides")] = 1;
  neptuneConfigSimple[U("ocean_tides")] = 0;
  web::json::value settingsOpm;
  settingsOpm[U("propagation_end_epoch")] = web::json::value::string(propagationEndEpochNeptuneOpm);
  settingsOpm[U("output_step_size")] = outputStepSizeNeptuneOpm;
  settingsOpm[U("neptune_config")] = neptuneConfigSimple;
  web::json::value propagateNeptuneSimpleRequestBody;
  propagateNeptuneSimpleRequestBody[U("CCSDS_OPM")] = ccsdsOpm;
  propagateNeptuneSimpleRequestBody[U("settings")] = settingsOpm;

  // send request for NEPTUNE propagation
  string neptuneUrlRequest = "/propagate-orbit/neptune/requests";
  http_client okapiRequestNeptune(baseUrl + neptuneUrlRequest);
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
  }
  string requestIdNeptune = connector.requestId;
  cout << "Request ID: " << requestIdNeptune << endl;

  // get results for NEPTUNE and print them in the terminal
  string neptuneUrlGet = "/propagate-orbit/neptune/opm/results/";
  http_client okapiGetNeptune(baseUrl + neptuneUrlGet + requestIdNeptune);
  http_request getNeptunePropagation(methods::GET);
  getNeptunePropagation.headers().add(U("access_token"), connector.accessToken);
  OkapiConnector::completeResult neptuneResult = connector.getResult(okapiGetNeptune, getNeptunePropagation);
  if (neptuneResult.error.code != 200 && neptuneResult.error.code != 202)
  {
    cout << "Retrieving NEPTUNE propagation response failed with status: " << neptuneResult.error.status << endl;
    cout << neptuneResult.error.message << endl;
  }
  int i = 0;
  while (neptuneResult.error.code == 202) {
    neptuneResult = connector.getResult(okapiGetNeptune, getNeptunePropagation);
    cout << "The request was successful. Your result is not ready yet.  Waiting: " << i << " s." << endl;
    i++;
    sleep(1);
//    if (i == 10)
//    {
//      cout << "OKAPI timeout" << endl;
//    }
  }
  neptuneResult = connector.getResult(okapiGetNeptune, getNeptunePropagation);
  if (neptuneResult.error.code != 200 && neptuneResult.error.code != 202)
  {
    cout << "Retrieving NEPTUNE propagation simple response failed with status: " << neptuneResult.error.status << endl;
    cout << neptuneResult.error.message << endl;
  }
  if (neptuneResult.error.code == 200) {
    cout << neptuneResult.body.serialize() << endl;
  }
  else {
    cout << "NEPTUNE propagation simple request failed with status: " << neptuneResult.error.status << endl;
    cout << neptuneResult.error.message << endl;
  }

  string neptuneGenericUrlGet = "/propagate-orbit/neptune/opm/results/";
  http_client okapiGetNeptuneGeneric(baseUrl + neptuneGenericUrlGet + requestIdNeptune + "/generic");
  http_request getNeptuneGenericPropagation(methods::GET);
  getNeptuneGenericPropagation.headers().add(U("access_token"), connector.accessToken);
  OkapiConnector::completeResult neptuneGenericResult = connector.getResult(okapiGetNeptuneGeneric, getNeptuneGenericPropagation);
  if (neptuneGenericResult.error.code != 200 && neptuneGenericResult.error.code != 202)
  {
    cout << "Retrieving NEPTUNE propagation response failed with status: " << neptuneGenericResult.error.status << endl;
    cout << neptuneGenericResult.error.message << endl;
  }
  i = 0;
  while (neptuneGenericResult.error.code == 202) {
    neptuneGenericResult = connector.getResult(okapiGetNeptuneGeneric, getNeptuneGenericPropagation);
    cout << "The request was successful. Your result is not ready yet.  Waiting: " << i << " s." << endl;
    i++;
    sleep(1);
//    if (i == 10)
//    {
//    cout << "OKAPI timeout" << endl;
//    }
  }
  neptuneGenericResult = connector.getResult(okapiGetNeptuneGeneric, getNeptuneGenericPropagation);
  if (neptuneGenericResult.error.code != 200 && neptuneGenericResult.error.code != 202)
  {
    cout << "Retrieving NEPTUNE propagation simple generic response failed with status: " << neptuneGenericResult.error.status << endl;
    cout << neptuneGenericResult.error.message << endl;
  }
  if (neptuneGenericResult.error.code == 200) {
    cout << neptuneGenericResult.body.serialize() << endl;
  }
  else {
    cout << "NEPTUNE propagation simple generic request failed with status: " << neptuneGenericResult.error.status << endl;
    cout << neptuneGenericResult.error.message << endl;
  }
}

/**
 * Tests the Orekit-numerical end points
 */
void orekitNumericalTest(OkapiConnector connector, string baseUrl)
{
  // user input Orekit simple
  double area = 0.01;
  double mass = 1.3;
  double x = 615.119526;
  double y = -7095.644839;
  double z = -678.668352;
  double x_dot = 0.390367;
  double y_dot = 0.741902;
  double z_dot = -7.39698;
  string epoch = "2016-07-23T00:31:50.000Z";
  string propagationEndEpochOrekitSimple = "2016-07-23T03:31:50.000Z";
  double outputStepSizeOrekitSimple = 30;

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
  // Yes, Orekit also uses Neptune configurations for now
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
  settingsSimple[U("propagation_end_epoch")] = web::json::value::string(propagationEndEpochOrekitSimple);
  settingsSimple[U("output_step_size")] = outputStepSizeOrekitSimple;
  settingsSimple[U("neptune_config")] = neptuneConfigSimple;
  web::json::value propagateOrekitSimpleRequestBody;
  propagateOrekitSimpleRequestBody[U("simple_state")] = simpleState;
  propagateOrekitSimpleRequestBody[U("settings")] = settingsSimple;


  // send request for Orekit-numerical propagation
  string orekitUrlRequest = "/propagate-orbit/orekit-numerical/requests";
  http_client okapiRequestOrekit(baseUrl + orekitUrlRequest);
  http_request requestOrekit(methods::POST);
  requestOrekit.set_body(propagateOrekitSimpleRequestBody);
  requestOrekit.headers().add(U("access_token"), connector.accessToken);
  OkapiConnector::completeResult orekitRequest = connector.sendRequest(okapiRequestOrekit, requestOrekit);
  if (orekitRequest.error.code == 200 || orekitRequest.error.code == 202)
  {
    cout << "Send Orekit-numerical propagation request completed" << endl;
  }
  else {
    cout << "Orekit-numerical propagation request failed with status: " << orekitRequest.error.status << endl;
    cout << orekitRequest.error.message << endl;
  }
  string requestIdOrekit = connector.requestId;
  cout << "Orekit-numerical request ID: " << requestIdOrekit << endl;

  // get results for Orekit-numerical
  string orekitUrlGet = "/propagate-orbit/orekit-numerical/simple/results/";
  http_client orekitUrl(baseUrl + orekitUrlGet + requestIdOrekit);
  http_request orekitMethod(methods::GET);
  orekitMethod.headers().add(U("access_token"), connector.accessToken);
  OkapiConnector::completeResult orekitResult = retrieveResult(connector, orekitUrl, orekitMethod);
  if (orekitResult.error.code != 200 && orekitResult.error.code != 202)
  {
    cout << "Retrieving Orekit-numerical propagation simple response failed with status: " << orekitResult.error.status << endl;
    cout << orekitResult.error.message << endl;
  }
  if (orekitResult.error.code == 200) {
    cout << orekitResult.body.serialize() << endl;
  }
  else {
    cout << "Orekit-numerical propagation simple request failed with status: " << orekitResult.error.status << endl;
    cout << orekitResult.error.message << endl;
  }
  
  string orekitOpmGenericUrlGet = "/propagate-orbit/orekit-numerical/opm/results/";
  http_client orekitOpmGenericUrl(baseUrl + orekitOpmGenericUrlGet + requestIdOrekit + "/generic");
  http_request orekitOpmGenericMethod(methods::GET);
  orekitOpmGenericMethod.headers().add(U("access_token"), connector.accessToken);
  OkapiConnector::completeResult orekitOpmGenericResult = retrieveResult(connector, orekitOpmGenericUrl, orekitOpmGenericMethod);
  if (orekitOpmGenericResult.error.code != 200 && orekitOpmGenericResult.error.code != 202)
  {
    cout << "Retrieving Orekit-numerical propagation OPM generic response failed with status: " << orekitOpmGenericResult.error.status << endl;
    cout << orekitOpmGenericResult.error.message << endl;
  }
  if (orekitOpmGenericResult.error.code == 200) {
    cout << orekitOpmGenericResult.body.serialize() << endl;
  }
  else {
    cout << "Orekit-numerical propagation OPM generic request failed with status: " << orekitOpmGenericResult.error.status << endl;
    cout << orekitOpmGenericResult.error.message << endl;
  }

  string orekitGenericUrlGet = "/propagate-orbit/orekit-numerical/simple/results/";
  http_client orekitGenericUrl(baseUrl + orekitGenericUrlGet + requestIdOrekit + "/generic");
  http_request orekitGenericMethod(methods::GET);
  orekitGenericMethod.headers().add(U("access_token"), connector.accessToken);
  OkapiConnector::completeResult orekitGenericResult = retrieveResult(connector, orekitGenericUrl, orekitGenericMethod);
  if (orekitGenericResult.error.code != 200 && orekitGenericResult.error.code != 202)
  {
    cout << "Retrieving Orekit-numerical propagation simple generic response failed with status: " << orekitGenericResult.error.status << endl;
    cout << orekitGenericResult.error.message << endl;
  }
  if (orekitGenericResult.error.code == 200) {
    cout << orekitGenericResult.body.serialize() << endl;
  }
  else {
    cout << "Orekit-numerical propagation simple generic request failed with status: " << orekitGenericResult.error.status << endl;
    cout << orekitGenericResult.error.message << endl;
  }
}

/**
 * Tests the SGP4 propagation end points
 */
void sgp4Test(OkapiConnector connector, string baseUrl)
{
  string propagationEndEpochPropagation = "2018-08-08T00:00:00.000Z";
  double outputStepSizePropagation = 100;
  string tlePropagation = "1 25544U 98067A   18218.76369510  .00001449  00000-0  29472-4 0  9993\n2 25544  51.6423 126.6422 0005481  33.3092  62.9075 15.53806849126382";
  
  web::json::value settingsSgp4;
  settingsSgp4[U("propagation_end_epoch")] = web::json::value::string(propagationEndEpochPropagation);
  settingsSgp4[U("output_step_size")] = web::json::value::number(outputStepSizePropagation);
  web::json::value propagateSgp4RequestBody;
  propagateSgp4RequestBody[U("tle")] = web::json::value::string(tlePropagation);
  propagateSgp4RequestBody[U("settings")] = settingsSgp4;
  
  // send request for SGP4 propagation
  string sgp4UrlRequest = "/propagate-orbit/sgp4/requests";
  http_client okapiRequestOrekit(baseUrl + sgp4UrlRequest);
  http_request requestSgp4(methods::POST);
  requestSgp4.set_body(propagateSgp4RequestBody);
  requestSgp4.headers().add(U("access_token"), connector.accessToken);
  OkapiConnector::completeResult sgp4Request = connector.sendRequest(okapiRequestOrekit, requestSgp4);
  if (sgp4Request.error.code == 200 || sgp4Request.error.code == 202)
  {
    cout << "Send SGP4 propagation request completed" << endl;
  }
  else {
    cout << "SGP4 propagation request failed with status: " << sgp4Request.error.status << endl;
    cout << sgp4Request.error.message << endl;
  }
  string requestIdSgp4 = connector.requestId;
  cout << "SGP4 request ID: " << requestIdSgp4 << endl;
  
  // get results for Orekit-numerical
  string sgp4UrlGet = "/propagate-orbit/sgp4/simple/results/";
  http_client sgp4Url(baseUrl + sgp4UrlGet + requestIdSgp4);
  http_request sgp4Method(methods::GET);
  sgp4Method.headers().add(U("access_token"), connector.accessToken);
  OkapiConnector::completeResult sgp4Result = retrieveResult(connector, sgp4Url, sgp4Method);
  if (sgp4Result.error.code != 200 && sgp4Result.error.code != 202)
  {
    cout << "Retrieving SGP4 propagation simple response failed with status: " << sgp4Result.error.status << endl;
    cout << sgp4Result.error.message << endl;
  }
  if (sgp4Result.error.code == 200) {
    cout << sgp4Result.body.serialize() << endl;
  }
  else {
    cout << "SGP4 propagation simple request failed with status: " << sgp4Result.error.status << endl;
    cout << sgp4Result.error.message << endl;
  }
  
  // get results for Orekit-numerical
  string sgp4OmmUrlGet = "/propagate-orbit/sgp4/omm/results/";
  http_client sgp4OmmUrl(baseUrl + sgp4OmmUrlGet + requestIdSgp4);
  http_request sgp4OmmMethod(methods::GET);
  sgp4OmmMethod.headers().add(U("access_token"), connector.accessToken);
  OkapiConnector::completeResult sgp4OmmResult = retrieveResult(connector, sgp4OmmUrl, sgp4OmmMethod);
  if (sgp4OmmResult.error.code != 200 && sgp4OmmResult.error.code != 202)
  {
    cout << "Retrieving SGP4 propagation OMM response failed with status: " << sgp4OmmResult.error.status << endl;
    cout << sgp4OmmResult.error.message << endl;
  }
  if (sgp4OmmResult.error.code == 200) {
    cout << sgp4OmmResult.body.serialize() << endl;
  }
  else {
    cout << "SGP4 propagation OMM request failed with status: " << sgp4OmmResult.error.status << endl;
    cout << sgp4OmmResult.error.message << endl;
  }
  
}

void riskEstimationTest(OkapiConnector connector, string baseUrl)
{

  string bodyString ="{\"CCSDS_CDM\":{\"CONSTELLATION\":\"TU Berlin\",\"CDM_ID\":\"24537247\",\"FILENAME\":\"42829_conj_43782_2019076092820_3987.xml\",\"INSERT_EPOCH\":\"2019-03-17 11:37:59\",\"CCSDS_CDM_VERS\":\"1.0\",\"CREATION_DATE\":\"2019-03-17 07:38:19\",\"CREATION_DATE_FRACTION\":\"0\",\"ORIGINATOR\":\"JSPOC\",\"MESSAGE_FOR\":\"TECHNOSAT\",\"MESSAGE_ID\":\"42829_conj_43782_2019076092820_0760816243987\",\"COMMENT_EMERGENCY_REPORTABLE\":null,\"TCA\":\"2019-03-17 09:28:20\",\"TCA_FRACTION\":\"23\",\"MISS_DISTANCE\":\"804\",\"MISS_DISTANCE_UNIT\":\"m\",\"RELATIVE_SPEED\":\"14997\",\"RELATIVE_SPEED_UNIT\":\"m\\/s\",\"RELATIVE_POSITION_R\":\"-71.4\",\"RELATIVE_POSITION_R_UNIT\":\"m\",\"RELATIVE_POSITION_T\":\"110.2\",\"RELATIVE_POSITION_T_UNIT\":\"m\",\"RELATIVE_POSITION_N\":\"793.3\",\"RELATIVE_POSITION_N_UNIT\":\"m\",\"RELATIVE_VELOCITY_R\":\"11.4\",\"RELATIVE_VELOCITY_R_UNIT\":\"m\\/s\",\"RELATIVE_VELOCITY_T\":\"-14856\",\"RELATIVE_VELOCITY_T_UNIT\":\"m\\/s\",\"RELATIVE_VELOCITY_N\":\"2053.1\",\"RELATIVE_VELOCITY_N_UNIT\":\"m\\/s\",\"COLLISION_PROBABILITY\":\"0\",\"COLLISION_PROBABILITY_METHOD\":\"FOSTER-1992\",\"SAT1_OBJECT\":\"OBJECT1\",\"SAT1_OBJECT_DESIGNATOR\":\"42829\",\"SAT1_CATALOG_NAME\":\"SATCAT\",\"SAT1_OBJECT_NAME\":\"TECHNOSAT\",\"SAT1_INTERNATIONAL_DESIGNATOR\":\"2017-042E\",\"SAT1_OBJECT_TYPE\":\"PAYLOAD\",\"SAT1_OPERATOR_CONTACT_POSITION\":\"https:\\/\\/www.space-track.org\\/expandedspacedata\\/query\\/class\\/organization\\/object\\/~~42829\\/orderby\\/ORG_NAME,INFO_ID\\/format\\/html\\/emptyresult\\/show\\/\",\"SAT1_OPERATOR_ORGANIZATION\":\"TU Berlin\",\"SAT1_OPERATOR_PHONE\":\"https:\\/\\/www.space-track.org\\/expandedspacedata\\/query\\/class\\/organization\\/object\\/~~42829\\/orderby\\/ORG_NAME,INFO_ID\\/format\\/html\\/emptyresult\\/show\\/\",\"SAT1_OPERATOR_EMAIL\":\"https:\\/\\/www.space-track.org\\/expandedspacedata\\/query\\/class\\/organization\\/object\\/~~42829\\/orderby\\/ORG_NAME,INFO_ID\\/format\\/html\\/emptyresult\\/show\\/\",\"SAT1_EPHEMERIS_NAME\":\"NONE\",\"SAT1_COVARIANCE_METHOD\":\"CALCULATED\",\"SAT1_MANEUVERABLE\":\"NO\",\"SAT1_REF_FRAME\":\"ITRF\",\"SAT1_GRAVITY_MODEL\":\"EGM-96: 36D 36O\",\"SAT1_ATMOSPHERIC_MODEL\":\"JBH09\",\"SAT1_N_BODY_PERTURBATIONS\":\"MOON,SUN\",\"SAT1_SOLAR_RAD_PRESSURE\":\"YES\",\"SAT1_EARTH_TIDES\":\"YES\",\"SAT1_INTRACK_THRUST\":\"NO\",\"SAT1_TIME_LASTOB_START\":\"2019-03-16 07:38:19\",\"SAT1_TIME_LASTOB_START_FRACTION\":\"0\",\"SAT1_TIME_LASTOB_END\":\"2019-03-17 07:38:19\",\"SAT1_TIME_LASTOB_END_FRACTION\":\"0\",\"SAT1_RECOMMENDED_OD_SPAN\":\"7.52\",\"SAT1_RECOMMENDED_OD_SPAN_UNIT\":\"d\",\"SAT1_ACTUAL_OD_SPAN\":\"7.52\",\"SAT1_ACTUAL_OD_SPAN_UNIT\":\"d\",\"SAT1_OBS_AVAILABLE\":\"183\",\"SAT1_OBS_USED\":\"183\",\"SAT1_RESIDUALS_ACCEPTED\":\"98.4\",\"SAT1_RESIDUALS_ACCEPTED_UNIT\":\"%\",\"SAT1_WEIGHTED_RMS\":\"1.113\",\"SAT1_COMMENT_APOGEE\":\"Apogee Altitude = 628   [km]\",\"SAT1_COMMENT_PERIGEE\":\"Perigee Altitude = 594   [km]\",\"SAT1_COMMENT_INCLINATION\":\"Inclination = 97.6  [deg]\",\"SAT1_AREA_PC\":\"0.3906\",\"SAT1_AREA_PC_UNIT\":\"m**2\",\"SAT1_CD_AREA_OVER_MASS\":\"0.0219084\",\"SAT1_CD_AREA_OVER_MASS_UNIT\":\"m**2\\/kg\",\"SAT1_CR_AREA_OVER_MASS\":\"0.00898291\",\"SAT1_CR_AREA_OVER_MASS_UNIT\":\"m**2\\/kg\",\"SAT1_THRUST_ACCELERATION\":\"0\",\"SAT1_THRUST_ACCELERATION_UNIT\":\"m\\/s**2\",\"SAT1_SEDR\":\"5.89239e-05\",\"SAT1_SEDR_UNIT\":\"W\\/kg\",\"SAT1_X\":\"6562.2804\",\"SAT1_X_UNIT\":\"km\",\"SAT1_Y\":\"1703.04577\",\"SAT1_Y_UNIT\":\"km\",\"SAT1_Z\":\"1592.077551\",\"SAT1_Z_UNIT\":\"km\",\"SAT1_X_DOT\":\"-1.28827778\",\"SAT1_X_DOT_UNIT\":\"km\\/s\",\"SAT1_Y_DOT\":\"-1.90418306\",\"SAT1_Y_DOT_UNIT\":\"km\\/s\",\"SAT1_Z_DOT\":\"7.30255187\",\"SAT1_Z_DOT_UNIT\":\"km\\/s\",\"SAT1_CR_R\":\"46.1461856511049\",\"SAT1_CR_R_UNIT\":\"m**2\",\"SAT1_CT_R\":\"42.3471255956732\",\"SAT1_CT_R_UNIT\":\"m**2\",\"SAT1_CT_T\":\"302.242625462294\",\"SAT1_CT_T_UNIT\":\"m**2\",\"SAT1_CN_R\":\"2.33965674350612\",\"SAT1_CN_R_UNIT\":\"m**2\",\"SAT1_CN_T\":\"-7.52607416991497\",\"SAT1_CN_T_UNIT\":\"m**2\",\"SAT1_CN_N\":\"26.3489367881701\",\"SAT1_CN_N_UNIT\":\"m**2\",\"SAT1_CRDOT_R\":\"-0.0547784235749886\",\"SAT1_CRDOT_R_UNIT\":\"m**2\\/s\",\"SAT1_CRDOT_T\":\"-0.295120151146788\",\"SAT1_CRDOT_T_UNIT\":\"m**2\\/s\",\"SAT1_CRDOT_N\":\"-0.000325743726599067\",\"SAT1_CRDOT_N_UNIT\":\"m**2\\/s\",\"SAT1_CRDOT_RDOT\":\"0.00031032143490407\",\"SAT1_CRDOT_RDOT_UNIT\":\"m**2\\/s**2\",\"SAT1_CTDOT_R\":\"-0.0501856442302755\",\"SAT1_CTDOT_R_UNIT\":\"m**2\\/s\",\"SAT1_CTDOT_T\":\"-0.0471861244930156\",\"SAT1_CTDOT_T_UNIT\":\"m**2\\/s\",\"SAT1_CTDOT_N\":\"-0.00252782542649572\",\"SAT1_CTDOT_N_UNIT\":\"m**2\\/s\",\"SAT1_CTDOT_RDOT\":\"6.07418730203252e-05\",\"SAT1_CTDOT_RDOT_UNIT\":\"m**2\\/s**2\",\"SAT1_CTDOT_TDOT\":\"5.45874543922052e-05\",\"SAT1_CTDOT_TDOT_UNIT\":\"m**2\\/s**2\",\"SAT1_CNDOT_R\":\"-0.00751381926357464\",\"SAT1_CNDOT_R_UNIT\":\"m**2\\/s\",\"SAT1_CNDOT_T\":\"-0.00371414808055227\",\"SAT1_CNDOT_T_UNIT\":\"m**2\\/s\",\"SAT1_CNDOT_N\":\"-0.0113588008096445\",\"SAT1_CNDOT_N_UNIT\":\"m**2\\/s\",\"SAT1_CNDOT_RDOT\":\"1.09417486735304e-05\",\"SAT1_CNDOT_RDOT_UNIT\":\"m**2\\/s**2\",\"SAT1_CNDOT_TDOT\":\"8.17704363245973e-06\",\"SAT1_CNDOT_TDOT_UNIT\":\"m**2\\/s**2\",\"SAT1_CNDOT_NDOT\":\"1.74453648788543e-05\",\"SAT1_CNDOT_NDOT_UNIT\":\"m**2\\/s**2\",\"SAT1_CDRG_R\":\"0\",\"SAT1_CDRG_R_UNIT\":\"m**3\\/kg\",\"SAT1_CDRG_T\":\"0\",\"SAT1_CDRG_T_UNIT\":\"m**3\\/kg\",\"SAT1_CDRG_N\":\"0\",\"SAT1_CDRG_N_UNIT\":\"m**3\\/kg\",\"SAT1_CDRG_RDOT\":\"0\",\"SAT1_CDRG_RDOT_UNIT\":\"m**3\\/(kg*s)\",\"SAT1_CDRG_TDOT\":\"0\",\"SAT1_CDRG_TDOT_UNIT\":\"m**3\\/(kg*s)\",\"SAT1_CDRG_NDOT\":\"0\",\"SAT1_CDRG_NDOT_UNIT\":\"m**3\\/(kg*s)\",\"SAT1_CDRG_DRG\":\"0\",\"SAT1_CDRG_DRG_UNIT\":\"m**4\\/kg**2\",\"SAT1_CSRP_R\":\"0\",\"SAT1_CSRP_R_UNIT\":\"m**3\\/kg\",\"SAT1_CSRP_T\":\"0\",\"SAT1_CSRP_T_UNIT\":\"m**3\\/kg\",\"SAT1_CSRP_N\":\"0\",\"SAT1_CSRP_N_UNIT\":\"m**3\\/kg\",\"SAT1_CSRP_RDOT\":\"0\",\"SAT1_CSRP_RDOT_UNIT\":\"m**3\\/(kg*s)\",\"SAT1_CSRP_TDOT\":\"0\",\"SAT1_CSRP_TDOT_UNIT\":\"m**3\\/(kg*s)\",\"SAT1_CSRP_NDOT\":\"0\",\"SAT1_CSRP_NDOT_UNIT\":\"m**3\\/(kg*s)\",\"SAT1_CSRP_DRG\":\"0\",\"SAT1_CSRP_DRG_UNIT\":\"m**4\\/kg**2\",\"SAT1_CSRP_SRP\":\"0\",\"SAT1_CSRP_SRP_UNIT\":\"m**4\\/kg**2\",\"SAT2_OBJECT\":\"OBJECT2\",\"SAT2_OBJECT_DESIGNATOR\":\"43782\",\"SAT2_CATALOG_NAME\":\"SATCAT\",\"SAT2_OBJECT_NAME\":\"OBJECT AA\",\"SAT2_INTERNATIONAL_DESIGNATOR\":\"2018-099AA\",\"SAT2_OBJECT_TYPE\":\"TBA\",\"SAT2_OPERATOR_CONTACT_POSITION\":\"https:\\/\\/www.space-track.org\\/expandedspacedata\\/query\\/class\\/organization\\/object\\/~~43782\\/orderby\\/ORG_NAME,INFO_ID\\/format\\/html\\/emptyresult\\/show\\/\",\"SAT2_OPERATOR_ORGANIZATION\":\"NONE\",\"SAT2_OPERATOR_PHONE\":\"https:\\/\\/www.space-track.org\\/expandedspacedata\\/query\\/class\\/organization\\/object\\/~~43782\\/orderby\\/ORG_NAME,INFO_ID\\/format\\/html\\/emptyresult\\/show\\/\",\"SAT2_OPERATOR_EMAIL\":\"https:\\/\\/www.space-track.org\\/expandedspacedata\\/query\\/class\\/organization\\/object\\/~~43782\\/orderby\\/ORG_NAME,INFO_ID\\/format\\/html\\/emptyresult\\/show\\/\",\"SAT2_EPHEMERIS_NAME\":\"NONE\",\"SAT2_COVARIANCE_METHOD\":\"CALCULATED\",\"SAT2_MANEUVERABLE\":\"N\\/A\",\"SAT2_REF_FRAME\":\"ITRF\",\"SAT2_GRAVITY_MODEL\":\"EGM-96: 36D 36O\",\"SAT2_ATMOSPHERIC_MODEL\":\"JBH09\",\"SAT2_N_BODY_PERTURBATIONS\":\"MOON,SUN\",\"SAT2_SOLAR_RAD_PRESSURE\":\"YES\",\"SAT2_EARTH_TIDES\":\"YES\",\"SAT2_INTRACK_THRUST\":\"NO\",\"SAT2_TIME_LASTOB_START\":\"2019-03-16 07:38:19\",\"SAT2_TIME_LASTOB_START_FRACTION\":\"0\",\"SAT2_TIME_LASTOB_END\":\"2019-03-17 07:38:19\",\"SAT2_TIME_LASTOB_END_FRACTION\":\"0\",\"SAT2_RECOMMENDED_OD_SPAN\":\"9.43\",\"SAT2_RECOMMENDED_OD_SPAN_UNIT\":\"d\",\"SAT2_ACTUAL_OD_SPAN\":\"9.43\",\"SAT2_ACTUAL_OD_SPAN_UNIT\":\"d\",\"SAT2_OBS_AVAILABLE\":\"140\",\"SAT2_OBS_USED\":\"138\",\"SAT2_RESIDUALS_ACCEPTED\":\"99.4\",\"SAT2_RESIDUALS_ACCEPTED_UNIT\":\"%\",\"SAT2_WEIGHTED_RMS\":\"1.317\",\"SAT2_COMMENT_APOGEE\":\"Apogee Altitude = 601   [km]\",\"SAT2_COMMENT_PERIGEE\":\"Perigee Altitude = 596   [km]\",\"SAT2_COMMENT_INCLINATION\":\"Inclination = 97.8  [deg]\",\"SAT2_AREA_PC\":\"0.059\",\"SAT2_AREA_PC_UNIT\":\"m**2\",\"SAT2_CD_AREA_OVER_MASS\":\"0.0281873\",\"SAT2_CD_AREA_OVER_MASS_UNIT\":\"m**2\\/kg\",\"SAT2_CR_AREA_OVER_MASS\":\"0.01374585\",\"SAT2_CR_AREA_OVER_MASS_UNIT\":\"m**2\\/kg\",\"SAT2_THRUST_ACCELERATION\":\"0\",\"SAT2_THRUST_ACCELERATION_UNIT\":\"m\\/s**2\",\"SAT2_SEDR\":\"8.22796e-05\",\"SAT2_SEDR_UNIT\":\"W\\/kg\",\"SAT2_X\":\"6562.413913\",\"SAT2_X_UNIT\":\"km\",\"SAT2_Y\":\"1702.252966\",\"SAT2_Y_UNIT\":\"km\",\"SAT2_Z\":\"1592.063015\",\"SAT2_Z_UNIT\":\"km\",\"SAT2_X_DOT\":\"2.046814905\",\"SAT2_X_DOT_UNIT\":\"km\\/s\",\"SAT2_Y_DOT\":\"-1.062667288\",\"SAT2_Y_DOT_UNIT\":\"km\\/s\",\"SAT2_Z_DOT\":\"-7.2948262\",\"SAT2_Z_DOT_UNIT\":\"km\\/s\",\"SAT2_CR_R\":\"99.9911568880684\",\"SAT2_CR_R_UNIT\":\"m**2\",\"SAT2_CT_R\":\"-85.5596052003614\",\"SAT2_CT_R_UNIT\":\"m**2\",\"SAT2_CT_T\":\"679.619604230875\",\"SAT2_CT_T_UNIT\":\"m**2\",\"SAT2_CN_R\":\"27.0614673334493\",\"SAT2_CN_R_UNIT\":\"m**2\",\"SAT2_CN_T\":\"-16.6568273265456\",\"SAT2_CN_T_UNIT\":\"m**2\",\"SAT2_CN_N\":\"63.3048647153926\",\"SAT2_CN_N_UNIT\":\"m**2\",\"SAT2_CRDOT_R\":\"0.107056879196751\",\"SAT2_CRDOT_R_UNIT\":\"m**2\\/s\",\"SAT2_CRDOT_T\":\"-0.659964067622497\",\"SAT2_CRDOT_T_UNIT\":\"m**2\\/s\",\"SAT2_CRDOT_N\":\"0.019485753052086\",\"SAT2_CRDOT_N_UNIT\":\"m**2\\/s\",\"SAT2_CRDOT_RDOT\":\"0.000708585747151425\",\"SAT2_CRDOT_RDOT_UNIT\":\"m**2\\/s**2\",\"SAT2_CTDOT_R\":\"-0.108846482024542\",\"SAT2_CTDOT_R_UNIT\":\"m**2\\/s\",\"SAT2_CTDOT_T\":\"0.0926867959261093\",\"SAT2_CTDOT_T_UNIT\":\"m**2\\/s\",\"SAT2_CTDOT_N\":\"-0.0294861841043052\",\"SAT2_CTDOT_N_UNIT\":\"m**2\\/s\",\"SAT2_CTDOT_RDOT\":\"-0.000115870505266814\",\"SAT2_CTDOT_RDOT_UNIT\":\"m**2\\/s**2\",\"SAT2_CTDOT_TDOT\":\"0.000118496023294848\",\"SAT2_CTDOT_TDOT_UNIT\":\"m**2\\/s**2\",\"SAT2_CNDOT_R\":\"-0.0141976572926528\",\"SAT2_CNDOT_R_UNIT\":\"m**2\\/s\",\"SAT2_CNDOT_T\":\"0.0382956717994341\",\"SAT2_CNDOT_T_UNIT\":\"m**2\\/s\",\"SAT2_CNDOT_N\":\"-0.00809689080893274\",\"SAT2_CNDOT_N_UNIT\":\"m**2\\/s\",\"SAT2_CNDOT_RDOT\":\"-2.78052024499934e-05\",\"SAT2_CNDOT_RDOT_UNIT\":\"m**2\\/s**2\",\"SAT2_CNDOT_TDOT\":\"1.54543876352614e-05\",\"SAT2_CNDOT_TDOT_UNIT\":\"m**2\\/s**2\",\"SAT2_CNDOT_NDOT\":\"2.37648026501431e-05\",\"SAT2_CNDOT_NDOT_UNIT\":\"m**2\\/s**2\",\"SAT2_CDRG_R\":\"0\",\"SAT2_CDRG_R_UNIT\":\"m**3\\/kg\",\"SAT2_CDRG_T\":\"0\",\"SAT2_CDRG_T_UNIT\":\"m**3\\/kg\",\"SAT2_CDRG_N\":\"0\",\"SAT2_CDRG_N_UNIT\":\"m**3\\/kg\",\"SAT2_CDRG_RDOT\":\"0\",\"SAT2_CDRG_RDOT_UNIT\":\"m**3\\/(kg*s)\",\"SAT2_CDRG_TDOT\":\"0\",\"SAT2_CDRG_TDOT_UNIT\":\"m**3\\/(kg*s)\",\"SAT2_CDRG_NDOT\":\"0\",\"SAT2_CDRG_NDOT_UNIT\":\"m**3\\/(kg*s)\",\"SAT2_CDRG_DRG\":\"0\",\"SAT2_CDRG_DRG_UNIT\":\"m**4\\/kg**2\",\"SAT2_CSRP_R\":\"0\",\"SAT2_CSRP_R_UNIT\":\"m**3\\/kg\",\"SAT2_CSRP_T\":\"0\",\"SAT2_CSRP_T_UNIT\":\"m**3\\/kg\",\"SAT2_CSRP_N\":\"0\",\"SAT2_CSRP_N_UNIT\":\"m**3\\/kg\",\"SAT2_CSRP_RDOT\":\"0\",\"SAT2_CSRP_RDOT_UNIT\":\"m**3\\/(kg*s)\",\"SAT2_CSRP_TDOT\":\"0\",\"SAT2_CSRP_TDOT_UNIT\":\"m**3\\/(kg*s)\",\"SAT2_CSRP_NDOT\":\"0\",\"SAT2_CSRP_NDOT_UNIT\":\"m**3\\/(kg*s)\",\"SAT2_CSRP_DRG\":\"0\",\"SAT2_CSRP_DRG_UNIT\":\"m**4\\/kg**2\",\"SAT2_CSRP_SRP\":\"0\",\"SAT2_CSRP_SRP_UNIT\":\"m**4\\/kg**2\",\"GID\":\"79\"}}";
  
  web::json::value riskBody;
  web::json::value ccsds_cdm;
  web::json::value overrides;
  
  ccsds_cdm[U("CONSTELLATION")] = web::json::value::string("TU Berlin");
  ccsds_cdm[U("CDM_ID")] = web::json::value::string("24537247");
  ccsds_cdm[U("FILENAME")] = web::json::value::string("42829_conj_43782_2019076092820_3987.xml");
  ccsds_cdm[U("INSERT_EPOCH")] = web::json::value::string("2019-03-17 11:37:59");
  ccsds_cdm[U("CCSDS_CDM_VERS")] = web::json::value::string("1.0");
  ccsds_cdm[U("CREATION_DATE")] = web::json::value::string("2019-03-17 07:38:19");
  ccsds_cdm[U("CREATION_DATE_FRACTION")] = web::json::value::string("0");
  ccsds_cdm[U("ORIGINATOR")] = web::json::value::string("JSPOC");
  ccsds_cdm[U("MESSAGE_FOR")] = web::json::value::string("TECHNOSAT");
  ccsds_cdm[U("MESSAGE_ID")] = web::json::value::string("42829_conj_43782_2019076092820_0760816243987");
  ccsds_cdm[U("TCA")] = web::json::value::string("2019-03-17 09:28:20");
  ccsds_cdm[U("TCA_FRACTION")] = web::json::value::string("23");
  ccsds_cdm[U("MISS_DISTANCE")] = web::json::value::string("804");
  ccsds_cdm[U("MISS_DISTANCE_UNIT")] = web::json::value::string("m");
  ccsds_cdm[U("RELATIVE_SPEED")] = web::json::value::string("14997");
  ccsds_cdm[U("RELATIVE_SPEED_UNIT")] = web::json::value::string("m\\/s");
  ccsds_cdm[U("RELATIVE_POSITION_R")] = web::json::value::string("-71.4");
  ccsds_cdm[U("RELATIVE_POSITION_R_UNIT")] = web::json::value::string("m");
  ccsds_cdm[U("RELATIVE_POSITION_T")] = web::json::value::string("110.2");
  ccsds_cdm[U("RELATIVE_POSITION_T_UNIT")] = web::json::value::string("m");
  ccsds_cdm[U("RELATIVE_POSITION_N")] = web::json::value::string("793.3");
  ccsds_cdm[U("RELATIVE_POSITION_N_UNIT")] = web::json::value::string("m");
  ccsds_cdm[U("RELATIVE_VELOCITY_R")] = web::json::value::string("11.4");
  ccsds_cdm[U("RELATIVE_VELOCITY_R_UNIT")] = web::json::value::string("m\\/s");
  ccsds_cdm[U("RELATIVE_VELOCITY_T")] = web::json::value::string("-14856");
  ccsds_cdm[U("RELATIVE_VELOCITY_T_UNIT")] = web::json::value::string("m\\/s");
  ccsds_cdm[U("RELATIVE_VELOCITY_N")] = web::json::value::string("2053.1");
  ccsds_cdm[U("RELATIVE_VELOCITY_N_UNIT")] = web::json::value::string("m\\/s");
  ccsds_cdm[U("COLLISION_PROBABILITY")] = web::json::value::string("0");
  ccsds_cdm[U("COLLISION_PROBABILITY_METHOD")] = web::json::value::string("FOSTER-1992");
  ccsds_cdm[U("SAT1_OBJECT")] = web::json::value::string("OBJECT1");
  ccsds_cdm[U("SAT1_OBJECT_DESIGNATOR")] = web::json::value::string("42829");
  ccsds_cdm[U("SAT1_CATALOG_NAME")] = web::json::value::string("SATCAT");
  ccsds_cdm[U("SAT1_OBJECT_NAME")] = web::json::value::string("TECHNOSAT");
  ccsds_cdm[U("SAT1_INTERNATIONAL_DESIGNATOR")] = web::json::value::string("2017-042E");
  ccsds_cdm[U("SAT1_OBJECT_TYPE")] = web::json::value::string("PAYLOAD");
  ccsds_cdm[U("SAT1_OPERATOR_CONTACT_POSITION")] = web::json::value::string("https:\\/\\/www.space-track.org\\/expandedspacedata\\/query\\/class\\/organization\\/object\\/~~42829\\/orderby\\/ORG_NAME,INFO_ID\\/format\\/html\\/emptyresult\\/show\\/");
  ccsds_cdm[U("SAT1_OPERATOR_ORGANIZATION")] = web::json::value::string("TU Berlin");
  ccsds_cdm[U("SAT1_OPERATOR_PHONE")] = web::json::value::string("https:\\/\\/www.space-track.org\\/expandedspacedata\\/query\\/class\\/organization\\/object\\/~~42829\\/orderby\\/ORG_NAME,INFO_ID\\/format\\/html\\/emptyresult\\/show\\/");
  ccsds_cdm[U("SAT1_OPERATOR_EMAIL")] = web::json::value::string("https:\\/\\/www.space-track.org\\/expandedspacedata\\/query\\/class\\/organization\\/object\\/~~42829\\/orderby\\/ORG_NAME,INFO_ID\\/format\\/html\\/emptyresult\\/show\\/");
  ccsds_cdm[U("SAT1_EPHEMERIS_NAME")] = web::json::value::string("NONE");
  ccsds_cdm[U("SAT1_COVARIANCE_METHOD")] = web::json::value::string("CALCULATED");
  ccsds_cdm[U("SAT1_MANEUVERABLE")] = web::json::value::string("NO");
  ccsds_cdm[U("SAT1_REF_FRAME")] = web::json::value::string("ITRF");
  ccsds_cdm[U("SAT1_GRAVITY_MODEL")] = web::json::value::string("EGM-96: 36D 36O");
  ccsds_cdm[U("SAT1_ATMOSPHERIC_MODEL")] = web::json::value::string("JBH09");
  ccsds_cdm[U("SAT1_N_BODY_PERTURBATIONS")] = web::json::value::string("MOON,SUN");
  ccsds_cdm[U("SAT1_SOLAR_RAD_PRESSURE")] = web::json::value::string("YES");
  ccsds_cdm[U("SAT1_EARTH_TIDES")] = web::json::value::string("YES");
  ccsds_cdm[U("SAT1_INTRACK_THRUST")] = web::json::value::string("NO");
  ccsds_cdm[U("SAT1_TIME_LASTOB_START")] = web::json::value::string("2019-03-16 07:38:19");
  ccsds_cdm[U("SAT1_TIME_LASTOB_START_FRACTION")] = web::json::value::string("0");
  ccsds_cdm[U("SAT1_TIME_LASTOB_END")] = web::json::value::string("2019-03-17 07:38:19");
  ccsds_cdm[U("SAT1_TIME_LASTOB_END_FRACTION")] = web::json::value::string("0");
  ccsds_cdm[U("SAT1_RECOMMENDED_OD_SPAN")] = web::json::value::string("7.52");
  ccsds_cdm[U("SAT1_RECOMMENDED_OD_SPAN_UNIT")] = web::json::value::string("d");
  ccsds_cdm[U("SAT1_ACTUAL_OD_SPAN")] = web::json::value::string("7.52");
  ccsds_cdm[U("SAT1_ACTUAL_OD_SPAN_UNIT")] = web::json::value::string("d");
  ccsds_cdm[U("SAT1_OBS_AVAILABLE")] = web::json::value::string("183");
  ccsds_cdm[U("SAT1_OBS_USED")] = web::json::value::string("183");
  ccsds_cdm[U("SAT1_RESIDUALS_ACCEPTED")] = web::json::value::string("98.4");
  ccsds_cdm[U("SAT1_RESIDUALS_ACCEPTED_UNIT")] = web::json::value::string("%");
  ccsds_cdm[U("SAT1_WEIGHTED_RMS")] = web::json::value::string("1.113");
  ccsds_cdm[U("SAT1_COMMENT_APOGEE")] = web::json::value::string("Apogee Altitude = 628   [km]");
  ccsds_cdm[U("SAT1_COMMENT_PERIGEE")] = web::json::value::string("Perigee Altitude = 594   [km]");
  ccsds_cdm[U("SAT1_COMMENT_INCLINATION")] = web::json::value::string("Inclination = 97.6  [deg]");
  ccsds_cdm[U("SAT1_AREA_PC")] = web::json::value::string("0.3906");
  ccsds_cdm[U("SAT1_AREA_PC_UNIT")] = web::json::value::string("m**2");
  ccsds_cdm[U("SAT1_CD_AREA_OVER_MASS")] = web::json::value::string("0.0219084");
  ccsds_cdm[U("SAT1_CD_AREA_OVER_MASS_UNIT")] = web::json::value::string("m**2\\/kg");
  ccsds_cdm[U("SAT1_CR_AREA_OVER_MASS")] = web::json::value::string("0.00898291");
  ccsds_cdm[U("SAT1_CR_AREA_OVER_MASS_UNIT")] = web::json::value::string("m**2\\/kg");
  ccsds_cdm[U("SAT1_THRUST_ACCELERATION")] = web::json::value::string("0");
  ccsds_cdm[U("SAT1_THRUST_ACCELERATION_UNIT")] = web::json::value::string("m\\/s**2");
  ccsds_cdm[U("SAT1_SEDR")] = web::json::value::string("5.89239e-05");
  ccsds_cdm[U("SAT1_SEDR_UNIT")] = web::json::value::string("W\\/kg");
  ccsds_cdm[U("SAT1_X")] = web::json::value::string("6562.2804");
  ccsds_cdm[U("SAT1_X_UNIT")] = web::json::value::string("km");
  ccsds_cdm[U("SAT1_Y")] = web::json::value::string("1703.04577");
  ccsds_cdm[U("SAT1_Y_UNIT")] = web::json::value::string("km");
  ccsds_cdm[U("SAT1_Z")] = web::json::value::string("1592.077551");
  ccsds_cdm[U("SAT1_Z_UNIT")] = web::json::value::string("km");
  ccsds_cdm[U("SAT1_X_DOT")] = web::json::value::string("-1.28827778");
  ccsds_cdm[U("SAT1_X_DOT_UNIT")] = web::json::value::string("km\\/s");
  ccsds_cdm[U("SAT1_Y_DOT")] = web::json::value::string("-1.90418306");
  ccsds_cdm[U("SAT1_Y_DOT_UNIT")] = web::json::value::string("km\\/s");
  ccsds_cdm[U("SAT1_Z_DOT")] = web::json::value::string("7.30255187");
  ccsds_cdm[U("SAT1_Z_DOT_UNIT")] = web::json::value::string("km\\/s");
  ccsds_cdm[U("SAT1_CR_R")] = web::json::value::string("46.1461856511049");
  ccsds_cdm[U("SAT1_CR_R_UNIT")] = web::json::value::string("m**2");
  ccsds_cdm[U("SAT1_CT_R")] = web::json::value::string("42.3471255956732");
  ccsds_cdm[U("SAT1_CT_R_UNIT")] = web::json::value::string("m**2");
  ccsds_cdm[U("SAT1_CT_T")] = web::json::value::string("302.242625462294");
  ccsds_cdm[U("SAT1_CT_T_UNIT")] = web::json::value::string("m**2");
  ccsds_cdm[U("SAT1_CN_R")] = web::json::value::string("2.33965674350612");
  ccsds_cdm[U("SAT1_CN_R_UNIT")] = web::json::value::string("m**2");
  ccsds_cdm[U("SAT1_CN_T")] = web::json::value::string("-7.52607416991497");
  ccsds_cdm[U("SAT1_CN_T_UNIT")] = web::json::value::string("m**2");
  ccsds_cdm[U("SAT1_CN_N")] = web::json::value::string("26.3489367881701");
  ccsds_cdm[U("SAT1_CN_N_UNIT")] = web::json::value::string("m**2");
  ccsds_cdm[U("SAT1_CRDOT_R")] = web::json::value::string("-0.0547784235749886");
  ccsds_cdm[U("SAT1_CRDOT_R_UNIT")] = web::json::value::string("m**2\\/s");
  ccsds_cdm[U("SAT1_CRDOT_T")] = web::json::value::string("-0.295120151146788");
  ccsds_cdm[U("SAT1_CRDOT_T_UNIT")] = web::json::value::string("m**2\\/s");
  ccsds_cdm[U("SAT1_CRDOT_N")] = web::json::value::string("-0.000325743726599067");
  ccsds_cdm[U("SAT1_CRDOT_N_UNIT")] = web::json::value::string("m**2\\/s");
  ccsds_cdm[U("SAT1_CRDOT_RDOT")] = web::json::value::string("0.00031032143490407");
  ccsds_cdm[U("SAT1_CRDOT_RDOT_UNIT")] = web::json::value::string("m**2\\/s**2");
  ccsds_cdm[U("SAT1_CTDOT_R")] = web::json::value::string("-0.0501856442302755");
  ccsds_cdm[U("SAT1_CTDOT_R_UNIT")] = web::json::value::string("m**2\\/s");
  ccsds_cdm[U("SAT1_CTDOT_T")] = web::json::value::string("-0.0471861244930156");
  ccsds_cdm[U("SAT1_CTDOT_T_UNIT")] = web::json::value::string("m**2\\/s");
  ccsds_cdm[U("SAT1_CTDOT_N")] = web::json::value::string("-0.00252782542649572");
  ccsds_cdm[U("SAT1_CTDOT_N_UNIT")] = web::json::value::string("m**2\\/s");
  ccsds_cdm[U("SAT1_CTDOT_RDOT")] = web::json::value::string("6.07418730203252e-05");
  ccsds_cdm[U("SAT1_CTDOT_RDOT_UNIT")] = web::json::value::string("m**2\\/s**2");
  ccsds_cdm[U("SAT1_CTDOT_TDOT")] = web::json::value::string("5.45874543922052e-05");
  ccsds_cdm[U("SAT1_CTDOT_TDOT_UNIT")] = web::json::value::string("m**2\\/s**2");
  ccsds_cdm[U("SAT1_CNDOT_R")] = web::json::value::string("-0.00751381926357464");
  ccsds_cdm[U("SAT1_CNDOT_R_UNIT")] = web::json::value::string("m**2\\/s");
  ccsds_cdm[U("SAT1_CNDOT_T")] = web::json::value::string("-0.00371414808055227");
  ccsds_cdm[U("SAT1_CNDOT_T_UNIT")] = web::json::value::string("m**2\\/s");
  ccsds_cdm[U("SAT1_CNDOT_N")] = web::json::value::string("-0.0113588008096445");
  ccsds_cdm[U("SAT1_CNDOT_N_UNIT")] = web::json::value::string("m**2\\/s");
  ccsds_cdm[U("SAT1_CNDOT_RDOT")] = web::json::value::string("1.09417486735304e-05");
  ccsds_cdm[U("SAT1_CNDOT_RDOT_UNIT")] = web::json::value::string("m**2\\/s**2");
  ccsds_cdm[U("SAT1_CNDOT_TDOT")] = web::json::value::string("8.17704363245973e-06");
  ccsds_cdm[U("SAT1_CNDOT_TDOT_UNIT")] = web::json::value::string("m**2\\/s**2");
  ccsds_cdm[U("SAT1_CNDOT_NDOT")] = web::json::value::string("1.74453648788543e-05");
  ccsds_cdm[U("SAT1_CNDOT_NDOT_UNIT")] = web::json::value::string("m**2\\/s**2");
  ccsds_cdm[U("SAT1_CDRG_R")] = web::json::value::string("0");
  ccsds_cdm[U("SAT1_CDRG_R_UNIT")] = web::json::value::string("m**3\\/kg");
  ccsds_cdm[U("SAT1_CDRG_T")] = web::json::value::string("0");
  ccsds_cdm[U("SAT1_CDRG_T_UNIT")] = web::json::value::string("m**3\\/kg");
  ccsds_cdm[U("SAT1_CDRG_N")] = web::json::value::string("0");
  ccsds_cdm[U("SAT1_CDRG_N_UNIT")] = web::json::value::string("m**3\\/kg");
  ccsds_cdm[U("SAT1_CDRG_RDOT")] = web::json::value::string("0");
  ccsds_cdm[U("SAT1_CDRG_RDOT_UNIT")] = web::json::value::string("m**3\\/(kg*s)");
  ccsds_cdm[U("SAT1_CDRG_TDOT")] = web::json::value::string("0");
  ccsds_cdm[U("SAT1_CDRG_TDOT_UNIT")] = web::json::value::string("m**3\\/(kg*s)");
  ccsds_cdm[U("SAT1_CDRG_NDOT")] = web::json::value::string("0");
  ccsds_cdm[U("SAT1_CDRG_NDOT_UNIT")] = web::json::value::string("m**3\\/(kg*s)");
  ccsds_cdm[U("SAT1_CDRG_DRG")] = web::json::value::string("0");
  ccsds_cdm[U("SAT1_CDRG_DRG_UNIT")] = web::json::value::string("m**4\\/kg**2");
  ccsds_cdm[U("SAT1_CSRP_R")] = web::json::value::string("0");
  ccsds_cdm[U("SAT1_CSRP_R_UNIT")] = web::json::value::string("m**3\\/kg");
  ccsds_cdm[U("SAT1_CSRP_T")] = web::json::value::string("0");
  ccsds_cdm[U("SAT1_CSRP_T_UNIT")] = web::json::value::string("m**3\\/kg");
  ccsds_cdm[U("SAT1_CSRP_N")] = web::json::value::string("0");
  ccsds_cdm[U("SAT1_CSRP_N_UNIT")] = web::json::value::string("m**3\\/kg");
  ccsds_cdm[U("SAT1_CSRP_RDOT")] = web::json::value::string("0");
  ccsds_cdm[U("SAT1_CSRP_RDOT_UNIT")] = web::json::value::string("m**3\\/(kg*s)");
  ccsds_cdm[U("SAT1_CSRP_TDOT")] = web::json::value::string("0");
  ccsds_cdm[U("SAT1_CSRP_TDOT_UNIT")] = web::json::value::string("m**3\\/(kg*s)");
  ccsds_cdm[U("SAT1_CSRP_NDOT")] = web::json::value::string("0");
  ccsds_cdm[U("SAT1_CSRP_NDOT_UNIT")] = web::json::value::string("m**3\\/(kg*s)");
  ccsds_cdm[U("SAT1_CSRP_DRG")] = web::json::value::string("0");
  ccsds_cdm[U("SAT1_CSRP_DRG_UNIT")] = web::json::value::string("m**4\\/kg**2");
  ccsds_cdm[U("SAT1_CSRP_SRP")] = web::json::value::string("0");
  ccsds_cdm[U("SAT1_CSRP_SRP_UNIT")] = web::json::value::string("m**4\\/kg**2");
  ccsds_cdm[U("SAT2_OBJECT")] = web::json::value::string("OBJECT2");
  ccsds_cdm[U("SAT2_OBJECT_DESIGNATOR")] = web::json::value::string("43782");
  ccsds_cdm[U("SAT2_CATALOG_NAME")] = web::json::value::string("SATCAT");
  ccsds_cdm[U("SAT2_OBJECT_NAME")] = web::json::value::string("OBJECT AA");
  ccsds_cdm[U("SAT2_INTERNATIONAL_DESIGNATOR")] = web::json::value::string("2018-099AA");
  ccsds_cdm[U("SAT2_OBJECT_TYPE")] = web::json::value::string("TBA");
  ccsds_cdm[U("SAT2_OPERATOR_CONTACT_POSITION")] = web::json::value::string("https:\\/\\/www.space-track.org\\/expandedspacedata\\/query\\/class\\/organization\\/object\\/~~43782\\/orderby\\/ORG_NAME,INFO_ID\\/format\\/html\\/emptyresult\\/show\\/");
  ccsds_cdm[U("SAT2_OPERATOR_ORGANIZATION")] = web::json::value::string("NONE");
  ccsds_cdm[U("SAT2_OPERATOR_PHONE")] = web::json::value::string("https:\\/\\/www.space-track.org\\/expandedspacedata\\/query\\/class\\/organization\\/object\\/~~43782\\/orderby\\/ORG_NAME,INFO_ID\\/format\\/html\\/emptyresult\\/show\\/");
  ccsds_cdm[U("SAT2_OPERATOR_EMAIL")] = web::json::value::string("https:\\/\\/www.space-track.org\\/expandedspacedata\\/query\\/class\\/organization\\/object\\/~~43782\\/orderby\\/ORG_NAME,INFO_ID\\/format\\/html\\/emptyresult\\/show\\/");
  ccsds_cdm[U("SAT2_EPHEMERIS_NAME")] = web::json::value::string("NONE");
  ccsds_cdm[U("SAT2_COVARIANCE_METHOD")] = web::json::value::string("CALCULATED");
  ccsds_cdm[U("SAT2_MANEUVERABLE")] = web::json::value::string("N\\/A");
  ccsds_cdm[U("SAT2_REF_FRAME")] = web::json::value::string("ITRF");
  ccsds_cdm[U("SAT2_GRAVITY_MODEL")] = web::json::value::string("EGM-96: 36D 36O");
  ccsds_cdm[U("SAT2_ATMOSPHERIC_MODEL")] = web::json::value::string("JBH09");
  ccsds_cdm[U("SAT2_N_BODY_PERTURBATIONS")] = web::json::value::string("MOON,SUN");
  ccsds_cdm[U("SAT2_SOLAR_RAD_PRESSURE")] = web::json::value::string("YES");
  ccsds_cdm[U("SAT2_EARTH_TIDES")] = web::json::value::string("YES");
  ccsds_cdm[U("SAT2_INTRACK_THRUST")] = web::json::value::string("NO");
  ccsds_cdm[U("SAT2_TIME_LASTOB_START")] = web::json::value::string("2019-03-16 07:38:19");
  ccsds_cdm[U("SAT2_TIME_LASTOB_START_FRACTION")] = web::json::value::string("0");
  ccsds_cdm[U("SAT2_TIME_LASTOB_END")] = web::json::value::string("2019-03-17 07:38:19");
  ccsds_cdm[U("SAT2_TIME_LASTOB_END_FRACTION")] = web::json::value::string("0");
  ccsds_cdm[U("SAT2_RECOMMENDED_OD_SPAN")] = web::json::value::string("9.43");
  ccsds_cdm[U("SAT2_RECOMMENDED_OD_SPAN_UNIT")] = web::json::value::string("d");
  ccsds_cdm[U("SAT2_ACTUAL_OD_SPAN")] = web::json::value::string("9.43");
  ccsds_cdm[U("SAT2_ACTUAL_OD_SPAN_UNIT")] = web::json::value::string("d");
  ccsds_cdm[U("SAT2_OBS_AVAILABLE")] = web::json::value::string("140");
  ccsds_cdm[U("SAT2_OBS_USED")] = web::json::value::string("138");
  ccsds_cdm[U("SAT2_RESIDUALS_ACCEPTED")] = web::json::value::string("99.4");
  ccsds_cdm[U("SAT2_RESIDUALS_ACCEPTED_UNIT")] = web::json::value::string("%");
  ccsds_cdm[U("SAT2_WEIGHTED_RMS")] = web::json::value::string("1.317");
  ccsds_cdm[U("SAT2_COMMENT_APOGEE")] = web::json::value::string("Apogee Altitude = 601   [km]");
  ccsds_cdm[U("SAT2_COMMENT_PERIGEE")] = web::json::value::string("Perigee Altitude = 596   [km]");
  ccsds_cdm[U("SAT2_COMMENT_INCLINATION")] = web::json::value::string("Inclination = 97.8  [deg]");
  ccsds_cdm[U("SAT2_AREA_PC")] = web::json::value::string("0.059");
  ccsds_cdm[U("SAT2_AREA_PC_UNIT")] = web::json::value::string("m**2");
  ccsds_cdm[U("SAT2_CD_AREA_OVER_MASS")] = web::json::value::string("0.0281873");
  ccsds_cdm[U("SAT2_CD_AREA_OVER_MASS_UNIT")] = web::json::value::string("m**2\\/kg");
  ccsds_cdm[U("SAT2_CR_AREA_OVER_MASS")] = web::json::value::string("0.01374585");
  ccsds_cdm[U("SAT2_CR_AREA_OVER_MASS_UNIT")] = web::json::value::string("m**2\\/kg");
  ccsds_cdm[U("SAT2_THRUST_ACCELERATION")] = web::json::value::string("0");
  ccsds_cdm[U("SAT2_THRUST_ACCELERATION_UNIT")] = web::json::value::string("m\\/s**2");
  ccsds_cdm[U("SAT2_SEDR")] = web::json::value::string("8.22796e-05");
  ccsds_cdm[U("SAT2_SEDR_UNIT")] = web::json::value::string("W\\/kg");
  ccsds_cdm[U("SAT2_X")] = web::json::value::string("6562.413913");
  ccsds_cdm[U("SAT2_X_UNIT")] = web::json::value::string("km");
  ccsds_cdm[U("SAT2_Y")] = web::json::value::string("1702.252966");
  ccsds_cdm[U("SAT2_Y_UNIT")] = web::json::value::string("km");
  ccsds_cdm[U("SAT2_Z")] = web::json::value::string("1592.063015");
  ccsds_cdm[U("SAT2_Z_UNIT")] = web::json::value::string("km");
  ccsds_cdm[U("SAT2_X_DOT")] = web::json::value::string("2.046814905");
  ccsds_cdm[U("SAT2_X_DOT_UNIT")] = web::json::value::string("km\\/s");
  ccsds_cdm[U("SAT2_Y_DOT")] = web::json::value::string("-1.062667288");
  ccsds_cdm[U("SAT2_Y_DOT_UNIT")] = web::json::value::string("km\\/s");
  ccsds_cdm[U("SAT2_Z_DOT")] = web::json::value::string("-7.2948262");
  ccsds_cdm[U("SAT2_Z_DOT_UNIT")] = web::json::value::string("km\\/s");
  ccsds_cdm[U("SAT2_CR_R")] = web::json::value::string("99.9911568880684");
  ccsds_cdm[U("SAT2_CR_R_UNIT")] = web::json::value::string("m**2");
  ccsds_cdm[U("SAT2_CT_R")] = web::json::value::string("-85.5596052003614");
  ccsds_cdm[U("SAT2_CT_R_UNIT")] = web::json::value::string("m**2");
  ccsds_cdm[U("SAT2_CT_T")] = web::json::value::string("679.619604230875");
  ccsds_cdm[U("SAT2_CT_T_UNIT")] = web::json::value::string("m**2");
  ccsds_cdm[U("SAT2_CN_R")] = web::json::value::string("27.0614673334493");
  ccsds_cdm[U("SAT2_CN_R_UNIT")] = web::json::value::string("m**2");
  ccsds_cdm[U("SAT2_CN_T")] = web::json::value::string("-16.6568273265456");
  ccsds_cdm[U("SAT2_CN_T_UNIT")] = web::json::value::string("m**2");
  ccsds_cdm[U("SAT2_CN_N")] = web::json::value::string("63.3048647153926");
  ccsds_cdm[U("SAT2_CN_N_UNIT")] = web::json::value::string("m**2");
  ccsds_cdm[U("SAT2_CRDOT_R")] = web::json::value::string("0.107056879196751");
  ccsds_cdm[U("SAT2_CRDOT_R_UNIT")] = web::json::value::string("m**2\\/s");
  ccsds_cdm[U("SAT2_CRDOT_T")] = web::json::value::string("-0.659964067622497");
  ccsds_cdm[U("SAT2_CRDOT_T_UNIT")] = web::json::value::string("m**2\\/s");
  ccsds_cdm[U("SAT2_CRDOT_N")] = web::json::value::string("0.019485753052086");
  ccsds_cdm[U("SAT2_CRDOT_N_UNIT")] = web::json::value::string("m**2\\/s");
  ccsds_cdm[U("SAT2_CRDOT_RDOT")] = web::json::value::string("0.000708585747151425");
  ccsds_cdm[U("SAT2_CRDOT_RDOT_UNIT")] = web::json::value::string("m**2\\/s**2");
  ccsds_cdm[U("SAT2_CTDOT_R")] = web::json::value::string("-0.108846482024542");
  ccsds_cdm[U("SAT2_CTDOT_R_UNIT")] = web::json::value::string("m**2\\/s");
  ccsds_cdm[U("SAT2_CTDOT_T")] = web::json::value::string("0.0926867959261093");
  ccsds_cdm[U("SAT2_CTDOT_T_UNIT")] = web::json::value::string("m**2\\/s");
  ccsds_cdm[U("SAT2_CTDOT_N")] = web::json::value::string("-0.0294861841043052");
  ccsds_cdm[U("SAT2_CTDOT_N_UNIT")] = web::json::value::string("m**2\\/s");
  ccsds_cdm[U("SAT2_CTDOT_RDOT")] = web::json::value::string("-0.000115870505266814");
  ccsds_cdm[U("SAT2_CTDOT_RDOT_UNIT")] = web::json::value::string("m**2\\/s**2");
  ccsds_cdm[U("SAT2_CTDOT_TDOT")] = web::json::value::string("0.000118496023294848");
  ccsds_cdm[U("SAT2_CTDOT_TDOT_UNIT")] = web::json::value::string("m**2\\/s**2");
  ccsds_cdm[U("SAT2_CNDOT_R")] = web::json::value::string("-0.0141976572926528");
  ccsds_cdm[U("SAT2_CNDOT_R_UNIT")] = web::json::value::string("m**2\\/s");
  ccsds_cdm[U("SAT2_CNDOT_T")] = web::json::value::string("0.0382956717994341");
  ccsds_cdm[U("SAT2_CNDOT_T_UNIT")] = web::json::value::string("m**2\\/s");
  ccsds_cdm[U("SAT2_CNDOT_N")] = web::json::value::string("-0.00809689080893274");
  ccsds_cdm[U("SAT2_CNDOT_N_UNIT")] = web::json::value::string("m**2\\/s");
  ccsds_cdm[U("SAT2_CNDOT_RDOT")] = web::json::value::string("-2.78052024499934e-05");
  ccsds_cdm[U("SAT2_CNDOT_RDOT_UNIT")] = web::json::value::string("m**2\\/s**2");
  ccsds_cdm[U("SAT2_CNDOT_TDOT")] = web::json::value::string("1.54543876352614e-05");
  ccsds_cdm[U("SAT2_CNDOT_TDOT_UNIT")] = web::json::value::string("m**2\\/s**2");
  ccsds_cdm[U("SAT2_CNDOT_NDOT")] = web::json::value::string("2.37648026501431e-05");
  ccsds_cdm[U("SAT2_CNDOT_NDOT_UNIT")] = web::json::value::string("m**2\\/s**2");
  ccsds_cdm[U("SAT2_CDRG_R")] = web::json::value::string("0");
  ccsds_cdm[U("SAT2_CDRG_R_UNIT")] = web::json::value::string("m**3\\/kg");
  ccsds_cdm[U("SAT2_CDRG_T")] = web::json::value::string("0");
  ccsds_cdm[U("SAT2_CDRG_T_UNIT")] = web::json::value::string("m**3\\/kg");
  ccsds_cdm[U("SAT2_CDRG_N")] = web::json::value::string("0");
  ccsds_cdm[U("SAT2_CDRG_N_UNIT")] = web::json::value::string("m**3\\/kg");
  ccsds_cdm[U("SAT2_CDRG_RDOT")] = web::json::value::string("0");
  ccsds_cdm[U("SAT2_CDRG_RDOT_UNIT")] = web::json::value::string("m**3\\/(kg*s)");
  ccsds_cdm[U("SAT2_CDRG_TDOT")] = web::json::value::string("0");
  ccsds_cdm[U("SAT2_CDRG_TDOT_UNIT")] = web::json::value::string("m**3\\/(kg*s)");
  ccsds_cdm[U("SAT2_CDRG_NDOT")] = web::json::value::string("0");
  ccsds_cdm[U("SAT2_CDRG_NDOT_UNIT")] = web::json::value::string("m**3\\/(kg*s)");
  ccsds_cdm[U("SAT2_CDRG_DRG")] = web::json::value::string("0");
  ccsds_cdm[U("SAT2_CDRG_DRG_UNIT")] = web::json::value::string("m**4\\/kg**2");
  ccsds_cdm[U("SAT2_CSRP_R")] = web::json::value::string("0");
  ccsds_cdm[U("SAT2_CSRP_R_UNIT")] = web::json::value::string("m**3\\/kg");
  ccsds_cdm[U("SAT2_CSRP_T")] = web::json::value::string("0");
  ccsds_cdm[U("SAT2_CSRP_T_UNIT")] = web::json::value::string("m**3\\/kg");
  ccsds_cdm[U("SAT2_CSRP_N")] = web::json::value::string("0");
  ccsds_cdm[U("SAT2_CSRP_N_UNIT")] = web::json::value::string("m**3\\/kg");
  ccsds_cdm[U("SAT2_CSRP_RDOT")] = web::json::value::string("0");
  ccsds_cdm[U("SAT2_CSRP_RDOT_UNIT")] = web::json::value::string("m**3\\/(kg*s)");
  ccsds_cdm[U("SAT2_CSRP_TDOT")] = web::json::value::string("0");
  ccsds_cdm[U("SAT2_CSRP_TDOT_UNIT")] = web::json::value::string("m**3\\/(kg*s)");
  ccsds_cdm[U("SAT2_CSRP_NDOT")] = web::json::value::string("0");
  ccsds_cdm[U("SAT2_CSRP_NDOT_UNIT")] = web::json::value::string("m**3\\/(kg*s)");
  ccsds_cdm[U("SAT2_CSRP_DRG")] = web::json::value::string("0");
  ccsds_cdm[U("SAT2_CSRP_DRG_UNIT")] = web::json::value::string("m**4\\/kg**2");
  ccsds_cdm[U("SAT2_CSRP_SRP")] = web::json::value::string("0");
  ccsds_cdm[U("SAT2_CSRP_SRP_UNIT")] = web::json::value::string("m**4\\/kg**2");
  ccsds_cdm[U("GID")] = web::json::value::string("79");
  
  riskBody[U("CCSDS_CDM")] = ccsds_cdm;
  
  //riskBody[U("overrides")];
  
  // send request for risk estimation
  string riskEstimationUrlRequest = "/estimate-risk/all-methods/requests";
  http_client okapiRequestRiskEstimation(baseUrl + riskEstimationUrlRequest);
  http_request requestRiskEstimation(methods::POST);
  requestRiskEstimation.set_body(riskBody);
  requestRiskEstimation.headers().add(U("access_token"), connector.accessToken);
  OkapiConnector::completeResult riskEstimationRequest = connector.sendRequest(okapiRequestRiskEstimation, requestRiskEstimation);
  if (riskEstimationRequest.error.code == 200 || riskEstimationRequest.error.code == 202)
  {
    cout << "Send risk estimation request completed" << endl;
  }
  else {
    cout << "Risk estimation request failed with status: " << riskEstimationRequest.error.status << endl;
    cout << riskEstimationRequest.error.message << endl;
  }
  string requestIdRiskEstimation = connector.requestId;
  cout << "Risk estimation request ID: " << requestIdRiskEstimation << endl;

  // get results for risk estimation
  string riskEstimationUrlGet = "/estimate-risk/all-methods/simple/results/";
  http_client riskEstimationUrl(baseUrl + riskEstimationUrlGet + requestIdRiskEstimation);
  http_request riskEstimationMethod(methods::GET);
  riskEstimationMethod.headers().add(U("access_token"), connector.accessToken);
  OkapiConnector::completeResult riskEstimationResult = retrieveResult(connector, riskEstimationUrl, riskEstimationMethod);
  if (riskEstimationResult.error.code != 200 && riskEstimationResult.error.code != 202)
  {
    cout << "Retrieving risk estimation response failed with status: " << riskEstimationResult.error.status << endl;
    cout << riskEstimationResult.error.message << endl;
  }
  if (riskEstimationResult.error.code == 200) {
    cout << riskEstimationResult.body.serialize() << endl;
  }
  else {
    cout << "Risk estimation request failed with status: " << riskEstimationResult.error.status << endl;
    cout << riskEstimationResult.error.message << endl;
  }

}

int main(int argc, char* argv[])
{
	// initializing communication
	OkapiConnector connector;
 
  // User input for authentication
  // Here you add your username (should be an e-mail address):
  string username = <username>;
  // Here you add your password:
  string password = <password>;
  // Correct URL and port for the v2019.11 release
  string baseUrl = "http://okapi.ddns.net:34569";

	// Authentication with Auth0 to retrieve the access token
  cout << "[Authentication] - started" << endl;
	OkapiConnector::completeResult initResult
      = connector.init(methods::POST,username,password);
  
  if (initResult.error.code == 200 || initResult.error.code == 202)
  {
    cout << "[Authentication] - completed" << endl;
  }
  else
  {
    cout << "[Authentication] - failed with status: " << initResult.error.status << endl;
    cout << initResult.error.message << endl;
    return -1;
  }

  // PASS PREDICTION
  cout << "[Predict passes] - started" << endl;
  predictPassesTests(connector, baseUrl);
  cout << "[Predict passes] - completed" << endl;


  // NEPTUNE propagation
  cout << "[Propagate orbit NEPTUNE] - started" << endl;
  neptuneTest(connector, baseUrl);
  cout << "[Propagate orbit NEPTUNE] - completed" << endl;

  // NEPTUNE co-variance propagation
  cout << "[Propagate OPM Co-variance NEPTUNE] - started" << endl;
  neptuneOpmCovarianceTest(connector, baseUrl);
  cout << "[Propagate OPM Co-variance NEPTUNE] - completed" << endl;
  
  // NEPTUNE maneuvre propagation enabled
  cout << "[Propagate OPM Maneuvre NEPTUNE] - started" << endl;
  neptuneOpmManeuvreTest(connector, baseUrl);
  cout << "[Propagate OPM Maneuvre NEPTUNE] - completed" << endl;

  // Orekit propagation
  cout << "[Propagate orbit Orekit-numerical] - started" << endl;
  orekitNumericalTest(connector, baseUrl);
  cout << "[Propagate orbit Orekit-numerical] - completed" << endl;


  // SGP4 propagation
  cout << "[Propagate orbit SGP4] - started" << endl;
  sgp4Test(connector, baseUrl);
  cout << "[Propagate orbit SGP4] - completed" << endl;


  // Risk Estimation
  cout << "[Estimate risk all methods] - started" << endl;
  riskEstimationTest(connector, baseUrl);
  cout << "[Estimate risk all methods] - completed" << endl;

  return 0;
}
