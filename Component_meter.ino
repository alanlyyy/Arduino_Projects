/*
  Analog Input

  Demonstrates analog input by reading an analog sensor on analog pin 0 and
  turning on and off a light emitting diode(LED) connected to digital pin 13.
  The amount of time the LED will be on and off depends on the value obtained
  by analogRead().

  The circuit:
  - potentiometer
    center pin of the potentiometer to the analog input 0
    one side pin (either one) to ground
    the other side pin to +5V
  - LED
    anode (long leg) attached to digital output 13 through 220 ohm resistor
    cathode (short leg) attached to ground

  - Note: because most Arduinos have a built-in LED attached to pin 13 on the
    board, the LED is optional.

  created by David Cuartielles
  modified 30 Aug 2011
  By Tom Igoe

  This example code is in the public domain.

  https://www.arduino.cc/en/Tutorial/BuiltInExamples/AnalogInput
*/

int sensorPin = A0;    // select the input pin for the potentiometer
int ledPin = 13;      // select the pin for the LED
int sensorValue = 0;  // variable to store the value coming from the sensor
bool flag = 0;         // used to track if capacitor has been discharged
int capacitor_discharge_switch = 13;
int V_OUT = 10;        // Setup Arduino output voltage pin
double resistor = 10000.00;

int V_COMP = 9; //output of comparator

//DIP switch connections
int mode0 = 6;
int mode1 = 7;
int mode2 = 8;
//int mode3 = 9;

#include <LiquidCrystal.h>
#include <stdio.h>

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

void setup() {
  // declare the ledPin as an OUTPUT:
  pinMode(ledPin, OUTPUT);

  pinMode(capacitor_discharge_switch,OUTPUT);

  pinMode(V_OUT, OUTPUT);

  pinMode(mode0, INPUT_PULLUP);
  pinMode(mode1, INPUT_PULLUP);
  pinMode(mode2, INPUT_PULLUP);
  //pinMode(mode3, INPUT_PULLUP);

  pinMode(V_COMP,INPUT_PULLUP);

  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);

  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
}

void output_LCD(double value, int mode){

  float decimal_0 = (value - int(value))*100;
  float decimal_1 = (decimal_0 - int(decimal_1))*100;
  char str[40];

  if (mode == 0){
    sprintf(str,"%d.%d%d ohms", int(value), int(decimal_0), int(decimal_1));
  } else if(mode ==1){
    sprintf(str,"%d.%d%d uF", int(value), int(decimal_0), int(decimal_1));
  } else if(mode ==2){
    sprintf(str,"Vf = %d.%d%d V", int(value), int(decimal_0), int(decimal_1));
  } else{
    sprintf(str,"Vbe = %d.%d%d V", int(value), int(decimal_0), int(decimal_1));
  }
  
  lcd.setCursor(0, 0);
  lcd.print(str);
}
void calculate_capacitance() {
  //polling analogReadMethod

  //Component read setup
  int COMP_IN = sensorPin;

  //pull low for 1s first
  digitalWrite(V_OUT, LOW);
  delay(1000);

    //record start time
  unsigned long timer_start = micros();
  
  //pull output high
  digitalWrite(V_OUT, HIGH);

  // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
  //3 = 615, 5 = 1024
  int voltage = analogRead(COMP_IN);

  //Vrc = 5V*(1 - e^(-1)) at 1 RC, Vrc = 3.16V or analog code = 614
  while ( voltage < 645) {

    //delayMicroseconds(3); //3 is lowest recommended value suggested in the help documents.

    // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V), 100uS/reading
    voltage = analogRead(COMP_IN);

  }

  //if voltage >= 4.9V, get the stop time
  unsigned long timer_end = micros();

  //Serial.println(voltage);
  
  unsigned long delta_T = timer_end - timer_start;

  //convert to C = delta_T(uS) / R to seconds before calculating capacitance.
  double capacitance =  delta_T / resistor;

  Serial.println("Delta_T");
  Serial.print(delta_T);

  Serial.print(" Capacitance = ");
  Serial.print(capacitance);
  Serial.println(" Micro Farads, ");

  //setup string for printing
  output_LCD(capacitance, 0);

}

