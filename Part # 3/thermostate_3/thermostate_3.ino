#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <Ticker.h>
#include "util.h"
#include "nextion.h"
Ticker timer1;
Ticker timer2;
Ticker timer3;
const char *ssid     = "Enter_you_AP_Name";
const char *password = "Enter_AP_Password";
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
int cool_pin=2;//D4
int fan_pin=5;//D1
int heat_pin=0;//D3
// Nextion TX to pin D7(RX) and RX to pin D6(TX) of Wemos mini
extern int Nixtion_TX_To_Wemos_RX_PIN=13;//D7
extern int Nixtion_RX_To_Wemos_TX_PIN=12;//D6
//###########################
const double VCC = 3.3;             // Wemos on board 3.3v vcc
const double resistor = 22000;      // 22k ohm series resistor
const double adc_resolution = 1023; // A0 =>10-bit adc
const double A = -0.0005057812260; //e-3  // thermistor equation parameters
const double B = 0.0004845349313; //e-4
const double C = -0.0000009438852509; //e-7
//###########################
double fan_stop_delay = .5;//fan will stop with given delay once AC/Heat is stopped 
const int numReadings = 20;// number of readings to smoth out the analog noise
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
bool unit_status;
bool ac_mode;
bool fan_status;
bool  temp_scale;
int actualTemp;
int setTemp;
uint32_t number = 0;
unsigned long StartTime = 0;
unsigned long CurrentTime =0;
int readings[numReadings];      // the readings from the analog input
int readIndex = 0;              // the index of the current reading
int total = 0;                  // the running total
int average = 0;                // the average

int inputPin = A0;
double adc_value;
void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  EEPROM.begin(512);
  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( "." );
  }


  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    readings[thisReading] = 0;
  }
  myNextion.setComponentText("t2", "Getting Time...");
  timeClient.begin();
  myNextion.init();

  running_avg();
  readfromeeprom();
  updatescreen();
  unitMainControl();
  timer1.attach(60, update_display_time_and_temp);//update display date time every minute
  timer2.attach(15, unitMainControl);//main contol run every 15 sec to control unit
  timer3.attach(1, running_avg);
  
  
 
pinMode(cool_pin, OUTPUT);
pinMode(heat_pin, OUTPUT);
pinMode(fan_pin, OUTPUT);
digitalWrite(cool_pin, LOW);
digitalWrite(heat_pin, LOW);
digitalWrite(fan_pin, LOW);
}
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
void loop() {

  String message = myNextion.listen(); //check for message
  if (message != "") { // if an input recieved from touch screen process it
    lcd_pcorcess(message);
    savetoeeprom();
    readfromeeprom();
  }
  
}

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
void unitMainControl(){
  updatescreen();
//AC= on ==> ac on , fan AUTO/ON
//Heat=on=>HEAT ON, fan AUTO/ON
Serial.println("################");
Serial.print("Actual Temperature is::");
Serial.println(actualTemp);
Serial.print("Desired Temperature is::");
Serial.println(setTemp);
if(ac_mode==false){
  Serial.println("Heat is ON");
}else{
  Serial.println("Ac is ON");
}

if(unit_status==true){
  Serial.println("Unit is ON");
}else{
  Serial.println("Unit is OFF");
}

    //Serial.println("Unit is on");
  if(actualTemp<setTemp && ac_mode==false && unit_status==true){//heating is on do this
    //Serial.println("Heating is on");
     digitalWrite(cool_pin, LOW);
    digitalWrite(fan_pin, HIGH);
    digitalWrite(heat_pin, HIGH);
  }else if(actualTemp>setTemp && ac_mode==true && unit_status==true){//ac is on do this
    //Serial.println("AC is on");
    digitalWrite(heat_pin, LOW);
    digitalWrite(fan_pin, HIGH);
    digitalWrite(cool_pin, HIGH);
  }else{
    Serial.println("Unit is paused");
    digitalWrite(cool_pin, LOW);
    digitalWrite(heat_pin, LOW);
    
    //##### Delay to turn off the FAN by given time//
        if(StartTime==0){ StartTime = millis();}
            CurrentTime = millis();
        if (CurrentTime - StartTime>(60*1000*fan_stop_delay))
        {
          digitalWrite(fan_pin, LOW);
          StartTime = 0;
          CurrentTime= 0;
        }
    //##### Delay to turn off the FAN by given time//
  }
  
Serial.println("################");
}
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
void updatescreen() {
    updateTemp();
    myNextion.setComponentText("t1", String(setTemp));
    //myNextion.setComponentText("t2", "Getting Time...");

    myNextion.sendCommand(processcommand("bt0.val=%d",unit_status)); //set "b0" image to 2
    myNextion.sendCommand("ref bt0"); //refresh
    
    myNextion.sendCommand(processcommand("bt1.val=%d" ,ac_mode));
    myNextion.sendCommand("ref bt1"); //refresh
    myNextion.sendCommand(processcommand("q1.picc=%d" ,ac_mode));
    
  if (temp_scale==0){
      myNextion.setComponentText("t0", String(actualTemp));
      myNextion.sendCommand(processcommand("bt2.val=%d",temp_scale));
      myNextion.sendCommand(processcommand("q0.picc=%d" ,temp_scale));
      myNextion.sendCommand("ref q0");
    }else{
        //myNextion.setComponentText("t0", String(getTemp("f")));
        myNextion.setComponentText("t0", String(actualTemp));
        myNextion.sendCommand(processcommand("bt2.val=%d" , temp_scale));
        myNextion.sendCommand(processcommand("q0.picc=%d" , temp_scale));
        myNextion.sendCommand("ref q0");
    }

}

void update_display_time_and_temp() {
  timeClient.update();
  myNextion.setComponentText("t2", getTimeStampString());
  myNextion.setComponentText("t3", stime);
}

void running_avg(){
 // subtract the last reading:
  total = total - readings[readIndex];
  // read from the sensor:
  readings[readIndex] = analogRead(A0);
  // add the reading to the total:
  total = total + readings[readIndex];
  // advance to the next position in the array:
  readIndex = readIndex + 1;

  // if we're at the end of the array...
  if (readIndex >= numReadings) {
    // ...wrap around to the beginning:
    readIndex = 0;
  }
  // calculate the average:
  average = total / numReadings;
  adc_value= average ;
  //updateTemp();
  //Serial.println(adc_value);
}
