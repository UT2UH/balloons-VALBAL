/*
  Stanford Student Space Initiative
  Balloons | VALBAL | February 2017
  Davy Ragland | dragland@stanford.edu

  File: Avionics.cpp
  --------------------------
  Implementation of Avionics.h
*/

#include "Avionics.h"

/**********************************  SETUP  ***********************************/
/*
 * Function: init
 * -------------------
 * This function initializes the avionics flight controller.
 */
void Avionics::init() {
  PCB.init();
  Serial.begin(CONSOLE_BAUD);
  printHeader();
  if(!SD.begin(SD_CS)) PCB.faultLED();
  setupLog();
  logHeader();
  if(!sensors.init())     logAlert("unable to initialize Sensors", true);
  if(!filter.init())      logAlert("unable to initialize Filters", true);
  if(!computer.init())    logAlert("unable to initialize Flight Controller", true);
  if(!gpsModule.init())   logAlert("unable to initialize GPS", true);
  if(!RBModule.init())    logAlert("unable to initialize RockBlock", true);
  data.SETUP_STATE = false;
}

/********************************  FUNCTIONS  *********************************/
/*
 * Function: updateState
 * -------------------
 * This function handles basic flight data collection.
 */
void Avionics::updateState() {
  if(!readData()) logAlert("unable to read Data", true);
  if(!processData()) logAlert("unable to process Data", true);
}

/*
 * Function: evaluateState
 * -------------------
 * This function intelligently calculates the current state.
 */
void Avionics::evaluateState() {
  if(!calcVitals())     logAlert("unable to calculate vitals", true);
  if(!calcDebug())      logAlert("unable to calculate debug", true);
  if(!calcIncentives()) logAlert("unable to calculate incentives", true);
  if(!calcCutdown())    logAlert("unable to calculate cutdown", true);
}

/*
 * Function: actuateState
 * -------------------
 * This function intelligently reacts to the current data frame.
 */
void Avionics::actuateState() {
  if(!runHeaters()) logAlert("unable to run heaters", true);
  if(!runValve())   logAlert("unable to run valve", true);
  if(!runBallast()) logAlert("unable to run ballast", true);
  if(!runCutdown()) logAlert("unable to run cutdown", true);
}

/*
 * Function: logState
 * -------------------
 * This function logs the current data frame.
 */
void Avionics::logState() {
  if(compressData() < 0) logAlert("unable to compress Data", true);
  if(!logData())         logAlert("unable to log Data", true);
  if(!debugState())      logAlert("unable to debug state", true);
  if (data.MINUTES > FILE_RESET_TIME) {
    dataFile.close();
    logFile.close();
    setupLog();
    printHeader();
  }
}

/*
 * Function: sendComms
 * -------------------
 * This function sends the current data frame down.
 */
void Avionics::sendComms() {
  if((millis() - data.COMMS_LAST) < COMMS_RATE) return;
  if(!sendSATCOMS()) logAlert("unable to communicate over RB", true);
  data.COMMS_LAST = millis();
}

/*
 * Function: sleep
 * -------------------
 * This function sleeps at the end of the loop.
 */
void Avionics::sleep() {
  gpsModule.smartDelay(LOOP_RATE);
}

/*
 * Function: finishedSetup
 * -------------------
 * This function returns true if the avionics has completed setup.
 */
bool Avionics::finishedSetup() {
  return !data.SETUP_STATE;
}

/*********************************  HELPERS  **********************************/
/*
 * Function: readData
 * -------------------
 * This function updates the current data frame.
 */
