#define OLED_ADDRESS            0x3C
#define SCREEN_WIDTH            128
#define SCREEN_HEIGHT           64

#include <Adafruit_GFX.h>

#include <Adafruit_BusIO_Register.h>
#include <Adafruit_I2CDevice.h>
#include <Adafruit_I2CRegister.h>
#include <Adafruit_SPIDevice.h>
#include <Adafruit_SH110X.h>
#include <Fonts/FreeMonoBoldOblique12pt7b.h>

Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

struct TGPS
{
  int Hours, Minutes, Seconds;
  unsigned int Satellites;
  int  GotNewTime;
} GPS;

void setup()
{
  Serial.begin(9600);
  Serial1.begin(9600);

  delay(500);

  SetupOLED();

  DisplayPosition();
}

void loop()
{
  CheckGPS();

  if (GPS.GotNewTime)
  {
    GPS.GotNewTime = 0;
    DisplayPosition();
  }
}

void SetupOLED()
{
  char Temp[32];
  
  Wire.begin();

  if (display.begin(OLED_ADDRESS, true))
  {
    Serial.println("OLED OK");
    
    display.display();

    display.setTextColor(SH110X_WHITE, SH110X_BLACK);
    
    display.setRotation(2);
  }
  else
  {
    Serial.println("OLED Fail");
  }
}

void DisplayPosition(void)
{
  int i;
  char Temp[16];
  
  display.clearDisplay();  

  display.setFont();
  display.setTextSize(0);

  display.setCursor(38, 4);
  display.print("GPS Clock");

  display.drawRect(34, 1, 60, 14, SH110X_WHITE);

  display.drawLine(1, 14, 128, 14, SH110X_WHITE);
  display.drawLine(1, 46, 128, 46, SH110X_WHITE);

  display.setCursor(100, 1);
  
  for (i=0; i<12; i++)
  {
    int x;

    x = i * 10 + 5;

    if (i < GPS.Satellites)
    {
      display.fillRect(x, 54, 9, 6, SH110X_WHITE);
    }
    else
    {
      display.drawRect(x, 54, 9, 6, SH110X_WHITE);
    }
  }

  display.setFont(&FreeMonoBoldOblique12pt7b);
  display.setCursor(8, 36);
  sprintf(Temp, "%02d:%02d:%02d", GPS.Hours, GPS.Minutes, GPS.Seconds);
  display.print(Temp);  

  display.display();
}

char Hex(int Character)
{
  char HexTable[] = "0123456789ABCDEF";
  
  return HexTable[Character];
}

int GPSChecksumOK(char *Buffer, int Count)
{
  unsigned char XOR, i, c;

  XOR = 0;
  for (i = 1; i < (Count-4); i++)
  {
    c = Buffer[i];
    XOR ^= c;
  }

  return (Buffer[Count-4] == '*') && (Buffer[Count-3] == Hex(XOR >> 4)) && (Buffer[Count-2] == Hex(XOR & 15));
}

void ProcessNMEA(char *Buffer, int Count)
{
  int Satellites, date;
  char ns, ew;
  char TimeString[16], LatString[16], LongString[16], Temp[4];

  Serial.print(Buffer);

  if (GPSChecksumOK(Buffer, Count))
  {
    Satellites = 0;
  
    if (strncmp(Buffer+3, "GGA", 3) == 0)
    {
      int lock;
      char hdop[16], Altitude[16];
      
      Serial.print(Buffer+1);
      
      if (sscanf(Buffer+7, "%16[^,],%16[^,],%c,%[^,],%c,%d,%d,%[^,],%[^,]", TimeString, LatString, &ns, LongString, &ew, &lock, &Satellites, hdop, Altitude) >= 1)
      { 
        // $GPGGA,124943.00,5157.01557,N,00232.66381,W,1,09,1.01,149.3,M,48.6,M,,*42
        Temp[0] = TimeString[0]; Temp[1] = TimeString[1]; Temp[2] = '\0';
        GPS.Hours = atoi(Temp);
        Temp[0] = TimeString[2]; Temp[1] = TimeString[3]; Temp[2] = '\0';
        GPS.Minutes = atoi(Temp);
        Temp[0] = TimeString[4]; Temp[1] = TimeString[5]; Temp[2] = '\0';
        GPS.Seconds = atoi(Temp);
        GPS.Satellites = Satellites;
        GPS.GotNewTime = 1;
      }
      
      Serial.print(GPS.Hours); Serial.print(":"); Serial.print(GPS.Minutes); Serial.print(":"); Serial.print(GPS.Seconds);Serial.print(" - ");
      Serial.println(GPS.Satellites);
    }
  }
}

void CheckGPS(void)
{
  static char Line[128];
  static int Length=0;
  unsigned char Character;

  while (Serial1.available())
  {
    Character = Serial1.read();
  
    if (Character == '$')
    {
      Line[0] = Character;
      Length = 1;
    }
    else if (Length >= (sizeof(Line)-2))
    {
      Length = 0;
    }
    else if ((Length > 0) && (Character != '\r'))
    {
      Line[Length++] = Character;
      if (Character == '\n')
      {
        Line[Length] = '\0';
        ProcessNMEA(Line, Length);
        Length = 0;
      }
    }
  }
}
