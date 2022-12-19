#include <OneWire.h>
#include <ESP8266WiFi.h>
#include "ThingSpeak.h"

const char* ssid = "Redmi 8";   // your network SSID (name) 
const char* password = "11111111";   // your network password

WiFiClient  client;

unsigned long MyChannelNumber = 1;
const char * myWriteAPIKey = "BCTXLYWJ8AD0UW76";

// Timer variables
unsigned long lastTime = 0;
unsigned long timerDelay = 16000;
 

// Variable to hold temperature readings
float temperatureC;

int vibration_Pin = A0;
int DS18S20_Pin =D5; //DS18S20 Signal pin on digital 1
int Buzzer_Pin= D3;

//Temperature chip i/o
OneWire ds(DS18S20_Pin);  // on digital pin 2

void setup() {
  Serial.begin(115200);  //Initialize serial
  
  WiFi.mode(WIFI_STA);   
  
  ThingSpeak.begin(client);  // Initialize ThingSpeak
  
  pinMode(Buzzer_Pin, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {

  
  
  if ((millis() - lastTime) > timerDelay) {
    
    // Connect or reconnect to WiFi
    if(WiFi.status() != WL_CONNECTED){
      Serial.print("Attempting to connect");
      while(WiFi.status() != WL_CONNECTED){
        WiFi.begin(ssid, password); 
        delay(5000);     
      } 
      Serial.println("\nConnected.");
    }

    //Code For Vibration Sensor
    int vibration;
    vibration=analogRead(vibration_Pin);//Connect the sensor to analog pin 0
    Serial.print("Vibration read: ");
    Serial.println(vibration,DEC);//

 
    //Code For Temperature Sensor
    float temperature = getTemp();
    Serial.print("Temperature read: ");
    Serial.print(temperature);
    Serial.println(" C");   
  
  if ((temperature>15) || (vibration>20)){
    digitalWrite(Buzzer_Pin, HIGH); // sets the digital pin 13 on
    digitalWrite(LED_BUILTIN, LOW);
    delay(1000);            // waits for a second
    digitalWrite(Buzzer_Pin, LOW);  // sets the digital pin 13 off
    digitalWrite(LED_BUILTIN, HIGH);
    delay(1000);            // waits for a second
  }
  //delay(500); //just here to slow down the output so it is easier to read


    // set the fields with the values
    ThingSpeak.setField(2, temperature);
    ThingSpeak.setField(1, vibration);

     // Write to ThingSpeak. There are up to 8 fields in a channel, allowing you to store up to 8 different
    // pieces of information in a channel.  Here, we write to field 1.
    int x = ThingSpeak.writeFields(MyChannelNumber,myWriteAPIKey);

    if(x == 200){
      Serial.println("Channel update temp successful.");
    }
    else{
      Serial.println("Problem updating channel Temp. HTTP error code " + String(x));
    }
    lastTime = millis();
}

}

float getTemp(){
  //returns the temperature from one DS18S20 in DEG Celsius

  byte data[12];
  byte addr[8];

  if ( !ds.search(addr)) {
      //no more sensors on chain, reset search
      ds.reset_search();
      return -1000;
  }

  if ( OneWire::crc8( addr, 7) != addr[7]) {
      Serial.println("CRC is not valid!");
      return -1001;
  }

  if ( addr[0] != 0x10 && addr[0] != 0x28) {
      Serial.print("Device is not recognized");
      return -1002;
  }

  ds.reset();
  ds.select(addr);
  ds.write(0x44,1); // start conversion, with parasite power on at the end

  byte present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE); // Read Scratchpad

  
  for (int i = 0; i < 9; i++) { // we need 9 bytes
    data[i] = ds.read();
  }
  
  ds.reset_search();
  
  byte MSB = data[1];
  byte LSB = data[0];

  float tempRead = ((MSB << 8) | LSB); //using two's compliment
  float TemperatureSum = tempRead / 16;
  
  return TemperatureSum;
  
}
