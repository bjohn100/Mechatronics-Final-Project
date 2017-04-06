//libraries
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>
#include <Servo.h>

//sensors front, left, right, side
const int anPin = 0;
const int anPin2 = 1;
const int anPin3 = 2;
const int anPin4 = 3;
long anVolt, anVolt2, anVolt3, anVolt4;
int value1, value2, value3, value4;

//wheels
Servo  myservo;
Servo myservo2;
int servoPin = 9;
int servo2Pin = 10;

//navigation
enum States {forward, backward, right, left};
States state = forward;
int adjusted_90 = 58; //turning
boolean turned_right, turned_left; //remember turns
int threshold; //drifting

//IMU
int x_ref, x, x_old;

#define BNO055_SAMPLERATE_DELAY_MS(100);
Adafruit_BNO055 bno = Adafruit_BNO055(55);


void setup(void) {

  Serial.begin(115200);

  /** IMU Setup **/

  if (!bno.begin()) { //error
    Serial.print("no BNO055 detected, check wiring or I2C ADDR");
    while (1);
  }

  delay(1000);

  bno.setExtCrystalUse(true); // use crystal for better accuracy
  sensors_event_t event;
  bno.getEvent(&event);

  x_ref = 180; //initial starting angle

}

void loop(void) {

  /** want to make sure that when a function is called 
   *  it is continuous. ex if we call drive forward, and 
   *  that moves, checks sensors, and corrects drifting
   *  need to make sure that while there isn't an obstacle
   *  it continues to move, check, correct. i think it is
   *  fine. maybe. but quick fix would be putting state = forward
   *  somewhere in the check front funciton
   *  
   *  sensors for back up not implemented notes below. 
   *  
   *  there are funcitons embedded in functions and 
   *  could easily have the same reading complexity 
   *  as last version.. . 
   */

  bno.setExtCrystalUse(true);
  delay(BNO055_SAMPLERATE_DELAY_MS);

  myservo.attach(servoPin);
  myservo2.attach(servo2Pin);

  switch (state) {

    case forward:
      /** while loop here on in function?
      need to continuously check sensor and drift
      **/
      
      drive_forward();
      break;

    case backward:
      drive_back(); 
      /** do we want it to turn after
      it backs up? then state = wherever
      need while loop somewhere...back up
      while sensors are reading obst. **/
      
      break;

    case left:
      turn_left();
      state = forward;
      break;

    case right:
      turn_right();
      state = forward;
      break;

      x_ref = x_ref % 360; //[0,360] range
  }
}




//avg sensor data. need to sample based on bandwidth
int read_sensor() {
  int samples = 20;
  long sum = 0;
  for (int i = 0; i < samples; i++) {
    anVolt = analogRead(anPin) / 4;
    sum += anVolt;
  }
  return sum / samples;
}

int read_sensor2() {
  int samples = 20;
  long sum = 0;
  for (int i = 0; i < samples; i++) {
    anVolt2 = analogRead(anPin2) / 4;
    sum += anVolt2;
  }
  return sum / samples;
}

int read_sensor3() {
  int samples = 20;
  long sum = 0;
  for (int i = 0; i < samples; i++) {
    anVolt3 = analogRead(anPin3) / 4;
    sum += anVolt3;
  }
  return sum / samples;
}

int read_sensor4() {
  int samples = 20;
  long sum = 0;
  for (int i = 0; i < samples; i++) {
    anVolt4 = analogRead(anPin4) / 4;
    sum += anVolt4;
  }
  return sum / samples;
}



//check front and react
void check_front() {
  value1 = read_sensor();
  value4 = read_sensor4();

  if ((value1 <= 5) || (value4 <= 5)) {
    myservo.detach();
    myservo2.detach();

    check_sides();

  } else if (turned_right == true) {
    check_left();

  } else if (turned_left == true) {
    check_right();
  }

}

  //check both sides for direction
  void check_sides() {
    value2 = read_sensor2();
    value3 = read_sensor3();

    //determine which way to turn
    if (value2 > value3) {
      turning_check();
    }
  }


  //check right
  void check_right() {
    value3 = read_sensor3();
    if (value3 >= 15) {
      delay(1000); //go forward more to clear the obstacle
      turn_right();
      turned_left = false;
    }
  }

  //check left
  void check_left() {
    value2 = read_sensor2();
    if (value2 >= 15) {
      delay(1000); //go forward more to clear the obstacle
      turn_left();
      turned_right = false;
    }
  }

  void turning_check() {

    /** want somehting in here like if turn multiple 
    times then call go back while sensors detect 
    obst. **/

    
    //if turn left twice, turn forward
    if (x_ref - 90 < 15) {
      state = right;
      turned_right, turned_left = false;

    } else {
      state = left;
      turned_left = true;
      break;
    }
    //if turn right twice, turn forward
    if (x_ref + 90 > 345) {
      state = left;
      turned_right, turned_right  = false;

    } else {
      state = right;
      turned_right = true;
      break;
    }
  }

  //drifting
  void drifting() {
    x = get_angle();

    //right
    if (x - x_ref > threshold) {
      myservo.write(98);
      myservo2.write(0);
    }
    //left
    else if (x - x_ref < threshold * -1) {
      myservo.write(180);
      myservo2.write(85);
    }
  }


  //driving forward
  void drive_forward() {
    //get angle here too?
    //while something? {
    myservo.attach(servoPin);
    myservo2.attach(servo2Pin);
    myservo.write(180);
    myservo2.write(0);

    drifting();
    check_front();
    //}
  }


  //backward
  void drive_back() {
    /** need to add something in side sensors
    if blocked or have turned back and forth
    so many times then state = backward
    need to know where it says move until sens cleared **/
    
    while (check_sides ... something) {
      myservo.write(0);
      myservo2.write(180);
    }
  }





  //right
  void turn_right() {
    myservo.attach(servoPin);
    myservo2.attach(servo2Pin);

    x_old = x_ref;
    x = x_old;

    while (abs(x_old - x) < adjusted_90) {
      drive_right();
      x = get_angle();
    }

    myservo.detach();
    myservo2.detach();

    x_ref = x_ref + 90;
    delay(500);
  }

  //left
  void turn_left() {
    myservo.attach(servoPin);
    myservo2.attach(servo2Pin);

    x_old = x_ref;
    x = x_old;

    //While the difference is less than adjusted_90, keep turning
    while (abs(x_old - x) < adjusted_90) {
      drive_left();
      x = get_angle();
    }

    myservo.detach();
    myservo2.detach();

    //Update x_ref
    x_ref = x_ref - 90;
    delay(500);
  }


  void drive_left() {
    myservo.write(0);
    myservo2.write(0);
  }

  void drive_right() {
    myservo.write(180);
    myservo2.write(180);
  }

  //get angle
  int get_angle() {
    sensors_event_t event;
    bno.getEvent(&event);
    x = ((int)event.orientation.x + 180) % 360;
    return x;
  }