bool Avionics::readData() {
  data.TIME            = sensors.getTime();
  data.LOOP_RATE       = millis() - data.LOOP_START;
  data.LOOP_START      = millis();
  data.MINUTES         += (double)(((double)data.LOOP_RATE) / 1000.0 / 60.0);
  data.ALTITUDE_LAST   = data.ALTITUDE_BMP;
  data.VOLTAGE         = sensors.getVoltage();
  data.CURRENT         = sensors.getCurrent();
  data.CURRENT_GPS     = sensors.getCurrentGPS();
  data.CURRENT_RB      = sensors.getCurrentRB();
  data.CURRENT_MOTORS  = sensors.getCurrentMotors();
  data.CURRENT_PAYLOAD = sensors.getCurrentPayload();
  data.TEMP            = sensors.getTemp();
  data.PRESS_BMP       = sensors.getPressure();
  data.ALTITUDE_BMP    = sensors.getAltitude();
  data.ASCENT_RATE     = sensors.getAscentRate();
  data.LAT_GPS         = gpsModule.getLatitude();
  data.LONG_GPS        = gpsModule.getLongitude();
  data.ALTITUDE_GPS    = gpsModule.getAltitude();
  data.HEADING_GPS     = gpsModule.getCourse();
  data.SPEED_GPS       = gpsModule.getSpeed();
  data.NUM_SATS_GPS    = gpsModule.getSats();
  data.LOOP_GOOD_STATE = !data.LOOP_GOOD_STATE;
  return true;
}

/*
 * Function: processData
 * -------------------
 * This function updates the current data frame with derived values
 */
bool Avionics::processData() {

//Kalman filtering etc goes here


  return true;
}

/*
 * Function: runHeaters
 * -------------------
 * This function thermally regulates the avionics.
 */
bool Avionics::runHeaters() {
  PCB.heater(data.TEMP);
  return true;
}

/*
 * Function: runValve
 * -------------------
 * This function actuates the valve based on the calculated incentive.
 */
bool Avionics::runValve() {
  if(data.FORCE_VALVE) {
    PCB.queueValve(true);
    data.FORCE_VALVE = false;
    data.VALVE_ALT_LAST = data.ALTITUDE_BMP;
  }
  else if(data.VALVE_INCENTIVE >= 1) {
    PCB.queueValve(false);
    data.VALVE_ALT_LAST = data.ALTITUDE_BMP;
  }
  data.VALVE_STATE = PCB.checkValve();
  return true;
}

/*
 * Function: runBallast
 * -------------------
 * This function actuates the valve based on the calculated incentive.
 */
bool Avionics::runBallast() {
  if(data.FORCE_BALLAST) {
    PCB.queueBallast(true);
    data.FORCE_BALLAST = false;
    data.BALLAST_ALT_LAST = data.ALTITUDE_BMP;
  }
  else if(data.BALLAST_INCENTIVE >= 1) {
    PCB.queueBallast(false);
    data.BALLAST_ALT_LAST = data.ALTITUDE_BMP;
  }
  data.BALLAST_STATE = PCB.checkBallast();
  return true;
}

/*
 * Function: runCutdown
 * -------------------
 * This function cuts down the payload if necessary.
 */
bool Avionics::runCutdown() {
  if(data.CUTDOWN_STATE) return true;
  if(data.SHOULD_CUTDOWN) {
    PCB.cutDown(true);
    gpsModule.smartDelay(CUTDOWN_TIME);
    PCB.cutDown(false);
    data.CUTDOWN_STATE = true;
    logAlert("completed cutdown", false);
  }
  return true;
}

/*
 * Function: sendSATCOMS
 * -------------------
 * This function sends the current data frame over the ROCKBLOCK IO.
 */
bool Avionics::sendSATCOMS() {
  logAlert("sending Rockblock message", false);
  data.RB_SENT_COMMS++;
  int16_t ret = RBModule.writeRead(COMMS_BUFFER, data.COMMS_LENGTH);
  if(ret < 0) {
    data.RB_GOOD_STATE  = false;
    return false;
  }
  data.RB_GOOD_STATE  = true;
  if(ret > 0) parseCommand(ret);
  return true;
}

/*
 * Function: parseCommand
 * -------------------
 * This function parses the command received from the RockBLOCK.
 */
void Avionics::parseCommand(int16_t len) {
  if(strncmp(COMMS_BUFFER, CUTDOWN_COMAND, len)) {
    data.SHOULD_CUTDOWN = true;
  }
  if(strncmp(COMMS_BUFFER, "SUDO VALVE", len)) {
    data.FORCE_VALVE = true;
  }
  if(strncmp(COMMS_BUFFER, "SUDO BALLAST", len)) {
    data.FORCE_BALLAST = true;
  }
}

