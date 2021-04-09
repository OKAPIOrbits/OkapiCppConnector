#include "OkapiConnector.h"

using std::cout;
using std::endl;
using std::string;

using namespace utility;
using namespace web;
using namespace web::http;
using namespace web::http::client;

void predictPassesTests(OkapiConnector connector)
{
  web::json::value groundLocationContent;
  groundLocationContent[U("altitude")] = web::json::value::number( 0.048);
  groundLocationContent[U("longitude")] = web::json::value::number(10.645);
  groundLocationContent[U("latitude")] = web::json::value::number(52.328);

  web::json::value timeWindowContent;
  timeWindowContent[U("start")] = web::json::value::string("2018-08-06T18:19:44.256Z");
  timeWindowContent[U("end")] = web::json::value::string("2018-08-07T00:00:00.000Z");

  web::json::value predictPassesSettingsSimple;
  predictPassesSettingsSimple[U("output_step_size")] = web::json::value::number(60);
  predictPassesSettingsSimple[U("geopotential_degree_order")] = web::json::value::number(2);

  web::json::value simpleState;
  simpleState[U("area")] = web::json::value::number(0.01);
  simpleState[U("mass")] = web::json::value::number(1.3);
  simpleState[U("x")] = web::json::value::number(-2915.65441951);
  simpleState[U("y")] = web::json::value::number(-3078.17058851);
  simpleState[U("z")] = web::json::value::number(5284.39698421);
  simpleState[U("x_dot")] = web::json::value::number(4.94176934);
  simpleState[U("y_dot")] = web::json::value::number(-5.83109248);
  simpleState[U("z_dot")] = web::json::value::number(-0.66365683);
  simpleState[U("epoch")] = web::json::value::string("2018-08-06T18:19:43.256Z");

  web::json::value passPredictionNumericalRequestBody;
  web::json::value groundLocation;
  groundLocation[U("type")] = web::json::value::string("ground_loc.json");
  groundLocation[U("content")] = groundLocationContent;
  passPredictionNumericalRequestBody[U("ground_location")] = groundLocation;
  web::json::value timeWindow;
  timeWindow[U("type")] = web::json::value::string("tw.json");
  timeWindow[U("content")] = timeWindowContent;
  passPredictionNumericalRequestBody[U("time_window")] = timeWindow;
  web::json::value orbit;
  orbit[U("type")] = web::json::value::string("state.json");
  orbit[U("content")] = simpleState;
  passPredictionNumericalRequestBody[U("orbit")] = orbit;
  web::json::value settings;
  settings[U("type")] = web::json::value::string("shared_prop_settings.json");
  settings[U("content")] = predictPassesSettingsSimple;
  passPredictionNumericalRequestBody[U("settings")] = settings;

  // Send request for NEPTUNE pass prediction
  OkapiConnector::OkapiResult responseNeptune = connector.sendRequest("/predict-passes/neptune/requests", passPredictionNumericalRequestBody);
  // Check response
  if (responseNeptune.error.code != 200 && responseNeptune.error.code != 202)
  {
    cout << "Neptune request failed with status: " << responseNeptune.error.status << endl;
    cout << responseNeptune.error.message << endl;
  }
  string requestIdPassPredictionNeptune = connector.getRequestId(responseNeptune);
  cout << "Neptune request ID: " << requestIdPassPredictionNeptune << endl;


  // Get results for NEPTUNE and print them in the terminal
  OkapiConnector::OkapiResult neptuneSimpleResult = connector.waitForProcessingAndGetValues("/predict-passes/neptune/results/" + requestIdPassPredictionNeptune + "/simple");
  if (neptuneSimpleResult.error.code != 200 && neptuneSimpleResult.error.code != 202)
  {
    cout << "Response failed with status: " << neptuneSimpleResult.error.status << endl;
    cout << neptuneSimpleResult.error.message << endl;
  }
  else
  {
    cout << neptuneSimpleResult.body.serialize() << endl;
  }

  OkapiConnector::OkapiResult neptuneSummaryResult = connector.waitForProcessingAndGetValues("/predict-passes/neptune/results/" + requestIdPassPredictionNeptune + "/summary");
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

void neptuneTest(OkapiConnector connector)
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
  double outputStepSizeNeptuneSimple = 180;

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
  neptuneConfigSimple[U("output_step_size")] = outputStepSizeNeptuneSimple;

  web::json::value settingsSimple;
  settingsSimple[U("propagation_end_epoch")] = web::json::value::string(propagationEndEpochNeptuneSimple);
  settingsSimple[U("more")] = neptuneConfigSimple;
  web::json::value propagateNeptuneSimpleRequestBody;
  web::json::value orbit;
  orbit[U("type")] = web::json::value::string("state.json");
  orbit[U("content")] = simpleState;
  web::json::value settings;
  settings[U("type")] = web::json::value::string("prop_settings.json");
  settings[U("content")] = settingsSimple;
  propagateNeptuneSimpleRequestBody[U("orbit")] = orbit;
  propagateNeptuneSimpleRequestBody[U("settings")] = settings;

  // Send request for NEPTUNE propagation
  OkapiConnector::OkapiResult neptuneRequest = connector.sendRequest("/propagate-orbit/neptune/requests", propagateNeptuneSimpleRequestBody);
  if (neptuneRequest.error.code == 200 || neptuneRequest.error.code == 202)
  {
    cout << "Send NEPTUNE propagation request completed" << endl;
  }
  else {
    cout << "NEPTUNE propagation request failed with status: " << neptuneRequest.error.status << endl;
    cout << neptuneRequest.error.message << endl;
  }
  string requestIdNeptune = connector.getRequestId(neptuneRequest);
  cout << "Request ID: " << requestIdNeptune << endl;

  // Get results in simple format for NEPTUNE and print them in the terminal
  OkapiConnector::OkapiResult neptuneSummaryResult = connector.waitForProcessingAndGetValues("/propagate-orbit/neptune/results/" + requestIdNeptune  + "/simple");
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

void riskEstimationTest(OkapiConnector connector)
{

  web::json::value ccsds_cdm;

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

  web::json::value riskBody;
  web::json::value conjunction;

  conjunction[U("type")] = web::json::value::string("cdm.json");
  conjunction[U("content")] = ccsds_cdm;
  riskBody[U("conjunction")] = conjunction;

  // Send request for risk estimation
  OkapiConnector::OkapiResult riskEstimationRequest = connector.sendRequest("/estimate-risk/foster-1992/requests", riskBody);
  if (riskEstimationRequest.error.code == 200 || riskEstimationRequest.error.code == 202)
  {
    cout << "Send risk estimation request completed" << endl;
  }
  else {
    cout << "Risk estimation request failed with status: " << riskEstimationRequest.error.status << endl;
    cout << riskEstimationRequest.error.message << endl;
  }
  string requestIdRiskEstimation = connector.getRequestId(riskEstimationRequest);
  cout << "Risk estimation request ID: " << requestIdRiskEstimation << endl;

  // Get results for risk estimation
  OkapiConnector::OkapiResult riskEstimationResult = connector.waitForProcessingAndGetValues("/estimate-risk/foster-1992/results/" + requestIdRiskEstimation + "/simple");
  if (riskEstimationResult.error.code != 200 && riskEstimationResult.error.code != 202)
  {
    cout << "Retrieving risk estimation response failed with status: " << riskEstimationResult.error.status << endl;
    cout << riskEstimationResult.error.message << endl;
  }
  if (riskEstimationResult.error.code == 200) {
    cout << riskEstimationResult.body.serialize() << endl;
  }
}

string addSatelliteTest(OkapiConnector okapi) {

  string newSatelliteId = "";

  web::json::value newSatellite;
  newSatellite[U("name")] = web::json::value::string("Sputnik");
  std::vector<web::json::value> noradIds;
  noradIds.push_back(web::json::value::number(1));
  newSatellite[U("norad_ids")] = web::json::value::array(noradIds);
  // This is a random ID, which will be changed by the backend but currently it is still required
  newSatellite[U("satellite_id")] = web::json::value::string("550e8400-e29b-11d4-a716-446655440000");
  newSatellite[U("space_track_status")] = web::json::value::string("sharing_agreement_signed");
  newSatellite[U("active")] = web::json::value::boolean(false);

  OkapiConnector::OkapiResult result = okapi.addSatellite(newSatellite);

  if (result.error.code == 200) {
    newSatellite = result.body;
    cout << newSatellite.serialize() << endl;
    newSatelliteId = newSatellite.as_object().at(U("satellite_id")).as_string();
  } else {
    cout << result.error.status << ": " << result.error.message << endl;
  }
  return newSatelliteId;
}

string updateSatelliteTest(OkapiConnector okapi, string satelliteId) {

  string satelliteName = "";

  web::json::value currentSatellite;
  currentSatellite[U("name")] = web::json::value::string("Sputnik-2");
  std::vector<web::json::value> noradIds;
  noradIds.push_back(web::json::value::number(1));
  currentSatellite[U("norad_ids")] = web::json::value::array(noradIds);
  // This is a random ID, which will be changed by the backend but currently it is still required
  currentSatellite[U("satellite_id")] = web::json::value::string(satelliteId);
  currentSatellite[U("space_track_status")] = web::json::value::string("sharing_agreement_signed");

  OkapiConnector::OkapiResult result = okapi.updateSatellite(currentSatellite, satelliteId);

  if (result.error.code == 200) {
    currentSatellite = result.body;
    cout << currentSatellite.serialize() << endl;
    satelliteName = currentSatellite.as_object().at(U("name")).as_string();
  } else {
    cout << result.error.status << ": " << result.error.message << endl;
  }
  return satelliteName;
}

void getSatellitesTest(OkapiConnector okapi) {

  OkapiConnector::OkapiResult result = okapi.getSatellites();

  if (result.error.code == 200) {
    cout << result.body.serialize() << endl;
  } else {
    cout << result.error.status << ": " << result.error.message << endl;
  }
}

string deleteSatelliteTest(OkapiConnector okapi, string satelliteId) {

  string satelliteName = "";

  OkapiConnector::OkapiResult result = okapi.deleteSatellite(satelliteId);

  if (result.error.code == 200) {
    web::json::value currentSatellite = result.body;
    cout << currentSatellite.serialize() << endl;
    satelliteName = currentSatellite.as_object().at(U("name")).as_string();
  } else {
    cout << result.error.status << ": " << result.error.message << endl;
  }
  return satelliteName;
}

string getConjunctionsTest(OkapiConnector okapi) {

  OkapiConnector::OkapiResult result = okapi.getConjunctions();

  string conjunctionId;
  if (result.body.as_object().at(U("elements")).as_array().size() > 0)
    conjunctionId = result.body.as_object().at(U("elements")).as_array()[0].at(U("conjunction_id")).as_string();

  if (result.error.code == 200) {
    cout << result.body.serialize() << endl;
  } else {
    cout << result.error.status << ": " << result.error.message << endl;
  }
  return conjunctionId;
}

void getCdmsTest(OkapiConnector okapi, string conjunctionId) {

  OkapiConnector::OkapiResult result = okapi.getCdms(conjunctionId);

  if (result.error.code == 200) {
    cout << result.body.serialize() << endl;
  } else {
    cout << result.error.status << ": " << result.error.message << endl;
  }
}

void getManeuverEvalsTest(OkapiConnector okapi, string conjunctionId) {

  OkapiConnector::OkapiResult result = okapi.getManeuverEvals(conjunctionId);

  if (result.error.code == 200) {
    cout << result.body.serialize() << endl;
  } else {
    cout << result.error.status << ": " << result.error.message << endl;
  }
}

int main(int argc, char* argv[])
{
	// initializing communication
	OkapiConnector connector;

	// Authentication with Auth0 to retrieve the access token
  cout << "[Authentication] - started" << endl;
	OkapiConnector::OkapiResult initResult
      = connector.init("https://api.okapiorbits.com/", "username", "password");

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

  cout << "[Add satellite] - started" << endl;
  string newSatelliteId = addSatelliteTest(connector);
  cout << "Added satellite with id: " << newSatelliteId << endl;
  cout << "[Add satellite] - completed" << endl;

  cout << "[Update satellite] - started" << endl;
  string satelliteName = updateSatelliteTest(connector, newSatelliteId);
  cout << "Updated satellite with name: " << satelliteName << endl;
  cout << "[Update satellite] - completed" << endl;

  cout << "[Get satellites] - started" << endl;
  getSatellitesTest(connector);
  cout << "[Get satellites] - completed" << endl;

  cout << "[Delete satellite] - started" << endl;
  satelliteName = deleteSatelliteTest(connector, newSatelliteId);
  cout << "Deleted satellite with name: " << satelliteName << endl;
  cout << "[Delete satellite] - completed" << endl;

  cout << "[Get conjunctions] - started" << endl;
  string conjunctionId = getConjunctionsTest(connector);
  cout << "Selected conjunction: " << conjunctionId << endl;
  cout << "[Get conjunctions] - completed" << endl;

  cout << "[Get cdms] - started" << endl;
  getCdmsTest(connector,conjunctionId);
  cout << "[Get cdms] - completed" << endl;

  cout << "[Get maneuver evals] - started" << endl;
  getManeuverEvalsTest(connector,conjunctionId);
  cout << "[Get maneuver evals] - completed" << endl;

  cout << "[Predict passes] - started" << endl;
  predictPassesTests(connector);
  cout << "[Predict passes] - completed" << endl;

  cout << "[Propagate orbit NEPTUNE] - started" << endl;
  neptuneTest(connector);
  cout << "[Propagate orbit NEPTUNE] - completed" << endl;

  cout << "[Estimate risk all methods] - started" << endl;
  riskEstimationTest(connector);
  cout << "[Estimate risk all methods] - completed" << endl;

  return 0;
}
