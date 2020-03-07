#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <Ticker.h>
#include "util.h"
#include "nextion.h"
Ticker blinker;
Ticker blinker2;
const char *ssid     = "your_ssid_name";
const char *password = "your_ssid_password";

bool unit_status;
bool ac_mode;
bool fan_status;
bool  temp_scale;
int actualTemp;
int setTemp;
//###########################
const double VCC = 3.3;             // NodeMCU on board 3.3v vcc
const double resistor = 22000;            // 22k ohm series resistor
const double adc_resolution = 1023; // A0 =>10-bit adc
const double A = -0.0005057812260; //e-3  // thermistor equation parameters
const double B = 0.0004845349313; //e-4
const double C = -0.0000009438852509; //e-7
//###########################
//NexText t0 = NexText(0, 2, "t0");
uint32_t number = 0;
//char commandbuffer[50];


void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  EEPROM.begin(512);
  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( "." );
  }

  timeClient.begin();
  myNextion.init();

  blinker.attach(60, update_display_time);//update display date time every minute
  blinker2.attach(5, updateTemp);//update display date time every minute
  
  readfromeeprom();
  updatescreen();// 
}
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
void loop() {

  String message = myNextion.listen(); //check for message
  if (message != "") { // if an input recieved from touch screen process it
    lcd_pcorcess(message);
    savetoeeprom();
    readfromeeprom();
    //updatescreen();
  }

//AC= on ==> ac on , fan AUTO/ON

//Heat=on=>HEAT ON, fan AUTO/ON

//fan on
//if(unit_status==true){//ac set to on do this
//  
//if(actualTemp<setTemp && ac_mode==false){//heating is on do this
//  
//}else if(actualTemp>setTemp && ac_mode==true){//ac is on do this
//  
//}
//
//
//}

  
}


void updatescreen() {
    myNextion.setComponentText("t1", String(setTemp));
    myNextion.setComponentText("t2", "Getting Time...");

    myNextion.sendCommand(processcommand("bt0.val=%d",unit_status)); //set "b0" image to 2
    myNextion.sendCommand("ref bt0"); //refresh
    
    myNextion.sendCommand(processcommand("bt1.val=%d" ,ac_mode));
    myNextion.sendCommand("ref bt1"); //refresh
    
  if (temp_scale==0){
      myNextion.setComponentText("t0", String(getTemp("c")));
      myNextion.sendCommand(processcommand("bt2.val=%d",temp_scale));
      myNextion.sendCommand(processcommand("q0.picc=%d" ,temp_scale));
      //myNextion.sendCommand("ref bt2");
      myNextion.sendCommand("ref q0");
    }else{
        myNextion.setComponentText("t0", String(getTemp("f")));
       myNextion.sendCommand(processcommand("bt2.val=%d" , temp_scale));
        myNextion.sendCommand(processcommand("q0.picc=%d" , temp_scale));
        //myNextion.sendCommand("ref bt2");
        myNextion.sendCommand("ref q0");
    }
    //updateTemp();
}

void update_display_time() {
  timeClient.update();
  myNextion.setComponentText("t2", getTimeStampString());
  myNextion.setComponentText("t3", stime);
}
