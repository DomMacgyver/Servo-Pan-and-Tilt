

#define CUSTOM_SETTINGS
#define INCLUDE_TERMINAL_SHIELD




#include "MeccaBrain.h"
#include <Keypad.h>

const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns
//define the cymbols on the buttons of the keypads
char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {7, 6, 5, 4}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {3, 2, 8, 9}; //connect to the column pinouts of the keypad

//initialize an instance of class NewKeypad
Keypad customKeypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);
MeccaBrain servo(12);//servo data pin is connected to digital pin 5
int scale = 80;
int pitch_velocity_upper = scale;
int pitch_velocity_lower = -scale;
int yaw_velocity_upper = scale;
int yaw_velocity_lower = -scale;
int pitch_accel = 80;
int yaw_accel = pitch_accel;
float p_point2, y_point2;
int res = scale / 20; // resolution
float threshold = scale / 20 + 1 ;

int values[4] = {0, 0, 0, 0};
String search = "ABCD";
int pad = 1;// 1 for pad, 2 not used, 3 for joystick
int temp_val = 0;
int key_state = 0;
String InString = "";
char key;
int moving;
int pitch_dest;
int yaw_dest;
int x, y;
int x_offset = 0;
int y_offset = 0;
float x_average, y_average = 0;
int pitch_velocity = 0;
int yaw_velocity = 0;
int pitch_velocity2 = 0;
int yaw_velocity2 = 0;
float sy, sx, by, bx, sy2, sx2, by2, bx2;
int delay1 = 1;
float alpha = 0.8;
float ay = alpha, ax = alpha;
float betay = 0.0, betax = 0.0;

int calibration_accuracy = 30;
int JoyStick_X = 0; //x
int JoyStick_Y = 1; //y
int initial_pitch = 90;
int pitch = initial_pitch;
int pitch2;
int initial_yaw = 90;
int yaw = initial_yaw;
int yaw2;
unsigned long int time1, time2, time3;
float time12, time22;
char val = '1';

int pneg = 0;
int yneg = 0;

//booleans
bool p_stat, p_stat2, y_stat, y_stat2 = true;

void setup() {
  Serial.begin(9600);
  Serial.println("pitch:");

  Serial.println(pitch, DEC);
  pinMode(13, OUTPUT);  //laser in pin 13
  pinMode(JoyStick_X, INPUT);
  pinMode(JoyStick_Y, INPUT);
  //Serial.begin(9600);
  //set the servo position to home, don't know why for loop is needed, just works with it
  for (int iter = 0; iter < 20; iter++)
  {
    servo.setServoPosition(0, map(yaw, 0, 180, 0x18, 0xE8));
    servo.setServoPosition(1, map(pitch, 0, 180, 0x18, 0xE8));
    servo.communicate();//initiate servo communication

    delay(15);
  }

  for (int iter = 0; iter < calibration_accuracy; iter++)
  {
    x_average += analogRead(JoyStick_X);
    y_average += analogRead(JoyStick_Y);
    delay(10);
  }
  x_average = x_average / (calibration_accuracy * 1.0);
  y_average = y_average / (calibration_accuracy * 1.0);

  x_offset = round(511.5 - x_average);
  y_offset = round(511.5 - y_average);
  sx2 = (analogRead(JoyStick_X) + x_offset) * 1.0;
  sy2 = (analogRead(JoyStick_Y) + y_offset) * 1.0;
  //delay(200);
  digitalWrite(13, HIGH);
  time1 = millis();
  time2 = millis();
}

