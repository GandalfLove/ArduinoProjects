#include <SPI.h> //Pins 11, 12, 13, 10, and 4 reserved for 
#include <Ethernet.h> 
#include <PubSubClient.h>
#include <DHT.h>
#define DHTPIN 2 //Assign DHT pin to 2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE); //DHT sensor configuration
unsigned long readTime;

//Ethernet Setup

byte mac[]    =   {0xFE, 0x85, 0xC8, 0x32, 0xD7, 0x78}; //Arduino Mac address
IPAddress ip      (192, 168, 0, 4); //Arduino IP address for static IP assignment
IPAddress server  (192, 168, 0, 170); //set to MQTT host server domain or IP
char message_buff[100]; // this buffers our incoming messages so we can do something on certain commands

EthernetClient ethClient;
PubSubClient mqttclient(ethClient);

//Callback Function 

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  int i = 0;
  for (i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    message_buff[i] = payload[i];
  }
  message_buff[i] = '\0';
  String msgString = String(message_buff);
  if (msgString.equals("OFF")) {
    mqttclient.publish("Temp/Humidity", "acknowedging OFF");
  }
  else if (msgString.equals("ON")) {
    mqttclient.publish("Temp/Humidity", "acknowedging ON");
  }
  Serial.println();
}

// Reconnect loop function

void reconnect() {
  
  while (!mqttclient.connected()) 
  {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (mqttclient.connect("ArduinoClient")) 
    {
      Serial.println("connected");
      //Upon Completion Publish Announcement
      mqttclient.publish("Temp/Humidity", "ArduinoConnected");
      //Resubscribe
      mqttclient.subscribe("Temp/Humidity");
    } else 
    {
      Serial.print("failed, rc=");
      Serial.print(mqttclient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup()
{
  Serial.begin(9600);

  mqttclient.setServer(server, 1883);
  mqttclient.setCallback(callback);

  Ethernet.begin(mac, ip);

  dht.begin();
  // Allow the hardware to initialize
  delay(2000);
  Serial.println(ip);
  readTime = 0;
}

void loop()
{

  if (!mqttclient.connected()) 
  {
    reconnect();
  }
  else 
  {
    mqttclient.connect ("ArduinoClient");
  }
  mqttclient.loop();

  //check if 5 seconds has elapsed since the last time we read the sensors.
  if (millis() > readTime + 5000) {
    sensorRead();
  }

}

void sensorRead() {
  readTime = millis();
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

  char buffer[10];
  dtostrf(((1.8 * t) + 32), 0, 0, buffer);
  mqttclient.publish("Temp/Humidity", buffer);
  //Serial.println(buffer);
  dtostrf(h , 0, 0, buffer);
  mqttclient.publish("Temp/Humidity", buffer);

  //client.publish("inTopic/humidity",sprintf(buf, "%f", h));
  /*Serial.print("Humidity: ");
    Serial.print(h);
    Serial.print(" %\t");
    Serial.print("Temperature: ");
    Serial.print(t);
    Serial.print(" *C ");
    Serial.print(f);
    Serial.print(" *F\t");
    Serial.print("Heat index: ");
    Serial.print(hic);
    Serial.print(" *C ");
    Serial.print(hif);
    Serial.println(" *F"); */
}
