
#include <HTTPClient.h>

#include <PubSubClient.h>
#include <rpcWiFi.h>
#include <TFT_eSPI.h>
#include "LIS3DHTR.h"

#include <SensirionI2CSht4x.h>
#include <Wire.h>

#include "Seeed_FS.h" //Including SD card library
//#include "RawImage.h" //Including image processing library

#define WIFISSID "BMT_4G"          // Put your WifiSSID here
#define PASSWORD "WIFIPASS" // Put your wifi password here

// #define WIFISSID "BMT-hotspot"                             // Put your WifiSSID here
// #define PASSWORD "simple4u"                    // Put your wifi password here

#define TOKEN "BBFF-" // Put your Ubidots' TOKEN
#define VARIABLE_LABEL "light"                      // Assign the variable label
#define DEVICE_LABEL "wio-terminal"                 // Assign the device label
#define MQTT_CLIENT_NAME "r6y1ax7mq8badrtharwat"    // MQTT client Name

#define VARIABLE_LABEL1 "light" // Assign the variable label
#define VARIABLE_LABEL2 "IMUx"
#define VARIABLE_LABEL3 "IMUy"
#define VARIABLE_LABEL4 "IMUz"
#define VARIABLE_LABEL5 "sound"

#define VARIABLE_vibration "vibration"
#define VARIABLE_incident "vibration_incident"
#define VARIABLE_sound "sound"
#define VARIABLE_temp "temp"
#define VARIABLE_humi "humi"

SensirionI2CSht4x sht4x;

/// @brief timers
unsigned long pixel_time = 0;
unsigned long IMU_time = millis();
unsigned long scan_time = millis();
unsigned long scan_time_gap = 100;

unsigned long DHT_time = millis();
unsigned long DHT_time_gap = 30000; // take it up to 60000 later

/// @brief the previous IMU
float previousX = 0;
float previousY = 0;
float previousZ = 0;
float previous_length = 0;
int counter = 0;

float max_vibration = 0;

LIS3DHTR<TwoWire> lis;

const long interval = 100;
unsigned long previousMillis = 0;

TFT_eSPI tft;

char mqttBroker[] = "industrial.api.ubidots.com";

WiFiClient wifiClient;
PubSubClient client(wifiClient);
PubSubClient client_oci(wifiClient);

// sensor values
static int lightValue = 0;
static float imuxValue = 0;
static float imuyValue = 0;
static float imuzValue = 0;
static int soundValue = 0;
int soundValue_max = 0;

static float temp = 0;
static float humi = 0;

// Space to store values to send
static char str_light[6];
static char str_imux[6];
static char str_imuy[6];
static char str_imuz[6];
static char str_sound[7];
static char str_vibration[6];
static char str_temp[6];
static char str_humidity[6];
char payload[1700];
char topic[150];

/// MQTT
char mqttBroker2[] = "130.162.49.247";
//char mqttBroker2[] = "150.136.105.107";
int port = 1883;
const char topic_vibration[] = "vibration";
const char topic_sound[] = "sound";
const char topic_incident[] = "incident";

const char *list[] = {"photos/o_16.bmp"};

void callback(char *topic, byte *payload, unsigned int length)
{
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for (int i = 0; i < length; i++)
    {
        Serial.print((char)payload[i]);
    }
}

void reconnect()
{
    // Loop until we're reconnected
    while (!client.connected())
    {
        Serial.println("Attempting MQTT connection...");

        // Attempt to connect
        if (client.connect(MQTT_CLIENT_NAME))
        {
            Serial.println("connected");
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 2 seconds");
            // Wait 2 seconds before retrying
            delay(2000);
        }
    }
}

void reconnect_wifi()
{
    // Set WiFi to station mode and disconnect from an AP if it was previously connected
    // WiFi.mode(WIFI_STA);
    // WiFi.disconnect();

    tft.setTextSize(3);
    tft.drawString("Oracle Sensor", 55, 15);

    tft.drawString("Reconnecting to WiFi", 20, 120);
    WiFi.begin(WIFISSID, PASSWORD);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        WiFi.begin(WIFISSID, PASSWORD);
    }
}
void reconnect_oci()
{
    // Loop until we're reconnected
    while (!client_oci.connected())
    {

        if (WiFi.status() != WL_CONNECTED)
        {
            Serial.println("WIFI NOT Connected...");
            reconnect_wifi();
        }

        Serial.println("Attempting OCI MQTT connection...");

        // Attempt to connect
        if (client_oci.connect(MQTT_CLIENT_NAME))
        {
            Serial.println("connected to OCI");
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.print(client_oci.state());
            Serial.println(" try again in 2 seconds");
            // Wait 2 seconds before retrying

            delay(2000);
        }
    }
}

