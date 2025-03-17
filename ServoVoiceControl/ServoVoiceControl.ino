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
const int nudge = 3;
const int panPin = 10;
const int tiltPin = 9;
ServoDevice pan = {Servo(), "Pan Servo", 180, 0, 10, 0, 90, 90};
ServoDevice tilt = {Servo(), "Tilt Servo", 180, 5, 10, 0, 90, 90};

//I2C communication
DFRobot_DF2301Q_I2C asr;

void setup() {
  Serial.begin(9600);

  servo_init(&pan, panPin);
  servo_init(&tilt, tiltPin);
  
  voice_init();
  
}

void loop() {

  //gets the command ID
  uint8_t CMDID = asr.getCMDID();

  //voice state
  switch (CMDID) {
    case 83: //Command Word: “Set servo to 10 degrees”
      set_servo(&pan, 10);
      Serial.println("received: " + CMDID); 
      break;
    
    case 87://Command Word: “Set servo to 90 degrees”
      set_servo(&pan, 90);
      Serial.println("received: " + CMDID);  
      break;

    case 5: //Command Word: "Right"
      set_servo(&pan, pan.currentDeg + 45);
      Serial.println("received: " + CMDID); 
      break;

    case 6: //Command Word: "Hard Right"
      set_servo(&pan, pan.currentDeg + 90);
      Serial.println("received: " + CMDID);  
      break;

    case 7: //Command Word: "Nudge Right"
      set_servo(&pan, pan.currentDeg + nudge);
      Serial.println("received: " + CMDID); 
      break;
    case 8: //Command Word: "Left"
      set_servo(&pan, pan.currentDeg - 45);
      Serial.println("received: " + CMDID); 
      break;
    case 9: //Command Word: "Hard Left"
      set_servo(&pan, pan.currentDeg - 90);
      Serial.println("received: " + CMDID); 
      break;
    case 10: //Command Word: "Nudge Left"
      set_servo(&pan, pan.currentDeg - nudge);
      Serial.println("received: " + CMDID); 
      break;
    case 11: //Command Word: "down"
      set_servo(&tilt, tilt.currentDeg - 45);
      Serial.println("received: " + CMDID); 
      break;
    case 12: //Command Word: "nudge down"
      set_servo(&tilt, tilt.currentDeg - nudge);
      Serial.println("received: " + CMDID); 
      break;
    case 13: //Command Word: "up"
      set_servo(&tilt, tilt.currentDeg + 45);
      Serial.println("received: " + CMDID); 
      break;
    case 14: //Command Word: "nudge up"
      set_servo(&tilt, tilt.currentDeg + nudge);
      Serial.println("received: " + CMDID); 
      break;
    case 15: //Command Word: "stop"
      set_servo(&pan, pan.currentDeg);
      set_servo(&tilt, tilt.currentDeg);
      Serial.println("received: " + CMDID); 
      break;
    case 16: //Command Word: "slow"
      pan.operating_speed = pan.operating_speed * 4 + 1;
      tilt.operating_speed = tilt.operating_speed * 4 + 1;
      Serial.println("received: " + CMDID);
      break;
    case 17: //Command Word: "fast"
      pan.operating_speed /= 4;
      tilt.operating_speed /= 4;
      Serial.println("received: " + CMDID);
      break;
    default:
      if (CMDID != 0) {
        Serial.print("CMDID = "); 
        Serial.println(CMDID);
      }
  }

  move_servo(&pan);
  move_servo(&tilt);
}



void set_servo(ServoDevice *ser, int degree) {
  ser->targetDeg = constrain(degree, ser->min_angle, ser->max_angle);
}

void move_servo(ServoDevice *ser) {
  //del = constrain(del, 0, 32000);
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
    asr.setMuteMode(0);

    /**
      @brief Set wake-up duration
      @param wakeTime - Wake-up duration (0-255)
    */
    asr.setWakeTime(20);

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
