/*
 * Fly Greenhouse Control by Francisco Branco
 * Sends emails hourly to from CEDOC.fly.control@gmail.com to same email, via SMTP2GO service
 * Registers in such email the temperature and humidity
 * 
 * username: CEDOC.fly.control@gmail.com
 * password: Save(the)Flies
 * 
 * Email client sketch for IDE v1.0.5 and w5100/w5200
 * Posted 7 May 2015 by SurferTim
 * 
 * Sensor data on DHT11 embedded on email SMTP2GO service client by Francisco Branco
 * 
 * Help from Arduino examples (more information on https://www.arduino.cc/)
 * 
 * Changelog:
 * 
 * - 1.1 added particular recipients for the alert (only when out of the acceptable temperature range)
 * 
 * - 1.0 debugged with basic functions as described before
 * 
 * Contacts:
 * francisco.branco@tecnico.ulisboa.pt
 * 
*/
 
#include <SPI.h>
#include <Ethernet.h>

#include "dht.h"
dht DHT; // Creats a DHT 

// this must be unique (can be assigned anything other than an existing mac address on the netwrok)
byte mac[] = { 0x90, 0xA2, 0xDA, 0x00, 0x59, 0x67 };
// change network settings to yours
// ip must be of the kinds of the ones existing in same network (hint: use DhcpAddressPrinter)
IPAddress ip( 10,20,0,149 );
// gateway must be the one found on your router
IPAddress gateway( 10,20,0,1 );
IPAddress myDns( 10,40,0,13 );
// subnet must be the same kind as mask on network
IPAddress subnet( 255, 255, 255, 0 );

char server[] = "mail.smtp2go.com"; // from SMTP2GO
// Sometimes need to change port
int port = 80;
 
EthernetClient client;

static const uint8_t analog_pins[] = {A0,A1,A2,A3,A4};

String str1 = "ALERT! Incubator ";
String str2 = "is at ";
String str3 = "degrees Celsius!";

unsigned long minute = 60000;
unsigned long previousMillis = 0;
int interval = 60;  // decide the interval of minutes between emails
int first = 1;
int rounds = 0; // VERY IMPORTANT VARIABLE - tells you in which state the system is (0 = normal log; 1 = alert; 2 = waiting alert)
int mins = 0;
int num_sensor = 5; // number of sensors connected

int correction_t[] = { -3, 1, 0, 1, 2 };  // correction for each sensor
int correction_h[] = { 10, -5, 10, -10, 10 };

void setup() {
  pinMode(LED_BUILTIN, OUTPUT); // blinking led when email is sent

  Serial.begin(115200);
  pinMode(4,OUTPUT);  // pins used by the ethernet shield
  digitalWrite(4,HIGH);
  if(Ethernet.begin(mac) == 0) {
    Ethernet.begin(mac, ip, myDns, gateway, subnet);
    Serial.println("swag");
  }
  
  delay(30000);
}