// Reading built-in sensor values
void read_builtin()
{
    lightValue = analogRead(WIO_LIGHT);
    Serial.print("Light = ");
    Serial.println(lightValue);

    imuxValue = lis.getAccelerationX();
    Serial.print("IMU_x = ");
    Serial.println(imuxValue);
    imuyValue = lis.getAccelerationY();
    Serial.print("IMU_y = ");
    Serial.println(imuyValue);
    imuzValue = lis.getAccelerationZ();
    Serial.print("IMU_z = ");
    Serial.println(imuzValue);

    soundValue = analogRead(WIO_MIC);
    Serial.print("Sound = ");
    Serial.println(soundValue);
}

// Sending data to Ubidots
void send_data()
{
    dtostrf(lightValue, 4, 0, str_light);
    dtostrf(imuxValue, 4, 3, str_imux);
    dtostrf(imuyValue, 4, 3, str_imuy);
    dtostrf(imuzValue, 4, 3, str_imuz);
    dtostrf(soundValue, 4, 0, str_sound);

    if (!client.connected())
    {
        reconnect();
    }

    // Builds the topic
    sprintf(topic, "%s", ""); // Cleans the topic content
    sprintf(topic, "%s%s", "/v2.0/devices/", DEVICE_LABEL);

    // Builds the payload
    sprintf(payload, "%s", "");                    // Cleans the payload
    sprintf(payload, "{\"%s\":", VARIABLE_LABEL1); // Adds the variable label
    sprintf(payload, "%s%s", payload, str_light);  // Adds the value
    sprintf(payload, "%s}", payload);              // Closes the dictionary brackets
    client.publish(topic, payload);
    delay(500);

    sprintf(payload, "%s", "");                    // Cleans the payload
    sprintf(payload, "{\"%s\":", VARIABLE_LABEL2); // Adds the variable label
    sprintf(payload, "%s%s", payload, str_imux);   // Adds the value
    sprintf(payload, "%s}", payload);              // Closes the dictionary brackets
    client.publish(topic, payload);
    delay(500);

    sprintf(payload, "%s", "");                    // Cleans the payload
    sprintf(payload, "{\"%s\":", VARIABLE_LABEL3); // Adds the variable label
    sprintf(payload, "%s%s", payload, str_imuy);   // Adds the value
    sprintf(payload, "%s}", payload);              // Closes the dictionary brackets
    client.publish(topic, payload);
    delay(500);

    sprintf(payload, "%s", "");                    // Cleans the payload
    sprintf(payload, "{\"%s\":", VARIABLE_LABEL4); // Adds the variable label
    sprintf(payload, "%s%s", payload, str_imuz);   // Adds the value
    sprintf(payload, "%s}", payload);              // Closes the dictionary brackets
    client.publish(topic, payload);
    delay(500);

    sprintf(payload, "%s", "");                    // Cleans the payload
    sprintf(payload, "{\"%s\":", VARIABLE_LABEL5); // Adds the variable label
    sprintf(payload, "%s%s", payload, str_sound);  // Adds the value
    sprintf(payload, "%s}", payload);              // Closes the dictionary brackets
    client.publish(topic, payload);
    delay(500);

    client.loop();
}

void send_vibration_incident()
{

    if (!client_oci.connected())
    {
        reconnect_oci();
    }
    // Builds the topic
    sprintf(topic, "%s", ""); // Cleans the topic content
    sprintf(topic, "%s%s", "sensor_alarms/", DEVICE_LABEL);

    // Builds the payload
    sprintf(payload, "%s", "");                      // Cleans the payload
    sprintf(payload, "{\"%s\":", VARIABLE_incident); // Adds the variable label
    sprintf(payload, "%s%s", payload, "1");          // Adds the value
    sprintf(payload, "%s}", payload);                // Closes the dictionary brackets
    client_oci.publish(topic, payload);

    // delay(500);

    client.loop();
}

void send_alive_status()
{

    if (!client_oci.connected())
    {
        reconnect_oci();
    }
    // Builds the topic
    sprintf(topic, "%s", ""); // Cleans the topic content
    sprintf(topic, "%s%s", "sensor/", DEVICE_LABEL);

    // Builds the payload
    sprintf(payload, "%s", "");                      // Cleans the payload
    sprintf(payload, "{\"%s\":", VARIABLE_incident); // Adds the variable label
    sprintf(payload, "%s%s", payload, "1");          // Adds the value
    sprintf(payload, "%s}", payload);                // Closes the dictionary brackets
    client_oci.publish(topic, payload);

    // delay(500);

    client.loop();
}

