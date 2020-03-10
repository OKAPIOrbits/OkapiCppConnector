#include "OkapiConnector.h"
#include <sys/stat.h>

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
OkapiConnector::Result retrieveResult(OkapiConnector connector, string baseUrl, string endpoint, string requestId, string resultType)
{
  // First call to the backend
  OkapiConnector::Result result = connector.getResult(baseUrl, endpoint, requestId, resultType);
  if (result.error.code != 200 && result.error.code != 202)
  {
    cout << "Retrieving response failed with status: " << result.error.status << endl;
    cout << result.error.message << endl;
    return result;
  }
  int i = 0;
  // Poll the backend until the result is ready and can be retrieved
  while (result.error.code == 202) {
    result = connector.getResult(baseUrl, endpoint, requestId, resultType);
    cout << "The request was successful. Your result is not ready yet.  Waiting: " << i << " s." << endl;
    i++;
    sleep(1);
  }
  // Final call to the backend
  return connector.getResult(baseUrl, endpoint, requestId, resultType);
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
  OkapiConnector::Result responseSGP4 = connector.sendRequest(baseUrl, "/predict-passes/sgp4/requests", passPredictionRequestBody);
  if (responseSGP4.error.code != 200 && responseSGP4.error.code != 202)
  {
    cout << "SGP4 request failed with status: " << responseSGP4.error.status << endl;
    cout << responseSGP4.error.message << endl;
    return;
  }
  string requestIdPassPredictionSgp4 = connector.requestId;
  cout << "SGP4 request ID: " << requestIdPassPredictionSgp4 << endl;
  
  // Get results for SGP4 and print them in the terminal
  OkapiConnector::Result sgp4SimpleResult = retrieveResult(connector, baseUrl, "/predict-passes/sgp4/results/", requestIdPassPredictionSgp4,"/simple");
  if (sgp4SimpleResult.error.code != 200 && sgp4SimpleResult.error.code != 202)
  {
    cout << "Response failed with status: " << sgp4SimpleResult.error.status << endl;
    cout << sgp4SimpleResult.error.message << endl;
  }
  else
  {
    cout << sgp4SimpleResult.body.serialize() << endl;
  }

  OkapiConnector::Result sgp4SummaryResult = retrieveResult(connector, baseUrl, "/predict-passes/sgp4/results/", requestIdPassPredictionSgp4,"/summary");
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
  OkapiConnector::Result responseOrekit = connector.sendRequest(baseUrl, "/predict-passes/orekit-numerical/requests", passPredictionNumericalRequestBody);
  // Check response
  if (responseOrekit.error.code != 200 && responseOrekit.error.code != 202)
  {
    cout << "Orekit request failed with status: " << responseOrekit.error.status << endl;
    cout << responseOrekit.error.message << endl;
  }
  string requestIdPassPredictionOrekit = connector.requestId;
  cout << "Orekit request ID: " << requestIdPassPredictionOrekit << endl;
  
  // Get results for OREKIT and print them in the terminal
  OkapiConnector::Result orekitSimpleResult = retrieveResult(connector, baseUrl, "/predict-passes/orekit-numerical/results/", requestIdPassPredictionOrekit,"/simple");
  if (orekitSimpleResult.error.code != 200 && orekitSimpleResult.error.code != 202)
  {
    cout << "Response failed with status: " << orekitSimpleResult.error.status << endl;
    cout << orekitSimpleResult.error.message << endl;
  }
  else
  {
    cout << orekitSimpleResult.body.serialize() << endl;
  }
  
  OkapiConnector::Result orekitSummaryResult = retrieveResult(connector, baseUrl, "/predict-passes/orekit-numerical/results/", requestIdPassPredictionOrekit,"/summary");
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
  OkapiConnector::Result neptuneRequest = connector.sendRequest(baseUrl, "/propagate-orbit/neptune/requests", propagateNeptuneSimpleRequestBody);
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
  OkapiConnector::Result neptuneSummaryResult = retrieveResult(connector, baseUrl, "/propagate-orbit/neptune/results/", requestIdNeptune, "/simple");
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
  OkapiConnector::Result neptuneRequest = connector.sendRequest(baseUrl, "/propagate-orbit/neptune/requests", propagateNeptuneSimpleRequestBody);
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
  OkapiConnector::Result neptuneResult = retrieveResult(connector, baseUrl, "/propagate-orbit/neptune/results/", requestIdNeptune, "/opm");
  if (neptuneResult.error.code != 200 && neptuneResult.error.code != 202)
  {
    cout << "Response failed with status: " << neptuneResult.error.status << endl;
    cout << neptuneResult.error.message << endl;
  }
  else
  {
    cout << neptuneResult.body.serialize() << endl;
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
  OkapiConnector::Result neptuneRequest = connector.sendRequest(baseUrl, "/propagate-orbit/neptune/requests", propagateNeptuneSimpleRequestBody);
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
  OkapiConnector::Result neptuneResult = retrieveResult(connector, baseUrl, "/propagate-orbit/neptune/opm/results/", requestIdNeptune, "/opm");
  if (neptuneResult.error.code != 200 && neptuneResult.error.code != 202)
  {
    cout << "Response failed with status: " << neptuneResult.error.status << endl;
    cout << neptuneResult.error.message << endl;
  }
  else
  {
    cout << neptuneResult.body.serialize() << endl;
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
  OkapiConnector::Result orekitRequest = connector.sendRequest(baseUrl, "/propagate-orbit/orekit-numerical/requests", propagateOrekitSimpleRequestBody);
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
  OkapiConnector::Result orekitResult = retrieveResult(connector, baseUrl, "/propagate-orbit/orekit-numerical/results/", requestIdOrekit, "simple");
  if (orekitResult.error.code != 200 && orekitResult.error.code != 202)
  {
    cout << "Response failed with status: " << orekitResult.error.status << endl;
    cout << orekitResult.error.message << endl;
  }
  else
  {
    cout << orekitResult.body.serialize() << endl;
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
  OkapiConnector::Result sgp4Request = connector.sendRequest(baseUrl, "/propagate-orbit/sgp4/requests", propagateSgp4RequestBody);
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
  OkapiConnector::Result sgp4Result = retrieveResult(connector, baseUrl, "/propagate-orbit/sgp4/results/", requestIdSgp4, "/simple");
  if (sgp4Result.error.code != 200 && sgp4Result.error.code != 202)
  {
    cout << "Retrieving SGP4 propagation simple response failed with status: " << sgp4Result.error.status << endl;
    cout << sgp4Result.error.message << endl;
  }
  if (sgp4Result.error.code == 200) {
    cout << sgp4Result.body.serialize() << endl;
  }
  
  // Get results in OMM format for SGP4
  OkapiConnector::Result sgp4OmmResult = retrieveResult(connector, baseUrl, "/propagate-orbit/sgp4/omm/results/", requestIdSgp4, "/omm");
  if (sgp4OmmResult.error.code != 200 && sgp4OmmResult.error.code != 202)
  {
    cout << "Retrieving SGP4 propagation OMM response failed with status: " << sgp4OmmResult.error.status << endl;
    cout << sgp4OmmResult.error.message << endl;
  }
  if (sgp4OmmResult.error.code == 200) {
    cout << sgp4OmmResult.body.serialize() << endl;
  }
  
}

void riskEstimationTest(OkapiConnector connector, string baseUrl)
{

  web::json::value riskBody;
  web::json::value conjunction;
  web::json::value ccsds_cdm;
  web::json::value overrides;
  web::json::value covarianceScalingRange;
  
  string cdmFile = "cdm.json";
  struct stat buffer;
  if (!(stat(cdmFile.c_str(), &buffer) == 0)) {
    cout << "No cdm.json file found. Skipping risk estimation test" << endl;
    return;
  }
  
  // Read the json file
  std::ifstream cdm_file(cdmFile);
  stringstream_t stringStream;
  stringStream << cdm_file.rdbuf();
  cdm_file.close();
  ccsds_cdm = json::value::parse(stringStream);
  
  conjunction[U("type")] = web::json::value::string("cdm.json");
  conjunction[U("content")] = ccsds_cdm[0];
  riskBody[U("conjunction")] = conjunction;
  
  // Send request for risk estimation
  OkapiConnector::Result riskEstimationRequest = connector.sendRequest(baseUrl, "/estimate-risk/alfano-2005/requests", riskBody);
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
  OkapiConnector::Result riskEstimationResult = retrieveResult(connector, baseUrl, "/estimate-risk/alfano-2005/results/", requestIdRiskEstimation, "/simple");
  if (riskEstimationResult.error.code != 200 && riskEstimationResult.error.code != 202)
  {
    cout << "Retrieving risk estimation response failed with status: " << riskEstimationResult.error.status << endl;
    cout << riskEstimationResult.error.message << endl;
  }
  if (riskEstimationResult.error.code == 200) {
    cout << riskEstimationResult.body.serialize() << endl;
  }

//  // Now we run a scenario where the along-track component of the covariances of both objects are scaled
//  covarianceScalingRange[U("lower_bound")] = 0.1;
//  covarianceScalingRange[U("upper_bound")] = 1.0;
//  covarianceScalingRange[U("step_size")] = 0.1;
//  overrides[U("SAT1_COVARIANCE_T_SCALING_RANGE")] = covarianceScalingRange;
//  overrides[U("SAT2_COVARIANCE_T_SCALING_RANGE")] = covarianceScalingRange;
//
//  riskBody[U("overrides")] = overrides;
//
//  // Send request for risk estimation with overrides
//  riskEstimationRequest = connector.sendRequest(baseUrl, "/estimate-risk/foster-1992/requests", riskBody);
//  if (riskEstimationRequest.error.code == 200 || riskEstimationRequest.error.code == 202)
//  {
//    cout << "Send risk estimation request completed" << endl;
//  }
//  else {
//    cout << "Risk estimation request failed with status: " << riskEstimationRequest.error.status << endl;
//    cout << riskEstimationRequest.error.message << endl;
//  }
//  requestIdRiskEstimation = connector.requestId;
//  cout << "Risk estimation request ID: " << requestIdRiskEstimation << endl;
//
//  // get results for risk estimation
//  riskEstimationResult = retrieveResult(connector, baseUrl, "/estimate-risk/foster-1992/simple/results/", requestIdRiskEstimation);
//  if (riskEstimationResult.error.code != 200 && riskEstimationResult.error.code != 202)
//  {
//    cout << "Retrieving risk estimation response failed with status: " << riskEstimationResult.error.status << endl;
//    cout << riskEstimationResult.error.message << endl;
//  }
//  if (riskEstimationResult.error.code == 200) {
//    cout << riskEstimationResult.body.serialize() << endl;
//  }
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
  // string baseUrl = "http://okapi.ddns.net:34568";
  string baseUrl = "http://80.158.41.0:8080";

	// Authentication with Auth0 to retrieve the access token
  cout << "[Authentication] - started" << endl;
	OkapiConnector::Result initResult
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

//  // PASS PREDICTION
//  cout << "[Predict passes] - started" << endl;
//  predictPassesTests(connector, baseUrl);
//  cout << "[Predict passes] - completed" << endl;
//
//  // NEPTUNE propagation
//  cout << "[Propagate orbit NEPTUNE] - started" << endl;
//  neptuneTest(connector, baseUrl);
//  cout << "[Propagate orbit NEPTUNE] - completed" << endl;
//
//  // NEPTUNE co-variance propagation
//  cout << "[Propagate OPM Co-variance NEPTUNE] - started" << endl;
//  neptuneOpmCovarianceTest(connector, baseUrl);
//  cout << "[Propagate OPM Co-variance NEPTUNE] - completed" << endl;
//
//  // NEPTUNE maneuvre propagation enabled
//  cout << "[Propagate OPM Maneuvre NEPTUNE] - started" << endl;
//  neptuneOpmManeuvreTest(connector, baseUrl);
//  cout << "[Propagate OPM Maneuvre NEPTUNE] - completed" << endl;
//
//  // Orekit propagation
//  cout << "[Propagate orbit Orekit-numerical] - started" << endl;
//  orekitNumericalTest(connector, baseUrl);
//  cout << "[Propagate orbit Orekit-numerical] - completed" << endl;
//
//
//  // SGP4 propagation
//  cout << "[Propagate orbit SGP4] - started" << endl;
//  sgp4Test(connector, baseUrl);
//  cout << "[Propagate orbit SGP4] - completed" << endl;


  // Risk Estimation
  cout << "[Estimate risk all methods] - started" << endl;
  riskEstimationTest(connector, baseUrl);
  cout << "[Estimate risk all methods] - completed" << endl;

  return 0;
}
