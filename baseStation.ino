#include <SPI.h>
#include <RH_RF95.h>
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 7
#define RF95_FREQ 915.0

uint32_t timer = millis(); //milliseconds timer
RH_RF95 rf95(RFM95_CS, RFM95_INT);
bool sent = false;
const byte numChars = 10000;
char receivedChars[numChars];

void setup() 
{
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST,LOW);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);  
  Serial.begin(115200);
  delay(1000);

  while (!rf95.init()) {
    Serial.println("LoRa radio init failed");
    Serial.println("Uncomment '#define SERIAL_DEBUG' in RH_RF95.cpp for detailed debug info");
    while (1);
  }

  if (!rf95.setFrequency(RF95_FREQ)) {
    Serial.println("setFrequency failed");
    while (1);
  }
  
  rf95.setTxPower(5, false);
  delay(1000);  
}

void loop()
{
  char radiopacket1[3];
  char radiopacket2[3];
  char radiopacket3[3];
  int bufIndex;
  bufIndex = sprintf(radiopacket1, "4 ");
  bufIndex = sprintf(radiopacket2, "5 ");
  bufIndex = sprintf(radiopacket3, "6 ");
  char serialRead = '0';
  if (timer > millis()) 
    timer = millis();
  serialRead = Serial.read();
  if(serialRead == '4')
  {
    timer = millis(); // reset the timer        
    rf95.send((uint8_t *)radiopacket1, 3);
    serialRead = '0';
  }
  else if(serialRead == '5')
  {
    timer = millis(); // reset the timer        
    rf95.send((uint8_t *)radiopacket2, 3);
    serialRead = '0';
  }
  else if(serialRead == '6')
  {
    timer = millis(); // reset the timer        
    rf95.send((uint8_t *)radiopacket3, 3);
    serialRead = '0';
  }
  
  rf95.waitPacketSent();
  uint8_t buf[38]; //21 for radio
  uint8_t len = sizeof(receivedChars); 
  
//  if (rf95.waitAvailableTimeout(1000))
//  {  
    if (rf95.recv(receivedChars, &len))
    {
      Serial.print((char*)receivedChars); 
    }
//  }
}