void calculate_capacitance_2() {
  //polling Digital ReadMethod

  //Component read setup
  int COMP_IN = sensorPin;

  //pull low for 1s first
  digitalWrite(V_OUT, LOW);
  delay(1000);

    //record start time
  unsigned long timer_start = micros();
  
  //pull output high
  digitalWrite(V_OUT, HIGH);

  // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
  bool V_comparator = digitalRead(V_COMP);

  //Vrc = 5V*(1 - e^(-1)) at 1 RC, Vrc = 3.16V or analog code = 614
  while ( V_comparator == HIGH) {
    delayMicroseconds(3); //3 is lowest recommended value suggested in the help documents.
    V_comparator = digitalRead(V_COMP);
  }

  //if voltage >= 4.9V, get the stop time
  unsigned long timer_end = micros();

  int voltage_code = analogRead(COMP_IN);
  
  //Serial.println(voltage_code);
  
  unsigned long delta_T = timer_end - timer_start;

  //convert to delta_T(uS) to seconds before calculating capacitance.
  double capacitance =  delta_T /resistor;

  Serial.print("Capacitance = ");
  Serial.print(capacitance);
  Serial.println(" Micro Farads, ");

  //setup string for printing
  output_LCD(capacitance, 0);

}

void capacitance_wrapper(){
    calculate_capacitance();
     
    //discharge capacitor by turning on transistor switch
    digitalWrite(capacitor_discharge_switch, HIGH);

    //delay 10 seconds after discharge
    delay(1000);
  
    //pull low for 1s first
    digitalWrite(V_OUT, LOW);
    delay(1000);
     
    //discharge capacitor by turning off transistor switch
    digitalWrite(capacitor_discharge_switch, LOW);

    delay(1000);   
}

void calculate_resistance() {

  //Component read setup
  int COMP_IN = sensorPin;

  //pull low for 100uS first
  digitalWrite(V_OUT, LOW);
  delayMicroseconds(100);

  //pull output high
  digitalWrite(V_OUT, HIGH);

  //read current voltage
  int COMPONENT_VOLTAGE = analogRead(COMP_IN);

  // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
  float voltage = COMPONENT_VOLTAGE * (5.0 / 1023.0);

  float resistance = (voltage / 5.00) * 5000.00 / ( 1.00 - (voltage / 5.00) );

  //Serial.print("Resistance = ");
  //Serial.print(resistance);
  //Serial.println(" ohms");
  
  //delayMicroseconds(100);
  
  //pull low for 100uS first
  digitalWrite(V_OUT, LOW);
  delayMicroseconds(100);

  //setup string for printing
  output_LCD(resistance, 1);
}

void calculate_diode() {

  //Component read setup
  int COMP_IN = sensorPin;

  //pull low for 100uS first
  digitalWrite(V_OUT, LOW);
  delayMicroseconds(100);

  //pull output high
  digitalWrite(V_OUT, HIGH);

  //read current voltage
  int COMPONENT_VOLTAGE = analogRead(COMP_IN);

  // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
  float voltage = COMPONENT_VOLTAGE * (5.0 / 1023.0);

  //Serial.print("Diode Vf = ");
  //Serial.print(voltage);
  //Serial.println(" Volts");

  //delayMicroseconds(100);
  
  //pull low for 100uS first
  digitalWrite(V_OUT, LOW);
  delayMicroseconds(100);

  //setup string for printing
  output_LCD(voltage, 2);
}
void calculate_transistor() {

  //Component read setup
  int COMP_IN = sensorPin;

  //pull low for 100uS first
  digitalWrite(V_OUT, LOW);
  delayMicroseconds(100);

  //pull output high
  digitalWrite(V_OUT, HIGH);

  //read current voltage
  int COMPONENT_VOLTAGE = analogRead(COMP_IN);

  // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
  float voltage = COMPONENT_VOLTAGE * (5.0 / 1023.0);

  //Serial.print("Transistor VBE = ");
  //Serial.print(voltage);
  //Serial.println(" Volts");

  //delayMicroseconds(100);
  
  //pull low for 100uS first
  digitalWrite(V_OUT, LOW);
  delayMicroseconds(100);

  //setup string for printing
  output_LCD(voltage,3);

}

void loop() {

  //clear LCD before printing.
  lcd.clear();
  
  if (digitalRead(mode0) == LOW){
    calculate_resistance();
  } else if (digitalRead(mode1) == LOW){
    calculate_transistor();
  } else if (digitalRead(mode2) == LOW){
    calculate_diode();
  } else{
    capacitance_wrapper();
  }
}
