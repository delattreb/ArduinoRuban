#define REDUCED_MODES // sketch too big for Arduino Leonardo flash, so invoke reduced modes
#include "WS2812FX.h"
#include "ec11.h"
#include "config.h"
#include <time.h>

// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
WS2812FX ws2812fx = WS2812FX(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

EC11 encoder;
boolean b_mode, b_effect = false;
static int mode = MODE_BRIGHTNESS;
static int effect = 45; //45  Selection 3-18-44-45-55
static int bright = 255; //0-255
static int speed = 3500;
int cpt = 0;

//Setup
void setup()
{
#ifdef DEBUG
  Serial.begin(BAUDS_RATE);
#endif 
  //Init Pinout
  pinMode(PIN_A, INPUT_PULLUP);
  pinMode(PIN_B, INPUT_PULLUP);
  pinMode(PIN_BUTTON, INPUT);
  prepare();

  ws2812fx.init();
  ws2812fx.setColor(LED_COLOR);
  ws2812fx.setBrightness(bright);
  ws2812fx.setMode(effect);
  ws2812fx.setSpeed(speed);
  ws2812fx.start();
}

void loop()
{
  ReadInput();
  ReadCoder();

  ws2812fx.setBrightness(bright);
  ws2812fx.setSpeed(speed);
  ws2812fx.service();
}

// ReadCoder
void ReadCoder()
{
  EC11Event e;
  if (encoder.read(&e))
  {
    if (e.type == EC11Event::StepCW)
    {
      if (mode == MODE_EFFECT)
      {
        effect += e.count;
        if (effect >= MAX_EFFEXT)
          effect = MAX_EFFEXT;
        ws2812fx.stop();
        ws2812fx.setMode(effect);
        ws2812fx.start();
      }
      if (mode == MODE_BRIGHTNESS)
      {
        bright += (e.count * BRIGHT_INC);
        if (bright >= MAX_BRIGHTNESS)
          bright = MAX_BRIGHTNESS;
      }
      if (mode == MODE_SPEED)
      {
        speed += (e.count * SPEED_INC);
        if (speed >= MAX_SPEED)
          speed = MAX_SPEED;
      }
    }
    else
    {
      if (mode == MODE_EFFECT)
      {
        effect -= e.count;
        if (effect <= 0)
          effect = 0;
        ws2812fx.stop();
        ws2812fx.setMode(effect);
        ws2812fx.start();
      }
      if (mode == MODE_BRIGHTNESS)
      {
        bright -= (e.count * BRIGHT_INC);
        if (bright <= 1)
          bright = 1;
      }
      if (mode == MODE_SPEED)
      {
        speed -= (e.count * SPEED_INC);
        if (speed <= 0)
          speed = 0;
      }
    }
#ifdef DEBUG
    Serial.print("E:");
    Serial.print(effect);
    Serial.print(" B:");
    Serial.print(bright);
    Serial.print(" S:");
    Serial.println(speed);
#endif // DEBUG
  }
}

// ReadInput
void ReadInput()
{
  //Read EC11 coder
  if (digitalRead(PIN_BUTTON) == HIGH && !b_mode)
  {
    b_mode = true;
    mode += 1;
    if (mode >= NB_MODE)
      mode = 0;
  }
  if (digitalRead(PIN_BUTTON) == LOW)
    b_mode = false;
}

// pinDidChange
void pinDidChange()
{
  encoder.checkPins(digitalRead(PIN_A), digitalRead(PIN_B));
}

// prepare
void prepare()
{
  attachInterrupt(0, pinDidChange, CHANGE);
  attachInterrupt(1, pinDidChange, CHANGE);
}
