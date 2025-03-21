/*!
 * @file  i2c.ino
 * @brief Control the voice recognition module via I2C
 * @n  Get the recognized command ID and play the corresponding reply audio according to the ID;
 * @n  Get and set the wake-up state duration
 * @copyright  Copyright (c) 2010 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @licence  The MIT License (MIT)
 * @author  [qsjhyy](yihuan.huang@dfrobot.com)
 * @version  V1.0
 * @date  2022-04-02
 * @url  https://github.com/DFRobot/DFRobot_DF2301Q
 */
#include "DFRobot_DF2301Q.h"
#include <Servo.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>

struct ServoDevice {
  Servo servo; //Servo object from ardunio lib
  char* name; //Servo name for debugin
  int max_angle; //max constraint 
  int min_angle; //max constraint
  int operating_speed; //interval between updates
  unsigned long start_time; //internal clock
  int targetDeg; //target angle
  int currentDeg; //current angle
};

enum State {
  MOVING,
  STOPPED,
  UPDATING
};
//constants
const int wake = 60;
const int nudge = 3;
const int hard = 90;
const int normal = 45;


const int panPin = 4;
const int tiltPin = 3;
const int ledPIN = 2;
const int csPIN = 10;
const int rstPIN = 8;
const int dcPIN = 9;

const int screenRefresh = 2000;
//init servo devices
ServoDevice pan = {Servo(), "Pan Servo", 180, 0, 10, 0, 90, 90};
ServoDevice tilt = {Servo(), "Tilt Servo", 180, 5, 10, 0, 90, 90};

//init display device
Adafruit_ST7789 tft = Adafruit_ST7789(csPIN, dcPIN, rstPIN);

State currState;
//voice module object
DFRobot_DF2301Q_I2C asr;

void setup() {
  Serial.begin(115200);
  
  servo_init(&pan, panPin);
  servo_init(&tilt, tiltPin);
  pinMode(ledPIN, OUTPUT);
  voice_init();
  currState = STOPPED;
  screen_init();
}



void loop() {
  
  uint8_t CMDID = asr.getCMDID();
  processIO(CMDID);



  switch (currState) {
    case MOVING:
      if (!isMoving(tilt) && !isMoving(pan)) {
        updatePos();
        updateCmd("Standby...");
        currState = STOPPED;
        break;
      }
      digitalWrite(ledPIN,HIGH);
      move_servo(&pan);
      move_servo(&tilt);
      updatePos();
      break;
    case UPDATING:
      digitalWrite(ledPIN,HIGH);
      updatePos();
      delay(1);
      if (isMoving(tilt) || isMoving(pan)) {
        currState = MOVING;
        updateCmd("Finishing...");
        delay(2);
      }
      else {
        currState = STOPPED;
        updateCmd("Standby...");
        delay(2);
      }


      break;
    case STOPPED:
      digitalWrite(ledPIN,LOW);
      break;
  }
  
  restart();
}

void checkDone() {}

bool isMoving(ServoDevice &device) {
  if(device.currentDeg != device.targetDeg) {
    return true;
  }
  else {
    return false;
  }
}


