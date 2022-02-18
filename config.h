#include <Wire.h>
#include <SPI.h>
#include <Adafruit_LIS3DH.h>
#include <Adafruit_Sensor.h>

// Used for software SPI
#define LIS3DH_CLK 13
#define LIS3DH_MISO 12
#define LIS3DH_MOSI 11
// Used for hardware & software SPI
#define LIS3DH_CS 10

// Adjust this number for the sensitivity of the 'click' force
// this strongly depend on the range! for 16G, try 5-10
// for 8G, try 10-20. for 4G try 20-40. for 2G try 40-80
#define CLICKTHRESHHOLD 40


// NEOPIXEL BEST PRACTICES for most reliable operation:
// - Add 1000 uF CAPACITOR between NeoPixel strip's + and - connections.
// - MINIMIZE WIRING LENGTH between microcontroller board and first pixel.
// - NeoPixel strip's DATA-IN should pass through a 300-500 OHM RESISTOR.
// - AVOID connecting NeoPixels on a LIVE CIRCUIT. If you must, ALWAYS
//   connect GROUND (-) first, then +, then data.
// - When using a 3.3V microcontroller with a 5V-powered NeoPixel strip,
//   a LOGIC-LEVEL CONVERTER on the data line is STRONGLY RECOMMENDED.
// (Skipping these may work OK on your workbench but can fail in the field)

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1:
#define LED_PIN    A4
#define SENSOR_PIN A0

// How many NeoPixels are attached to the Arduino?
#define LED_COUNT 68

// Declare our NeoPixel strip object:
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
// Argument 1 = Number of pixels in NeoPixel strip
// Argument 2 = Arduino pin number (most are valid)
// Argument 3 = Pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)

// Build lookup table/palette for the color animations so they aren't computed at runtime.
// The colorPalette two-dimensional array below has a row for each color animation and a column
// for each step within the animation.  Each value is a 24-bit RGB color.  By looping through
// the columns of a row the colors of pixels will animate.
const int colorSteps = 8;   // Number of rows/animations.
const int colorCount = 24;  // Number of columns/steps.
const uint32_t colorPalette[colorCount][colorSteps] PROGMEM = {
  // Complimentary colors  
  { 0xFF0000, 0xDA2424, 0xB64848, 0x916D6D, 0x6D9191, 0x48B6B6, 0x24DADA, 0x00FFFF }, // Red-cyan
  { 0xFFFF00, 0xDADA24, 0xB6B648, 0x91916D, 0x6D6D91, 0x4848B6, 0x2424DA, 0x0000FF }, // Yellow-blue
  { 0x00FF00, 0x24DA24, 0x48B648, 0x6D916D, 0x916D91, 0xB648B6, 0xDA24DA, 0xFF00FF }, // Green-magenta

  // Adjacent colors (on color wheel).
  { 0xFF0000, 0xFF2400, 0xFF4800, 0xFF6D00, 0xFF9100, 0xFFB600, 0xFFDA00, 0xFFFF00 }, // Red-yellow
  { 0xFFFF00, 0xDAFF00, 0xB6FF00, 0x91FF00, 0x6DFF00, 0x48FF00, 0x24FF00, 0x00FF00 }, // Yellow-green
  { 0x00FF00, 0x00FF24, 0x00FF48, 0x00FF6D, 0x00FF91, 0x00FFB6, 0x00FFDA, 0x00FFFF }, // Green-cyan
  { 0x00FFFF, 0x00DAFF, 0x00B6FF, 0x0091FF, 0x006DFF, 0x0048FF, 0x0024FF, 0x0000FF }, // Cyan-blue
  { 0x0000FF, 0x2400FF, 0x4800FF, 0x6D00FF, 0x9100FF, 0xB600FF, 0xDA00FF, 0xFF00FF }, // Blue-magenta
  { 0xFF00FF, 0xFF00DA, 0xFF00B6, 0xFF0091, 0xFF006D, 0xFF0048, 0xFF0024, 0xFF0000 }, // Magenta-red

  // Other combos.
  { 0xFF0000, 0xDA2400, 0xB64800, 0x916D00, 0x6D9100, 0x48B600, 0x24DA00, 0x00FF00 }, // Red-green
  { 0xFFFF00, 0xDAFF24, 0xB6FF48, 0x91FF6D, 0x6DFF91, 0x48FFB6, 0x24FFDA, 0x00FFFF }, // Yellow-cyan
  { 0x00FF00, 0x00DA24, 0x00B648, 0x00916D, 0x006D91, 0x0048B6, 0x0024DA, 0x0000FF }, // Green-blue
  { 0x00FFFF, 0x24DAFF, 0x48B6FF, 0x6D91FF, 0x916DFF, 0xB648FF, 0xDA24FF, 0xFF00FF }, // Cyan-magenta
  { 0x0000FF, 0x2400DA, 0x4800B6, 0x6D0091, 0x91006D, 0xB60048, 0xDA0024, 0xFF0000 }, // Blue-red
  { 0xFF00FF, 0xFF24DA, 0xFF48B6, 0xFF6D91, 0xFF916D, 0xFFB648, 0xFFDA24, 0xFFFF00 }, // Magenta-yellow  

  // Solid colors fading to dark.
  { 0xFF0000, 0xDF0000, 0xBF0000, 0x9F0000, 0x7F0000, 0x5F0000, 0x3F0000, 0x1F0000 }, // Red
  { 0xFF9900, 0xDF8500, 0xBF7200, 0x9F5F00, 0x7F4C00, 0x5F3900, 0x3F2600, 0x1F1300 }, // Orange
  { 0xFFFF00, 0xDFDF00, 0xBFBF00, 0x9F9F00, 0x7F7F00, 0x5F5F00, 0x3F3F00, 0x1F1F00 }, // Yellow
  { 0x00FF00, 0x00DF00, 0x00BF00, 0x009F00, 0x007F00, 0x005F00, 0x003F00, 0x001F00 }, // Green
  { 0x0000FF, 0x0000DF, 0x0000BF, 0x00009F, 0x00007F, 0x00005F, 0x00003F, 0x00001F }, // Blue
  { 0x4B0082, 0x410071, 0x380061, 0x2E0051, 0x250041, 0x1C0030, 0x120020, 0x090010 }, // Indigo
  { 0x8B00FF, 0x7900DF, 0x6800BF, 0x56009F, 0x45007F, 0x34005F, 0x22003F, 0x11001F }, // Violet
  { 0xFFFFFF, 0xDFDFDF, 0xBFBFBF, 0x9F9F9F, 0x7F7F7F, 0x5F5F5F, 0x3F3F3F, 0x1F1F1F }, // White

  // Rainbow colors.
  { 0xFF0000, 0xFF9900, 0xFFFF00, 0x00FF00, 0x0000FF, 0x4B0082, 0x8B00FF, 0xFFFFFF }
};
