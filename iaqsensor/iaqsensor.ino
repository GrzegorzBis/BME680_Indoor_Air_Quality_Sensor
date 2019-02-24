//#include <Arduino.h>
#define INTL_EN

/************************************************************************
 *                                                                      *
 *  This source code needs to be compiled for the board                 *
 *  WEMOS D1 mini pro                                                   *
 *                                                                      *
 ************************************************************************
 *                                                                      *
 *    airRohr firmware                                                  *
 *    Copyright (C) 2016-2018  Code for Stuttgart a.o.                  *
 *                                                                      *
 * This program is free software: you can redistribute it and/or modify *
 * it under the terms of the GNU General Public License as published by *
 * the Free Software Foundation, either version 3 of the License, or    *
 * (at your option) any later version.                                  *
 *                                                                      *
 * This program is distributed in the hope that it will be useful,      *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of       *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
 * GNU General Public License for more details.                         *
 *                                                                      *
 * You should have received a copy of the GNU General Public License    *
 * along with this program. If not, see <http://www.gnu.org/licenses/>. *
 ************************************************************************
 * Extensions connected via I2C:                                        *
 * BME680, OLED Display with SSD1306 (128x64 px)                        *
 *                                                                      *
 * Wiring Instruction                                                   *
 * (see labels on display or sensor board)                              *
 *      VCC       ->     Pin 3V3                                        *
 *      GND       ->     Pin GND                                        *
 *      SCL       ->     Pin D1(Wemos)/Pin D4(NodeMCU)                  *
 *      SDA       ->     Pin D2(Wemos)/Pin D4(NodeMCU)                  *
 ************************************************************************/
// increment on change
#define SOFTWARE_VERSION "GRB-2019-001"
/*****************************************************************
 * Includes                                                      *
 *****************************************************************/
#include <FS.h>                     // must be first
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266httpUpdate.h>
#include <SoftwareSerial.h>
#include "./oledfont.h"        // avoids including the default Arial font, needs to be included before SSD1306.h
#include <SSD1306.h>
#include <base64.h>
#include <ArduinoJson.h>
#include <Adafruit_BME680.h>
#include <time.h>
#include <coredecls.h>
#include <ThingSpeak.h>

#if defined(INTL_BG)
#include "intl_bg.h"
#elif defined(INTL_CZ)
#include "intl_cz.h"
#elif defined(INTL_EN)
#include "intl_en.h"
#elif defined(INTL_ES)
#include "intl_es.h"
#elif defined(INTL_FR)
#include "intl_fr.h"
#elif defined(INTL_IT)
#include "intl_it.h"
#elif defined(INTL_LU)
#include "intl_lu.h"
#elif defined(INTL_NL)
#include "intl_nl.h"
#elif defined(INTL_PL)
#include "intl_pl.h"
#elif defined(INTL_PT)
#include "intl_pt.h"
#elif defined(INTL_RU)
#include "intl_ru.h"
#elif defined(INTL_SE)
#include "intl_se.h"
#else
#include "intl_de.h"
#endif
#include "ext_def.h"
#include "html-content.h"

/******************************************************************
 * Constants                                                      *
 ******************************************************************/
const unsigned long DISPLAY_UPDATE_INTERVAL_MS = 5000;
const unsigned long ONE_DAY_IN_MS = 24 * 60 * 60 * 1000;
const unsigned long DURATION_BEFORE_FORCED_RESTART_MS = ONE_DAY_IN_MS * 28;  // force a reboot every ~4 weeks

/******************************************************************
 * The variables inside the cfg namespace are persistent          *
 * configuration values. They have defaults which can be          *
 * configured at compile-time via the ext_def.h file              *
 * They can be changed by the user via the web interface, the     *
 * changes are persisted to the flash and read back after reboot. *
 * Note that the names of these variables can't be easily changed *
 * as they are part of the json format used to persist the data.  *
 ******************************************************************/
namespace cfg {
  char wlanssid[35] = WLANSSID;
  char wlanpwd[65] = WLANPWD;

  char current_lang[3] = "EN";
  char www_username[65] = WWW_USERNAME;
  char www_password[65] = WWW_PASSWORD;
  bool www_basicauth_enabled = WWW_BASICAUTH_ENABLED;

  char fs_ssid[33] = FS_SSID;
  char fs_pwd[65] = FS_PWD;

  char version_from_local_config[20] = "";
  bool bme680_read = BME680_READ;
  bool send2thingspeak = SEND2TSPK;
  bool has_display = HAS_DISPLAY;
  int  debug = DEBUG;

  unsigned long time_for_wifi_config = 600000;
  unsigned long sending_intervall_ms = 145000;

  // ThingSpeak
  char myWriteAPIKey[65];
  unsigned long myChannelNumber;

  void initNonTrivials(const char* id) {
    strcpy(cfg::current_lang, CURRENT_LANG);
    if (fs_ssid[0] == '\0') {
      strcpy(fs_ssid, "IAQsensor-");
      strcat(fs_ssid, id);
    }
  }
}

#define JSON_BUFFER_SIZE 2000

WiFiClient  client;

long int sample_count = 0;
bool bme680_init_failed = false;

ESP8266WebServer server(80);
int TimeZone = 1;

/*****************************************************************
 * Display definitions                                           *
 *****************************************************************/
SSD1306 display(0x3c, I2C_PIN_SDA, I2C_PIN_SCL);
/*****************************************************************
 * BME680 declaration                                            *
 *****************************************************************/
Adafruit_BME680 bme680;
bool bme680_read_avail = false;
bool send_now = false;
unsigned long starttime;
unsigned long time_point_device_start_ms;
unsigned long act_micro;
unsigned long act_milli;
unsigned long last_micro = 0;
unsigned long min_micro = 1000000000;
unsigned long max_micro = 0;
unsigned long sending_time = 0;
unsigned long last_update_attempt;

float last_value_BME680_T = -128.0;
float last_value_BME680_H = -1.0;
float last_value_BME680_P = -1.0;
float last_value_BME680_G = -128.0;
String last_data_string = "";

String esp_chipid;

long last_page_load = millis();

bool wificonfig_loop = false;

bool first_cycle = true;

bool sntp_time_is_set = false;

bool got_ntp = false;

unsigned long count_sends = 0;
unsigned long next_display_millis = 0;
unsigned long next_display_count = 0;

struct struct_wifiInfo {
  char ssid[35];
  uint8_t encryptionType;
  int32_t RSSI;
  int32_t channel;
  bool isHidden;
};

struct struct_wifiInfo *wifiInfo;
uint8_t count_wifiInfo;

template<typename T, std::size_t N> constexpr std::size_t array_num_elements(const T(&)[N]) {
  return N;
}

template<typename T, std::size_t N> constexpr std::size_t capacity_null_terminated_char_array(const T(&)[N]) {
  return N - 1;
}

#define msSince(timestamp_before) (act_milli - (timestamp_before))

const char data_first_part[] PROGMEM = "{\"software_version\": \"{v}\", \"sensordatavalues\":[";

/*****************************************************************
 * Debug output                                                  *
 *****************************************************************/
void debug_out(const String& text, const int level, const bool linebreak) {
  if (level <= cfg::debug) {
    if (linebreak) {
      Serial.println(text);
    } else {
      Serial.print(text);
    }
  }
}

/*****************************************************************
 * display values                                                *
 *****************************************************************/
void display_debug(const String& text1, const String& text2) {
  debug_out(F("output debug text to displays..."), DEBUG_MIN_INFO, 1);
  debug_out(text1 + "\n" + text2, DEBUG_MAX_INFO, 1);
  if (cfg::has_display) {
    display.clear();
    display.displayOn();
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.drawString(0, 12, text1);
    display.drawString(0, 24, text2);
    display.display();
  }
}

/*****************************************************************
 * check display values, return '-' if undefined                 *
 *****************************************************************/
String check_display_value(double value, double undef, uint8_t len, uint8_t str_len) {
  String s = (value != undef ? Float2String(value, len) : "-");
  while (s.length() < str_len) {
    s = " " + s;
  }
  return s;
}

/*****************************************************************
 * convert float to string with a                                *
 * precision of two (or a given number of) decimal places        *
 *****************************************************************/
String Float2String(const double value) {
  return Float2String(value, 2);
}

String Float2String(const double value, uint8_t digits) {
  // Convert a float to String with two decimals.
  char temp[15];

  dtostrf(value, 13, digits, temp);
  String s = temp;
  s.trim();
  return s;
}

/*****************************************************************
 * convert value to json string                                  *
 *****************************************************************/
