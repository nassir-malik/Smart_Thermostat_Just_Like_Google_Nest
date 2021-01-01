#include <Nextion.h>
extern bool unit_status;
extern bool ac_mode;
extern bool fan_status;
extern bool  temp_scale;
extern int actualTemp;
extern int setTemp;
extern int Nixtion_TX_To_Wemos_RX_PIN;
extern int Nixtion_RX_To_Wemos_TX_PIN;

                    
SoftwareSerial nextion(Nixtion_TX_To_Wemos_RX_PIN, Nixtion_RX_To_Wemos_TX_PIN);
Nextion myNextion(nextion, 9600); //create a Nextion object named myNextion using the nextion serial port @ 9600bps

void updateTemp() {
        getTemp();
  if (temp_scale==0){//(°C:0,°F:1)
    myNextion.sendCommand("bt2.picc=0");
    myNextion.setComponentText("t0", String(actualTemp));
    Serial.print("Actual Temperature is (°C)::");
    Serial.println(String(actualTemp));
  }else{
    myNextion.sendCommand("bt2.picc=1");
    myNextion.setComponentText("t0", String(actualTemp));
    Serial.print("Actual Temperature is (°F)::");
    Serial.println(String(actualTemp));
  }

}
void lcd_pcorcess(String message){
      int str_len = message.length() + 1;
    char char_array[str_len];
    message.toCharArray(char_array, str_len);
    
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
        Serial.println("Message not recegnized");
        //Serial.println(message);
        break;
    }
}
