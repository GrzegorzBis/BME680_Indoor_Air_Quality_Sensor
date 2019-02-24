// Language config
#define CURRENT_LANG INTL_LANG

// Wifi config
#define WLANSSID "wifi-disabled"
#define WLANPWD ""

// BasicAuth config
#define WWW_USERNAME "admin"
#define WWW_PASSWORD "feinstaub"
#define WWW_BASICAUTH_ENABLED 0

// Sensor Wifi config (config mode)
#define FS_SSID ""
#define FS_PWD ""

// Wohin gehen die Daten?
#define SEND2TSPK 0

// NTP Server
#define NTP_SERVER "0.europe.pool.ntp.org"

// IMPORTANT: NO MORE CHANGES TO VARIABLE NAMES NEEDED FOR EXTERNAL APIS

// define pins for I2C
#define I2C_PIN_SCL D4
#define I2C_PIN_SDA D3

// BME280, temperature, humidity, pressure
#define BME680_READ 1

// OLED Display SSD1306 angeschlossen?
#define HAS_DISPLAY 0

// Wieviele Informationen sollen über die serielle Schnittstelle ausgegeben werden?
#define DEBUG 3

// Definition der Debuglevel
#define DEBUG_ERROR 1
#define DEBUG_WARNING 2
#define DEBUG_MIN_INFO 3
#define DEBUG_MED_INFO 4
#define DEBUG_MAX_INFO 5