String Value2Json(const String& type, const String& value) {
  String s = F("{\"value_type\":\"{t}\",\"value\":\"{v}\"},");
  s.replace("{t}", type);
  s.replace("{v}", value);
  return s;
}

/*****************************************************************
 * convert string value to json string                           *
 *****************************************************************/
String Var2Json(const String& name, const String& value) {
  String s = F("\"{n}\":\"{v}\",");
  String tmp = value;
  tmp.replace("\\", "\\\\"); tmp.replace("\"", "\\\"");
  s.replace("{n}", name);
  s.replace("{v}", tmp);
  return s;
}

/*****************************************************************
 * convert boolean value to json string                          *
 *****************************************************************/
String Var2Json(const String& name, const bool value) {
  String s = F("\"{n}\":\"{v}\",");
  s.replace("{n}", name);
  s.replace("{v}", (value ? "true" : "false"));
  return s;
}

/*****************************************************************
 * convert int value to json string                          *
 *****************************************************************/
String Var2Json(const String& name, const int value) {
  String s = F("\"{n}\":\"{v}\",");
  s.replace("{n}", name);
  s.replace("{v}", String(value));
  return s;
}

/*****************************************************************
 * convert unsigned long value to json string                          *
 *****************************************************************/
String Var2Json(const String& name, const unsigned long value) {
  String s = F("\"{n}\":\"{v}\",");
  s.replace("{n}", name);
  s.replace("{v}", String(value));
  return s;
}

/*****************************************************************
 * read config from spiffs                                       *
 *****************************************************************/
void readConfig() {
  using namespace cfg;
  String json_string = "";
  debug_out(F("mounting FS..."), DEBUG_MIN_INFO, 1);
  bool pms24_read = 0;
  bool pms32_read = 0;

  if (SPIFFS.begin()) {
    debug_out(F("mounted file system..."), DEBUG_MIN_INFO, 1);
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      debug_out(F("reading config file..."), DEBUG_MIN_INFO, 1);
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        debug_out(F("opened config file..."), DEBUG_MIN_INFO, 1);
        const size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        StaticJsonBuffer<JSON_BUFFER_SIZE> jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(json_string);
        debug_out(F("File content: "), DEBUG_MAX_INFO, 0);
        debug_out(String(buf.get()), DEBUG_MAX_INFO, 1);
        debug_out(F("JSON Buffer content: "), DEBUG_MAX_INFO, 0);
        debug_out(json_string, DEBUG_MAX_INFO, 1);
        if (json.success()) {
          debug_out(F("parsed json..."), DEBUG_MIN_INFO, 1);
          if (json.containsKey("SOFTWARE_VERSION")) {
            strcpy(version_from_local_config, json["SOFTWARE_VERSION"]);
          }

#define setFromJSON(key)    if (json.containsKey(#key)) key = json[#key];
#define strcpyFromJSON(key) if (json.containsKey(#key)) strcpy(key, json[#key]);
          strcpyFromJSON(current_lang);
          strcpyFromJSON(wlanssid);
          strcpyFromJSON(wlanpwd);
          strcpyFromJSON(www_username);
          strcpyFromJSON(www_password);
          strcpyFromJSON(fs_ssid);
          strcpyFromJSON(fs_pwd);
          strcpyFromJSON(myWriteAPIKey);
          setFromJSON(www_basicauth_enabled);
          setFromJSON(bme680_read);
          setFromJSON(send2thingspeak);
          setFromJSON(has_display);
          setFromJSON(debug);
          setFromJSON(sending_intervall_ms);
          setFromJSON(time_for_wifi_config);
          setFromJSON(myChannelNumber);
          configFile.close();
#undef setFromJSON
#undef strcpyFromJSON
        } else {
          debug_out(F("failed to load json config"), DEBUG_ERROR, 1);
        }
      }
    } else {
      debug_out(F("config file not found ..."), DEBUG_ERROR, 1);
    }
  } else {
    debug_out(F("failed to mount FS"), DEBUG_ERROR, 1);
  }
}

/*****************************************************************
 * write config to spiffs                                        *
 *****************************************************************/
void writeConfig() {
  using namespace cfg;
  String json_string = "{";
  debug_out(F("saving config..."), DEBUG_MIN_INFO, 1);

#define copyToJSON_Bool(varname) json_string += Var2Json(#varname,varname);
#define copyToJSON_Int(varname) json_string += Var2Json(#varname,varname);
#define copyToJSON_Ulong(varname) json_string += Var2Json(#varname,varname);
#define copyToJSON_String(varname) json_string += Var2Json(#varname,String(varname));
  copyToJSON_String(current_lang);
  copyToJSON_String(SOFTWARE_VERSION);
  copyToJSON_String(wlanssid);
  copyToJSON_String(wlanpwd);
  copyToJSON_String(www_username);
  copyToJSON_String(www_password);
  copyToJSON_String(fs_ssid);
  copyToJSON_String(fs_pwd);
  copyToJSON_Bool(www_basicauth_enabled);
  copyToJSON_Bool(bme680_read);
  copyToJSON_Bool(send2thingspeak);
  copyToJSON_Bool(has_display);
  copyToJSON_String(debug);
  copyToJSON_String(sending_intervall_ms);
  copyToJSON_String(time_for_wifi_config);
  copyToJSON_String(myWriteAPIKey);
  copyToJSON_Int(myChannelNumber);
#undef copyToJSON_Bool
#undef copyToJSON_Int
#undef copyToJSON_Ulong
#undef copyToJSON_String

  json_string.remove(json_string.length() - 1);
  json_string += "}";

  debug_out(json_string, DEBUG_MIN_INFO, 1);
  File configFile = SPIFFS.open("/config.json", "w");
  if (configFile) {
    configFile.print(json_string);
    debug_out(F("Config written: "), DEBUG_MIN_INFO, 0);
    debug_out(json_string, DEBUG_MIN_INFO, 1);
    configFile.close();
  } else {
    debug_out(F("failed to open config file for writing"), DEBUG_ERROR, 1);
  }
}

/*****************************************************************
 * html helper functions                                         *
 *****************************************************************/

String make_header(const String& title) {
  String s = FPSTR(WEB_PAGE_HEADER);
  s.replace("{tt}", FPSTR(INTL_PM_SENSOR));
  s.replace("{h}", FPSTR(INTL_HOME));
  if(title != " ") {
    s.replace("{n}", F("&raquo;"));
  } else {
    s.replace("{n}", "");
  }
  s.replace("{t}", title);
  s.replace("{id}", esp_chipid);
  s.replace("{mac}", WiFi.macAddress());
  s.replace("{fwt}", FPSTR(INTL_FIRMWARE));
  s.replace("{fw}", SOFTWARE_VERSION);
  return s;
}

String make_footer() {
  String s = FPSTR(WEB_PAGE_FOOTER);
  s.replace("{t}", FPSTR(INTL_BACK_TO_HOME));
  return s;
}

String form_input(const String& name, const String& info, const String& value, const int length) {
  String s = F( "<tr>"
          "<td>{i} </td>"
          "<td style='width:90%;'>"
          "<input type='text' name='{n}' id='{n}' placeholder='{i}' value='{v}' maxlength='{l}'/>"
          "</td>"
          "</tr>");
  String t_value = value;
  t_value.replace("'", "&#39;");
  s.replace("{i}", info);
  s.replace("{n}", name);
  s.replace("{v}", t_value);
  s.replace("{l}", String(length));
  return s;
}

String form_password(const String& name, const String& info, const String& value, const int length) {
  String s = F( "<tr>"
          "<td>{i} </td>"
          "<td style='width:90%;'>"
          "<input type='password' name='{n}' id='{n}' placeholder='{i}' value='{v}' maxlength='{l}'/>"
          "</td>"
          "</tr>");
  String password = "";
  for (uint8_t i = 0; i < value.length(); i++) {
    password += "*";
  }
  s.replace("{i}", info);
  s.replace("{n}", name);
  s.replace("{v}", password);
  s.replace("{l}", String(length));
  return s;
}

String form_checkbox(const String& name, const String& info, const bool checked, const bool linebreak = true) {
  String s = F("<label for='{n}'><input type='checkbox' name='{n}' value='1' id='{n}' {c}/> {i}</label><br/>");
  if (checked) {
    s.replace("{c}", F(" checked='checked'"));
  } else {
    s.replace("{c}", "");
  };
  s.replace("{i}", info);
  s.replace("{n}", name);
  if (! linebreak) {
    s.replace("<br/>", "");
  }
  return s;
}

