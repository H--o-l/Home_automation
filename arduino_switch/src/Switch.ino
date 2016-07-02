


/*============================ Include ============================*/

//#include <Arduino.h>
#include <SPI.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include <nRF24L01.h>
#include <pins_arduino.h>
#include <printf.h>
#include <RF24.h>
#include <RF24_config.h>
#include <OneWire.h>

/*============================ Macro ============================*/

#define msleep(X) delay(X)

#define DEBUG
#ifndef DEBUG
#define PRINTF(...);
#define setupSerial();
#else
#define setupSerial(); {Serial.begin(57600); printf_begin();}
#define PRINTF printf
#endif

/*============================ Global variable ============================*/

// Set up nRF24L01 radio on SPI pin for CE, CSN
RF24 radio(9,10);

OneWire DS18B20(7);  // Temperature chip i/o on digital pin 2

volatile int wakeUpFromInterrupt = 1;

/*============================ Local Function interface ============================*/

static void setupNRF24(void);
static void send(char * iData);
static void received(char* oData, long iMsTimeOut);
static long Send_light_state(long state, long bat, long temp, long iMsTimeOut);
static void Sleep(void);
static long getDS18B20(void);

/*============================ Function implementation ============================*/

/*------------------------------- Main functions -------------------------------*/

void setup(void){
  setupSerial();
  PRINTF("Low power switch\n\r");
  setupNRF24();
  pinMode(2, INPUT); /* button */
  analogReference(INTERNAL); /* 1.1V internal reference */
  delay(100);
}

void loop(void){
  static int  state = 0;
  long bat = 0;

  Sleep();
  delay(90);
  bat = (long)((float)analogRead(A3) * (110.0 / 243.0));
  delay(10);
  state = (state) ? 0:1;
  if(Send_light_state(state, bat, getDS18B20(), 10000)){
    PRINTF("Send state %d ok\n\r", state);
  }else{
    PRINTF("Send state %d Nok !\n\r", state);
  }
  delay(200);
}

/*------------------------------- Sleep mode -------------------------------*/

void pin2Interrupt(void)
{
  if(wakeUpFromInterrupt == 0){
    detachInterrupt(0);
    wakeUpFromInterrupt = 1;
  }
}
 
static void Sleep(void)  
{  
  while(digitalRead(2) == HIGH){
    PRINTF("can't sleep");
    return;
  }
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);  
  sleep_enable();  
  attachInterrupt(0, pin2Interrupt, RISING);
  wakeUpFromInterrupt = 0;
  sleep_mode();  
  sleep_disable();   
  while(!wakeUpFromInterrupt); // wait IT catch before start run
}  

/*------------------------------- Send light State -------------------------------*/

static long Send_light_state(long state, long bat, long temp, long iMsTimeOut){
  char lInBuffer[32] = {0};
  char lOutBuffer[32] = {0};
  long lTimeReference;
  long lTime;

  lTimeReference = millis();
  lTime = lTimeReference;

  radio.powerUp();
  while(1){
    if(state == 1){
      PRINTF("Send light on  - bat = %03ld - temp = %03ld\n\r", bat, temp);
      sprintf(lOutBuffer, "On  %03ld %03ld", bat, temp);
    }else if(state == 0){
      PRINTF("Send light off - bat = %03ld - temp = %03ld\n\r", bat, temp);
      sprintf(lOutBuffer, "Off %03ld %03ld", bat, temp);
    }
    send(lOutBuffer);
    received(lInBuffer, 200);
    if(  (  (strcmp(lInBuffer, "on") == 0)
          &&(state == 1))
       ||(  (strcmp(lInBuffer, "off") == 0)
          &&(state == 0))){
      radio.powerDown();
      return 1;
    }
    lTime = millis(); 
    if((lTime - lTimeReference) > iMsTimeOut){
      break;
    }
    delay(200);
  }
  radio.powerDown();    
  return 0;
}

/*------------------------------- nRF24 -------------------------------*/

static void setupNRF24(void){
  radio.begin();
  radio.enableDynamicPayloads();
  radio.setRetries(20,10);
  radio.openWritingPipe(0xF0F0F0F0E2LL);
  radio.openReadingPipe(1,0x7365727632LL);
  radio.startListening();
  radio.printDetails();
  msleep(100);
  radio.powerDown(); 
}

static void send(char* iData){
  if(strlen(iData) <= 32){
    radio.stopListening();
    if(!radio.write(iData, strlen(iData))){
      PRINTF("Radio.write failed : %s\n\r", iData);
    }
    radio.startListening();
  }else{
    PRINTF("Data to long  : %s\n\r", iData);
  }
}

static void received(char* oData, long iMsTimeOut){
  long lTimeReference;
  long lTime;

  lTimeReference = millis();
  lTime = lTimeReference;

  //msleep(iMsTimeOut); /* TODO */
  while(  ((lTime - lTimeReference) < iMsTimeOut)
        ||(iMsTimeOut == -1)){
    if(radio.available()){
      uint8_t len = radio.getDynamicPayloadSize();
      if(len <= 32){
        radio.read(oData, len);
        oData[len] = 0;
      }else{
        PRINTF("Error in received function\n\r");
        oData[0] = 0;
      }
      return;
    }
    lTime = millis(); 
  }
  //PRINTF("Received time out %10ld %10ld %10ld\n\r", lTimeReference, lTime, lTime - lTimeReference);
  oData[0] = 0;
}


/*------------------------------- DS18B20 (Temp) -------------------------------*/

static long getDS18B20(void){
  //returns the temperature from one DS18S20 in DEG Celsius
  byte data[12];
  byte addr[8];

  if ( !DS18B20.search(addr)) {
      //no more sensors on chain, reset search
      DS18B20.reset_search();
      return -1000;
  }

  if ( OneWire::crc8( addr, 7) != addr[7]) {
      PRINTF("DS18B20 CRC is not valid!\n");
      return -1000;
  }

  if ( addr[0] != 0x10 && addr[0] != 0x28) {
      PRINTF("DS18B20 is not recognized\n");
      return -1000;
  }

  DS18B20.reset();
  DS18B20.select(addr);
  DS18B20.write(0x44,1); // start conversion, with parasite power on at the end

  byte present = DS18B20.reset();
  DS18B20.select(addr);    
  DS18B20.write(0xBE); // Read Scratchpad

  
  for (int i = 0; i < 9; i++) { // we need 9 bytes
    data[i] = DS18B20.read();
  }
  
  DS18B20.reset_search();
  
  byte MSB = data[1];
  byte LSB = data[0];

  float tempRead = ((MSB << 8) | LSB); //using two's compliment
  float TemperatureSum = tempRead / 16;
  
  return TemperatureSum*10;
}
