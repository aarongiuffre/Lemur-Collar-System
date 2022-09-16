#include <Adafruit_GPS.h>
#include <SoftwareSerial.h>
#include <SPI.h>
#include <SD.h>

#define GPSSerial Serial1
//RX is digital pin 10 (connect to TX of other device)
//TX is digital pin 11 (connect to RX of other device)
SoftwareSerial mySerial(11,12); 
Adafruit_GPS GPS(&GPSSerial);

const int chipSelect = 4;
uint32_t GPStimer = millis();
uint32_t radioTimer = millis();
bool firstGPSread = false;
const byte numChars = 4;
const byte bigNumChars = 500;
const byte smallChunkChars = 16;
char receivedChars[numChars];
char tempChars[numChars]; 
uint8_t chunkBuf[smallChunkChars];
static char baseStationCheck = '0';
char baseStationCode = '6';
int bigBufIndex = 0;
char messageFromPC[numChars] = {0};
int integerFromPC = 1;
float floatFromPC = 1.0;
boolean newData = false;
bool handshook = false;
uint8_t handshakeBuf[1] = {'X'};
bool shookOnce = false;
bool readyTx = false;

void printGPS() 
{
  char c = GPS.read();
  if (GPS.newNMEAreceived()) 
  {
    if (!GPS.parse(GPS.lastNMEA())) // this also sets the newNMEAreceived() flag to false
      return;
  }
  if (GPStimer > millis()) 
    GPStimer = millis();
  if ((millis() - GPStimer > 900000 || firstGPSread == false) && baseStationCheck != baseStationCode) 
  { 
    if (GPS.fix) 
    {
      GPStimer = millis(); // reset the GPStimer
      if(GPS.year != 0)
      {
        firstGPSread = true;
        File dataFile = SD.open("datalog.csv", FILE_WRITE);
        if(dataFile)
        {
          dataFile.print("20"); dataFile.print(GPS.year, DEC);
          if(GPS.month < 10)
          {
            dataFile.print('0');
          }
          dataFile.print(GPS.month, DEC); 
          if(GPS.day < 10)
          {
            dataFile.print('0');
          }
          dataFile.print(GPS.day, DEC); 
          dataFile.print(' ');
          if((GPS.hour) < 10)
          {
            dataFile.print('0');
          }
          dataFile.print((GPS.hour), DEC); 
          dataFile.print(':');
          if(GPS.minute < 10)
          {
            dataFile.print('0');
          }
          dataFile.print(GPS.minute, DEC); 
          dataFile.print(' ');
          dataFile.print(GPS.latitude, 4); dataFile.print(GPS.lat);
          dataFile.print(' ');
          dataFile.print(GPS.longitude, 4); dataFile.print(GPS.lon);
          dataFile.print(',');
          dataFile.close();
        }
      }
    }
  }
}

void setup() 
{ 
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  Serial1.begin(9600);
  while (!Serial) 
  {
    delay(1); // wait for serial port to connect. Needed for native USB port only
  }
  if (!SD.begin(chipSelect)) 
  {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    while (1);
  }

  // 9600 NMEA is the default baud rate for Adafruit MTK GPS's- some use 4800
  GPS.begin(9600);
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);
  GPS.sendCommand(PGCMD_ANTENNA);
  delay(1000);

  // set the data rate for the SoftwareSerial port
  mySerial.begin(9600);
  while(mySerial.available()){  //is there anything to read?
  char getData = mySerial.read();  //if yes, read it
    }   // don't do anything with it.

  Serial.print("Initializing SD card...");

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) 
  {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    while (1);
  }
  Serial.println("card initialized.");
}

void loop() 
{ 
  printGPS();

  static boolean recvInProgress = false;
  static byte ndx = 0;
  char startMarker = '<';
  char endMarker = '>';
  char readyTxMarker = 'X';
  char rc;

  while(mySerial.available() > 0 && newData == false) 
  {
    rc = mySerial.read(); //read a character

    if (recvInProgress == true) //past a '<' character
    {
      if (rc != endMarker) //inside a '<'___'>'
      {
          receivedChars[ndx] = rc;
          ndx++;
          if (ndx >= numChars) 
          {
              ndx = numChars - 1;
          }
      }
      else //reached a '>' character
      {
          //receivedChars[ndx] = '\0'; // terminate the string
          recvInProgress = false;
          ndx = 0;
          newData = true;
      }
    }
    else if (rc == startMarker) 
    {
      recvInProgress = true;
    }
  }

  if (newData == true) 
  {
    strcpy(tempChars, receivedChars);
        // this temporary copy is necessary to protect the original data
        //   because strtok() used in parseData() replaces the commas with \0
    char * strtokIndx; // this is used by strtok() as an index

    strtokIndx = strtok(NULL, ","); // this continues where the previous call left off
    integerFromPC = atoi(strtokIndx);     // convert this part to an integer
    floatFromPC = atof(strtokIndx);     // convert this part to a float
    if(baseStationCheck != baseStationCode)
    {
      File dataFile = SD.open("datalog.csv", FILE_WRITE);

      if(dataFile && (tempChars[0] != baseStationCode))
      {
        dataFile.print("20"); dataFile.print(GPS.year, DEC);
        if(GPS.month < 10)
        {
          dataFile.print('0');
        }
        dataFile.print(GPS.month, DEC); 
        if(GPS.day < 10)
        {
          dataFile.print('0');
        }
        dataFile.print(GPS.day, DEC); 
        dataFile.print(' ');
        if((GPS.hour) < 10)
        {
          dataFile.print('0');
        }
        dataFile.print((GPS.hour), DEC); 
        dataFile.print(':');
        if(GPS.minute < 10)
        {
          dataFile.print('0');
        }
        dataFile.print(GPS.minute, DEC); 
        dataFile.print(' ');
        
        dataFile.print(tempChars[0]);
        dataFile.print(' ');
        dataFile.print((int8_t)tempChars[2]);
        dataFile.print(',');
        dataFile.close();
      }
      else if(tempChars[0] == baseStationCode)
      {
        baseStationCheck = baseStationCode;
        readyTx = true;
        //mySerial.write(handshake, 1);
      }
      newData = false;
    }
    if(tempChars[0] == 'X')
    {
      //Serial.println("here");
      readyTx = true;
      newData = false;
    }
  }
  
  if(baseStationCheck == baseStationCode)
  {
    char datChar;

    File dataFileOut = SD.open("datalog.csv", FILE_READ);
    dataFileOut.seek(bigBufIndex);
    //Serial.println(readyTx);
    if(dataFileOut.available())
    {
      //Serial.println("here");
      if(readyTx == true)
      {
        int index = 1;
        char datChar;
        chunkBuf[0] = '<';
        while(index < smallChunkChars - 1)  
        {
          datChar = dataFileOut.read();
          if(datChar != -1)
          {
            chunkBuf[index] = datChar;
            bigBufIndex++;
          }
          else
          {
            chunkBuf[index] = ' ';
          }
          index++;
        }         
//        dataFileOut.close();
        chunkBuf[smallChunkChars - 1] = '>';
        Serial.write(chunkBuf, smallChunkChars);
        mySerial.write(chunkBuf, smallChunkChars);
        //handshook = false;
        readyTx = false;
        delay(10);
      }
      dataFileOut.close();
    }
    else
    {
      //Serial.println("here");
      baseStationCheck = '0';
      bigBufIndex = 0;
    }
  }
}
