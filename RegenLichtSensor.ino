

#include "Arduino.h"
#include "lin.h"

Lin lin;

#ifndef SIM
#define simprt p
#undef assert  // I don't want to assert in embedded code
#define assert
#else
#define simprt printf
#include <stdarg.h>
#include <assert.h>
#endif

// include the library code:
#include <LiquidCrystal.h>

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2, MCP_cs = 6;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);


uint8_t adr_master = 0x30;
uint8_t adr_slave = 0x22;
uint8_t result = 0x00;
uint8_t zaehler = 0x00;
int programmzaehler = 0;


//Byte 0 = Zähler
uint8_t master1[5] = { 0x00, 0x60, 0x21, 0x00, 0x00 }; //Dieses Masterframe wird nach Programmstart 20 mal gesendet
uint8_t master2[5] = { 0x00, 0x60, 0x21, 0xFF, 0x00 }; //Daraufhin wird dieses Masterframe 19 mal gesendet
uint8_t master3[5] = { 0x00, 0x40, 0x00, 0xFF, 0x00 }; //Daraufhin wird dieses Masterframe "ewig" gesendet

uint8_t wakeup[2] = { 0x00, 0x00 }; //Dieses Wake-Up-Frame wird alle 10 Kommandos gesendet


uint8_t slave[6] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };  //Hier kommt das Ergebnis rein




void setup() {
  // put your setup code here, to run once:
  pinMode(MCP_cs, OUTPUT);
  digitalWrite(MCP_cs, LOW);
  lin.begin(19200);
  lcd.begin(16, 2);
  lcd.print("Start");
  delay(1000);
  digitalWrite(MCP_cs, HIGH);
  delay(250);
}


 void loop()
{

  //Erst MASTERFRAME    
  
  //Die ersten 20 Durchläufe Masterframe 1 senden
  if(programmzaehler <= 19) {
    //Zähler erhöhen und aktualisieren
    master1[0] = zaehler;
    lin.send(adr_master, master1, 5, 1);
  }
  //Die nächsten 19 Durchgänge Masterframe 2 senden
  else if(programmzaehler > 19 && programmzaehler <= 38) {    
    //Zähler erhöhen und aktualisieren
    master2[0] = zaehler;
    lin.send(adr_master, master2, 5, 1);
  }
  //Daraufhin Masterframe 3 senden
  else if(programmzaehler > 38) {
    //Zähler erhöhen und aktualisieren
    master3[0] = zaehler;
    lin.send(adr_master, master3, 5, 1);
  }

  
  //Dann SLAVEFRAME
  //Empfangene Daten werden im Array "slave" gespeichert
  result = lin.recv(adr_slave, slave, 6, 1);


  //Alle 10 Durchgänge WAKEUP-FRAME
  if(programmzaehler % 10 == 0) {
    lin.send(0xDF, wakeup, 2, 1);
  }
  


  programmzaehler++;
  zaehler++;
  if(zaehler > 0x0F) zaehler = 0x00;



   //TEST! Nach circa 110 Durchläufen scheint kein Regenwert mehr ausgegeben zu werden
   //Weiter untersuchen, ob dies nur durch "neustart" gelöst werden kann
  if(programmzaehler > 110) programmzaehler = 0;


  
  //Die unteren 4 Bits geben den Helligkeitswert an
  //Daher werden nur die unteren 4 Bit ausgewertet
  int helligkeit = slave[3] & B00001111;
  
  //Die oberen 4 Bits den Regenwert
  //Die oberen 4 Bit werden dann unten geschoben, damit der Zahlenwert passt
  int regen = (slave[3] & B11110000) >> 4;

  
  lcd.setCursor(0, 0);
  lcd.print("Hell: ");
  lcd.print(helligkeit);
  lcd.print(" #");
  lcd.print(programmzaehler);
  
  lcd.setCursor(0, 1);
  lcd.print("Regen: ");
  lcd.print(regen);

  delay(50);
}

