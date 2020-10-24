/*
OLED connections
 GND - GND
 VCC - VCC
 SDA - Arduino pin A4
 SCL - Arduino pin A5 
*/

#include "U8glib.h"

U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_DEV_0|U8G_I2C_OPT_NO_ACK|U8G_I2C_OPT_FAST);  // Fast I2C / TWI

#define MOVING_AVERAGE_COUNT 16

unsigned int samples0[MOVING_AVERAGE_COUNT];
unsigned int samples1[MOVING_AVERAGE_COUNT];
unsigned int samples2[MOVING_AVERAGE_COUNT];
unsigned int samples3[MOVING_AVERAGE_COUNT];

int maIndex0 = 0;
int maIndex1 = 0;
int maIndex2 = 0;
int maIndex3 = 0;

int total0 = 0;
int total1 = 0;
int total2 = 0;
int total3 = 0;

void setup() 
{  
  // not really necessary
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  pinMode(A3, INPUT);
  
  u8g.setColorIndex(1);
  u8g.setFont(u8g_font_fur11);
  
  memset(samples0, 0, MOVING_AVERAGE_COUNT * sizeof(unsigned int));
  memset(samples1, 0, MOVING_AVERAGE_COUNT * sizeof(unsigned int));
  memset(samples2, 0, MOVING_AVERAGE_COUNT * sizeof(unsigned int));
  memset(samples3, 0, MOVING_AVERAGE_COUNT * sizeof(unsigned int));
  
  // fill the moving average buffer before displaying anything, 
  // otherwise numbers will take a few seconds to settle
  for (int i = 0; i < MOVING_AVERAGE_COUNT; i++)
    sample();
}

char vbuf0[16];
char vbuf1[16];
char vbuf2[16];
char vbuf3[16];

void draw4() {
  u8g.drawStr( 2, 12, vbuf0);
  u8g.drawStr( 2, 24, vbuf1);
  u8g.drawStr( 2, 36, vbuf2);
  u8g.drawStr( 2, 48, vbuf3);
}

void draw3() {
  u8g.drawStr( 2, 12, vbuf0);
  u8g.drawStr( 2, 24, vbuf1);
  u8g.drawStr( 2, 36, vbuf2);
}

void draw2() {
  u8g.drawStr( 2, 12, vbuf0);
  u8g.drawStr( 2, 24, vbuf1);
}

unsigned long lastScreenUpdate = 0;

#define DO_MOVING_AVERAGE(which)\
  total##which -= samples##which[ maIndex##which ];\
  samples##which[ maIndex##which ] = analogRead(A##which);\
  total##which += samples##which[ maIndex##which ];\
  maIndex##which = (maIndex##which + 1) % MOVING_AVERAGE_COUNT;

void sample()
{  
  DO_MOVING_AVERAGE(0)
  DO_MOVING_AVERAGE(1)
  DO_MOVING_AVERAGE(2)
  DO_MOVING_AVERAGE(3)
}

void updateScreen() 
{
  unsigned long now = millis();
  if ( now - lastScreenUpdate < 100 )
    return;
    
  sample();
    
  float v0 = 5 * (total0 / (float)MOVING_AVERAGE_COUNT) / 1023.0f;
  float v1 = 5 * (total1 / (float)MOVING_AVERAGE_COUNT) / 1023.0f;
  float v2 = 5 * (total2 / (float)MOVING_AVERAGE_COUNT) / 1023.0f;
  float v3 = 5 * (total3 / (float)MOVING_AVERAGE_COUNT) / 1023.0f;
    
  // set these to your resistor actual values
  float r11 =  985.2;
  float r12 =  978.3;
  float r21 =  978.9;
  float r22 = 2169;
  float r31 =  992.1;
  float r32 = 2947;
    
  v0 *= 1;
  v1 *= (r11 + r12) / r11;
  v2 *= (r21 + r22) / r21;
  v3 *= (r31 + r32) / r31;
  
  // adjust to match your voltmeter results
  v0 *= 1.0048;
  v1 *= 1.0045;
  v2 *= 1.0044;
  v3 *= 1.0043;
  
  v3 -= v2;
  v2 -= v1;
  v1 -= v0;
    
  int whole0 = (int)v0;
  int whole1 = (int)v1;
  int whole2 = (int)v2;
  int whole3 = (int)v3;
  int fraction0 = (v0 * 100) - (whole0 * 100);
  int fraction1 = (v1 * 100) - (whole1 * 100);
  int fraction2 = (v2 * 100) - (whole2 * 100);
  int fraction3 = (v3 * 100) - (whole3 * 100);
  
  sprintf(vbuf0, "v0: %d.%02d", whole0, fraction0);
  sprintf(vbuf1, "v1: %d.%02d", whole1, fraction1);
  sprintf(vbuf2, "v2: %d.%02d", whole2, fraction2);
  sprintf(vbuf3, "v3: %d.%02d", whole3, fraction3);

  u8g.firstPage();
  if ( v3 >= 0 && v2 >= 0 ) {
    do {
      draw4();
    } 
    while( u8g.nextPage() );
  }
  else if ( v2 >= 0 ) {
    do {
      draw3();
    } 
    while( u8g.nextPage() );
  }
  else {
    do {
      draw2();
    } 
    while( u8g.nextPage() );
  }
  
  lastScreenUpdate = millis();  
}

void loop()
{
  updateScreen();
}
