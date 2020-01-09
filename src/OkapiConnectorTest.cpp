#include "OkapiConnector.h"

using std::cout;
using std::endl;
using std::string;

using namespace utility;
using namespace web;
using namespace web::http;
using namespace web::http::client;

/**
 * This is a little helper function. it retrieves the result from the backend. Polls for the result until it is ready.
 */
OkapiConnector::CompleteResult retrieveResult(OkapiConnector connector, string baseUrl, string endpoint, string requestId)
{
  // First call to the backend
  OkapiConnector::CompleteResult result = connector.getResult(baseUrl, endpoint, requestId);
  if (result.error.code != 200 && result.error.code != 202)
  {
    cout << "Retrieving response failed with status: " << result.error.status << endl;
    cout << result.error.message << endl;
    return result;
  }
  int i = 0;
  // Poll the backend until the result is ready and can be retrieved
  while (result.error.code == 202) {
    result = connector.getResult(baseUrl, endpoint, requestId);
    cout << "The request was successful. Your result is not ready yet.  Waiting: " << i << " s." << endl;
    i++;
    sleep(1);
  }
  // Final call to the backend
  return connector.getResult(baseUrl, endpoint, requestId);
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
  
  // Send request for SGP4 pass prediction
  OkapiConnector::CompleteResult responseSGP4 = connector.sendRequest(baseUrl, "/predict-passes/sgp4/requests", passPredictionRequestBody);
  if (responseSGP4.error.code != 200 && responseSGP4.error.code != 202)
  {
    cout << "SGP4 request failed with status: " << responseSGP4.error.status << endl;
    cout << responseSGP4.error.message << endl;
    return;
  }
  string requestIdPassPredictionSgp4 = connector.requestId;
  cout << "SGP4 request ID: " << requestIdPassPredictionSgp4 << endl;
  
  // Get results for SGP4 and print them in the terminal
  OkapiConnector::CompleteResult sgp4SimpleResult = retrieveResult(connector, baseUrl, "/predict-passes/sgp4/simple/results/", requestIdPassPredictionSgp4);
  if (sgp4SimpleResult.error.code != 200 && sgp4SimpleResult.error.code != 202)
  {
    cout << "Response failed with status: " << sgp4SimpleResult.error.status << endl;
    cout << sgp4SimpleResult.error.message << endl;
  }
  else
  {
    cout << sgp4SimpleResult.body.serialize() << endl;
  }

  OkapiConnector::CompleteResult sgp4SummaryResult = retrieveResult(connector, baseUrl, "/predict-passes/sgp4/summary/results/", requestIdPassPredictionSgp4);
  if (sgp4SummaryResult.error.code != 200 && sgp4SummaryResult.error.code != 202)
  {
    cout << "Response failed with status: " << sgp4SummaryResult.error.status << endl;
    cout << sgp4SummaryResult.error.message << endl;
  }
  else
  {
    cout << sgp4SummaryResult.body.serialize() << endl;
  }
  
//  // Send request for NEPTUNE pass prediction
//  OkapiConnector::CompleteResult responseNeptune = connector.sendRequest(baseUrl, "/predict-passes/neptune/requests", passPredictionNumericalRequestBody);
//  // Check response
//  if (responseNeptune.error.code != 200 && responseNeptune.error.code != 202)
//  {
//    cout << "Neptune request failed with status: " << responseNeptune.error.status << endl;
//    cout << responseNeptune.error.message << endl;
//  }
//  string requestIdPassPredictionNeptune = connector.requestId;
//  cout << "Neptune request ID: " << requestIdPassPredictionNeptune << endl;
//
//
//  // Get results for NEPTUNE and print them in the terminal
//  OkapiConnector::CompleteResult neptuneSimpleResult = retrieveResult(connector, baseUrl, "/predict-passes/neptune/simple/results/", requestIdPassPredictionNeptune);
//  if (neptuneSimpleResult.error.code != 200 && neptuneSimpleResult.error.code != 202)
//  {
//    cout << "Response failed with status: " << neptuneSimpleResult.error.status << endl;
//    cout << neptuneSimpleResult.error.message << endl;
//  }
//  else
//  {
//    cout << neptuneSimpleResult.body.serialize() << endl;
//  }
//
//  OkapiConnector::CompleteResult neptuneSummaryResult = retrieveResult(connector, baseUrl, "/predict-passes/neptune/summary/results/", requestIdPassPredictionNeptune);
//  if (neptuneSummaryResult.error.code != 200 && neptuneSummaryResult.error.code != 202)
//  {
//    cout << "Response failed with status: " << neptuneSummaryResult.error.status << endl;
//    cout << neptuneSummaryResult.error.message << endl;
//  }
//  else
//  {
//    cout << neptuneSummaryResult.body.serialize() << endl;
//  }
  
  
  // Send request for OREKIT pass prediction
  OkapiConnector::CompleteResult responseOrekit = connector.sendRequest(baseUrl, "/predict-passes/orekit-numerical/requests", passPredictionNumericalRequestBody);
  // Check response
  if (responseOrekit.error.code != 200 && responseOrekit.error.code != 202)
  {
    cout << "Orekit request failed with status: " << responseOrekit.error.status << endl;
    cout << responseOrekit.error.message << endl;
  }
  string requestIdPassPredictionOrekit = connector.requestId;
  cout << "Orekit request ID: " << requestIdPassPredictionOrekit << endl;
  
  // Get results for OREKIT and print them in the terminal
  OkapiConnector::CompleteResult orekitSimpleResult = retrieveResult(connector, baseUrl, "/predict-passes/orekit-numerical/simple/results/", requestIdPassPredictionOrekit);
  if (orekitSimpleResult.error.code != 200 && orekitSimpleResult.error.code != 202)
  {
    cout << "Response failed with status: " << orekitSimpleResult.error.status << endl;
    cout << orekitSimpleResult.error.message << endl;
  }
  else
  {
    cout << orekitSimpleResult.body.serialize() << endl;
  }
  
  OkapiConnector::CompleteResult orekitSummaryResult = retrieveResult(connector, baseUrl, "/predict-passes/orekit-numerical/summary/results/", requestIdPassPredictionOrekit);
  if (orekitSummaryResult.error.code != 200 && orekitSummaryResult.error.code != 202)
  {
    cout << "Response failed with status: " << orekitSummaryResult.error.status << endl;
    cout << orekitSummaryResult.error.message << endl;
  }
  else
  {
    cout << orekitSummaryResult.body.serialize() << endl;
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

  // Send request for NEPTUNE propagation
  OkapiConnector::CompleteResult neptuneRequest = connector.sendRequest(baseUrl, "/propagate-orbit/neptune/requests", propagateNeptuneSimpleRequestBody);
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

  // Get results in simple format for NEPTUNE and print them in the terminal
  OkapiConnector::CompleteResult neptuneSummaryResult = retrieveResult(connector, baseUrl, "/propagate-orbit/neptune/simple/results/", requestIdNeptune);
  if (neptuneSummaryResult.error.code != 200 && neptuneSummaryResult.error.code != 202)
  {
    cout << "Response failed with status: " << neptuneSummaryResult.error.status << endl;
    cout << neptuneSummaryResult.error.message << endl;
  }
  else
  {
    cout << neptuneSummaryResult.body.serialize() << endl;
  }
  
  // Get results in generic format for NEPTUNE and print them in the terminal
  OkapiConnector::CompleteResult neptuneGenericResult = retrieveResult(connector, baseUrl, "/propagate-orbit/neptune/simple/results/", requestIdNeptune + "/generic");
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

  // Send request for NEPTUNE propagation
  OkapiConnector::CompleteResult neptuneRequest = connector.sendRequest(baseUrl, "/propagate-orbit/neptune/requests", propagateNeptuneSimpleRequestBody);
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

  // Get results in OPM format for NEPTUNE and print them in the terminal
  OkapiConnector::CompleteResult neptuneResult = retrieveResult(connector, baseUrl, "/propagate-orbit/neptune/opm/results/", requestIdNeptune);
  if (neptuneResult.error.code != 200 && neptuneResult.error.code != 202)
  {
    cout << "Response failed with status: " << neptuneResult.error.status << endl;
    cout << neptuneResult.error.message << endl;
  }
  else
  {
    cout << neptuneResult.body.serialize() << endl;
  }
  
  // Get results in OPM format for NEPTUNE and print them in the terminal
  OkapiConnector::CompleteResult neptuneGenericResult = retrieveResult(connector, baseUrl, "/propagate-orbit/neptune/opm/results/", requestIdNeptune + "/generic");
  if (neptuneGenericResult.error.code != 200 && neptuneGenericResult.error.code != 202)
  {
    cout << "Response failed with status: " << neptuneGenericResult.error.status << endl;
    cout << neptuneGenericResult.error.message << endl;
  }
  else
  {
    cout << neptuneGenericResult.body.serialize() << endl;
  }
}

/**
 * Tests the NEPTUNE OPM endpoints with enabled maneuvre propagation
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

  // Send request for NEPTUNE propagation
  OkapiConnector::CompleteResult neptuneRequest = connector.sendRequest(baseUrl, "/propagate-orbit/neptune/requests", propagateNeptuneSimpleRequestBody);
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

  // Get results in OPM format for NEPTUNE and print them in the terminal
  OkapiConnector::CompleteResult neptuneResult = retrieveResult(connector, baseUrl, "/propagate-orbit/neptune/opm/results/", requestIdNeptune);
  if (neptuneResult.error.code != 200 && neptuneResult.error.code != 202)
  {
    cout << "Response failed with status: " << neptuneResult.error.status << endl;
    cout << neptuneResult.error.message << endl;
  }
  else
  {
    cout << neptuneResult.body.serialize() << endl;
  }
  
  // Get results in OPM format for NEPTUNE and print them in the terminal
  OkapiConnector::CompleteResult neptuneGenericResult = retrieveResult(connector, baseUrl, "/propagate-orbit/neptune/opm/results/", requestIdNeptune + "/generic");
  if (neptuneGenericResult.error.code != 200 && neptuneGenericResult.error.code != 202)
  {
    cout << "Response failed with status: " << neptuneGenericResult.error.status << endl;
    cout << neptuneGenericResult.error.message << endl;
  }
  else
  {
    cout << neptuneGenericResult.body.serialize() << endl;
  }
}

/**
 * Tests the Orekit-numerical endpoints
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


  // Send request for Orekit-numerical propagation
  OkapiConnector::CompleteResult orekitRequest = connector.sendRequest(baseUrl, "/propagate-orbit/orekit-numerical/requests", propagateOrekitSimpleRequestBody);
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

  // Get results in simple format for Orekit-numerical and print them in the terminal
  OkapiConnector::CompleteResult orekitResult = retrieveResult(connector, baseUrl, "/propagate-orbit/orekit-numerical/simple/results/", requestIdOrekit);
  if (orekitResult.error.code != 200 && orekitResult.error.code != 202)
  {
    cout << "Response failed with status: " << orekitResult.error.status << endl;
    cout << orekitResult.error.message << endl;
  }
  else
  {
    cout << orekitResult.body.serialize() << endl;
  }
  
  // Get results in OPM format for Orekit-numerical and print them in the terminal
  OkapiConnector::CompleteResult orekitOpmGenericResult = retrieveResult(connector, baseUrl, "/propagate-orbit/orekit-numerical/opm/results/", requestIdOrekit + "/generic");
  if (orekitOpmGenericResult.error.code != 200 && orekitOpmGenericResult.error.code != 202)
  {
    cout << "Response failed with status: " << orekitOpmGenericResult.error.status << endl;
    cout << orekitOpmGenericResult.error.message << endl;
  }
  else
  {
    cout << orekitOpmGenericResult.body.serialize() << endl;
  }
  
  // Get results in generic simple format for Orekit-numerical and print them in the terminal
  OkapiConnector::CompleteResult orekitGenericResult = retrieveResult(connector, baseUrl, "/propagate-orbit/orekit-numerical/simple/results/", requestIdOrekit + "/generic");
  if (orekitGenericResult.error.code != 200 && orekitGenericResult.error.code != 202)
  {
    cout << "Response failed with status: " << orekitGenericResult.error.status << endl;
    cout << orekitGenericResult.error.message << endl;
  }
  else
  {
    cout << orekitGenericResult.body.serialize() << endl;
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
  
  // Send request for SGP4 propagation
  OkapiConnector::CompleteResult sgp4Request = connector.sendRequest(baseUrl, "/propagate-orbit/sgp4/requests", propagateSgp4RequestBody);
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
  
  // Get results in simple format for SGP4
  OkapiConnector::CompleteResult sgp4Result = retrieveResult(connector, baseUrl, "/propagate-orbit/sgp4/simple/results/", requestIdSgp4);
  if (sgp4Result.error.code != 200 && sgp4Result.error.code != 202)
  {
    cout << "Retrieving SGP4 propagation simple response failed with status: " << sgp4Result.error.status << endl;
    cout << sgp4Result.error.message << endl;
  }
  if (sgp4Result.error.code == 200) {
    cout << sgp4Result.body.serialize() << endl;
  }
  
  // Get results in OMM format for SGP4
  OkapiConnector::CompleteResult sgp4OmmResult = retrieveResult(connector, baseUrl, "/propagate-orbit/sgp4/omm/results/", requestIdSgp4);
  if (sgp4OmmResult.error.code != 200 && sgp4OmmResult.error.code != 202)
  {
    cout << "Retrieving SGP4 propagation OMM response failed with status: " << sgp4OmmResult.error.status << endl;
    cout << sgp4OmmResult.error.message << endl;
  }
  if (sgp4OmmResult.error.code == 200) {
    cout << sgp4OmmResult.body.serialize() << endl;
  }
  
  // Get results in OMM generic format for SGP4
  OkapiConnector::CompleteResult sgp4OmmGenericResult = retrieveResult(connector, baseUrl, "/propagate-orbit/sgp4/omm/results/", requestIdSgp4 + "/generic");
  if (sgp4OmmGenericResult.error.code != 200 && sgp4OmmGenericResult.error.code != 202)
  {
    cout << "Retrieving SGP4 propagation OMM response failed with status: " << sgp4OmmResult.error.status << endl;
    cout << sgp4OmmGenericResult.error.message << endl;
  }
  if (sgp4OmmGenericResult.error.code == 200) {
    cout << sgp4OmmGenericResult.body.serialize() << endl;
  }
  
}

void riskEstimationTest(OkapiConnector connector, string baseUrl)
{

  web::json::value riskBody;
  web::json::value ccsds_cdm;
  web::json::value overrides;
  web::json::value covarianceScalingRange;
  
  ccsds_cdm[U("CONSTELLATION")] = web::json::value::string("OKAPI");
  ccsds_cdm[U("CDM_ID")] = web::json::value::string("24537247");
  ccsds_cdm[U("FILENAME")] = web::json::value::string("1234_conj_5678_202001_1234.xml");
  ccsds_cdm[U("INSERT_EPOCH")] = web::json::value::string("2020-01-20 11:37:59");
  ccsds_cdm[U("CCSDS_CDM_VERS")] = web::json::value::string("1.0");
  ccsds_cdm[U("CREATION_DATE")] = web::json::value::string("2020-01-20 07:38:19");
  ccsds_cdm[U("CREATION_DATE_FRACTION")] = web::json::value::string("0");
  ccsds_cdm[U("ORIGINATOR")] = web::json::value::string("JSPOC");
  ccsds_cdm[U("MESSAGE_FOR")] = web::json::value::string("OKAPISat");
  ccsds_cdm[U("MESSAGE_ID")] = web::json::value::string("1234_conj_5678_202001_1234");
  ccsds_cdm[U("TCA")] = web::json::value::string("2020-01-20 09:28:20");
  ccsds_cdm[U("TCA_FRACTION")] = web::json::value::string("23");
  ccsds_cdm[U("MISS_DISTANCE")] = web::json::value::string("800");
  ccsds_cdm[U("MISS_DISTANCE_UNIT")] = web::json::value::string("m");
  ccsds_cdm[U("RELATIVE_SPEED")] = web::json::value::string("14990");
  ccsds_cdm[U("RELATIVE_SPEED_UNIT")] = web::json::value::string("m\\/s");
  ccsds_cdm[U("RELATIVE_POSITION_R")] = web::json::value::string("-71.0");
  ccsds_cdm[U("RELATIVE_POSITION_R_UNIT")] = web::json::value::string("m");
  ccsds_cdm[U("RELATIVE_POSITION_T")] = web::json::value::string("110.0");
  ccsds_cdm[U("RELATIVE_POSITION_T_UNIT")] = web::json::value::string("m");
  ccsds_cdm[U("RELATIVE_POSITION_N")] = web::json::value::string("793.0");
  ccsds_cdm[U("RELATIVE_POSITION_N_UNIT")] = web::json::value::string("m");
  ccsds_cdm[U("RELATIVE_VELOCITY_R")] = web::json::value::string("11.0");
  ccsds_cdm[U("RELATIVE_VELOCITY_R_UNIT")] = web::json::value::string("m\\/s");
  ccsds_cdm[U("RELATIVE_VELOCITY_T")] = web::json::value::string("-14850");
  ccsds_cdm[U("RELATIVE_VELOCITY_T_UNIT")] = web::json::value::string("m\\/s");
  ccsds_cdm[U("RELATIVE_VELOCITY_N")] = web::json::value::string("2053.0");
  ccsds_cdm[U("RELATIVE_VELOCITY_N_UNIT")] = web::json::value::string("m\\/s");
  ccsds_cdm[U("COLLISION_PROBABILITY")] = web::json::value::string("0");
  ccsds_cdm[U("COLLISION_PROBABILITY_METHOD")] = web::json::value::string("FOSTER-1992");
  ccsds_cdm[U("SAT1_OBJECT")] = web::json::value::string("OBJECT1");
  ccsds_cdm[U("SAT1_OBJECT_DESIGNATOR")] = web::json::value::string("1234");
  ccsds_cdm[U("SAT1_CATALOG_NAME")] = web::json::value::string("SATCAT");
  ccsds_cdm[U("SAT1_OBJECT_NAME")] = web::json::value::string("OKAPISat");
  ccsds_cdm[U("SAT1_INTERNATIONAL_DESIGNATOR")] = web::json::value::string("2021-001E");
  ccsds_cdm[U("SAT1_OBJECT_TYPE")] = web::json::value::string("PAYLOAD");
  ccsds_cdm[U("SAT1_OPERATOR_CONTACT_POSITION")] = web::json::value::string("https:\\/\\/www.space-track.org\\/expandedspacedata\\/query\\/class\\/organization\\/object\\/~~1234\\/orderby\\/ORG_NAME,INFO_ID\\/format\\/html\\/emptyresult\\/show\\/");
  ccsds_cdm[U("SAT1_OPERATOR_ORGANIZATION")] = web::json::value::string("OKAPI");
  ccsds_cdm[U("SAT1_OPERATOR_PHONE")] = web::json::value::string("https:\\/\\/www.space-track.org\\/expandedspacedata\\/query\\/class\\/organization\\/object\\/~~1234\\/orderby\\/ORG_NAME,INFO_ID\\/format\\/html\\/emptyresult\\/show\\/");
  ccsds_cdm[U("SAT1_OPERATOR_EMAIL")] = web::json::value::string("https:\\/\\/www.space-track.org\\/expandedspacedata\\/query\\/class\\/organization\\/object\\/~~1234\\/orderby\\/ORG_NAME,INFO_ID\\/format\\/html\\/emptyresult\\/show\\/");
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
  ccsds_cdm[U("SAT1_TIME_LASTOB_START")] = web::json::value::string("2020-01-19 07:38:19");
  ccsds_cdm[U("SAT1_TIME_LASTOB_START_FRACTION")] = web::json::value::string("0");
  ccsds_cdm[U("SAT1_TIME_LASTOB_END")] = web::json::value::string("2020-01-20 07:38:19");
  ccsds_cdm[U("SAT1_TIME_LASTOB_END_FRACTION")] = web::json::value::string("0");
  ccsds_cdm[U("SAT1_RECOMMENDED_OD_SPAN")] = web::json::value::string("7.02");
  ccsds_cdm[U("SAT1_RECOMMENDED_OD_SPAN_UNIT")] = web::json::value::string("d");
  ccsds_cdm[U("SAT1_ACTUAL_OD_SPAN")] = web::json::value::string("7.02");
  ccsds_cdm[U("SAT1_ACTUAL_OD_SPAN_UNIT")] = web::json::value::string("d");
  ccsds_cdm[U("SAT1_OBS_AVAILABLE")] = web::json::value::string("183");
  ccsds_cdm[U("SAT1_OBS_USED")] = web::json::value::string("183");
  ccsds_cdm[U("SAT1_RESIDUALS_ACCEPTED")] = web::json::value::string("98.0");
  ccsds_cdm[U("SAT1_RESIDUALS_ACCEPTED_UNIT")] = web::json::value::string("%");
  ccsds_cdm[U("SAT1_WEIGHTED_RMS")] = web::json::value::string("1.013");
  ccsds_cdm[U("SAT1_COMMENT_APOGEE")] = web::json::value::string("Apogee Altitude = 601   [km]");
  ccsds_cdm[U("SAT1_COMMENT_PERIGEE")] = web::json::value::string("Perigee Altitude = 600   [km]");
  ccsds_cdm[U("SAT1_COMMENT_INCLINATION")] = web::json::value::string("Inclination = 97.0  [deg]");
  ccsds_cdm[U("SAT1_AREA_PC")] = web::json::value::string("0.4");
  ccsds_cdm[U("SAT1_AREA_PC_UNIT")] = web::json::value::string("m**2");
  ccsds_cdm[U("SAT1_CD_AREA_OVER_MASS")] = web::json::value::string("0.02");
  ccsds_cdm[U("SAT1_CD_AREA_OVER_MASS_UNIT")] = web::json::value::string("m**2\\/kg");
  ccsds_cdm[U("SAT1_CR_AREA_OVER_MASS")] = web::json::value::string("0.009");
  ccsds_cdm[U("SAT1_CR_AREA_OVER_MASS_UNIT")] = web::json::value::string("m**2\\/kg");
  ccsds_cdm[U("SAT1_THRUST_ACCELERATION")] = web::json::value::string("0");
  ccsds_cdm[U("SAT1_THRUST_ACCELERATION_UNIT")] = web::json::value::string("m\\/s**2");
  ccsds_cdm[U("SAT1_SEDR")] = web::json::value::string("5.9e-05");
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
  ccsds_cdm[U("SAT2_OBJECT_DESIGNATOR")] = web::json::value::string("5678");
  ccsds_cdm[U("SAT2_CATALOG_NAME")] = web::json::value::string("SATCAT");
  ccsds_cdm[U("SAT2_OBJECT_NAME")] = web::json::value::string("OBJECT 2");
  ccsds_cdm[U("SAT2_INTERNATIONAL_DESIGNATOR")] = web::json::value::string("1954-00-1");
  ccsds_cdm[U("SAT2_OBJECT_TYPE")] = web::json::value::string("TBA");
  ccsds_cdm[U("SAT2_OPERATOR_CONTACT_POSITION")] = web::json::value::string("https:\\/\\/www.space-track.org\\/expandedspacedata\\/query\\/class\\/organization\\/object\\/~~5678\\/orderby\\/ORG_NAME,INFO_ID\\/format\\/html\\/emptyresult\\/show\\/");
  ccsds_cdm[U("SAT2_OPERATOR_ORGANIZATION")] = web::json::value::string("NONE");
  ccsds_cdm[U("SAT2_OPERATOR_PHONE")] = web::json::value::string("https:\\/\\/www.space-track.org\\/expandedspacedata\\/query\\/class\\/organization\\/object\\/~~5678\\/orderby\\/ORG_NAME,INFO_ID\\/format\\/html\\/emptyresult\\/show\\/");
  ccsds_cdm[U("SAT2_OPERATOR_EMAIL")] = web::json::value::string("https:\\/\\/www.space-track.org\\/expandedspacedata\\/query\\/class\\/organization\\/object\\/~~5678\\/orderby\\/ORG_NAME,INFO_ID\\/format\\/html\\/emptyresult\\/show\\/");
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
  ccsds_cdm[U("SAT2_TIME_LASTOB_START")] = web::json::value::string("2020-01-19 07:38:19");
  ccsds_cdm[U("SAT2_TIME_LASTOB_START_FRACTION")] = web::json::value::string("0");
  ccsds_cdm[U("SAT2_TIME_LASTOB_END")] = web::json::value::string("2020-01-20 07:38:19");
  ccsds_cdm[U("SAT2_TIME_LASTOB_END_FRACTION")] = web::json::value::string("0");
  ccsds_cdm[U("SAT2_RECOMMENDED_OD_SPAN")] = web::json::value::string("7.12");
  ccsds_cdm[U("SAT2_RECOMMENDED_OD_SPAN_UNIT")] = web::json::value::string("d");
  ccsds_cdm[U("SAT2_ACTUAL_OD_SPAN")] = web::json::value::string("7.12");
  ccsds_cdm[U("SAT2_ACTUAL_OD_SPAN_UNIT")] = web::json::value::string("d");
  ccsds_cdm[U("SAT2_OBS_AVAILABLE")] = web::json::value::string("140");
  ccsds_cdm[U("SAT2_OBS_USED")] = web::json::value::string("138");
  ccsds_cdm[U("SAT2_RESIDUALS_ACCEPTED")] = web::json::value::string("99.4");
  ccsds_cdm[U("SAT2_RESIDUALS_ACCEPTED_UNIT")] = web::json::value::string("%");
  ccsds_cdm[U("SAT2_WEIGHTED_RMS")] = web::json::value::string("1.12");
  ccsds_cdm[U("SAT2_COMMENT_APOGEE")] = web::json::value::string("Apogee Altitude = 601   [km]");
  ccsds_cdm[U("SAT2_COMMENT_PERIGEE")] = web::json::value::string("Perigee Altitude = 600   [km]");
  ccsds_cdm[U("SAT2_COMMENT_INCLINATION")] = web::json::value::string("Inclination = 97.0  [deg]");
  ccsds_cdm[U("SAT2_AREA_PC")] = web::json::value::string("0.06");
  ccsds_cdm[U("SAT2_AREA_PC_UNIT")] = web::json::value::string("m**2");
  ccsds_cdm[U("SAT2_CD_AREA_OVER_MASS")] = web::json::value::string("0.03");
  ccsds_cdm[U("SAT2_CD_AREA_OVER_MASS_UNIT")] = web::json::value::string("m**2\\/kg");
  ccsds_cdm[U("SAT2_CR_AREA_OVER_MASS")] = web::json::value::string("0.01");
  ccsds_cdm[U("SAT2_CR_AREA_OVER_MASS_UNIT")] = web::json::value::string("m**2\\/kg");
  ccsds_cdm[U("SAT2_THRUST_ACCELERATION")] = web::json::value::string("0");
  ccsds_cdm[U("SAT2_THRUST_ACCELERATION_UNIT")] = web::json::value::string("m\\/s**2");
  ccsds_cdm[U("SAT2_SEDR")] = web::json::value::string("8.2e-05");
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
  
  
  // Send request for risk estimation
  OkapiConnector::CompleteResult riskEstimationRequest = connector.sendRequest(baseUrl, "/estimate-risk/all-methods/requests", riskBody);
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

  // Get results for risk estimation
  OkapiConnector::CompleteResult riskEstimationResult = retrieveResult(connector, baseUrl, "/estimate-risk/all-methods/simple/results/", requestIdRiskEstimation);
  if (riskEstimationResult.error.code != 200 && riskEstimationResult.error.code != 202)
  {
    cout << "Retrieving risk estimation response failed with status: " << riskEstimationResult.error.status << endl;
    cout << riskEstimationResult.error.message << endl;
  }
  if (riskEstimationResult.error.code == 200) {
    cout << riskEstimationResult.body.serialize() << endl;
  }
  
  // Now we run a scenario where the along-track component of the covariances of both objects are scaled
  covarianceScalingRange[U("lower_bound")] = 0.1;
  covarianceScalingRange[U("upper_bound")] = 1.0;
  covarianceScalingRange[U("step_size")] = 0.1;
  overrides[U("SAT1_COVARIANCE_T_SCALING_RANGE")] = covarianceScalingRange;
  overrides[U("SAT2_COVARIANCE_T_SCALING_RANGE")] = covarianceScalingRange;
  
  riskBody[U("overrides")] = overrides;
  
  // Send request for risk estimation with overrides
  riskEstimationRequest = connector.sendRequest(baseUrl, "/estimate-risk/foster-1992/requests", riskBody);
  if (riskEstimationRequest.error.code == 200 || riskEstimationRequest.error.code == 202)
  {
    cout << "Send risk estimation request completed" << endl;
  }
  else {
    cout << "Risk estimation request failed with status: " << riskEstimationRequest.error.status << endl;
    cout << riskEstimationRequest.error.message << endl;
  }
  requestIdRiskEstimation = connector.requestId;
  cout << "Risk estimation request ID: " << requestIdRiskEstimation << endl;

  // get results for risk estimation
  riskEstimationResult = retrieveResult(connector, baseUrl, "/estimate-risk/foster-1992/simple/results/", requestIdRiskEstimation);
  if (riskEstimationResult.error.code != 200 && riskEstimationResult.error.code != 202)
  {
    cout << "Retrieving risk estimation response failed with status: " << riskEstimationResult.error.status << endl;
    cout << riskEstimationResult.error.message << endl;
  }
  if (riskEstimationResult.error.code == 200) {
    cout << riskEstimationResult.body.serialize() << endl;
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
  // Correct URL and port for the v2020.01 release
  string baseUrl = "http://okapi.ddns.net:34568";

	// Authentication with Auth0 to retrieve the access token
  cout << "[Authentication] - started" << endl;
	OkapiConnector::CompleteResult initResult
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
