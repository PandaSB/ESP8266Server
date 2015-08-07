/* DHTServer - ESP8266 Webserver with a DHT sensor as an input

   Based on ESP8266Webserver, DHTexample, and BlinkWithoutDelay (thank you)
   Based on DHT Server from Adafruit
   Based on Send sensor data (DHT11 & BMP180) to ThingSpeak from instructables.com 
*/

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <DHT.h>
#define DHTTYPE DHT11
#define DHTPIN  2


boolean gettemperature(); 
void updateThingSpeak(String tsData); 

const char*      ssid              = "<WIFI SSID>";
const char*      password          = "<WIFI PASSWORD>";
const char*      thingSpeakAddress = "api.thingspeak.com";
const String     writeAPIKey       = "<THINGSPEAK API KEY>";
const long       interval          = 10000;   // interval at which to read sensor (ms)
const long       interval_sent     = 10 * 6;  // * interval


ESP8266WebServer server(80);
WiFiClient       client;
DHT              dht(DHTPIN, DHTTYPE, 11); 
 
float            humidity, temp_f, temp_c;  // Values read from sensor
String           webString        = "";     // String to display
unsigned long    previousMillis   = 0;      // will store last temp was read
unsigned long    sent_loop        = 0 ; 

 
void handle_root() {
  server.send(200, "text/plain", "Hello from the weather esp8266, read from /tempF , /tempC or /humidity");
  delay(100);
}

void handle_tempf() {
  webString="Temperature: "+String((int)temp_f)+" F";
  server.send(200, "text/plain", webString);
  
}

void handle_tempc() {
    webString="Temperature: "+String((int)temp_c)+" C";
    server.send(200, "text/plain", webString);
}

void handle_humidity() {  
    webString="Humidity: "+String((int)humidity)+"%";
    server.send(200, "text/plain", webString);               // send to someones browser when asked
}
 
void setup(void)
{
  // You can open the Arduino IDE Serial Monitor window to see what the code is doing
  Serial.begin(115200);  // Serial connection from ESP-01 via 3.3v console cable
  dht.begin();           // initialize temperature sensor

  // Connect to WiFi network
  WiFi.begin(ssid, password);
  Serial.print("\n\r \n\rWorking to connect");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("DHT Weather Reading Server");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
   
  server.on("/",        handle_root);
  server.on("/tempF",   handle_tempf);
  server.on("/tempC",   handle_tempc);
  server.on("/humidity",handle_humidity);
  
  server.begin();
  Serial.println("HTTP server started");
}
 
void loop(void)
{
  server.handleClient();
  if ( gettemperature() == true)            // read sensor ;
  {
    if ( sent_loop >= interval_sent )
    {
      Serial.println("Send Data to IoT Server");
      sent_loop = 0 ; 
      updateThingSpeak ("field1="+String(temp_c)+"&field2="+String(humidity)) ;
    }
    else 
    {
      sent_loop ++ ; 
    }
  }
} 

boolean gettemperature() {
  unsigned long currentMillis = millis(); 
  if(currentMillis - previousMillis >= interval) 
  {
    previousMillis = currentMillis;   

    humidity = dht.readHumidity();          // Read humidity (percent)
    temp_c = dht.readTemperature(false);    // Read temperature as Celcuis
    temp_f = dht.convertCtoF(temp_c);       // Convert in Fahrenheit

    if (isnan(humidity) || isnan(temp_f)) {
      Serial.println("Failed to read from DHT sensor!");
      return ( false);
    }
    Serial.println("read from DHT sensor! ");
    return (true);
  }
  return ( false);  
}

void updateThingSpeak(String tsData)
{
  if (client.connect(thingSpeakAddress, 80))
  {
    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: "+writeAPIKey+"\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(tsData.length());
    client.print("\n\n");
    client.print(tsData);
    if (client.connected())
    {
      Serial.println("Connecting to ThingSpeak...");
    }
    else
    {
        Serial.println("Connection to ThingSpeak failed ");
    }
  }
  else
  {
    Serial.println("Connection to ThingSpeak Failed");
  }
  client.flush();
  client.stop();

}
