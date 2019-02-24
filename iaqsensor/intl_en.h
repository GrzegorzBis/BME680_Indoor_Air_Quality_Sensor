﻿/*
 *	airRohr firmware
 *	Copyright (C) 2016-2018  Code for Stuttgart a.o.
 * 
 *  English translations
 * 
 *	Texts should be as short as possible
 *	We only have 512 kB for the entire firmware
 */

const char INTL_LANG[] = "EN";
const char INTL_PM_SENSOR[] PROGMEM = "IAQ sensor";
const char INTL_CONFIGURATION[] PROGMEM = "Configuration";
const char INTL_WIFI_SETTINGS[] PROGMEM = "WiFi Settings";
const char INTL_WIFI_NETWORKS[] PROGMEM = "Loading wifi networks ...";
const char INTL_LANGUAGE[] PROGMEM = "Language";
const char INTL_NO_NETWORKS[] PROGMEM =  "No WiFi Network Found";
const char INTL_NETWORKS_FOUND[] PROGMEM = "Found Networks: ";
const char INTL_AB_HIER_NUR_ANDERN[] PROGMEM = "Advanced settings (оnly if you know what you are doing)";
const char INTL_SAVE[] PROGMEM = "Save";
const char INTL_SENSORS[] PROGMEM = "Sensors";
const char INTL_BME680[] PROGMEM = "BME680 ({t}, {h}, {p}, {g})";
const char INTL_BASICAUTH[] PROGMEM = "Authorization";
const char INTL_FS_WIFI[] PROGMEM = "WiFi Sensor";
const char INTL_FS_WIFI_DESCRIPTION[] PROGMEM = "WiFi Sensor in configuration mode";
const char INTL_FS_WIFI_NAME[] PROGMEM = "Name";
const char INTL_MORE_SETTINGS[] PROGMEM ="More settings";
const char INTL_DISPLAY[] PROGMEM = "OLED SSD1306";
const char INTL_DEBUG_LEVEL[] PROGMEM = "Debug&nbsp;Level";
const char INTL_MEASUREMENT_INTERVAL[] PROGMEM = "Measuring interval";
const char INTL_DURATION_ROUTER_MODE[] PROGMEM = "Duration router mode";
const char INTL_MORE_APIS[] PROGMEM = "More APIs";
const char INTL_SERVER[] PROGMEM = "Server"; 
const char INTL_PATH[] PROGMEM = "Path"; 
const char INTL_PORT[] PROGMEM = "Port"; 
const char INTL_USER[] PROGMEM = "User"; 
const char INTL_PASSWORD[] PROGMEM = "Password"; 
const char INTL_SEND_TO[] PROGMEM = "Send to {v}"; 
const char INTL_READ_FROM[] PROGMEM = "Read from {v}";
const char INTL_SENSOR_IS_REBOOTING[] PROGMEM = "Sensor is rebooting.";
const char INTL_RESTART_DEVICE[] PROGMEM = "Restart device";
const char INTL_DELETE_CONFIG[] PROGMEM = "delete saved configuration";
const char INTL_RESTART_SENSOR[] PROGMEM = "Restart sensor";
const char INTL_HOME[] PROGMEM = "Home";
const char INTL_BACK_TO_HOME[] PROGMEM = "Back to home page";
const char INTL_CURRENT_DATA[] PROGMEM = "Current data";
const char INTL_CONFIGURATION_DELETE[] PROGMEM = "Configuration delete";
const char INTL_CONFIGURATION_REALLY_DELETE[] PROGMEM = "Are you sure you want to delete the configuration?";
const char INTL_DELETE[] PROGMEM = "Delete";
const char INTL_CANCEL[] PROGMEM = "Cancel";
const char INTL_REALLY_RESTART_SENSOR[] PROGMEM = "Are you sure you want to restart the sensor?";
const char INTL_RESTART[] PROGMEM = "Restart";
const char INTL_SAVE_AND_RESTART[] PROGMEM = "Save and restart";
const char INTL_FIRMWARE[] PROGMEM = "Firmware version";
const char INTL_DEBUG_SETTING_TO[] PROGMEM = "Debug setting to ";
const char INTL_NONE[] PROGMEM = "off";
const char INTL_ERROR[] PROGMEM = "only errors";
const char INTL_WARNING[] PROGMEM = "warnings";
const char INTL_MIN_INFO[] PROGMEM = "min. info";
const char INTL_MED_INFO[] PROGMEM = "mid. info";
const char INTL_MAX_INFO[] PROGMEM = "max. info";
const char INTL_CONFIG_DELETED[] PROGMEM = "Configuration is deleted";
const char INTL_CONFIG_CAN_NOT_BE_DELETED[] PROGMEM = "Configuration can not be deleted";
const char INTL_CONFIG_NOT_FOUND[] PROGMEM = "Configuration not found";
const char INTL_TIME_TO_FIRST_MEASUREMENT[] PROGMEM = "Still {v} seconds until first measurement.";
const char INTL_TIME_SINCE_LAST_MEASUREMENT[] PROGMEM = " seconds since last measurement.";
const char INTL_TEMPERATURE[] PROGMEM = "temperature";
const char INTL_HUMIDITY[] PROGMEM = "humidity";
const char INTL_PRESSURE[] PROGMEM = "air pressure";
const char INTL_GAS[] PROGMEM = "gas resistance";
const char INTL_DATE[] PROGMEM = "Date";
const char INTL_TIME[] PROGMEM = "Time";
const char INTL_SIGNAL_STRENGTH[] PROGMEM = "signal strength";
const char INTL_SIGNAL_QUALITY[] PROGMEM = "signal quality";
const char INTL_NUMBER_OF_MEASUREMENTS[] PROGMEM = "Number of measurements:";
const char INTL_SENSOR[] PROGMEM = "Sensor";
const char INTL_PARAMETER[] PROGMEM = "Parameter";
const char INTL_VALUE[] PROGMEM = "Value";
const char INTL_TS_API_KEY[] = "API Key";
const char INTL_TS_CHAN_NUM[] = "Channel Number";