/*
 * Function: calcVitals
 * -------------------
 * This function calculates if the current state is within bounds.
 */
bool Avionics::calcVitals() {
  data.BAT_GOOD_STATE    = (data.VOLTAGE >= 3.63);
  data.CURR_GOOD_STATE   = (data.CURRENT > -5.0 && data.CURRENT <= 500.0);
  data.PRES_GOOD_STATE   = (data.ALTITUDE_BMP > -50 && data.ALTITUDE_BMP < 200);
  data.TEMP_GOOD_STATE   = (data.TEMP > 15 && data.TEMP < 50);
  data.GPS_GOOD_STATE    = (data.LAT_GPS != 1000.0 && data.LAT_GPS != 0.0 && data.LONG_GPS != 1000.0 && data.LONG_GPS != 0.0);
  return true;
}

/*
 * Function: calcDebug
 * -------------------
 * This function calculates if the avionics is in debug mode.
 */
bool Avionics::calcDebug() {
  if(data.DEBUG_STATE   && (data.ALTITUDE_LAST >= DEBUG_ALT) && (data.ALTITUDE_BMP >= DEBUG_ALT)) {
    data.DEBUG_STATE = false;
  }
  return true;
}

/*
 * Function: calcIncentives
 * -------------------
 * This function gets the updated incentives from the flight computer.
 */
bool Avionics::calcIncentives() {
  computer.updateControllerConstants( data.INCENTIVE_THRESHOLD, data.RE_ARM_CONSTANT);
  computer.updateValveConstants(data.VALVE_SETPOINT, data.VALVE_VELOCITY_CONSTANT, data.VALVE_ALTITUDE_DIFF_CONSTANT, data.VALVE_LAST_ACTION_CONSTANT);
  computer.updateBallastConstants(data.BALLAST_SETPOINT, data.BALLAST_VELOCITY_CONSTANT, data.BALLAST_ALTITUDE_DIFF_CONSTANT, data.BALLAST_LAST_ACTION_CONSTANT);
  data.VALVE_INCENTIVE   = computer.getValveIncentive(data.ASCENT_RATE, data.ALTITUDE_BMP, data.VALVE_ALT_LAST);
  data.BALLAST_INCENTIVE = computer.getBallastIncentive(data.ASCENT_RATE, data.ALTITUDE_BMP, data.BALLAST_ALT_LAST);
  return true;
}

/*
 * Function: calcCutdown
 * -------------------
 * This function calculates if the avionics should cutdown.
 */
bool Avionics::calcCutdown() {
  if(CUTDOWN_GPS_ENABLE && data.GPS_GOOD_STATE &&
    (((data.LAT_GPS < GPS_FENCE_LAT_MIN) || (data.LAT_GPS > GPS_FENCE_LAT_MAX)) ||
    ((data.LONG_GPS < GPS_FENCE_LON_MIN) || (data.LONG_GPS > GPS_FENCE_LON_MAX)))
  ) data.SHOULD_CUTDOWN  = true;

  if(CUTDOWN_ALT_ENABLE && !data.CUTDOWN_STATE &&
    (data.ALTITUDE_LAST >= CUTDOWN_ALT) &&
    (data.ALTITUDE_BMP  >= CUTDOWN_ALT)
  ) data.SHOULD_CUTDOWN  = true;
  return true;
}

/*
 * Function: printHeader
 * -------------------
 * This function prints the CSV header.
 */
void Avionics::printHeader() {
  if(!Serial) PCB.faultLED();
  Serial.print("Stanford Student Space Initiative Balloons Launch ");
  Serial.print(MISSION_NUMBER);
  Serial.print('\n');
  Serial.print(CSV_DATA_HEADER);
  Serial.print('\n');
}

/*
 * Function: setupLog
 * -------------------
 * This function initializes the SD card file.
 */
void Avionics::setupLog() {
  Serial.println("Card Initialitzed");
  char filename[] = "LOGGER00.txt";
  for (uint8_t i = 0; i < 100; i++) {
    filename[6] = i / 10 + '0';
    filename[7] = i % 10 + '0';
    if (! SD.exists(filename)) {
      dataFile = SD.open(filename, FILE_WRITE);
      break;
    }
  }
  logFile = SD.open("EVENTS.txt", FILE_WRITE);
  if (!dataFile || !logFile) {
    PCB.faultLED();
    Serial.println ("ERROR: COULD NOT CREATE FILE");
  }
  else {
    Serial.print("Logging to: ");
    Serial.println(filename);
  }
}

