#include <SPI.h>
#include <LiquidCrystal.h>
#include <SimpleDHT.h>
#include <Thread.h>
#include <Encoder.h>
#define ENCODER_DO_NOT_USE_INTERRUPTS
#include <ShiftRegister74HC595.h>
#include <EEPROM.h>
// create a global shift register object
// parameters: (number of shift registers, data pin, clock pin, latch pin)
ShiftRegister74HC595 sr (1, 6, 9, 8); 


int eeptr = 0;
char tag='i';
int pinDHT22 = 7;
int SWpin=4;
int CLKpin=2;
int DTpin=3;
SimpleDHT22 dht22(pinDHT22);
LiquidCrystal lcd(10);
float t = 0;
float h = 0;
int turncounter =3600;
bool mayturn = false;
int currentmenu=0;
int maxturninterval=3600;
float maxtemp=38.5;
float mintemp=37.5;
int heaterpin = 0;
int turnerpin = 1;

Thread myThread = Thread();
Encoder myEnc(CLKpin, DTpin);

// callback for myThread
void readSensors(){

if (mayturn) {
  sr.set(turnerpin, HIGH);
  mayturn=false;
 }
  
 turncounter=turncounter-2;
 if (turncounter<=0){
  mayturn=true;
  turncounter =maxturninterval;
 } 

if (mayturn) {
  sr.set(turnerpin, LOW);
  }
 
 int err = SimpleDHTErrSuccess;
  if ((err = dht22.read2(&t, &h, NULL)) != SimpleDHTErrSuccess) {
    //Serial.print("Read DHT22 failed, err="); Serial.println(err);
      lcd.setCursor(0, 0);
      lcd.print("DHT22 err=");
      lcd.setCursor(0, 1);
      lcd.print(err);
      delay(2000);      
    return;
  }else{
  if (t>=maxtemp){
     sr.set(heaterpin, HIGH);
  }  
  if (t<=mintemp){
     sr.set(heaterpin, LOW);
  }  
    
  }
}
  
void loadfromeeprom(){
  // Serial.print("load from eeprom");
  EEPROM.get(eeptr, tag);
  eeptr += sizeof(char);
  if (tag=='i') {
    // Serial.print("i tag");
  EEPROM.get(eeptr, maxturninterval);
  eeptr += sizeof(int);
  turncounter=maxturninterval;
  EEPROM.get(eeptr, maxtemp);
  eeptr += sizeof(float);    
  EEPROM.get(eeptr, maxtemp);
  eeptr += sizeof(float);  
  }
}

void savetoeeprom(){
  //tag[10]="incubator";
  eeptr=0;
  tag='i';
  EEPROM.put(eeptr, tag);
  eeptr += sizeof(char);
  EEPROM.put(eeptr, maxturninterval);
  eeptr += sizeof(int);
  EEPROM.put(eeptr, maxtemp);
  eeptr += sizeof(float);    
  EEPROM.put(eeptr, maxtemp);
  eeptr += sizeof(float);  

}

void setup() {
 // Serial.begin(9600);
  
  // set up the LCD's number of columns and rows: 
  lcd.begin(16, 2);
  loadfromeeprom();
  // Print a message to the LCD.
  pinMode(SWpin, INPUT_PULLUP);
  sr.setAllHigh(); // set all pins HIGH
  myThread.onRun(readSensors);
  myThread.setInterval(2000);
}

void render(){
  switch (currentmenu) {
    case 0:
        lcd.setCursor(0, 0);
        lcd.print("t:");
        lcd.setCursor(2, 0);
        lcd.print(t);
        lcd.setCursor(8, 0);
        lcd.print("h:");
        lcd.setCursor(10, 0);
        lcd.print(h); 
        lcd.setCursor(0, 1);
        lcd.print("turn after"); 
        lcd.setCursor(11, 1);
        lcd.print(turncounter);
        lcd.print("  ");
        break;
    case 1:
        lcd.setCursor(0, 0);
        lcd.print("high limit         ");
        lcd.setCursor(0, 1);
        lcd.print("temperature         ");
        break ;  
    case 2:
        lcd.setCursor(0, 0);
        lcd.print("low limit        ");
        lcd.setCursor(0, 1);
        lcd.print("temperature        ");
        break ; 
    case 3:
        lcd.setCursor(0, 0);
        lcd.print("turner           ");
        lcd.setCursor(0, 1);
        lcd.print("interval         ");
        break ;  
    case 4:
        lcd.setCursor(0, 0);
        lcd.print("Output          ");
        lcd.setCursor(0, 1);
        lcd.print("control         ");
        break ;  
 case 5:
        lcd.setCursor(0, 0);
        lcd.print("save to          ");
        lcd.setCursor(0, 1);
        lcd.print("eeprom         ");
        break ;       
 case 6:
        lcd.setCursor(0, 0);
        lcd.print("load from          ");
        lcd.setCursor(0, 1);
        lcd.print("eeprom         ");
        break ;                                            
  }      
}