const char LUFTDATEN_INFO_LOGO_SVG[] PROGMEM = "<svg id=\"Layer_1\" xmlns=\"http://www.w3.org/2000/svg\" x=\"0px\" y=\"0px\" width=\"100px\" height=\"88.891px\" viewBox=\"0 0 100 88.891\" enable-background=\"new 0 0 100 88.891\"><path fill=\"#fff\" d=\"M90.28 66.57v.1H83.33c-2.04 0-3.7-1.66-3.7-3.7 0-2.05 1.66-3.7 3.7-3.7h5.56c2.03 0 3.7-1.67 3.7-3.7 0-2.06-1.67-3.72-3.7-3.72s-3.72-1.66-3.72-3.7c0-2.05 1.66-3.7 3.7-3.7.26 0 .5.02.75.07 5.8.4 10.37 5.15 10.37 11.04 0 5.66-4.24 10.32-9.72 11z\"/><path fill=\"#fff\" d=\"M70.37 44.44c-2.04 0-3.7-1.65-3.7-3.7 0-2.04 1.66-3.7 3.7-3.7.26 0 .5.02.75.07 5.78.4 10.36 5.17 10.36 11.05 0 5.66-4.24 10.33-9.72 11v.1H29.63c-2.04 0-3.7 1.67-3.7 3.7 0 2.06 1.65 3.72 3.7 3.72h42.14v.1c5.47.68 9.7 5.35 9.7 11 0 5.9-4.57 10.65-10.35 11.05-.25.04-.5.07-.75.07-2.04 0-3.7-1.67-3.7-3.7 0-2.06 1.66-3.72 3.7-3.72s3.7-1.66 3.7-3.7c0-2.05-1.66-3.7-3.7-3.7H28.23v-.1c-5.47-.7-9.7-5.35-9.7-11.02 0-1.95.53-3.76 1.42-5.35C8.35 53.6 0 42.6 0 29.64 0 13.27 13.27 0 29.63 0c11.12 0 20.8 6.13 25.86 15.2 1.22-.22 2.47-.38 3.76-.38 10.92 0 19.98 7.88 21.85 18.26-2.57-1.84-5.64-3.05-8.97-3.36-2.55-4.48-7.35-7.5-12.87-7.5-2.96 0-5.7.9-8.03 2.4-2.28-9.85-11.06-17.2-21.6-17.2C17.36 7.4 7.4 17.35 7.4 29.6c0 12.28 9.96 22.23 22.23 22.23h40.74c2.04 0 3.7-1.66 3.7-3.7 0-2.05-1.65-3.7-3.7-3.7z\"/></svg>";