/*
 * Function: logHeader
 * -------------------
 * This function logs the CSV header.
 */
void Avionics::logHeader() {
  dataFile.print("Stanford Student Space Initiative Balloons Launch ");
  dataFile.print(MISSION_NUMBER);
  dataFile.print('\n');
  dataFile.print(CSV_DATA_HEADER);
  dataFile.print('\n');
  dataFile.flush();
}

/*
 * Function: logAlert
 * -------------------
 * This function logs important information whenever a specific event occurs.
 */
void Avionics::logAlert(const char* debug, bool fatal) {
  if(fatal) PCB.faultLED();
  if(logFile) {
    logFile.print(data.TIME);
    logFile.print(',');
    if(fatal) logFile.print("FATAL ERROR!!!!!!!!!!: ");
    else logFile.print("Alert: ");
    logFile.print(debug);
    logFile.print("...\n");
    logFile.flush();
  }
  if(data.DEBUG_STATE) {
    Serial.print(data.TIME);
    Serial.print(',');
    if(fatal) Serial.print("FATAL ERROR!!!!!!!!!!: ");
    else Serial.print("Alert: ");
    Serial.print(debug);
    Serial.print("...\n");
  }
}

/*
 * Function: debugState
 * -------------------
 * This function provides debuging information.
 */
bool Avionics::debugState() {
  if(data.DEBUG_STATE) {
    printState();
  }
  return true;
}

/*
 * Function: printState
 * -------------------
 * This function prints the current avionics state.
 */
void Avionics::printState() {
  Serial.print(data.TIME);
  Serial.print(',');
  Serial.print(data.LOOP_RATE);
  Serial.print(',');
  Serial.print(data.VOLTAGE);
  Serial.print(',');
  Serial.print(data.CURRENT);
  Serial.print(',');
  Serial.print(data.ALTITUDE_BMP);
  Serial.print(',');
  Serial.print(data.ASCENT_RATE);
  Serial.print(',');
  Serial.print(data.TEMP);
  Serial.print(',');
  Serial.print(data.LAT_GPS, 4);
  Serial.print(',');
  Serial.print(data.LONG_GPS, 4);
  Serial.print(',');
  Serial.print(data.SPEED_GPS);
  Serial.print(',');
  Serial.print(data.HEADING_GPS);
  Serial.print(',');
  Serial.print(data.ALTITUDE_GPS);
  Serial.print(',');
  Serial.print(data.PRESS_BMP);
  Serial.print(',');
  Serial.print(data.NUM_SATS_GPS);
  Serial.print(',');
  Serial.print(data.RB_SENT_COMMS);
  Serial.print(',');
  Serial.print(data.CUTDOWN_STATE);
  Serial.print('\n');
}

/*
 * Function: logData
 * -------------------
 * This function logs the current data frame.
 */
bool Avionics::logData() {
  bool sucess = true;
  dataFile.print(data.TIME);
  dataFile.print(',');
  dataFile.print(data.LOOP_RATE);
  dataFile.print(',');
  dataFile.print(data.VOLTAGE);
  dataFile.print(',');
  dataFile.print(data.CURRENT);
  dataFile.print(',');
  dataFile.print(data.ALTITUDE_BMP);
  dataFile.print(',');
  dataFile.print(data.ASCENT_RATE);
  dataFile.print(',');
  dataFile.print(data.TEMP);
  dataFile.print(',');
  dataFile.print(data.LAT_GPS, 4);
  dataFile.print(',');
  dataFile.print(data.LONG_GPS, 4);
  dataFile.print(',');
  dataFile.print(data.SPEED_GPS);
  dataFile.print(',');
  dataFile.print(data.HEADING_GPS);
  dataFile.print(',');
  dataFile.print(data.ALTITUDE_GPS);
  dataFile.print(',');
  dataFile.print(data.PRESS_BMP);
  dataFile.print(',');
  dataFile.print(data.NUM_SATS_GPS);
  dataFile.print(',');
  dataFile.print(data.RB_SENT_COMMS);
  if(dataFile.print(',') != 1) sucess = false;
  dataFile.print(data.CUTDOWN_STATE);
  dataFile.print('\n');
  dataFile.flush();
  return sucess;
}