/*
@desc -
*input the command id returned by the voice module

*do the command associated with the id
*set the current state of the ardunio
*print to debug that you recieved the command
*print to the lcd that you are doing the command
*/
void processIO(uint8_t CMDID) {
  switch (CMDID) {
      case 83: //Command Word: “Set servo to 10 degrees”
        set_servo(&pan, 10);
        currState = MOVING;
        Serial.print("received: ");
        Serial.println(CMDID);
        updateCmd("Setting servo to 10 degrees...");
        break;
      
      case 87://Command Word: “Set servo to 90 degrees”
        set_servo(&pan, 90);
        currState = MOVING;
        Serial.print("received: ");
        Serial.println(CMDID); 
        updateCmd("Setting servo to 90 degrees...");
        break;

      case 5: //Command Word: "Right"
        set_servo(&pan, pan.currentDeg + normal);
        currState = MOVING;
        Serial.print("received: ");
        Serial.println(CMDID);
        updateCmd("Panning Right...");
        break;

      case 6: //Command Word: "Hard Right"
        set_servo(&pan, pan.currentDeg + hard);
        currState = MOVING;
        Serial.print("received: ");
        Serial.println(CMDID); 
        updateCmd("Panning Hard Right...");
        break;

      case 7: //Command Word: "Nudge Right"
        set_servo(&pan, pan.currentDeg + nudge);
        currState = MOVING;
        Serial.print("received: ");
        Serial.println(CMDID);
        updateCmd("Nudging Right...");
        break;
      case 8: //Command Word: "Left"
        set_servo(&pan, pan.currentDeg - normal);
        currState = MOVING;
        Serial.print("received: ");
        Serial.println(CMDID);
        updateCmd("Panning Left...");
        break;
      case 9: //Command Word: "Hard Left"
        set_servo(&pan, pan.currentDeg - hard);
        currState = MOVING;
        Serial.print("received: ");
        Serial.println(CMDID); 
        updateCmd("Panning Hard Left...");
        break;
      case 10: //Command Word: "Nudge Left"
        set_servo(&pan, pan.currentDeg - nudge);
        currState = MOVING;
        Serial.print("received: ");
        Serial.println(CMDID); 
        updateCmd("Nudging Left...");
        break;
      case 11: //Command Word: "down"
        set_servo(&tilt, tilt.currentDeg - normal);
        currState = MOVING;
        Serial.print("received: ");
        Serial.println(CMDID); 
        updateCmd("Tilting Down...");
        break;
      case 12: //Command Word: "nudge down"
        set_servo(&tilt, tilt.currentDeg - hard);
        currState = MOVING;
        Serial.print("received: ");
        Serial.println(CMDID);
        updateCmd("Nudging Down"); 
        break;
      case 13: //Command Word: "up"
        set_servo(&tilt, tilt.currentDeg + normal);
        currState = MOVING;
        Serial.print("received: ");
        Serial.println(CMDID); 
        updateCmd("Tilting Up...");
        break;
      case 14: //Command Word: "nudge up"
        set_servo(&tilt, tilt.currentDeg + nudge);
        currState = MOVING;
        Serial.print("received: ");
        Serial.println(CMDID);
        updateCmd("Nudging Down...");
        break;
      case 15: //Command Word: "stop"
        currState = STOPPED;
        set_servo(&pan, pan.currentDeg);
        set_servo(&tilt, tilt.currentDeg);
        Serial.print("received: ");
        Serial.println(CMDID); 
        updateCmd("Stopping All Movement...");
        break;
      case 16: //Command Word: "slow"
        currState = UPDATING;
        pan.operating_speed = pan.operating_speed * 4 + 1;
        tilt.operating_speed = tilt.operating_speed * 4 + 1;
        Serial.print("received: ");
        Serial.println(CMDID);
        updateCmd("Decreasing Speed...");
        break;
      case 17: //Command Word: "fast"
        currState = UPDATING;
        pan.operating_speed /= 4;
        tilt.operating_speed /= 4;
        Serial.print("received: ");
        Serial.println(CMDID);
        updateCmd("Increasing Speed...");
        break;
      default:
        if (CMDID != 0) {
          Serial.print("CMDID = "); 
          Serial.println(CMDID);
        }
    }


}

/*
@desc -
*set the target degree for a servo device
*/
void set_servo(ServoDevice *ser, int degree) {
  ser->targetDeg = constrain(degree, ser->min_angle, ser->max_angle);
}

/*
@desc - 
restart the wake time for the voice module to avoid overflow and non-response
*/
void restart() {
  static unsigned long lastTime = 0;
  if (millis() - lastTime >= 20000) { // Check if 5 seconds have passed
    lastTime = millis(); // Update lastTime
    asr.setWakeTime(wake);
  }
}

