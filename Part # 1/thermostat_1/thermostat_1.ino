#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <Ticker.h>
#include "util.h"
#include "nextion.h"
Ticker blinker;
Ticker blinker2;
Ticker blinker3;
const char *ssid     = "your_ssid_name";
const char *password = "ssid_passoword";

bool unit_status;
bool ac_mode;
bool fan_status;
bool temp_scale;
int actualTemp=71;
int setTemp;

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
  blinker2.attach(5, updateTemp);//update display with current temp every 5 seconds
  blinker3.attach(5, unitMainControl);
  readfromeeprom();
  updatescreen();
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
//unitMainControl();

yield();
  
}

void unitMainControl(){
 //AC= on ==> ac on , fan AUTO/ON
//Heat=on=>HEAT ON, fan AUTO/ON
//
Serial.println("################");
if(unit_status==true){//Unit is set to on then do this
  Serial.println("Unit is on");
  if(actualTemp<setTemp && ac_mode==false){//heating is on do this
    Serial.println("Heating is on");
  }else if(actualTemp>setTemp && ac_mode==true){//ac is on do this
    Serial.println("AC is on");
  }else{
    Serial.println("Unit is paused");
  }
} 
Serial.println("################");
}

void updatescreen() {
    myNextion.setComponentText("t1", String(setTemp));
    myNextion.setComponentText("t2", "Getting Time...");

    myNextion.sendCommand(processcommand("bt0.val=%d",unit_status)); //set "b0" image to 2
    myNextion.sendCommand("ref bt0"); //refresh
    
    myNextion.sendCommand(processcommand("bt1.val=%d" ,ac_mode));
    myNextion.sendCommand("ref bt1"); //refresh
    
  if (temp_scale==0){
      myNextion.setComponentText("t0", "71");
      myNextion.sendCommand(processcommand("bt2.val=%d",temp_scale));
      myNextion.sendCommand(processcommand("q0.picc=%d" ,temp_scale));
      //myNextion.sendCommand("ref bt2");
      myNextion.sendCommand("ref q0");
    }else{
        myNextion.setComponentText("t0", "71");
       myNextion.sendCommand(processcommand("bt2.val=%d" , temp_scale));
        myNextion.sendCommand(processcommand("q0.picc=%d" , temp_scale));
        //myNextion.sendCommand("ref bt2");
        myNextion.sendCommand("ref q0");
    }
}

void update_display_time() {
  timeClient.update();
  myNextion.setComponentText("t2", getTimeStampString());
  myNextion.setComponentText("t3", stime);
}