/*
 * Function: compressData
 * -------------------
 * This function compresses the data frame into a bit stream.
 */
int16_t Avionics::compressData() {
  int16_t length = 0;
  size_t varSize = 0;

  varSize = sizeof(data.TIME);
  memcpy(COMMS_BUFFER + length, &data.TIME, varSize);
  length += varSize;
  COMMS_BUFFER[length] = ','; length++;

  varSize = sizeof(data.LOOP_RATE);
  memcpy(COMMS_BUFFER + length, &data.LOOP_RATE, varSize);
  length += varSize;
  COMMS_BUFFER[length] = ','; length++;

  varSize = sizeof(data.VOLTAGE);
  memcpy(COMMS_BUFFER + length, &data.VOLTAGE, varSize);
  length += varSize;
  COMMS_BUFFER[length] = ','; length++;

  varSize = sizeof(data.CURRENT);
  memcpy(COMMS_BUFFER + length, &data.CURRENT, varSize);
  length += varSize;
  COMMS_BUFFER[length] = ','; length++;

  varSize = sizeof(data.ALTITUDE_BMP);
  memcpy(COMMS_BUFFER + length, &data.ALTITUDE_BMP, varSize);
  length += varSize;
  COMMS_BUFFER[length] = ','; length++;

  varSize = sizeof(data.ASCENT_RATE);
  memcpy(COMMS_BUFFER + length, &data.ASCENT_RATE, varSize);
  length += varSize;
  COMMS_BUFFER[length] = ','; length++;

  varSize = sizeof(data.TEMP);
  memcpy(COMMS_BUFFER + length, &data.TEMP, varSize);
  length += varSize;
  COMMS_BUFFER[length] = ','; length++;

  varSize = sizeof(data.LAT_GPS);
  memcpy(COMMS_BUFFER + length, &data.LAT_GPS, varSize);
  length += varSize;
  COMMS_BUFFER[length] = ','; length++;

  varSize = sizeof(data.LONG_GPS);
  memcpy(COMMS_BUFFER + length, &data.LONG_GPS, varSize);
  length += varSize;
  COMMS_BUFFER[length] = ','; length++;

  varSize = sizeof(data.SPEED_GPS);
  memcpy(COMMS_BUFFER + length, &data.SPEED_GPS, varSize);
  length += varSize;
  COMMS_BUFFER[length] = ','; length++;

  varSize = sizeof(data.HEADING_GPS);
  memcpy(COMMS_BUFFER + length, &data.HEADING_GPS, varSize);
  length += varSize;
  COMMS_BUFFER[length] = ','; length++;

  varSize = sizeof(data.ALTITUDE_GPS);
  memcpy(COMMS_BUFFER + length, &data.ALTITUDE_GPS, varSize);
  length += varSize;
  COMMS_BUFFER[length] = ','; length++;

  varSize = sizeof(data.PRESS_BMP);
  memcpy(COMMS_BUFFER + length, &data.PRESS_BMP, varSize);
  length += varSize;
  COMMS_BUFFER[length] = ','; length++;

  varSize = sizeof(data.NUM_SATS_GPS);
  memcpy(COMMS_BUFFER + length, &data.NUM_SATS_GPS, varSize);
  length += varSize;
  COMMS_BUFFER[length] = ','; length++;

  varSize = sizeof(data.RB_SENT_COMMS);
  memcpy(COMMS_BUFFER + length, &data.RB_SENT_COMMS, varSize);
  length += varSize;
  COMMS_BUFFER[length] = ','; length++;

  varSize = sizeof(data.CUTDOWN_STATE);
  memcpy(COMMS_BUFFER + length, &data.CUTDOWN_STATE, varSize);
  length += varSize;
  data.COMMS_LENGTH = length;
  return length;
}