void send_data_oci(float vibration_avg, int sound)
{

    Serial.println("to send ");
    Serial.print((String)vibration_avg);
    Serial.print(",");
    Serial.print((String)sound);
    Serial.print(",");
    Serial.print((String)temp);
    Serial.print(",");
    Serial.println((String)humi);

    dtostrf(temp, 4, 2, str_temp);
    dtostrf(humi, 4, 2, str_humidity);
    dtostrf(sound, 4, 0, str_sound);
    dtostrf(vibration_avg, 4, 2, str_vibration);

    Serial.println("to send encoded ");
    Serial.print(str_sound);
    Serial.print(",");
    Serial.print(str_humidity);
    Serial.print(",");
    Serial.print(str_temp);
    Serial.print(",");
    Serial.println(str_vibration);

    if (!client_oci.connected())
    {
        reconnecting_message();

        reconnect_oci();
    }

    // Builds the topic
    sprintf(topic, "%s", ""); // Cleans the topic content
    sprintf(topic, "%s%s", "sensor/", DEVICE_LABEL);
    sprintf(payload, "{\"%s\":", VARIABLE_vibration); // Adds the variable label
    sprintf(payload, "%s%s", payload, str_vibration); // Adds the value

    sprintf(payload, "%s,\"%s\":", payload, VARIABLE_sound); // Adds the variable label
    sprintf(payload, "%s%s", payload, str_sound);            // Adds the value

    sprintf(payload, "%s,\"%s\":", payload, VARIABLE_temp); // Adds the variable label
    sprintf(payload, "%s%s", payload, str_temp);            // Adds the value

    sprintf(payload, "%s,\"%s\":", payload, VARIABLE_humi); // Adds the variable label
    sprintf(payload, "%s%s", payload, str_humidity);        // Adds the value

    sprintf(payload, "%s}", payload); // Closes the dictionary brackets
    Serial.println(payload);

    client_oci.publish(topic, payload);

    // delay(500);

    client_oci.loop();
}

void setup()
{
    Serial.begin(115200);

    // Initialise SD card
//    if (!SD.begin(SDCARD_SS_PIN, SDCARD_SPI))
//    {
//        while (1)
//            ;
//    }

    Wire.begin();
    sht4x.begin(Wire);

    lis.begin(Wire1);
    pinMode(WIO_MIC, INPUT);
    pinMode(WIO_LIGHT, INPUT);

    tft.begin();
    tft.setRotation(3);
    tft.setTextSize(2);

    

    tft.fillScreen(TFT_BLACK);

    // drawImage<uint16_t>(list[0],0,0);

    lis.setOutputDataRate(LIS3DHTR_DATARATE_25HZ); // Data output rate
    lis.setFullScaleRange(LIS3DHTR_RANGE_2G);

    //  while(!Serial);

    // Set WiFi to station mode and disconnect from an AP if it was previously connected
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    tft.setTextSize(3);
    tft.drawString("Oracle Sensor", 55, 15);
    tft.setTextSize(2);
    tft.drawString("Connecting to OCI", 20, 120);

    WiFi.begin(WIFISSID, PASSWORD);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        WiFi.begin(WIFISSID, PASSWORD);
    }

    tft.fillScreen(TFT_BLACK);
    tft.drawString("Connected to OCI", 20, 120);

    delay(1000);

    /// ubidots
    // client.setServer(mqttBroker, 1883);
    // client.setCallback(callback);

    client_oci.setServer(mqttBroker2, 1883);
    client_oci.setCallback(callback);

    // buzzer
    pinMode(WIO_BUZZER, OUTPUT);
}

void loop_old()
{
    read_builtin(); // Reading buile-in sensor values
    send_data();    // Sending data to Ubidots
    delay(5000);
}