String form_checkbox_sensor(const String& name, const String& info, const bool checked) {
  return form_checkbox(name, add_sensor_type(info), checked);
}

String form_submit(const String& value) {
  String s = F( "<tr>"
          "<td>&nbsp;</td>"
          "<td>"
          "<input type='submit' name='submit' value='{v}' />"
          "</td>"
          "</tr>");
  s.replace("{v}", value);
  return s;
}

String form_select_lang() {
  String s_select = F(" selected='selected'");
  String s = F( "<tr>"
          "<td>{t}</td>"
          "<td>"
          "<select name='current_lang'>"
          "<option value='DE'{s_DE}>Deutsch (DE)</option>"
          "<option value='BG'{s_BG}>Bulgarian (BG)</option>"
          "<option value='CZ'{s_CZ}>Český (CZ)</option>"
          "<option value='EN'{s_EN}>English (EN)</option>"
          "<option value='ES'{s_ES}>Español (ES)</option>"
          "<option value='FR'{s_FR}>Français (FR)</option>"
          "<option value='IT'{s_IT}>Italiano (IT)</option>"
          "<option value='LU'{s_LU}>Lëtzebuergesch (LU)</option>"
          "<option value='NL'{s_NL}>Nederlands (NL)</option>"
          "<option value='PL'{s_PL}>Polski (PL)</option>"
          "<option value='PT'{s_PT}>Português (PT)</option>"
          "<option value='RU'{s_RU}>Русский (RU)</option>"
          "<option value='SE'{s_SE}>Svenska (SE)</option>"
          "</select>"
          "</td>"
          "</tr>");

  s.replace("{t}", FPSTR(INTL_LANGUAGE));

  s.replace("{s_" + String(cfg::current_lang) + "}", s_select);
  while (s.indexOf("{s_") != -1) {
    s.remove(s.indexOf("{s_"), 6);
  }
  return s;
}

static String tmpl(const String& patt, const String& value) {
  String s = patt;
  s.replace("{v}", value);
  return s;
}

static String tmpl(const String& patt, const String& value1, const String& value2) {
  String s = patt;
  s.replace("{v1}", value1);
  s.replace("{v2}", value2);
  return s;
}

static String tmpl(const String& patt, const String& value1, const String& value2, const String& value3) {
  String s = patt;
  s.replace("{v1}", value1);
  s.replace("{v2}", value2);
  s.replace("{v3}", value3);
  return s;
}

String line_from_value(const String& name, const String& value) {
  String s = F("<br/>{n}: {v}");
  s.replace("{n}", name);
  s.replace("{v}", value);
  return s;
}

String table_row_from_value(const String& sensor, const String& param, const String& value, const String& unit) {
  String s = F( "<tr>"
          "<td>{s}</td>"
          "<td>{p}</td>"
          "<td class='r'>{v}&nbsp;{u}</td>"
          "</tr>");
  s.replace("{s}", sensor);
  s.replace("{p}", param);
  s.replace("{v}", value);
  s.replace("{u}", unit);
  return s;
}

static int32_t calcWiFiSignalQuality(int32_t rssi) {
  if (rssi > -50) {
    rssi = -50;
  }
  if (rssi < -100) {
    rssi = -100;
  }
  return (rssi + 100) * 2;
}

String wlan_ssid_to_table_row(const String& ssid, const String& encryption, int32_t rssi) {
  String s = F( "<tr>"
          "<td>"
          "<a href='#wlanpwd' onclick='setSSID(this)' class='wifi'>{n}</a>&nbsp;{e}"
          "</td>"
          "<td style='width:80%;vertical-align:middle;'>"
          "{v}%"
          "</td>"
          "</tr>");
  s.replace("{n}", ssid);
  s.replace("{e}", encryption);
  s.replace("{v}", String(calcWiFiSignalQuality(rssi)));
  return s;
}

String warning_first_cycle() {
  String s = FPSTR(INTL_TIME_TO_FIRST_MEASUREMENT);
  unsigned long time_to_first = cfg::sending_intervall_ms - msSince(starttime);
  if (time_to_first > cfg::sending_intervall_ms) {
    time_to_first = 0;
  }
  s.replace("{v}", String((long)((time_to_first + 500) / 1000)));
  return s;
}

String age_last_values() {
  String s = "<b>";
  unsigned long time_since_last = msSince(starttime);
  if (time_since_last > cfg::sending_intervall_ms) {
    time_since_last = 0;
  }
  s += String((long)((time_since_last + 500) / 1000));
  s += FPSTR(INTL_TIME_SINCE_LAST_MEASUREMENT);
  s += F("</b><br/><br/>");
  return s;
}

String add_sensor_type(const String& sensor_text) {
  String s = sensor_text;
  s.replace("{t}", FPSTR(INTL_TEMPERATURE));
  s.replace("{h}", FPSTR(INTL_HUMIDITY));
  s.replace("{p}", FPSTR(INTL_PRESSURE));
  s.replace("{g}", FPSTR(INTL_GAS));
  return s;
}

/*****************************************************************
 * Webserver request auth: prompt for BasicAuth
 *
 * -Provide BasicAuth for all page contexts except /values and images
 *****************************************************************/
static bool webserver_request_auth() {
  debug_out(F("validate request auth..."), DEBUG_MIN_INFO, 1);
  if (cfg::www_basicauth_enabled && ! wificonfig_loop) {
    if (!server.authenticate(cfg::www_username, cfg::www_password)) {
      server.requestAuthentication();
      return false;
    }
  }
  return true;
}

static void sendHttpRedirect(ESP8266WebServer &httpServer) {
  httpServer.sendHeader(F("Location"), F("http://192.168.4.1/config"));
  httpServer.send(302, FPSTR(TXT_CONTENT_TYPE_TEXT_HTML), "");
}

/*****************************************************************
 * Webserver root: show all options                              *
 *****************************************************************/
void webserver_root() {
  if (WiFi.status() != WL_CONNECTED) {
    sendHttpRedirect(server);
  } else {
    if (!webserver_request_auth())
    { return; }

    String page_content = make_header(" ");
    last_page_load = millis();
    debug_out(F("output root page..."), DEBUG_MIN_INFO, 1);
    page_content += FPSTR(WEB_ROOT_PAGE_CONTENT);
    page_content.replace("{t}", FPSTR(INTL_CURRENT_DATA));
    page_content.replace(F("{conf}"), FPSTR(INTL_CONFIGURATION));
    page_content.replace(F("{conf_delete}"), FPSTR(INTL_CONFIGURATION_DELETE));
    page_content.replace(F("{restart}"), FPSTR(INTL_RESTART_SENSOR));
    page_content += make_footer();
    server.send(200, FPSTR(TXT_CONTENT_TYPE_TEXT_HTML), page_content);
  }
}

static int constexpr constexprstrlen(const char* str) {
  return *str ? 1 + constexprstrlen(str + 1) : 0;
}

/*****************************************************************
 * Webserver config: show config page                            *
 *****************************************************************/