void loop() {
  if (pad == 1 )
  {
    moving = 0;
    while (moving == 0)
    {

      key = customKeypad.getKey();
      if (key) {
        Serial.println(key);

        if (isAlpha(key))
        {
          if (key_state == 2)
          {
            values[temp_val] = InString.toInt();
            InString = "";
          }
          key_state = 1;
          temp_val = search.indexOf(key);

        } else if (isDigit(key))
        {
          if (key_state == 1)
          {
            key_state = 2;
            // convert the incoming byte to a char and add it to the string:
            InString += (char)key;
          }
          else if (key_state == 2)
          {
            InString += (char)key;
          }
          else
          { }
        } else if (key == '#')
        {
          if (key_state == 2)
          {
            values[temp_val] = InString.toInt();
            InString = "";
          }
          key_state = 0;
          if (values[1] != 0 || values[3] != 0) //if any velocity is not set to zero
          {
            pneg = 1; yneg = 1;
            if (values[1] == 0)
            {
              values[0] = pitch;
              //values[1] = 1;
            }
            if (values[0] < pitch)
            {
              values[1] = -1 * values[1];
              pneg = -1;
            }
            if (values[3] == 0)
            {
              values[2] = yaw;
              // values[3] = 1;
            }
            if (values[2] < yaw)
            {
              values[3] = -1 * values[3];
              yneg = -1;
            }

            moving = 1;
            Serial.println();
            Serial.println("Pitch_angle: ");
            Serial.println(values[0], DEC);
            Serial.print("\t");
            Serial.println("Pitch_velocity: ");
            Serial.println(values[1], DEC);
            Serial.print("\t");
            Serial.println("Yaw_angle: ");
            Serial.println(values[2], DEC);
            Serial.print("\t");
            Serial.println("Yaw_velocity: ");
            Serial.println(values[3], DEC);
          }
        }
      }
      /* values[0] = 90;
        values[1] = 10;
        values[2] = 90;
        values[3] = 10;
        moving = 1;*/


    }
    p_stat = true; p_stat2 = true; y_stat = true; y_stat2 = true;
    float pt_temp = values[1] * 1.0 * pneg / (pitch_accel * 1.0);
    float yt_temp = values[3] * 1.0 * yneg / (yaw_accel * 1.0);
    Serial.println(pt_temp, 5);
    Serial.println(yt_temp, 5);
    p_point2 = round(values[0] - (pt_temp * (values[1] - (0.5 * (pitch_accel) * (pt_temp)))));
    y_point2 = round(values[2] - yt_temp * (values[3] - (0.5 * yaw_accel * yt_temp)));
    Serial.println("pitch point 2:");
    Serial.println(p_point2, 5);
    Serial.println("yaw point 2:");
    Serial.println(y_point2, 5);
    pitch2 = pitch; yaw2 = yaw;
    time2 = millis();
    initial_yaw = yaw;
    time1 = millis();
    initial_pitch = pitch;
    Serial.println("pitch:");

    Serial.println(pitch, DEC);
    time3 = millis();
    while (pitch != values[0] || yaw != values[2])
    {
      // Serial.println("pitch:");

      if (pitch != values[0])
      {
        pitch2 = pitch_accel_to_velocity(initial_pitch, p_point2, values[0], pitch, time1, pitch_accel, values[1] * 1.0, pneg);
      }
      Serial.print('\n');
      float tempa = (1.0 * millis() - 1.0 * time3) / 1000.0;
      Serial.print(tempa, 5);

      Serial.print(",");
      Serial.print(pitch2, DEC);
      //Serial.println(pitch2, DEC);
      //Serial.println("yaw:");

      //Serial.println(yaw,DEC);
      if (yaw != values[2])
      {
        yaw2 = yaw_accel_to_velocity(initial_yaw, y_point2, values[2], yaw, time2, yaw_accel, values[3] * 1.0, yneg);
      }
      // Serial.println(yaw2,DEC);
      if (pitch2 != pitch && yaw2 != yaw)
      {
        pitch = pitch2;
        yaw = yaw2;

        servo.setServoPosition(0, map(yaw, 0, 180, 0x18, 0xE8));
        servo.setServoPosition(1, map(pitch, 0, 180, 0x18, 0xE8));
        servo.communicate();//initiate servo communication



      }
      else if (pitch2 != pitch)
      {
        pitch = pitch2;

        servo.setServoPosition(1, map(pitch, 0, 180, 0x18, 0xE8));
        servo.communicate();//initiate servo communication

      }
      else if (yaw2 != yaw)
      {
        yaw = yaw2;

        servo.setServoPosition(0, map(yaw, 0, 180, 0x18, 0xE8));
        servo.communicate();//initiate servo communication

      }
      else
      {}
      delay(delay1);
    }

  } else  if (pad == 2)
  {


  }
  else {
    //set the servo position

    sx = ax * (analogRead(JoyStick_X) + x_offset) * 1.0 + (1 - ax) * (sx2);
    sy = ay * (analogRead(JoyStick_Y) + y_offset) * 1.0 + (1 - ay) * (sy2);


    yaw_velocity2 = map(sy, 0, 1023, yaw_velocity_lower, yaw_velocity_upper);
    pitch_velocity2 = map(sx, 0, 1023, -pitch_velocity_lower, -pitch_velocity_upper);
    sx2 = sx; sy2 = sy;
    Serial.print(yaw_velocity2 , DEC);
    Serial.print(",");
    Serial.print(pitch_velocity2 , DEC);
    Serial.print(";");

    if (yaw_velocity2 <= threshold && yaw_velocity2 >= -threshold)
    {
      yaw_velocity2 = 0;
    }
    if (pitch_velocity2 <= threshold && pitch_velocity2 >= -threshold)
    {
      pitch_velocity2 = 0;
    }
    if (abs(yaw_velocity - yaw_velocity2) >= res)
    {
      yaw_velocity = yaw_velocity2;
      time2 = millis();
      initial_yaw = yaw;
    }
    if (abs(pitch_velocity - pitch_velocity2) >= res)
    {
      pitch_velocity = pitch_velocity2;
      time1 = millis();
      initial_pitch = pitch;
    }

    pitch2 = pitch_move_at_velocity(initial_pitch, time1, pitch_velocity);
    yaw2 = yaw_move_at_velocity(initial_yaw, time2, yaw_velocity);
    if (pitch2 != pitch && yaw2 != yaw)
    {
      pitch = pitch2;
      yaw = yaw2;

      servo.setServoPosition(0, map(yaw, 0, 180, 0x18, 0xE8));
      servo.setServoPosition(1, map(pitch, 0, 180, 0x18, 0xE8));
      servo.communicate();//initiate servo communication



    }
    else if (pitch2 != pitch)
    {
      pitch = pitch2;

      servo.setServoPosition(1, map(pitch, 0, 180, 0x18, 0xE8));
      servo.communicate();//initiate servo communication

    }
    else if (yaw2 != yaw)
    {
      yaw = yaw2;

      servo.setServoPosition(0, map(yaw, 0, 180, 0x18, 0xE8));
      servo.communicate();//initiate servo communication

    }
    else
    {}
    delay(delay1);
  }
}
int pitch_accel_to_velocity(int init_pitch, int pitch_point2, int end_pitch, int current_pitch, unsigned long int start_time1, int pitch_acceleration, float p_velocity, int neg)
{
  time12 = (millis() - start_time1) / 1000.0;
  //Serial.println(time12, 5);
  float new_pitch = current_pitch;
  float v1 = pitch_acceleration * time12 * neg;
  //Serial.println( p_stat == true);
  if (v1 < p_velocity && p_stat == true)
  {
    //Serial.println("yes");
    new_pitch = round(init_pitch + 0.5 * v1 * time12);
  }
  else if (current_pitch * neg < pitch_point2 * neg)
  {
    // Serial.println("maybee");
    if (p_stat)
    {
      p_stat = false;
      time1 = millis();
      initial_pitch = current_pitch;
    }
    else
    {
      new_pitch = round(p_velocity * time12) + init_pitch;
    }

  }
  else if (current_pitch * neg < end_pitch * neg)
  {
    if (p_stat2)
    {
      p_stat2 = false;
      time1 = millis();
      initial_pitch = current_pitch;
    }
    else
    {
      new_pitch = round((p_velocity - (0.5 * v1 * neg)) * time12) + init_pitch;
    }
    if (new_pitch * neg > end_pitch * neg)
    {
      new_pitch = end_pitch;
    }
  }
  else {}

  if (new_pitch > 180)
  {
    new_pitch = 180;
  }
  if (new_pitch < 0)
  {
    new_pitch = 0;
  }
  return new_pitch;


}
int yaw_accel_to_velocity(int init_yaw, int yaw_point2, int end_yaw, int current_yaw, unsigned long int start_time2, int yaw_acceleration, float y_velocity, int neg)
{
  time22 = (millis() - start_time2) / 1000.0;
  float new_yaw = current_yaw;
  float v1 = yaw_acceleration * time12 * neg;
  if (v1 < y_velocity && y_stat)
  {
    new_yaw = round(init_yaw + 0.5 * v1 * time22);
  }
  else if (current_yaw * neg < yaw_point2 * neg)
  {
    if (y_stat)
    {
      y_stat = false;
      time2 = millis();
      initial_yaw = current_yaw;
    }
    else
    {
      new_yaw = round(y_velocity * time22) + init_yaw;
    }

  }
  else if (current_yaw * neg < end_yaw * neg)
  {
    if (y_stat2)
    {
      y_stat2 = false;
      time2 = millis();
      initial_yaw = current_yaw;
    }
    else
    {
      new_yaw = round((y_velocity - (0.5 * v1 * neg)) * time22) + init_yaw;
    }
    if (new_yaw * neg > end_yaw * neg)
    {
      new_yaw = end_yaw;
    }
  }
  else {}

  if (new_yaw > 180)
  {
    new_yaw = 180;
  }
  if (new_yaw < 0)
  {
    new_yaw = 0;
  }
  return new_yaw;


}

int pitch_move_at_velocity(int init_pitch, unsigned long int start_time1, float velocity1)
{
  time12 = (millis() - start_time1) / 1000.0;
  int new_pitch = round(velocity1 * time12) + init_pitch;
  // Serial.print(pitch);
  // Serial.print("\t");
  if (new_pitch > 180)
  {
    new_pitch = 180;
  }
  if (new_pitch < 0)
  {
    new_pitch = 0;
  }
  return new_pitch;


}
int yaw_move_at_velocity(int init_yaw, unsigned long int start_time2, float velocity2)
{
  time22 = (millis() - start_time2) / 1000.0;
  int new_yaw = round(velocity2 * time22) + init_yaw;
  // Serial.print(pitch);
  // Serial.print("\t");
  if (new_yaw > 180)
  {
    new_yaw = 180;
  }
  if (new_yaw < 0)
  {
    new_yaw = 0;
  }
  return new_yaw;


}