void loop()
{

    float x = 0;
    float y = 0;
    float z = 0;

    int i;

    if (millis() - scan_time >= scan_time_gap)
    {

        /// @brief vibrations
        float length = 0;
        x = lis.getAccelerationX();
        y = lis.getAccelerationY();
        z = lis.getAccelerationZ();

        length = ((x * x) + (y * y) + (z * z));
        length = sqrt(length);
        // Serial.print("length : ");
        // Serial.println(length);

        float netvibration = sqrt((previous_length - length) * (previous_length - length));
        // Serial.print("net vibrations : ");
        // Serial.println(netvibration);
        netvibration = netvibration * 100;

        /// @brief sound
        soundValue = analogRead(WIO_MIC);

//        if (millis() - IMU_time <= 1000)
        if (millis() - IMU_time <= 1000)
        {

            // rolling average
            max_vibration += netvibration;
            if (soundValue_max < soundValue)
                soundValue_max = soundValue;

            // soundValue_max+=soundValue;

            /// @brief how many readings
            counter++;

            /// @brief keep alive & check for messaages
            // client_oci.loop();
        }
        else
        {

            Serial.println("****VIBRATION AVG****");
            float max_vibration_avg = (max_vibration / 10);
            Serial.println(max_vibration_avg);

            Serial.println("****Sound MAX****");
            Serial.println(soundValue_max);

            // post to oci
            send_data_oci(max_vibration, soundValue_max);

            /// post to ubidots
            // ubidots.add("vibrations", max_vibration/10);
            //  ubidots.send();

            if (max_vibration_avg > 30)
            {

                Serial.println("****VIBRATION INCIDENT****");

                /// @brief buzz an incident
                // analogWrite(WIO_BUZZER, 128);

                send_vibration_incident();
                // read_builtin();
                // send_data();

                tft.fillScreen(TFT_RED);
                tft.drawString("Vibration Incident", 20, 120);
            }
            else
            {

                /// @brief buzzer off
                analogWrite(WIO_BUZZER, 0);

                /// @brief screen back to normal
                tft.fillScreen(TFT_BLACK);
                tft.setTextSize(3);
                tft.drawString("Oracle Sensor", 55, 15);
                tft.setTextSize(2);
                tft.drawString("Vibrations", 20, 100);
                tft.drawString((String)max_vibration_avg, 20, 120);

                tft.drawString("Noise", 180, 100);
                tft.drawString((String)soundValue_max, 180, 120);
            }

            // reset it all for the next cycle
            max_vibration = 0;
            soundValue_max = 0;
            counter = 0;
            IMU_time = millis();
        }
        previous_length = length;
        scan_time = millis();
    }

    if (millis() - DHT_time >= DHT_time_gap || temp == 0)
    {
        Serial.println("getting H/T");
        read_sht40();
        DHT_time = millis();
    }
}

void draw_screen()
{

    // tft.setTextSize(3);
    // tft.drawString("Vibrations", 40, 20);

    // tft.setTextSize(2);
    // /// @brief screen back to normal
    // tft.fillScreen(TFT_BLACK);
    // tft.drawString("Vibrations", 20, 100);
    // tft.drawString((String)max_vibration_avg, 20, 120);

    // tft.drawString("Noise", 180, 100);
    // tft.drawString((String)soundValue_max, 180, 120);
}

void vibration_incident()
{
}

void reconnecting_message()
{
    tft.fillScreen(TFT_BLACK);
    tft.drawString("reconnecting", 20, 120);
}

void connected_message()
{
    tft.fillScreen(TFT_BLACK);
    tft.drawString("connected", 20, 120);
}

void read_sht40()
{

    Serial.println("getting reading");
    uint16_t error;
    char errorMessage[256];
    error = sht4x.measureHighPrecision(temp, humi);
    if (error)
    {
        Serial.print("Error trying to execute measureHighPrecision(): ");
        errorToString(error, errorMessage, 256);
        Serial.println(errorMessage);
    }
    else
    {
        Serial.print("Temperature:");
        Serial.print(temp);
        Serial.print("\t");
        Serial.print("Humidity:");
        Serial.println(humi);
    }
}




void send_http()
{
    String serverName = "http://192.168.1.106:1880/update-sensor";
    HTTPClient http;
    http.begin(wifiClient, serverName); 


 // Specify content-type header
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");
      // Data to send with HTTP POST
      String httpRequestData = "api_key=tPmAT5Ab3j7F&sensor=BME280&value1=24.25&value2=49.54&value3=1005.14";           
      // Send HTTP POST request
      int httpResponseCode = http.POST(httpRequestData);
      
      // If you need an HTTP request with a contents type: application/json, use the following:
      //http.addHeader("Content-Type", "application/json");
      //int httpResponseCode = http.POST("{\"api_key\":\"tPmAT5Ab3j7F9\",\"sensor\":\"BME280\",\"value1\":\"24.25\",\"value2\":\"49.54\",\"value3\":\"1005.14\"}");

      // If you need an HTTP request with a content type: text/plain
      //http.addHeader("Content-Type", "text/plain");
      //int httpResponseCode = http.POST("Hello, World!");
     
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
        
      // Free resources
      http.end();
}
