/*
  MQUnifiedsensor Library - reading an MQ9

  Demonstrates the use a MQ9 sensor.
  Library originally added 01 may 2019
  by Miguel A Califa, Yersson Carrillo, Ghiordy Contreras, Mario Rodriguez
 
  Added example
  modified 23 May 2019
  by Miguel Califa 

  Updated library usage
  modified 26 March 2020
  by Miguel Califa 

  Wiring:
  https://github.com/miguel5612/MQSensorsLib_Docs/blob/master/static/img/MQ_Arduino.PNG
  Please take care, arduino A0 pin represent the analog input configured on #define pin

 This example code is in the public domain.

*/

//Include the library
#include <MQUnifiedsensor.h>
/************************Hardware Related Macros************************************/
#define         Board                   ("Arduino UNO")
#define         Pin                     (A0)  //Analog input 4 of your arduino
/***********************Software Related Macros************************************/
#define         Type                    ("MQ-9") //MQ9
#define         Voltage_Resolution      (5)
#define         ADC_Bit_Resolution      (10) // For arduino UNO/MEGA/NANO
#define         RatioMQ9CleanAir        (9.6) //RS / R0 = 60 ppm 
#define         PreaheatControlPin5      (3) // Preaheat pin to control with 5 volts
#define         PreaheatControlPin14      (4) // Preaheat pin to control with 1.4 volts
/*****************************Globals***********************************************/
//Declare Sensor
MQUnifiedsensor MQ9(Board, Voltage_Resolution, ADC_Bit_Resolution, Pin, Type);

void setup() {
  //Init the serial port communication - to debug the library
  Serial.begin(9600); //Init serial port
  pinMode(PreaheatControlPin5, OUTPUT);
  pinMode(PreaheatControlPin14, OUTPUT);

  //Set math model to calculate the PPM concentration and the value of constants
  MQ9.setRegressionMethod(1); //_PPM =  a*ratio^b
  MQ9.setA(1000.5); MQ9.setB(-2.186); // Configurate the ecuation values to get LPG concentration

  /*
    Exponential regression:
  GAS     | a      | b
  LPG     | 1000.5 | -2.186
  CH4     | 4269.6 | -2.648
  CO      | 599.65 | -2.244
  */
  
  /*****************************  MQ Init ********************************************/ 
  //Remarks: Configure the pin of arduino as input.
  /************************************************************************************/ 
  MQ9.init(); 
  /* 
    //If the RL value is different from 10K please assign your RL value with the following method:
    MQ9.setRL(10);
  */
  /*****************************  MQ CAlibration ********************************************/ 
  // Explanation: 
  // In this routine the sensor will measure the resistance of the sensor supposing before was pre-heated
  // and now is on clean air (Calibration conditions), and it will setup R0 value.
  // We recomend execute this routine only on setup or on the laboratory and save on the eeprom of your arduino
  // This routine not need to execute to every restart, you can load your R0 if you know the value
  // Acknowledgements: https://jayconsystems.com/blog/understanding-a-gas-sensor
  // ISSUE 44 - MQ9 needs a low/high temperature cycle like MQ7 #44

  Serial.println("Preheating, please wait 2.5 minutes");
  digitalWrite(PreaheatControlPin5, HIGH);
  digitalWrite(PreaheatControlPin14, LOW);
  delay(60000);
  digitalWrite(PreaheatControlPin5, LOW);
  digitalWrite(PreaheatControlPin14, HIGH);
  delay(90000);
  digitalWrite(PreaheatControlPin5, HIGH);
  digitalWrite(PreaheatControlPin14, LOW);
  delay(60000);
  digitalWrite(PreaheatControlPin5, LOW);
  digitalWrite(PreaheatControlPin14, HIGH);
  delay(90000);
  digitalWrite(PreaheatControlPin5, HIGH);
  digitalWrite(PreaheatControlPin14, LOW);
  
  // End pre-heat cycle

  Serial.print("Calibrating please wait.");
  float calcR0 = 0;
  for(int i = 1; i<=10; i ++)
  {
    MQ9.update(); // Update data, the arduino will be read the voltage on the analog pin
    calcR0 += MQ9.calibrate(RatioMQ9CleanAir);
    Serial.print(".");
  }
  MQ9.setR0(calcR0/10);
  Serial.println("  done!.");
  
  if(isinf(calcR0)) {Serial.println("Warning: Conection issue founded, R0 is infite (Open circuit detected) please check your wiring and supply"); while(1);}
  if(calcR0 == 0){Serial.println("Warning: Conection issue founded, R0 is zero (Analog pin with short circuit to ground) please check your wiring and supply"); while(1);}
  /*****************************  MQ CAlibration ********************************************/ 
  MQ9.serialDebug(true);
}

void loop() {
  digitalWrite(PreaheatControlPin5, LOW);
  digitalWrite(PreaheatControlPin14, HIGH);
  delay(90000);
  digitalWrite(PreaheatControlPin5, HIGH);
  digitalWrite(PreaheatControlPin14, LOW);
  
  MQ9.update(); // Update data, the arduino will be read the voltage on the analog pin
  MQ9.readSensor(); // Sensor will read PPM concentration using the model and a and b values setted before or in the setup
  MQ9.serialDebug(); // Will print the table on the serial port
  
}