/*
@desc-

move the servo device. 
operating speed is the interval between digital writes.
T.F. speed is inversly related to operating_speed var
*/
void move_servo(ServoDevice *ser) {

  if (millis() - ser->start_time > ser->operating_speed)
  {
    ser->start_time = millis();
    if (ser->targetDeg < ser->currentDeg)
    {
      ser->currentDeg--;
      ser->servo.write(ser->currentDeg);
    }
    else if (ser->targetDeg > ser->currentDeg)
    {
      ser->currentDeg++;
      ser->servo.write(ser->currentDeg);
    }
  }
}

void servo_init(ServoDevice *ser, int pin) {
  ser->servo.attach(pin);
  if(!ser->servo.attached()) {
      Serial.println("Failed to Init Servo " + String(ser->name));
  }
  ser->servo.write(ser->currentDeg);
  delay(1000);
  Serial.println(String(ser->name) + " successfull init");
}

void voice_init() {

  while (!(asr.begin())) {
      Serial.println("Communication with device failed, please check connection");
      delay(3000);
    }
    Serial.println("Begin ok!");

    /**
    * @brief Set voice volume
    * @param voc - Volume value(1~7)
    */
    asr.setVolume(4);

    /**
      @brief Set mute mode
      @param mode - Mute mode; set value 1: mute, 0: unmute
    */
    asr.setMuteMode(1);

    /**
      @brief Set wake-up duration
      @param wakeTime - Wake-up duration (0-255)
    */
    asr.setWakeTime(wake);

    /**
      @brief Get wake-up duration
      @return The currently-set wake-up period
    */
    uint8_t wakeTime = 0;
    wakeTime = asr.getWakeTime();
    Serial.print("wakeTime = ");
    Serial.println(wakeTime);

    // asr.playByCMDID(1);   // Wake-up command

    /**
      @brief Play the corresponding reply audio according to the ID
      @param CMDID - command word ID
    */
    //asr.playByCMDID(23);  // Command word ID

}

void screen_init() {
  tft.init(240, 320);
  tft.setFont();
  tft.setRotation(1);
  tft.fillScreen(ST77XX_BLACK);
  tft.enableDisplay(true);
  updateCmd("Say \"Hello Robot\" To Begin");

  tft.setCursor(5, 105); 
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(3); 
  tft.print("Servo Pan: ");

  tft.setCursor(5, 145);  
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(3); 
  tft.print("Servo Tilt: ");

  tft.setCursor(5, 185);  
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(3); 
  tft.print("Servo Speed: ");

  tft.drawRect(0,90,320,140, ST77XX_WHITE);

  updatePos();
}

/*
@desc -

updates the lcd values of servo speed and current degree
*/
void updatePos() {
  static unsigned long lastUpdateTime = 0;
  int rectX = 240;
  int rectY = 105;
 
  Serial.println("Updating Screen");
  lastUpdateTime = millis();  
  tft.fillRect(240,105,75,115,ST77XX_BLACK);
  
  
  tft.setCursor(rectX,rectY);
  tft.setTextColor(ST77XX_RED);
  tft.setTextSize(3); 
  tft.print(pan.currentDeg);

  
  tft.setCursor(rectX,rectY+40);
  tft.setTextColor(ST77XX_RED);
  tft.setTextSize(3); 
  tft.print(tilt.currentDeg);

  
  tft.setCursor(rectX,rectY+80);
  tft.setTextColor(ST77XX_RED);
  tft.setTextSize(3); 
  tft.print(tilt.operating_speed);
  
    
}

/*
@desc -
Updates lcd to display the current command being processed
*/
void updateCmd(char* cmdWord) {
    tft.fillRect(0,0,320,90,ST77XX_BLACK);
    // Clear only the area that will be updated (to improve efficiency)
    tft.setCursor(0, 0);  // Move cursor for the pan status
    tft.setTextColor(ST77XX_YELLOW);
    tft.setTextSize(2); 
    tft.print(cmdWord);
}
