#include <Nextion.h>
//#include "util.h"
extern bool unit_status;
extern bool ac_mode;
extern bool fan_status;
extern bool  temp_scale;
extern int actualTemp;
extern int setTemp;
                      //D7, D6
SoftwareSerial nextion(12, 13);// Nextion RX to pin D7(TX) and TX to pin D6(RX) of Arduino
Nextion myNextion(nextion, 9600); //create a Nextion object named myNextion using the nextion serial port @ 9600bps

void updateTemp() {
          Serial.print("Temp is ::");// 0=c 1=f
        
  if (temp_scale==0){//(°C:0,°F:1)
    myNextion.setComponentText("t0", String(getTemp("c")));
    //myNextion.setComponentText("t1", String(getTemp("c")));
    Serial.println(getTemp("c"));
  }else{
    myNextion.setComponentText("t0", String(getTemp("f")));
    //myNextion.setComponentText("t1", String(getTemp("f")));
    Serial.println(getTemp("f"));
  }
}
void lcd_pcorcess(String message){
      int str_len = message.length() + 1;
    char char_array[str_len];
    message.toCharArray(char_array, str_len);
    
   //myNextion.setComponentText("t0", "00");
      //int brightness = analogRead(A0);
  //int bright = map(100, 0, 1024, 0, 100);
  //String dim = "dim=" + String(bright);
  //myNextion.sendCommand(dim.c_str());

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//room 21.30 kohm 71f
//ice  ?
//boilling ?

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
Serial.println(message);
    switch ((int) strtol(&char_array[5], 0, 16)) {
      case 1://65 00 01 01 ff ff ff 
        myNextion.buttonToggle(unit_status, "bt0", 0, 2);
        Serial.print("unit on/off");
        Serial.println(unit_status);
        break;
      case 2://65 00 02 01 ff ff ff
        myNextion.buttonToggle(ac_mode, "bt1", 0, 2);
        Serial.print("cool/heat");
        Serial.println(ac_mode);
        break;
      case 7://65 00 07 01 ff ff ff
        myNextion.buttonToggle(temp_scale, "bt2", 0, 2);
        Serial.print("Pressed C/F");// 0=c 1=f
        Serial.println(temp_scale);
        updateTemp();
        myNextion.sendCommand("ref bt2"); //refresh
        break;
      case 5://65 00 05 01 ff ff ff
        Serial.println("temp up");//max 90F or 
        if(setTemp<90){setTemp +=1;}
        myNextion.setComponentText("t1", String(setTemp));
        Serial.println(setTemp);
        break;
      case 6://65 00 06 01 ff ff ff
        Serial.println("temp down");// min temp 60F
        if(setTemp>60){setTemp -=1;}
        myNextion.setComponentText("t1", String(setTemp));
        Serial.println(setTemp);
        break;
      case 13://65 00 0D 01 ff ff ff
        Serial.println("Fan Auto/On");
        break;
      case 11://65 00 0B 01 ff ff ff
        Serial.println("manu");
        break;
      default:
        //Serial.println("Message not recegnized");
        //Serial.println(message);
        break;
    }
}