void loop() {
  if(myThread.shouldRun())
    myThread.run();
    render();
    long newPosition = myEnc.read();
    if (newPosition >0) {
    currentmenu=currentmenu+1;
    if (currentmenu>6) {currentmenu=0;} 
    myEnc.write(0);
        delay(200);
         lcd.setCursor(0, 0);
        lcd.print("                  ");
        lcd.setCursor(0, 1);
        lcd.print("                  ");       
 
    }
    if (newPosition < 0) {
    currentmenu=currentmenu-1;
    if (currentmenu<0) {currentmenu=6;} 
    myEnc.write(0);
        delay(200); 
         lcd.setCursor(0, 0);
        lcd.print("                  ");
        lcd.setCursor(0, 1);
        lcd.print("                  ");   

    }
   myEnc.write(0);
   int buttonState = digitalRead(SWpin);
   if (buttonState==LOW){
    delay(500);
    if (currentmenu==3){turnersetup();}
    if (currentmenu==1){upperlimitsetup();}
    if (currentmenu==2){lowlimitsetup();}  
    if (currentmenu==4){srcontrol();}
    if (currentmenu==5){savetoeeprom();currentmenu=0; delay(500);}   
     if (currentmenu==6){loadfromeeprom();currentmenu=0; delay(500);}         
    }
}

void turnersetup(){
        lcd.setCursor(0, 0);
        lcd.print("turn interval    ");
        lcd.setCursor(0, 1);
        lcd.print("              ");
        while (true) {  
 if(myThread.shouldRun())
    myThread.run();          
        lcd.setCursor(0, 1);
        lcd.print(maxturninterval);  
        int buttonState = digitalRead(SWpin);
        if (buttonState==LOW){
              turncounter=maxturninterval;
             delay(500);
            return;
        }
    long newPosition = myEnc.read();
    if (newPosition >0) {
    maxturninterval=maxturninterval+300;
            lcd.setCursor(0, 1);
        lcd.print("              ");
    if (maxturninterval>32000) {maxturninterval=0;} 
    myEnc.write(0);
    delay(150); 
    }
    if (newPosition < 0) {
    maxturninterval=maxturninterval-300;
            lcd.setCursor(0, 1);
        lcd.print("              ");
    if (maxturninterval<0) {maxturninterval=32000;} 
    myEnc.write(0);
    delay(150); 
    }
   myEnc.write(0);
        
  }
}

void upperlimitsetup(){
        lcd.setCursor(0, 0);
        lcd.print("high limit         ");
        lcd.setCursor(0, 1);
        lcd.print("              ");
        while (true) {
  if(myThread.shouldRun())
    myThread.run();           
        lcd.setCursor(0, 1);
        lcd.print(maxtemp);  
        int buttonState = digitalRead(SWpin);
        if (buttonState==LOW){
             delay(500);
            return;
        }
    long newPosition = myEnc.read();
    if (newPosition >0) {
    maxtemp=maxtemp+0.1;
            lcd.setCursor(0, 1);
        lcd.print("              ");
    if (maxtemp>42) {maxtemp=20;} 
    myEnc.write(0);
    delay(150); 
    }
    if (newPosition < 0) {
    maxtemp=maxtemp-0.1;
            lcd.setCursor(0, 1);
        lcd.print("              ");
    if (maxtemp<20) {maxtemp=42;} 
    myEnc.write(0);
    delay(150); 
    }
   myEnc.write(0);
        
  }
}

void lowlimitsetup(){
        lcd.setCursor(0, 0);
        lcd.print("low limit         ");
        lcd.setCursor(0, 1);
        lcd.print("              ");
        while (true) { 
 if(myThread.shouldRun())
    myThread.run();           
        lcd.setCursor(0, 1);
        lcd.print(mintemp);  
        int buttonState = digitalRead(SWpin);
        if (buttonState==LOW){
             delay(500);
            return;
        }
    long newPosition = myEnc.read();
    if (newPosition >0) {
    mintemp=mintemp+0.1;
            lcd.setCursor(0, 1);
        lcd.print("              ");
    if (mintemp>42) {mintemp=20;} 
    myEnc.write(0);
    delay(150); 
    }
    if (newPosition < 0) {
    mintemp=mintemp-0.1;
            lcd.setCursor(0, 1);
        lcd.print("              ");
    if (mintemp<20) {mintemp=42;} 
    myEnc.write(0);
    delay(150); 
    }
   myEnc.write(0);
        
  }
}

void srcontrol(){
        int relnumber=0;
        lcd.setCursor(0, 0);
        lcd.print("relays control    ");
        lcd.setCursor(0, 1);
        lcd.print("return         ");
        while (true) { 
  if(myThread.shouldRun())
    myThread.run();          
        lcd.setCursor(0, 1);
        if (relnumber==0){
          lcd.print("return         ");
            }else{
        lcd.print(relnumber);
        lcd.setCursor(3, 1);
          lcd.print("status:");
        lcd.setCursor(10, 1);
        uint8_t state = sr.get(relnumber-1);
        if (state==0){
          lcd.print("ON ");
          }else{
          lcd.print("OFF");
            
            }

          }  
        int buttonState = digitalRead(SWpin);
        if (buttonState==LOW){
          if (relnumber==0){
             delay(500);
            return;
          }else{
           uint8_t state = sr.get(relnumber-1);
           sr.set(relnumber-1, not(state)); 
           delay(500); 
          }
        }
    long newPosition = myEnc.read();
    if (newPosition >0) {
    relnumber=relnumber+1;
            lcd.setCursor(0, 1);
        lcd.print("              ");
    if (relnumber>8) {relnumber=0;} 
    myEnc.write(0);
    delay(150); 
    }
    if (newPosition < 0) {
    relnumber=relnumber-1;
            lcd.setCursor(0, 1);
        lcd.print("              ");
    if (relnumber<0) {relnumber=8;} 
    myEnc.write(0);
    delay(150); 
    }
   myEnc.write(0);
        
  }
}
