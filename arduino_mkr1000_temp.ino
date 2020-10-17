#include <SPI.h>
#include <WiFi101.h> 
#include<WiFiSSLClient.h>
#include <ArduinoHttpClient.h>
#include <OneWire.h> 
#include <DallasTemperature.h>
#include <RTCZero.h>

#include "arduino_secrets.h"  

#define ONE_WIRE_BUS 2            // data wire is on pin -2 on the Arduino mkr1000

OneWire oneWire(ONE_WIRE_BUS);        // for any OneWire devices, not just Maxim/Dallas
DallasTemperature sensors(&oneWire);  // pass reference to Dallas Temperature IC

/* Create an rtc object */
RTCZero rtc;

/* Change these values to set the current initial time */
const byte seconds = 0;
const byte minutes = 43;
const byte hours = 9;

/* Change these values to set the current initial date */
const byte day = 17;
const byte month = 10;
const byte year = 20;





char ssid[] = SECRET_SSID;    //  your network SSID (name) 
char pass[] = SECRET_PASS;  // your network password 
const char HOST[] = SECRET_HOST;   //the host API
String httpsRequest = SECRET_REQUEST; // your API URL
String key = SECRET_KEY; // your API KEY  

 WiFiSSLClient wifi;
 HttpClient client = HttpClient(wifi, HOST, 443);
 


void setup() { 
 Serial.begin(9600); 
 
 rtc.begin(); // initialize RTC

  rtc.setTime(hours, minutes, seconds);
  rtc.setDate(day, month, year);

  rtc.setAlarmTime(11, 0, 30);
  rtc.enableAlarm(rtc.MATCH_HHMMSS);
 
  rtc.attachInterrupt(alarmMatch);
 
 while (!Serial)
 {
  ;
 } 
 delay(2000); 

 // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue:
    while (true);
  }


 Serial.print("Connecting Wifi: "); 
 Serial.println(ssid); 
 while (WiFi.begin(ssid, pass) != WL_CONNECTED) { 
   Serial.print("."); 
   delay(500); 
 } 
 Serial.println(""); 
 Serial.println("WiFi connected"); //could use printWifiStatus();

 
} //setup 

void loop() { 
 String warning = "This is a warning message"; 
 warning.replace(" ", "%20"); 

 sensors.begin();    // start Dallas Temperature IC Control Library

/*********************************/
  //Get devices list
  DeviceAddress deviceAddress;
  int devicecount = sensors.getDeviceCount();
  for (int i = 0; i < devicecount; i++) {
    if (sensors.getAddress(deviceAddress, i)) {
      Serial.print("Device ");
      Serial.println(i+1);
      sensors.setResolution(deviceAddress, 12);     // set temperature resolution (9 through 12)
    }
  }
/*********************************/

Serial.print(" Requesting temperatures..."); 
 sensors.requestTemperatures(); // Send the command to get temperature readings 
 Serial.println("DONE"); 
/********************************************************************/
 Serial.print("Temperature is: "); 
 float temp1 = sensors.getTempCByIndex(0);  // Why "byIndex"?  
   // You can have more than one DS18B20 on the same bus.  
   // 0 refers to the first IC on the wire 

   // Check if any reads failed and exit early (to try again).
  if (isnan(temp1))
  {
    Serial.println("Failed to read from sensor!");
    return;
  }
 Serial.print(temp1);
 Serial.print(" "); 
 print2digits(rtc.getHours());
 Serial.print(":");
 print2digits(rtc.getMinutes());

  send_request(temp1, "externalTemp", "Bxl","C°",key,warning);

  // if the server's disconnected, stop the client:
 /* if (!client.connected()) {
    Serial.println();
    Serial.println("disconnected from server.");
    client.stop();

    // do nothing forevermore:
    while (true);
  }*/

 delay(3600000); //60 secondes = 60000 
} // /loop


void send_request(float measure_value, String measure_type , String origin, String measure_unit, String key ,String warning) { 
 // convert values to String 
 String _measure_value = String(measure_value);
 String _measure_type = String(measure_type);
 String _measure_unit = String(measure_unit);  
 String _warning = warning;
 Serial.print(" Begin send Request "); 
 String contentType = "application/x-www-form-urlencoded";
 String postData = "measure_value=" + _measure_value + "&measure_type=" + _measure_type + "&measure_unit=" + _measure_unit + "&origin=" + origin + "&key=" + key;

 Serial.println(HOST + httpsRequest + '/' + postData); 

  client.post(httpsRequest, contentType, postData);

int statusCode = client.responseStatusCode();
  String response = client.responseBody();

  Serial.print("Status code: ");
  Serial.println(statusCode);
  Serial.print("Response: ");
  Serial.println(response);

  Serial.println("Wait five seconds");
  delay(5000);
  

}  

//Triggered when alarm matches timer
void alarmMatch()
{
  Serial.println("Alarm Match!");
  Serial.print(" "); 
 print2digits(rtc.getHours());
  Serial.print(":");
  print2digits(rtc.getMinutes());
  Serial.println("End Alarm");
}

//
// function to format with two decimal
//
void print2digits(int number) {
  if (number < 10) {
    Serial.print("0"); // print a 0 before if the number is < than 10
  }
  Serial.print(number);
}


/*
 * Get wifi status
 * 
 */
void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
