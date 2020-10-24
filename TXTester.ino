/*
 
 Connections for nRF24L01 pins
 1 - GND
 2 - VCC 3.3V !!! NOT 5V
 3 - CE to Arduino pin 9
 4 - CSN to Arduino pin 10
 5 - SCK to Arduino pin 13
 6 - MOSI to Arduino pin 11
 7 - MISO to Arduino pin 12
 8 - UNUSED
 
 Connections for OLED screen pins
 GND - GND
 VCC - VCC (mine seems to work with both 5V and 3.3V
 SDA - arduino analog pin 4
 SCL - arduino analog pin 5 
 
 */

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

const uint64_t pipeIn =  0xE8E8F0F0E1LL; // match this on both sides!

RF24 radio(9, 10);

// The sizeof this struct should not exceed 32 bytes
struct MyData {
  byte throttle;
  byte yaw;
  byte pitch;
  byte roll;
  byte dial1;
  byte dial2;
  byte switches; // bitflag
};

MyData data;

unsigned long packetsRead = 0;
unsigned long lastUpdate = 0;
int packetsSec = 0;
unsigned long lastRecvTime = 0;
unsigned long drops = 0;

/**************************************************/

// writes a single line of formatted text to the screen at the current cursor position - keep it short!
void displayln(const char* format, ...)
{
  char buffer[64];
  
  va_list args;
  va_start(args, format);
  vsprintf(buffer, format, args);
  va_end(args);
  
  int len = strlen(buffer);
  for (uint8_t i = 0; i < len; i++) {
    display.write(buffer[i]);
  }
}

/**************************************************/

void setup()
{
  radio.begin();
  radio.setDataRate(RF24_250KBPS); // Both endpoints must have this set the same
  radio.setAutoAck(false); // Both endpoints must have this set the same

  radio.openReadingPipe(1,pipeIn);
  radio.startListening();

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D (for the 128x64)
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.clearDisplay();
    
  memset(&data, 0, sizeof(MyData));

}

/**************************************************/

void recvData()
{  
  while ( radio.available() ) {        
    radio.read(&data, sizeof(MyData));
    packetsRead++;
    lastRecvTime = millis();
  }
  
  unsigned long now = millis();
  if ( now - lastRecvTime > 1000 ) {
    drops++;
  }
}

/**************************************************/

void displayBits(byte b, int howManyBits)
{
  for (int i = 0; i < howManyBits; i++) {
    int mask = 1 << i;
    if ( b & mask )
      display.write('*');
    else
      display.write('-');
  }
}

/**************************************************/

void updateScreen()
{
  unsigned long now = millis();
  if ( now - lastUpdate > 1000 ) {
    packetsSec = packetsRead;
    packetsRead = 0;
    lastUpdate = now;
  }
      
  char buf[16];
    
  display.clearDisplay();
  display.setCursor(0,0);
  displayln("Packets = %d\n", packetsSec);
  
  displayln("%d\n", data.throttle);
  displayln("%d\n", data.yaw);
  displayln("%d\n", data.pitch);
  displayln("%d\n", data.roll);
  displayln("%d\n", data.dial1);
  displayln("%d\n", data.dial2);
  
  // draw rectangle for joystick outline
  display.drawRect(30,12, 40,40, 1);
  display.drawRect(75,12, 40,40, 1);
  
  int d1 = map(data.throttle, 0, 255, 0, 100);
  int d2 = map(data.yaw,      0, 255, 0, 100);
  int d3 = map(data.pitch,    0, 255, 0, 100);
  int d4 = map(data.roll,     0, 255, 0, 100);
  
  // draw dot for current joystick position
  display.fillRect(30+40*(d2/100.0)-3, 52-40*(d1/100.0)-3, 6,6, 1);
  display.fillRect(75+40*(d4/100.0)-3, 52-40*(d3/100.0)-3, 6,6, 1);
  
  display.setCursor(0,7*8);
    
  displayBits(data.switches, 8);
  display.write(' ');
  
  displayln("%ld drops\n", drops);
    
  display.display();
}

/**************************************************/

void loop()
{
  recvData();
  updateScreen();
}






