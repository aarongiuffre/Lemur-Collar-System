#include <SPI.h>
#include <RH_RF95.h>
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 7
#define RF95_FREQ 915.0

uint32_t timer = millis(); //milliseconds timer
int rssi;
RH_RF95 rf95(RFM95_CS, RFM95_INT);
char baseStationCheck = 1;
boolean newData = false;
uint8_t handshakeBuf[3] = {'<', 'X', '>'};

void setup() 
{
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST,LOW);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);  
  Serial.begin(9600);
  Serial1.begin(9600);
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
  char radiopacket[3];
  int bufIndex;
  bufIndex = sprintf(radiopacket, "3 ");
  if (timer > millis()) 
    timer = millis();
  if (millis() - timer > 300000)  
  {
    timer = millis(); // reset the timer        
    rf95.send((uint8_t *)radiopacket, 3);
  }

  rf95.waitPacketSent();
  uint8_t buf[3];
  uint8_t len = sizeof(buf);
  
  if (!(Serial1.available() > 0))// && rf95.waitAvailableTimeout(500) )
  {  
    if (rf95.recv(buf, &len))
    {
      baseStationCheck = (char*)buf[0];
      int8_t rssi = rf95.lastRssi();
      Serial.print((char*)buf); 
      Serial.print(rssi, DEC); 
      Serial.print(',');
      const byte numRadioChars = 4;
      char receivedRadioChars[numRadioChars];
      char tempChars[numRadioChars]; 
      strcpy(tempChars, receivedRadioChars);
      uint8_t sendBuf[5];
      sendBuf[0] = '<';
      sendBuf[1] = buf[0];
      sendBuf[2] = ',';
      sendBuf[3] = rssi;
      sendBuf[4] = '>';
      Serial1.write(sendBuf, 5);
    }
  }
  else 
  {
    static boolean recvInProgress = false;
    static byte ndx = 0;
    char startMarker = '<';
    char endMarker = '>';
    char rc;
    const byte numSerialChars = 14;
    char receivedSerialChars[numSerialChars];

    while(Serial1.available() > 0)
    {
      rc = Serial1.read();

      if (recvInProgress == true) 
      {
        if (rc != endMarker) 
        {
          receivedSerialChars[ndx] = rc;
          ndx++;
          if (ndx >= numSerialChars) 
          {
              ndx = numSerialChars - 1;
          }
        }
        else 
        {
          receivedSerialChars[numSerialChars] = '\0'; // terminate the string
          recvInProgress = false;
          ndx = 0;
          Serial.print(receivedSerialChars);
          rf95.send((uint8_t *)receivedSerialChars, numSerialChars);
          Serial1.write(handshakeBuf, 3);
        }
      }
      else if (rc == startMarker) 
      {
        recvInProgress = true;
      }
    }
  }
}