void loop() {

  unsigned long currentMillis = millis();
  String alert = "\0";
  float t[4] = {0};
  float h[4] = {0};
  String body = "\0";


  for(int i = 0; i < num_sensor; i++) {
    DHT.read11(analog_pins[i]); // Reads the data from the sensor
    t[i] = DHT.temperature + correction_t[i]; // Gets the values of the temperature
    h[i] = DHT.humidity + correction_h[i]; // Gets the values of the humidity

    alert = alert + "I" + i + ": "+ t[i] + " C, " + h[i] + " %" + "\t"; // creates alert sentence
    Serial.println(alert);

    /*if (t[i] >= 20 || t[i] <= 15) {
      alert = "Attention!! Fluctuation";
      
      body = body + "Problem on " + "I" + i + ": "+ t[i] + " C" + "\n";

      if (rounds != 2) {
        first = 1;
        rounds = 1;
      }
    }
    if (h[i] >= 90 || h[i] <= 50) {
      alert = "Attention!! Fluctuation";
      
      body = body + "Problem on " + "I" + i + ": "+ h[i] + " %" + "\n";

      if (rounds != 2) {
        first = 1;
        rounds = 1;
      }
    }*/
   
  }
  
  if (currentMillis - previousMillis >= minute || first == 1) { // can add functionality to request email here
    // save the last time you sent and email
    previousMillis = currentMillis;

    if(mins > interval || first == 1) {
      if(sendEmail(alert)) Serial.println(F("Email sent"));  // sends emails
      else Serial.println(F("Email failed"));

      digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
      delay(1000);                       // wait for a second
      digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
      delay(1000);                       // wait for a second

      mins = 0; // time reset
      first = 0;  // this variable overides the system for the first email
      if (rounds == 1) rounds = 2;
      else rounds = 0;
    }
    mins = mins + 1;
  }
  
  /*// Printing the results on the serial monitor
  for (int i = 0; i < 5; i++) {
    Serial.print("Temperature = "); // debugging material
    Serial.println(t[i]);
  }
  DHT.read11(A0);
  Serial.println(DHT.temperature);*/

  //Serial.println("Waiting...");
  delay(2000); // wait 2 seconds
}








 

 

 
byte sendEmail(String alert)
{
  byte thisByte = 0;
  byte respCode;
 
  if(client.connect(server,port) == 1) {
    Serial.println(F("connected"));
  } else {
    Serial.println(F("connection failed"));
    return 0;
  }
 
  if(!eRcv()) return 0;
 
  Serial.println(F("Sending hello"));
// replace 1.2.3.4 with your Arduino's ip
// MAJOR CHANGE, SEE IF WORKS (10, 71, 246, 134)
  client.println("EHLO 10, 20, 0, 149");
  if(!eRcv()) return 0;
 
  Serial.println(F("Sending auth login"));
  client.println("auth login");
  if(!eRcv()) return 0;
 
  Serial.println(F("Sending User"));
// Change to your base64 encoded user
  client.println("Q0VET0MuZmx5LmNvbnRyb2xAZ21haWwuY29t");
 
  if(!eRcv()) return 0;
 
  Serial.println(F("Sending Password"));
// change to your base64 encoded password
  client.println("aWY4TG1ZbzZvVEtD");
 
  if(!eRcv()) return 0;
 
// change to your email address (sender)
  Serial.println(F("Sending From"));
  client.println("MAIL From: <CEDOC.fly.control@gmail.com>");
  if(!eRcv()) return 0;
 
// change to recipient address
  Serial.println(F("Sending To"));
  client.println("RCPT To: <CEDOC.fly.control@gmail.com>");
  //else client.println("RCPT To: <CEDOC.fly.control@gmail.com> <cesar.mendes@nms.unl.pt> <francisco.branco@tecnico.ulisboa.pt>");
  if(!eRcv()) return 0;
 
  Serial.println(F("Sending DATA"));
  client.println("DATA");
  if(!eRcv()) return 0;
 
  Serial.println(F("Sending email"));
 
// change to recipient address
  client.println("To: You <CEDOC.fly.control@gmail.com>");
  //else client.println("To: You <CEDOC.fly.control@gmail.com> <cesar.mendes@nms.unl.pt> <francisco.branco@tecnico.ulisboa.pt>");
 
// change to your address
  client.println("From: Me <CEDOC.fly.control@gmail.com>");
 
  client.println("Subject: " + alert + "\r\n");

  client.println(alert + "\r\n");
  //client.println(alert + "\r\n" + body + "\r\n");
  //else client.println(body + "\r\n");
 
  client.println(".");
 
  if(!eRcv()) return 0;
 
  Serial.println(F("Sending QUIT"));
  client.println("QUIT");
  if(!eRcv()) return 0;
 
  client.stop();
 
  Serial.println(F("disconnected"));
 
  return 1;
}
 
byte eRcv()
{
  byte respCode;
  byte thisByte;
  int loopCount = 0;
 
  while(!client.available()) {
    delay(1);
    loopCount++;
 
    // if nothing received for 10 seconds, timeout
    if(loopCount > 10000) {
      client.stop();
      Serial.println(F("\r\nTimeout"));
      return 0;
    }
  }
 
  respCode = client.peek();
 
  while(client.available())
  {  
    thisByte = client.read();    
    Serial.write(thisByte);
  }
 
  if(respCode >= '4')
  {
    efail();
    return 0;  
  }
 
  return 1;
}
 
 
void efail()
{
  byte thisByte = 0;
  int loopCount = 0;
 
  client.println(F("QUIT"));
 
  while(!client.available()) {
    delay(1);
    loopCount++;
 
    // if nothing received for 10 seconds, timeout
    if(loopCount > 10000) {
      client.stop();
      Serial.println(F("\r\nTimeout"));
      return;
    }
  }
 
  while(client.available())
  {  
    thisByte = client.read();    
    Serial.write(thisByte);
  }
 
  client.stop();
 
  Serial.println(F("disconnected"));
}