void webserver_config() {
  if (!webserver_request_auth())
  { return; }

  String page_content = make_header(FPSTR(INTL_CONFIGURATION));
  String masked_pwd = "";
  last_page_load = millis();

  debug_out(F("output config page ..."), DEBUG_MIN_INFO, 1);
  if (wificonfig_loop) {  // scan for wlan ssids
    page_content += FPSTR(WEB_CONFIG_SCRIPT);
  }
  using namespace cfg;
  if (server.method() == HTTP_GET) {
    page_content += F("<form method='POST' action='/config' style='width:100%;'>\n<b>");
    page_content += FPSTR(INTL_WIFI_SETTINGS);
    page_content += F("</b><br/>");
    debug_out(F("output config page 1"), DEBUG_MIN_INFO, 1);
    if (wificonfig_loop) {  // scan for wlan ssids
      page_content += F("<div id='wifilist'>");
      page_content += FPSTR(INTL_WIFI_NETWORKS);
      page_content += F("</div><br/>");
    }
    page_content += FPSTR(TABLE_TAG_OPEN);
    page_content += form_input("wlanssid", FPSTR(INTL_FS_WIFI_NAME), wlanssid, capacity_null_terminated_char_array(wlanssid));
    page_content += form_password("wlanpwd", FPSTR(INTL_PASSWORD), wlanpwd, capacity_null_terminated_char_array(wlanpwd));
    page_content += FPSTR(TABLE_TAG_CLOSE_BR);
    page_content += F("<hr/>\n<b>");

    page_content += FPSTR(INTL_AB_HIER_NUR_ANDERN);
    page_content += F("</b><br/><br/>\n<b>");
    if (! wificonfig_loop) {
      page_content += FPSTR(INTL_BASICAUTH);
      page_content += F("</b><br/>");
      page_content += FPSTR(TABLE_TAG_OPEN);
      page_content += form_input("www_username", FPSTR(INTL_USER), www_username, capacity_null_terminated_char_array(www_username));
      page_content += form_password("www_password", FPSTR(INTL_PASSWORD), www_password, capacity_null_terminated_char_array(www_password));
      page_content += form_checkbox("www_basicauth_enabled", FPSTR(INTL_BASICAUTH), www_basicauth_enabled);

      page_content += FPSTR(TABLE_TAG_CLOSE_BR);
      page_content += F("\n<b>");
      page_content += FPSTR(INTL_FS_WIFI);
      page_content += F("</b><br/>");
      page_content += FPSTR(INTL_FS_WIFI_DESCRIPTION);
      page_content += FPSTR(BR_TAG);
      page_content += FPSTR(TABLE_TAG_OPEN);
      page_content += form_input("fs_ssid", FPSTR(INTL_FS_WIFI_NAME), fs_ssid, capacity_null_terminated_char_array(fs_ssid));
      page_content += form_password("fs_pwd", FPSTR(INTL_PASSWORD), fs_pwd, capacity_null_terminated_char_array(fs_pwd));
      page_content += FPSTR(TABLE_TAG_CLOSE_BR);
      
      page_content += F("\n<b>APIs</b><br/>");
      page_content += form_checkbox("send2thingspeak", F("API thingspeak.com"), send2thingspeak, false);
      page_content += F("<br/>");
      page_content += FPSTR(TABLE_TAG_OPEN);
      page_content += form_input("myWriteAPIKey", FPSTR(INTL_TS_API_KEY), myWriteAPIKey, capacity_null_terminated_char_array(myWriteAPIKey));
      page_content += form_input("myChannelNumber", FPSTR(INTL_TS_CHAN_NUM), String(myChannelNumber), 1);      
      page_content += FPSTR(TABLE_TAG_CLOSE_BR);
      page_content += F("<br/>");
     
      page_content += FPSTR(INTL_SENSORS);
      page_content += F("</b><br/>");
      page_content += form_checkbox_sensor("bme680_read", FPSTR(INTL_BME680), bme680_read);
      page_content += F("<br/><br/>\n<b>");
    }
    page_content += FPSTR(INTL_MORE_SETTINGS);
    page_content += F("</b><br/>");
    page_content += form_checkbox("has_display", FPSTR(INTL_DISPLAY), has_display);

    if (! wificonfig_loop) {
      page_content += FPSTR(TABLE_TAG_OPEN);
      page_content += form_select_lang();
      page_content += form_input("debug", FPSTR(INTL_DEBUG_LEVEL), String(debug), 1);
      page_content += form_input("sending_intervall_ms", FPSTR(INTL_MEASUREMENT_INTERVAL), String(sending_intervall_ms / 1000), 5);
      page_content += form_input("time_for_wifi_config", FPSTR(INTL_DURATION_ROUTER_MODE), String(time_for_wifi_config / 1000), 5);
      page_content += FPSTR(TABLE_TAG_CLOSE_BR);
      page_content += F("\n<b>");

      page_content += form_submit(FPSTR(INTL_SAVE_AND_RESTART));
      page_content += FPSTR(TABLE_TAG_CLOSE_BR);
      page_content += F("<br/></form>");
    }

    if (wificonfig_loop) {  // scan for wlan ssids
      page_content += FPSTR(TABLE_TAG_OPEN);
      page_content += form_submit(FPSTR(INTL_SAVE_AND_RESTART));
      page_content += FPSTR(TABLE_TAG_CLOSE_BR);
      page_content += F("<br/></form>");
      page_content += F("<script>window.setTimeout(load_wifi_list,1000);</script>");
    }
  } else {
#define readCharParam(param) \
    if (server.hasArg(#param)){ \
      server.arg(#param).toCharArray(param, sizeof(param)); \
    }

#define readBoolParam(param) \
    param = false; \
    if (server.hasArg(#param)){ \
      param = server.arg(#param) == "1";\
    }

#define readIntParam(param) \
    if (server.hasArg(#param)){ \
      int val = server.arg(#param).toInt(); \
      if (val != 0){ \
        param = val; \
      } \
    }

#define readTimeParam(param) \
    if (server.hasArg(#param)){ \
      int val = server.arg(#param).toInt(); \
      param = val*1000; \
    }

#define readPasswdParam(param) \
    if (server.hasArg(#param)){ \
      masked_pwd = ""; \
      for (uint8_t i=0;i<server.arg(#param).length();i++) \
        masked_pwd += "*"; \
      if (masked_pwd != server.arg(#param) || server.arg(#param) == "") {\
        server.arg(#param).toCharArray(param, sizeof(param)); \
      }\
    }
    if (server.hasArg("wlanssid") && server.arg("wlanssid") != "") {
      readCharParam(wlanssid);
      readPasswdParam(wlanpwd);
    }
    if (! wificonfig_loop) {
      readCharParam(current_lang);
      readCharParam(www_username);
      readPasswdParam(www_password);
      readBoolParam(www_basicauth_enabled);
      readCharParam(fs_ssid);
      if (server.hasArg("fs_pwd") && ((server.arg("fs_pwd").length() > 7) || (server.arg("fs_pwd").length() == 0))) {
        readPasswdParam(fs_pwd);
      }
      readBoolParam(send2thingspeak);
      readBoolParam(bme680_read);

      readIntParam(debug);
      readTimeParam(sending_intervall_ms);
      readTimeParam(time_for_wifi_config);
    }

    readBoolParam(has_display);

#undef readCharParam
#undef readBoolParam
#undef readIntParam
#undef readTimeParam
#undef readPasswdParam

    page_content += line_from_value(tmpl(FPSTR(INTL_SEND_TO), F("thingspeak.com")), String(send2thingspeak));
    page_content += line_from_value(tmpl(FPSTR(INTL_READ_FROM), "BME680"), String(bme680_read));
    page_content += line_from_value(FPSTR(INTL_DISPLAY), String(has_display));
    page_content += line_from_value(FPSTR(INTL_DEBUG_LEVEL), String(debug));
    page_content += line_from_value(FPSTR(INTL_MEASUREMENT_INTERVAL), String(sending_intervall_ms));
    page_content += F("<br/><br/>");
    page_content += FPSTR(INTL_SENSOR_IS_REBOOTING);
  }
  page_content += make_footer();

  server.sendHeader(F("Cache-Control"), F("no-cache, no-store, must-revalidate"));

  server.sendHeader(F("Pragma"), F("no-cache"));

  server.sendHeader(F("Expires"), F("0"));
  server.send(200, FPSTR(TXT_CONTENT_TYPE_TEXT_HTML), page_content);

  if (server.method() == HTTP_POST) {
    display_debug(F("Writing config"), F("and restarting"));
    writeConfig();
    delay(500);
    ESP.restart();
  }
}

/*****************************************************************
 * Webserver wifi: show available wifi networks                  *
 *****************************************************************/
void webserver_wifi() {
  debug_out(F("wifi networks found: "), DEBUG_MIN_INFO, 0);
  debug_out(String(count_wifiInfo), DEBUG_MIN_INFO, 1);
  String page_content = "";
  if (count_wifiInfo == 0) {
    page_content += BR_TAG;
    page_content += FPSTR(INTL_NO_NETWORKS);
    page_content += BR_TAG;
  } else {
    std::unique_ptr<int[]> indices(new int[count_wifiInfo]);
    debug_out(F("output config page 2"), DEBUG_MIN_INFO, 1);
    for (int i = 0; i < count_wifiInfo; i++) {
      indices[i] = i;
    }
    for (int i = 0; i < count_wifiInfo; i++) {
      for (int j = i + 1; j < count_wifiInfo; j++) {
        if (wifiInfo[indices[j]].RSSI > wifiInfo[indices[i]].RSSI) {
          std::swap(indices[i], indices[j]);
        }
      }
    }
    debug_out(F("output config page 3"), DEBUG_MIN_INFO, 1);
    int duplicateSsids = 0;
    for (int i = 0; i < count_wifiInfo; i++) {
      if (indices[i] == -1) {
        continue;
      }
      for (int j = i + 1; j < count_wifiInfo; j++) {
        if (strncmp(wifiInfo[indices[i]].ssid, wifiInfo[indices[j]].ssid, 35) == 0) {
          indices[j] = -1; // set dup aps to index -1
          ++duplicateSsids;
        }
      }
    }

    page_content += FPSTR(INTL_NETWORKS_FOUND);
    page_content += String(count_wifiInfo - duplicateSsids);
    page_content += FPSTR(BR_TAG);
    page_content += FPSTR(BR_TAG);
    page_content += FPSTR(TABLE_TAG_OPEN);
    //if(n > 30) n=30;
    for (int i = 0; i < count_wifiInfo; ++i) {
      if (indices[i] == -1 || wifiInfo[indices[i]].isHidden) {
        continue;
      }
      // Print SSID and RSSI for each network found
      page_content += wlan_ssid_to_table_row(wifiInfo[indices[i]].ssid, ((wifiInfo[indices[i]].encryptionType == ENC_TYPE_NONE) ? " " : u8"🔒"), wifiInfo[indices[i]].RSSI);
    }
    page_content += FPSTR(TABLE_TAG_CLOSE_BR);
    page_content += FPSTR(BR_TAG);
  }
  server.send(200, FPSTR(TXT_CONTENT_TYPE_TEXT_HTML), page_content);
}

/*****************************************************************
 * Webserver root: show latest values                            *
 *****************************************************************/
void webserver_values() {
  if (WiFi.status() != WL_CONNECTED) {
    sendHttpRedirect(server);
  } else {
    String page_content = make_header(FPSTR(INTL_CURRENT_DATA));
    const String unit_T = "°C";
    const String unit_H = "%";
    const String unit_P = "hPa";
    const String unit_G = "KOhms";
    last_page_load = millis();

    const int signal_quality = calcWiFiSignalQuality(WiFi.RSSI());
    debug_out(F("output values to web page..."), DEBUG_MIN_INFO, 1);
    if (first_cycle) {
      page_content += F("<b style='color:red'>");
      page_content += warning_first_cycle();
      page_content += F("</b><br/><br/>");
    } else {
      page_content += age_last_values();
    }
    page_content += F("<table cellspacing='0' border='1' cellpadding='5'>");
    page_content += tmpl(F("<tr><th>{v1}</th><th>{v2}</th><th>{v3}</th>"), FPSTR(INTL_SENSOR), FPSTR(INTL_PARAMETER), FPSTR(INTL_VALUE));
    if (cfg::bme680_read) {
      page_content += FPSTR(EMPTY_ROW);
      page_content += table_row_from_value(FPSTR(SENSORS_BME680), FPSTR(INTL_TEMPERATURE), check_display_value(last_value_BME680_T, -128, 1, 0), unit_T);
      page_content += table_row_from_value(FPSTR(SENSORS_BME680), FPSTR(INTL_HUMIDITY), check_display_value(last_value_BME680_H, -1, 1, 0), unit_H);
      page_content += table_row_from_value(FPSTR(SENSORS_BME680), FPSTR(INTL_PRESSURE),  check_display_value(last_value_BME680_P / 100.0, (-1 / 100.0), 2, 0), unit_P);
      page_content += table_row_from_value(FPSTR(SENSORS_BME680), FPSTR(INTL_GAS), check_display_value(last_value_BME680_G, -128, 1, 0), unit_G);
    }

    page_content += FPSTR(EMPTY_ROW);
    page_content += table_row_from_value("WiFi", FPSTR(INTL_SIGNAL_STRENGTH),  String(WiFi.RSSI()), "dBm");
    page_content += table_row_from_value("WiFi", FPSTR(INTL_SIGNAL_QUALITY), String(signal_quality), "%");

    page_content += FPSTR(EMPTY_ROW);
    page_content += F("<tr><td colspan='2'>");
    page_content += FPSTR(INTL_NUMBER_OF_MEASUREMENTS);
    page_content += F("</td><td class='r'>");
    page_content += String(count_sends);
    page_content += F("</td></tr>");
    page_content += FPSTR(TABLE_TAG_CLOSE_BR);
    page_content += make_footer();
    server.send(200, FPSTR(TXT_CONTENT_TYPE_TEXT_HTML), page_content);
  }
}

/*****************************************************************
 * Webserver set debug level                                     *
 *****************************************************************/
void webserver_debug_level() {
  if (!webserver_request_auth())
  { return; }

  String page_content = make_header(FPSTR(INTL_DEBUG_LEVEL));
  last_page_load = millis();
  debug_out(F("output change debug level page..."), DEBUG_MIN_INFO, 1);

  if (server.hasArg("lvl")) {
    const int lvl = server.arg("lvl").toInt();
    if (lvl >= 0 && lvl <= 5) {
      cfg::debug = lvl;
      page_content += F("<h3>");
      page_content += FPSTR(INTL_DEBUG_SETTING_TO);
      page_content += F(" ");

      static constexpr std::array<const char *, 6> lvlText PROGMEM = {
        INTL_NONE, INTL_ERROR, INTL_WARNING, INTL_MIN_INFO, INTL_MED_INFO, INTL_MAX_INFO
      };

      page_content += FPSTR(lvlText[lvl]);
      page_content += F(".</h3>");
    }
  }
  page_content += make_footer();
  server.send(200, FPSTR(TXT_CONTENT_TYPE_TEXT_HTML), page_content);
}

/*****************************************************************
 * Webserver remove config                                       *
 *****************************************************************/
void webserver_removeConfig() {
  if (!webserver_request_auth())
  { return; }

  String page_content = make_header(FPSTR(INTL_DELETE_CONFIG));
  String message_string = F("<h3>{v}.</h3>");
  last_page_load = millis();
  debug_out(F("output remove config page..."), DEBUG_MIN_INFO, 1);

  if (server.method() == HTTP_GET) {
    page_content += FPSTR(WEB_REMOVE_CONFIG_CONTENT);
    page_content.replace("{t}", FPSTR(INTL_CONFIGURATION_REALLY_DELETE));
    page_content.replace("{b}", FPSTR(INTL_DELETE));
    page_content.replace("{c}", FPSTR(INTL_CANCEL));

  } else {
    if (SPIFFS.exists("/config.json")) {  //file exists
      debug_out(F("removing config.json..."), DEBUG_MIN_INFO, 1);
      if (SPIFFS.remove("/config.json")) {
        page_content += tmpl(message_string, FPSTR(INTL_CONFIG_DELETED));
      } else {
        page_content += tmpl(message_string, FPSTR(INTL_CONFIG_CAN_NOT_BE_DELETED));
      }
    } else {
      page_content += tmpl(message_string, FPSTR(INTL_CONFIG_NOT_FOUND));
    }
  }
  page_content += make_footer();
  server.send(200, FPSTR(TXT_CONTENT_TYPE_TEXT_HTML), page_content);
}

/*****************************************************************
 * Webserver reset NodeMCU                                       *
 *****************************************************************/
void webserver_reset() {
  if (!webserver_request_auth())
  { return; }

  String page_content = make_header(FPSTR(INTL_RESTART_SENSOR));
  last_page_load = millis();
  debug_out(F("output reset NodeMCU page..."), DEBUG_MIN_INFO, 1);

  if (server.method() == HTTP_GET) {
    page_content += FPSTR(WEB_RESET_CONTENT);
    page_content.replace("{t}", FPSTR(INTL_REALLY_RESTART_SENSOR));
    page_content.replace("{b}", FPSTR(INTL_RESTART));
    page_content.replace("{c}", FPSTR(INTL_CANCEL));
  } else {
    ESP.restart();
  }
  page_content += make_footer();
  server.send(200, FPSTR(TXT_CONTENT_TYPE_TEXT_HTML), page_content);
}

/*****************************************************************
 * Webserver data.json                                           *
 *****************************************************************/
void webserver_data_json() {
  String s1 = "";
  unsigned long age = 0;
  debug_out(F("output data json..."), DEBUG_MIN_INFO, 1);
  if (first_cycle) {
    s1 = FPSTR(data_first_part);
    s1.replace("{v}", SOFTWARE_VERSION);
    s1 += "]}";
    age = cfg::sending_intervall_ms - msSince(starttime);
    if (age > cfg::sending_intervall_ms) {
      age = 0;
    }
    age = 0 - age;
  } else {
    s1 = last_data_string;
    debug_out(F("last data: "), DEBUG_MIN_INFO, 0);
    debug_out(s1, DEBUG_MIN_INFO, 1);
    age = msSince(starttime);
    if (age > cfg::sending_intervall_ms) {
      age = 0;
    }
  }
  String s2 = F(", \"age\":\"");
  s2 += String((long)((age + 500) / 1000));
  s2 += F("\", \"sensordatavalues\"");
  debug_out(F("replace with: "), DEBUG_MIN_INFO, 0);
  debug_out(s2, DEBUG_MIN_INFO, 1);
  s1.replace(F(", \"sensordatavalues\""), s2);
  debug_out(F("replaced: "), DEBUG_MIN_INFO, 0);
  debug_out(s1, DEBUG_MIN_INFO, 1);
  server.send(200, FPSTR(TXT_CONTENT_TYPE_JSON), s1);
}

/*****************************************************************
 * Webserver prometheus metrics endpoint                         *
 *****************************************************************/
void webserver_prometheus_endpoint() {
  debug_out(F("output prometheus endpoint..."), DEBUG_MIN_INFO, 1);
  String data_4_prometheus = F("software_version{version=\"{ver}\",{id}} 1\nuptime_ms{{id}} {up}\nsending_intervall_ms{{id}} {si}\nnumber_of_measurements{{id}} {cs}\n");
  String id = F("node=\"esp8266-");
  id += esp_chipid + "\"";
  debug_out(F("Parse JSON for Prometheus"), DEBUG_MIN_INFO, 1);
  debug_out(last_data_string, DEBUG_MED_INFO, 1);
  data_4_prometheus.replace("{id}", id);
  data_4_prometheus.replace("{ver}", SOFTWARE_VERSION);
  data_4_prometheus.replace("{up}", String(msSince(time_point_device_start_ms)));
  data_4_prometheus.replace("{si}", String(cfg::sending_intervall_ms));
  data_4_prometheus.replace("{cs}", String(count_sends));
  StaticJsonBuffer<JSON_BUFFER_SIZE> jsonBuffer;
  JsonObject& json2data = jsonBuffer.parseObject(last_data_string);
  if (json2data.success()) {
    for (uint8_t i = 0; i < json2data["sensordatavalues"].size() - 1; i++) {
      String tmp_str = json2data["sensordatavalues"][i]["value_type"].as<char*>();
      data_4_prometheus += tmp_str + "{" + id + "} ";
      tmp_str = json2data["sensordatavalues"][i]["value"].as<char*>();
      data_4_prometheus += tmp_str + "\n";
    }
    data_4_prometheus += F("last_sample_age_ms{");
    data_4_prometheus += id + "} " + String(msSince(starttime)) + "\n";
  } else {
    debug_out(FPSTR(DBG_TXT_DATA_READ_FAILED), DEBUG_ERROR, 1);
  }
  debug_out(data_4_prometheus, DEBUG_MED_INFO, 1);
  server.send(200, FPSTR(TXT_CONTENT_TYPE_TEXT_PLAIN), data_4_prometheus);
}

/*****************************************************************
 * Webserver Images                                              *
 *****************************************************************/
static void webserver_images() {
  if (server.arg("name") == F("luftdaten_logo")) {
    debug_out(F("output luftdaten.info logo..."), DEBUG_MIN_INFO, 1);
    server.send(200, FPSTR(TXT_CONTENT_TYPE_IMAGE_SVG), FPSTR(LUFTDATEN_INFO_LOGO_SVG));
  } else {
    webserver_not_found();
  }
}

/*****************************************************************
 * Webserver page not found                                      *
 *****************************************************************/
void webserver_not_found() {
  last_page_load = millis();
  debug_out(F("output not found page..."), DEBUG_MIN_INFO, 1);
  if (WiFi.status() != WL_CONNECTED) {
    if ((server.uri().indexOf(F("success.html")) != -1) || (server.uri().indexOf(F("detect.html")) != -1)) {
      server.send(200, FPSTR(TXT_CONTENT_TYPE_TEXT_HTML), FPSTR(WEB_IOS_REDIRECT));
    } else {
      sendHttpRedirect(server);
    }
  } else {
    server.send(404, FPSTR(TXT_CONTENT_TYPE_TEXT_PLAIN), F("Not found."));
  }
}

/*****************************************************************
 * Webserver setup                                               *
 *****************************************************************/
void setup_webserver() {
  server.on("/", webserver_root);
  server.on("/config", webserver_config);
  server.on("/wifi", webserver_wifi);
  server.on("/values", webserver_values);
  server.on("/generate_204", webserver_config);
  server.on("/fwlink", webserver_config);
  server.on("/debug", webserver_debug_level);
  server.on("/removeConfig", webserver_removeConfig);
  server.on("/reset", webserver_reset);
  server.on("/data.json", webserver_data_json);
  server.on("/metrics", webserver_prometheus_endpoint);
  server.on("/images", webserver_images);
  server.onNotFound(webserver_not_found);

  debug_out(F("Starting Webserver... "), DEBUG_MIN_INFO, 0);
//  debug_out(IPAddress2String(WiFi.localIP()), DEBUG_MIN_INFO, 1);
  debug_out(WiFi.localIP().toString(), DEBUG_MIN_INFO, 1);
  server.begin();
}

static int selectChannelForAp(struct struct_wifiInfo *info, int count) {
  std::array<int, 14> channels_rssi;
  std::fill(channels_rssi.begin(), channels_rssi.end(), -100);

  for (int i = 0; i < count; i++) {
    if (info[i].RSSI > channels_rssi[info[i].channel]) {
      channels_rssi[info[i].channel] = info[i].RSSI;
    }
  }

  if ((channels_rssi[1] < channels_rssi[6]) && (channels_rssi[1] < channels_rssi[11])) {
    return 1;
  } else if ((channels_rssi[6] < channels_rssi[1]) && (channels_rssi[6] < channels_rssi[11])) {
    return 6;
  } else {
    return 11;
  }
}

/*****************************************************************
 * WifiConfig                                                    *
 *****************************************************************/
void wifiConfig() {
  debug_out(F("Starting WiFiManager"), DEBUG_MIN_INFO, 1);
  debug_out(F("AP ID: "), DEBUG_MIN_INFO, 0);
  debug_out(cfg::fs_ssid, DEBUG_MIN_INFO, 1);
  debug_out(F("Password: "), DEBUG_MIN_INFO, 0);
  debug_out(cfg::fs_pwd, DEBUG_MIN_INFO, 1);

  wificonfig_loop = true;

  WiFi.disconnect(true);
  debug_out(F("scan for wifi networks..."), DEBUG_MIN_INFO, 1);
  count_wifiInfo = WiFi.scanNetworks(false, true);
  wifiInfo = new struct_wifiInfo[count_wifiInfo];
  for (int i = 0; i < count_wifiInfo; i++) {
    uint8_t* BSSID;
    String SSID;
    WiFi.getNetworkInfo(i, SSID, wifiInfo[i].encryptionType, wifiInfo[i].RSSI, BSSID, wifiInfo[i].channel, wifiInfo[i].isHidden);
    SSID.toCharArray(wifiInfo[i].ssid, 35);
  }

  WiFi.mode(WIFI_AP);
  const IPAddress apIP(192, 168, 4, 1);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(cfg::fs_ssid, cfg::fs_pwd, selectChannelForAp(wifiInfo, count_wifiInfo));
  debug_out(String(WLANPWD), DEBUG_MIN_INFO, 1);

  DNSServer dnsServer;
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(53, "*", apIP);             // 53 is port for DNS server

  // 10 minutes timeout for wifi config
  last_page_load = millis();
  while (((millis() - last_page_load) < cfg::time_for_wifi_config)) {
    dnsServer.processNextRequest();
    server.handleClient();
    wdt_reset(); // nodemcu is alive
    yield();
  }

  // half second to answer last requests
  last_page_load = millis();
  while ((millis() - last_page_load) < 500) {
    dnsServer.processNextRequest();
    server.handleClient();
    yield();
  }

  WiFi.disconnect(true);
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_STA);

  delete []wifiInfo;

  dnsServer.stop();

  delay(100);

  debug_out(F("Connecting to "), DEBUG_MIN_INFO, 0);
  debug_out(cfg::wlanssid, DEBUG_MIN_INFO, 1);

  WiFi.begin(cfg::wlanssid, cfg::wlanpwd);

  debug_out(F("---- Result Webconfig ----"), DEBUG_MIN_INFO, 1);
  debug_out(F("WLANSSID: "), DEBUG_MIN_INFO, 0);
  debug_out(cfg::wlanssid, DEBUG_MIN_INFO, 1);
  debug_out(F("----\nSend to ..."), DEBUG_MIN_INFO, 1);
  debug_out(F("Dusti: "), DEBUG_MIN_INFO, 0);
  debug_out(String(cfg::send2thingspeak), DEBUG_MIN_INFO, 1);
  debug_out("----", DEBUG_MIN_INFO, 1);
  debug_out(F("Display: "), DEBUG_MIN_INFO, 0);
  debug_out(String(cfg::has_display), DEBUG_MIN_INFO, 1);
  debug_out(F("Debug: "), DEBUG_MIN_INFO, 0);
  debug_out(String(cfg::debug), DEBUG_MIN_INFO, 1);
  
  wificonfig_loop = false;
}

static void waitForWifiToConnect(int maxRetries) {
  int retryCount = 0;
  while ((WiFi.status() != WL_CONNECTED) && (retryCount <  maxRetries)) {
    delay(500);
    debug_out(".", DEBUG_MIN_INFO, 0);
    ++retryCount;
  }
}

/*****************************************************************
 * WiFi auto connecting script                                   *
 *****************************************************************/
void connectWifi() {
  debug_out(String(WiFi.status()), DEBUG_MIN_INFO, 1);
  WiFi.disconnect();
  WiFi.setOutputPower(20.5);
  WiFi.setPhyMode(WIFI_PHY_MODE_11N);
  WiFi.mode(WIFI_STA);
  WiFi.begin(cfg::wlanssid, cfg::wlanpwd); // Start WiFI

  debug_out(F("Connecting to "), DEBUG_MIN_INFO, 0);
  debug_out(cfg::wlanssid, DEBUG_MIN_INFO, 1);

  waitForWifiToConnect(40);
  debug_out("", DEBUG_MIN_INFO, 1);
  if (WiFi.status() != WL_CONNECTED) {
    String fss = String(cfg::fs_ssid);
    display_debug(fss.substring(0, 16), fss.substring(16));
    wifiConfig();
    if (WiFi.status() != WL_CONNECTED) {
      waitForWifiToConnect(20);
      debug_out("", DEBUG_MIN_INFO, 1);
    }
  }
  debug_out(F("WiFi connected\nIP address: "), DEBUG_MIN_INFO, 0);
  debug_out(WiFi.localIP().toString(), DEBUG_MIN_INFO, 1);
}

/*****************************************************************
 * read    sensor values                                     *
 *****************************************************************/
static String sensorBME680() {
  String s;

  debug_out(String(FPSTR(DBG_TXT_START_READING)) + FPSTR(SENSORS_BME680), DEBUG_MED_INFO, 1);
  
  if (!bme680.performReading()) {
    last_value_BME680_T = -128.0;
    last_value_BME680_H = -1.0;
    last_value_BME680_P = -1.0;
    last_value_BME680_G = -128.0;
    debug_out(String(FPSTR(SENSORS_BME680)) + FPSTR(DBG_TXT_COULDNT_BE_READ), DEBUG_ERROR, 1);
  } else {
    const auto t = bme680.temperature;
    const auto p = bme680.pressure;    
    const auto h = bme680.humidity;
    const auto g = bme680.gas_resistance;
    
    debug_out(FPSTR(DBG_TXT_TEMPERATURE), DEBUG_MIN_INFO, 0);
    debug_out(Float2String(t) + " C", DEBUG_MIN_INFO, 1);
    debug_out(FPSTR(DBG_TXT_HUMIDITY), DEBUG_MIN_INFO, 0);
    debug_out(Float2String(h) + " %", DEBUG_MIN_INFO, 1);
    debug_out(FPSTR(DBG_TXT_PRESSURE), DEBUG_MIN_INFO, 0);
    debug_out(Float2String(p / 100.0) + " hPa", DEBUG_MIN_INFO, 1);
    debug_out(FPSTR(DBG_TXT_GAS), DEBUG_MIN_INFO, 0);
    debug_out(Float2String(g / 100.0) + " KOhms", DEBUG_MIN_INFO, 1);
    
    last_value_BME680_T = t;
    last_value_BME680_H = h;
    last_value_BME680_P = p;
    last_value_BME680_G = g;
    s += Value2Json(F("BME680_temperature"), Float2String(last_value_BME680_T));
    s += Value2Json(F("BME680_humidity"), Float2String(last_value_BME680_H));
    s += Value2Json(F("BME680_pressure"), Float2String(last_value_BME680_P));
    s += Value2Json(F("BME680_gas_resistance"), Float2String(last_value_BME680_G));
  }
  debug_out("----", DEBUG_MIN_INFO, 1);

  debug_out(String(FPSTR(DBG_TXT_END_READING)) + FPSTR(SENSORS_BME680), DEBUG_MED_INFO, 1);
  bme680_read_avail = true;
  return s;
}

/*****************************************************************
 * display values                                                *
 *****************************************************************/
void display_values() {
  double t_value = -128.0;
  double h_value = -1.0;
  double p_value = -1.0;
  double g_value = -128.0;
  String t_sensor = "";
  String h_sensor = "";
  String p_sensor = "";
  String g_sensor = "";
  String display_header = "";
  String display_lines[4] = { "", "", "", ""};
  int screen_count = 0;
  int screens[3];
  int line_count = 0;
  debug_out(F("output values to display..."), DEBUG_MIN_INFO, 1);
  if (cfg::bme680_read) {
    t_value = last_value_BME680_T;
    t_sensor = FPSTR(SENSORS_BME680);
    h_value = last_value_BME680_H;
    h_sensor = FPSTR(SENSORS_BME680);
    p_value = last_value_BME680_P;
    p_sensor = FPSTR(SENSORS_BME680);
    g_value = last_value_BME680_G;
    g_sensor = FPSTR(SENSORS_BME680);
  }
  if (cfg::bme680_read) {
    screens[screen_count++] = 1;
  }
  screens[screen_count++] = 2;  // Wifi info
  screens[screen_count++] = 3;  // chipID, firmware and count of measurements
  if (cfg::has_display) {
    switch (screens[next_display_count % screen_count]) {
    case (1):
      display_header = t_sensor;
      if (h_sensor != "" && t_sensor != h_sensor) {
        display_header += " / " + h_sensor;
      }
      if ((h_sensor != "" && p_sensor != "" && (h_sensor != p_sensor)) || (h_sensor == "" && p_sensor != "" && (t_sensor != p_sensor))) {
        display_header += " / " + p_sensor;
      }
      if (t_sensor != "") { display_lines[line_count++] = "Temp.: " + check_display_value(t_value, -128, 1, 6) + " °C"; }
      if (h_sensor != "") { display_lines[line_count++] = "Hum.:  " + check_display_value(h_value, -1, 1, 6) + " %"; }
      if (p_sensor != "") { display_lines[line_count++] = "Pres.: " + check_display_value(p_value / 100, (-1 / 100.0), 1, 6) + " hPa"; }
      if (g_sensor != "") { display_lines[line_count++] = "Gas.: " + check_display_value(g_value / 1000.0, (-1/ 1000.0), 1, 6) + " KOhms"; }
      break;
    case (2):
      display_header = F("Wifi info");
      display_lines[0] = "IP: " + WiFi.localIP().toString();
      display_lines[1] = "SSID:" + WiFi.SSID();
      display_lines[2] = "Signal: " + String(calcWiFiSignalQuality(WiFi.RSSI())) + "%";
      break;
    case (3):
      display_header = F("Device Info");
      display_lines[0] = "ID: " + esp_chipid;
      display_lines[1] = "FW: " + String(SOFTWARE_VERSION);
      display_lines[2] = "Measurements: " + String(count_sends);
      break;
    }
    if (cfg::has_display) {
      display.clear();
      display.displayOn();
      display.setTextAlignment(TEXT_ALIGN_CENTER);
      display.drawString(64, 1, display_header);
      display.setTextAlignment(TEXT_ALIGN_LEFT);
      display.drawString(0, 16, display_lines[0]);
      display.drawString(0, 28, display_lines[1]);
      display.drawString(0, 40, display_lines[2]);
      display.drawString(0, 52, display_lines[3]);
      display.display();
    }
  }
  yield();
  next_display_count += 1;
  next_display_millis = millis() + DISPLAY_UPDATE_INTERVAL_MS;
}

/*****************************************************************
 * Init OLED display                                             *
 *****************************************************************/
void init_display() {
  display.init();
}

/*****************************************************************
 * Init BME680                                                   *
 *****************************************************************/
bool initBME680(char addr) {
  debug_out(F("Trying BME680 sensor on "), DEBUG_MIN_INFO, 0);
  debug_out(String(addr, HEX), DEBUG_MIN_INFO, 0);

  if (bme680.begin(addr)) {
    debug_out(F(" ... found"), DEBUG_MIN_INFO, 1);
    if (!bme680.setTemperatureOversampling(BME680_OS_8X)) {
      debug_out(F("setTemperatureOversampling failed"), DEBUG_MIN_INFO, 1);
    }
    if (!bme680.setHumidityOversampling(BME680_OS_2X)) {
      debug_out(F("setHumidityOversampling failed"), DEBUG_MIN_INFO, 1);
    }
    if (!bme680.setPressureOversampling(BME680_OS_4X)) {
      debug_out(F("setPressureOversampling failed"), DEBUG_MIN_INFO, 1);
    }
    if (!bme680.setIIRFilterSize(BME680_FILTER_SIZE_3)) {
      debug_out(F("setIIRFilterSize failed"), DEBUG_MIN_INFO, 1);
    }
    if (!bme680.setGasHeater(320,150)) {     //320*C for 150ms
      debug_out(F("setGasHeater failed"), DEBUG_MIN_INFO, 1);
    }
    return true;
  } else {
    debug_out(F(" ... not found"), DEBUG_MIN_INFO, 1);
    return false;
  }
}

static void powerOnTestSensors() {
  if (cfg::bme680_read) {
    debug_out(F("Read BME680..."), DEBUG_MIN_INFO, 1);
    if (!initBME680(0x77)) {
      debug_out(F("Check BME680 wiring"), DEBUG_MIN_INFO, 1);
      bme680_init_failed = 1;
    }
  }
}

static void logEnabledDisplays() {
  if (cfg::has_display) {
    debug_out(F("Show on OLED..."), DEBUG_MIN_INFO, 1);
  }
}

void time_is_set (void) {
  sntp_time_is_set = true;
}

static bool acquireNetworkTime() {
  int retryCount = 0;
  debug_out(F("Setting time using SNTP"), DEBUG_MIN_INFO, 1);
  time_t now = time(nullptr);
  debug_out(ctime(&now), DEBUG_MIN_INFO, 1);
  debug_out(F("NTP.org:"),DEBUG_MIN_INFO,1);
  settimeofday_cb(time_is_set);
  configTime(8 * 3600, 0, "pool.ntp.org");
  while (retryCount++ < 20) {
    // later than 2000/01/01:00:00:00
    if (sntp_time_is_set) {
      now = time(nullptr);
      debug_out(ctime(&now), DEBUG_MIN_INFO, 1);
      return true;
    }
    delay(500);
    debug_out(".",DEBUG_MIN_INFO,0);
  }
  debug_out(F("\nrouter/gateway:"),DEBUG_MIN_INFO,1);
  retryCount = 0;
  configTime(0, 0, WiFi.gatewayIP().toString().c_str());
  while (retryCount++ < 20) {
    // later than 2000/01/01:00:00:00
    if (sntp_time_is_set) {
      now = time(nullptr);
      debug_out(ctime(&now), DEBUG_MIN_INFO, 1);
      return true;
    }
    delay(500);
    debug_out(".",DEBUG_MIN_INFO,0);
  }
  return false;
}

/*****************************************************************
 * The Setup                                                     *
 *****************************************************************/
void setup() {
  Serial.begin(115200);         // Output to Serial at 9600 baud

  esp_chipid = String(ESP.getChipId());
  cfg::initNonTrivials(esp_chipid.c_str());
  readConfig();

  init_display();
  setup_webserver();
  display_debug(F("Connecting to"), String(cfg::wlanssid));
  connectWifi();
  got_ntp = acquireNetworkTime();
  debug_out(F("\nNTP time "), DEBUG_MIN_INFO, 0);
  debug_out(String(got_ntp?"":"not ")+F("received"), DEBUG_MIN_INFO, 1);
  debug_out(F("\nChipId: "), DEBUG_MIN_INFO, 0);
  debug_out(esp_chipid, DEBUG_MIN_INFO, 1);

  powerOnTestSensors();
  logEnabledDisplays();

  String server_name = F("IAQsensor-");
  server_name += esp_chipid;
  if (MDNS.begin(server_name.c_str())) {
    MDNS.addService("http", "tcp", 80);
  }
  if (cfg::send2thingspeak) {
    ThingSpeak.begin(client);
  }
  
  delay(50);

  // sometimes parallel sending data and web page will stop nodemcu, watchdogtimer set to 30 seconds
  wdt_disable();
  wdt_enable(30000);

  starttime = millis();                                   // store the start time
  time_point_device_start_ms = starttime;
  next_display_millis = starttime + DISPLAY_UPDATE_INTERVAL_MS;
}

static void checkForceRestart() {
  if (msSince(time_point_device_start_ms) > DURATION_BEFORE_FORCED_RESTART_MS) {
    ESP.restart();
  }
}

/*****************************************************************
 * And action                                                    *
 *****************************************************************/
void loop() {
  String result_BME680 = "";
  unsigned long sum_send_time = 0;
  unsigned long start_send;

  act_micro = micros();
  act_milli = millis();
  send_now = msSince(starttime) > cfg::sending_intervall_ms;

  sample_count++;

  wdt_reset(); // nodemcu is alive

  if (last_micro != 0) {
    unsigned long diff_micro = act_micro - last_micro;
    if (max_micro < diff_micro) {
      max_micro = diff_micro;
    }
    if (min_micro > diff_micro) {
      min_micro = diff_micro;
    }
  }
  last_micro = act_micro;

  server.handleClient();

  if (send_now) {
    if (cfg::bme680_read && (! bme680_init_failed)) {
      debug_out(String(FPSTR(DBG_TXT_CALL_SENSOR)) + FPSTR(SENSORS_BME680), DEBUG_MAX_INFO, 1);
      result_BME680 = sensorBME680();                 // getting temperature, humidity and pressure (optional)
    }
  }

  if ((cfg::has_display) && (act_milli > next_display_millis)) {
    display_values();
  }

  if (send_now) {
    debug_out(F("Creating data string:"), DEBUG_MIN_INFO, 1);
    String data = FPSTR(data_first_part);
    data.replace("{v}", SOFTWARE_VERSION);
    String data_sample_times  = Value2Json(F("samples"), String(sample_count));
    data_sample_times += Value2Json(F("min_micro"), String(min_micro));
    data_sample_times += Value2Json(F("max_micro"), String(max_micro));

    String signal_strength = String(WiFi.RSSI());
    debug_out(F("WLAN signal strength: "), DEBUG_MIN_INFO, 0);
    debug_out(signal_strength, DEBUG_MIN_INFO, 0);
    debug_out(" dBm", DEBUG_MIN_INFO, 1);
    debug_out("----", DEBUG_MIN_INFO, 1);

    server.handleClient();
    yield();
    server.stop();

    if (cfg::bme680_read && (! bme680_init_failed)) {
      data += result_BME680;
      if (cfg::send2thingspeak && bme680_read_avail == true ) {
        debug_out(String(FPSTR(DBG_TXT_SENDING_TO_THINGSPEAK)) + F("(BME680): "), DEBUG_MIN_INFO, 1);
        start_send = millis();
        // set the fields with the values
        ThingSpeak.setField(1, last_value_BME680_T);
        ThingSpeak.setField(2, last_value_BME680_H);
        ThingSpeak.setField(3, last_value_BME680_P);
        ThingSpeak.setField(4, last_value_BME680_G);
        ThingSpeak.writeFields(cfg::myChannelNumber, cfg::myWriteAPIKey);
        bme680_read_avail = false;
        sum_send_time += millis() - start_send;
      }
    }

    data_sample_times += Value2Json("signal", signal_strength);
    data += data_sample_times;

    if ((unsigned)(data.lastIndexOf(',') + 1) == data.length()) {
      data.remove(data.length() - 1);
    }
    data += "]}";

    server.begin();

    checkForceRestart();

    sending_time = (4 * sending_time + sum_send_time) / 5;
    debug_out(F("Time for sending data (ms): "), DEBUG_MIN_INFO, 0);
    debug_out(String(sending_time), DEBUG_MIN_INFO, 1);


    // reconnect to WiFi if disconnected
    if (WiFi.status() != WL_CONNECTED) {
      debug_out(F("Connection lost, reconnecting "), DEBUG_MIN_INFO, 0);
      WiFi.reconnect();
      waitForWifiToConnect(20);
      debug_out("", DEBUG_MIN_INFO, 1);
    }

    // Resetting for next sampling
    last_data_string = data;
    sample_count = 0;
    last_micro = 0;
    min_micro = 1000000000;
    max_micro = 0;
    sum_send_time = 0;
    starttime = millis();                               // store the start time
    first_cycle = false;
    count_sends += 1;
  }
  yield();
//  if (sample_count % 500 == 0) { Serial.println(ESP.getFreeHeap(),DEC); }
}
