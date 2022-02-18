#include "config.h"

int SENSOR_PIN = A0;    // select the input pin for the potentiometer
int ledPin = 13;      // select the pin for the LED

// software SPI
// Adafruit_LIS3DH lis = Adafruit_LIS3DH(LIS3DH_CS, LIS3DH_MOSI, LIS3DH_MISO, LIS3DH_CLK);
// hardware SPI
//Adafruit_LIS3DH lis = Adafruit_LIS3DH(LIS3DH_CS);
// I2C
Adafruit_LIS3DH lis = Adafruit_LIS3DH();

sensors_event_t getAccelData () {
  lis.read();      // get X Y and Z data at once
  // Then print out the raw data
//  Serial.print("X:  "); Serial.print(lis.x);
//  Serial.print("  \tY:  "); Serial.print(lis.y);
//  Serial.print("  \tZ:  "); Serial.print(lis.z);

  /* Or....get a new sensor event, normalized */
  sensors_event_t event;
  lis.getEvent(&event);

#ifdef PROJ_DEBUG
  /* Display the results (acceleration is measured in m/s^2) */
  Serial.print("\t\tX: "); Serial.print(event.acceleration.x);
  Serial.print(" \tY: "); Serial.print(event.acceleration.y);
  Serial.print(" \tZ: "); Serial.print(event.acceleration.z);
  Serial.println(" m/s^2 ");

  Serial.println();
#endif
  return event;  
}

int avg (int * arr, uint8_t len) {
  // bypass error
  if (len <=0)
    return 0;
    
  int sum = 0;
  for (uint8_t ind = 0; ind < len; ++ind) {
    sum += arr[ind];
  }
  return sum/len;
}

sensors_event_t getAmbientConditions () {
#define SAMPLES 10
  static uint8_t index = 0;
  static int lightSamples[SAMPLES] = {0};
  // read the value from the sensor; scale it to max brightness 0-255 (256/1024)
  #define MINIMUM 5
  #define MAXIMUM 255
  int sensorValue = analogRead(SENSOR_PIN)/4;
  lightSamples [index] = (sensorValue+MINIMUM)% (MAXIMUM-MINIMUM +1) - MINIMUM;
  
  Serial.print("[SENSOR_PIN] ");
  Serial.print(sensorValue);
  Serial.print("...\n");
  strip.setBrightness(avg(lightSamples, SAMPLES)); // Set BRIGHTNESS (min = 5)

  return getAccelData ();
//  clickDetect ();  
}

void stripOff () {
  strip.clear ();
  strip.show ();  
}

uint8_t colorZap(uint32_t color, int wait) {
  for(int i=0; i<LED_COUNT; i++) { // For each pixel in strip...
    strip.setPixelColor(i, color);         //  Set pixel's color (in RAM)
  }
  strip.show();                          //  Update strip to match
  delay(wait);                           //  Pause for a moment
  sensors_event_t  event = getAmbientConditions ();
  if (event.acceleration.z < 0){
    stripOff ();
    return 0;
  }
  
  return 1;
}

uint8_t colorFill (uint32_t color, int wait) {
  
}
// Fill strip pixels one after another with a color. Strip is NOT cleared
// first; anything there will be covered pixel by pixel. Pass in color
// (as a single 'packed' 32-bit value, which you can get by calling
// strip.Color(red, green, blue) as shown in the loop() function above),
// and a delay time (in milliseconds) between pixels.
uint8_t colorWipe(uint32_t color, int wait) {
  for(int i=0; i<LED_COUNT; i++) { // For each pixel in strip...
    strip.setPixelColor(i, color);         //  Set pixel's color (in RAM)
    strip.show();                          //  Update strip to match
    delay(wait);                           //  Pause for a moment
    sensors_event_t  event = getAmbientConditions ();
    if (event.acceleration.z < 0){
      stripOff ();
      return 0;
    }
  }
  return 1;
}

// Theater-marquee-style chasing lights. Pass in a color (32-bit value,
// a la strip.Color(r,g,b) as mentioned above), and a delay time (in ms)
// between frames.
/*#define THEATER_SPACING (LED_COUNT/20)
uint8_t theaterChase(uint32_t color, int wait) {
  for(uint8_t a=0; a<10; a++) {  // Repeat 10 times...
    for(uint8_t b=0; b < THEATER_SPACING; b++) { //  'b' counts from 0 to 2...
      strip.clear();         //   Set all pixels in RAM to 0 (off)
      // 'c' counts up from 'b' to end of strip in steps of 3...
      for(int c=b; c<LED_COUNT; c += 3) {
        strip.setPixelColor(c, color); // Set pixel 'c' to value 'color'
      }
      strip.show(); // Update strip with new contents
      delay(wait);  // Pause for a moment
      sensors_event_t  event = getAmbientConditions ();
      if (event.acceleration.z < 0){
        stripOff ();
        return 0;
      }
    }
  }
  return 1;
}
*/

uint8_t twinkle(uint32_t color, int wait) {
  for(int a=0; a<10; a++) {  // Repeat 10 times...
    for(int b=0; b<3; b++) { //  'b' counts from 0 to 2...
      strip.clear();         //   Set all pixels in RAM to 0 (off)
      // 'c' counts up from 'b' to end of strip in steps of 3...
      for(int c=b; c<strip.numPixels(); c += 3) {
        strip.setPixelColor(c, color); // Set pixel 'c' to value 'color'
      }
      strip.show(); // Update strip with new contents
      delay(wait);  // Pause for a moment
    }
  }
  return 1;
}

//Theatre-style crawling lights.
#define THEATER_SPACING (LED_COUNT/3)
uint8_t theaterChase(uint32_t c, uint8_t wait) {
  for (int j=0; j<10; j++) {  //do 10 cycles of chasing
    for (int q=0; q < THEATER_SPACING; q++) {
      for (int i=0; i < LED_COUNT; i+=THEATER_SPACING) {
        strip.setPixelColor(i+q, c);    //turn every third pixel on
      }
      strip.show();

      delay(wait);
      sensors_event_t  event = getAmbientConditions ();
      if (event.acceleration.z < 0){
        stripOff ();
        return 0;
      }

      for (int i=0; i < LED_COUNT; i+=THEATER_SPACING) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
      
    }
  }
  return 1;
}

// FuzzyRainbow cycle along whole strip. Pass delay time (in ms) between frames.
uint8_t fuzzyRainbow(int wait) {
  // Hue of first pixel runs 5 complete loops through the color wheel.
  // Color wheel has a range of 65536 but it's OK if we roll over, so
  // just count from 0 to 5*65536. Adding 256 to firstPixelHue each time
  // means we'll make 5*65536/256 = 1280 passes through this outer loop:
  for(long firstPixelHue = 0; firstPixelHue < 5*65536; firstPixelHue += 256) {
    
    for(int i=0; i<LED_COUNT; i++) { // For each pixel in strip...
      // Offset pixel hue by an amount to make one full revolution of the
      // color wheel (range of 65536) along the length of the strip
      // (LED_COUNT steps):
      int pixelHue = firstPixelHue + ((random(256)/255) * 65536L / LED_COUNT);
      // strip.ColorHSV() can take 1 or 3 arguments: a hue (0 to 65535) or
      // optionally add saturation and value (brightness) (each 0 to 255).
      // Here we're using just the single-argument hue variant. The result
      // is passed through strip.gamma32() to provide 'truer' colors
      // before assigning to each pixel:
      strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue)));
    }
    strip.show(); // Update strip with new contents
    delay(wait);  // Pause for a moment
    sensors_event_t  event = getAmbientConditions ();
    if (event.acceleration.z < 0){
      stripOff ();
      return 0;
    }
  }
  return 1;
}

// Rainbow cycle along whole strip. Pass delay time (in ms) between frames.
uint8_t rainbow(int wait) {
  // Hue of first pixel runs 5 complete loops through the color wheel.
  // Color wheel has a range of 65536 but it's OK if we roll over, so
  // just count from 0 to 5*65536. Adding 256 to firstPixelHue each time
  // means we'll make 5*65536/256 = 1280 passes through this outer loop:
  for(long firstPixelHue = 0; firstPixelHue < 5*65536; firstPixelHue += 256) {
    
    for(int i=0; i<LED_COUNT; i++) { // For each pixel in strip...
      // Offset pixel hue by an amount to make one full revolution of the
      // color wheel (range of 65536) along the length of the strip
      // (LED_COUNT steps):
      int pixelHue = firstPixelHue + (i * 65536L / LED_COUNT);
      // strip.ColorHSV() can take 1 or 3 arguments: a hue (0 to 65535) or
      // optionally add saturation and value (brightness) (each 0 to 255).
      // Here we're using just the single-argument hue variant. The result
      // is passed through strip.gamma32() to provide 'truer' colors
      // before assigning to each pixel:
      strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue)));
    }
    strip.show(); // Update strip with new contents
    delay(wait);  // Pause for a moment
    sensors_event_t  event = getAmbientConditions ();
    if (event.acceleration.z < 0){
      stripOff ();
      return 0;
    }
  }
  return 1;
}

#define COLOR_CYCLE 65536 / 90; // One cycle of color wheel over 90 frames
// Rainbow-enhanced theater marquee. Pass delay time (in ms) between frames.
uint8_t theaterChaseRainbow(int wait) {
  int firstPixelHue = 0;     // First pixel starts at red (hue 0)
  for(uint8_t a=0; a<30; a++) {  // Repeat 30 times...
    for(uint8_t b=0; b<3; b++) { //  'b' counts from 0 to 2...
      strip.clear();         //   Set all pixels in RAM to 0 (off)
      // 'c' counts up from 'b' to end of strip in increments of 3...
      for(int c=b; c<LED_COUNT; c += 3) {
        // hue of pixel 'c' is offset by an amount to make one full
        // revolution of the color wheel (range 65536) along the length
        // of the strip (LED_COUNT steps):
        int      hue   = firstPixelHue + c * 65536L / LED_COUNT;
        uint32_t color = strip.gamma32(strip.ColorHSV(hue)); // hue -> RGB
        strip.setPixelColor(c, color); // Set pixel 'c' to value 'color'
      }
      firstPixelHue += COLOR_CYCLE;
      strip.show();                // Update strip with new contents
      delay(wait);                 // Pause for a moment
      sensors_event_t  event = getAmbientConditions ();
      if (event.acceleration.z < 0){
        stripOff ();
        return 0;
      }
    }
  }
  return 1;
}


uint8_t colorZapCycle (int wait)   {
  int on = 0;
  static int next = 0;
  static int red = 0;
  static int green = 0;
  static int blue = 0;
  int color = 0;  
  switch (next) {
  case 0:
    red = ++red%256;
    if (0 == red){
      next = 1;
      green = 1; 
    }
    break;
  case 1:
    green = ++green%256;
    if (0 == green){
      next = 2;
      blue = 1; 
    }
    break;
  case 2:
    blue = ++blue%256;
    if (0 == blue){
      next = 0;
      red = 1; 
    }
    break;
  }
  // Fill along the length of the strip in various colors...
  return colorZap (strip.Color(red, green, blue), wait);
}

uint8_t colorSeqCycle (int wait) {
  static int red = 127;
  static int green = 0;
  static int blue = 85;

  if (blue < 85) {
    red = (3+red)%256;
    if (0 == red){
      green = 1; 
    }
  }
  if (red > 85) {
    green = (3+green)%256;
    if (0 == green){
      blue = 1; 
    }
  }  
  if (green > 85) {
    blue = (3+blue)%256;
    if (0 == blue){
      red = 1; 
    }
  }
  // Fill along the length of the strip in various colors...
  return theaterChase (strip.Color(red, green, blue), wait);
  
}

uint8_t colorWipeCycle (int wait) {
  static int red = 0;
  static int green = 0;
  static int blue = 0;

  if (blue < 85) {
    red = (3+red)%256;
//    if (0 == red){
//      green = 1; 
//    }
  }
  if (red > 85) {
    green = (3+green)%256;
//    if (0 == green){
//      blue = 1; 
//    }
  }  
  if (green > 85) {
    blue = (3+blue)%256;
//    if (0 == blue){
//      red = 1; 
//    }
  }
  // Fill along the length of the strip in various colors...
  return colorWipe (strip.Color(red, green, blue), wait);
}

// List of animations speeds (in milliseconds).  This is how long an animation spends before
// changing to the next step.  Higher values are slower.
const uint16_t speeds[5] = { 400, 200, 100, 50, 25 };

// Global state used by the sketch:
uint8_t colorIndex = 0;
uint8_t animationIndex = 0;
uint8_t speedIndex = 2;

uint8_t lanternLightPulsing (int wait) {
  for (int i = 0; i < LED_COUNT; ++i) {
    // Animation 0, solid color pulse of all pixels.
    uint8_t currentStep = (millis()/speeds[speedIndex])%(colorSteps*2-2);
    if (currentStep >= colorSteps) {
      currentStep = colorSteps-(currentStep-(colorSteps-2));
    }
    // Note that colors are stored in flash memory so they need to be read
    // using the pgmspace.h functions.
    uint32_t color = pgm_read_dword_near(&colorPalette[colorIndex][currentStep]);
    strip.setPixelColor(i, color);
 }
  strip.show();                // Update strip with new contents
  delay(wait);                 // Pause for a moment
  sensors_event_t  event = getAmbientConditions ();
  if (event.acceleration.z < 0){
    stripOff ();
    return 0;
  }
  return 1;
}


uint8_t lanternLightModulate (int wait){
  for (int i = 0; i < LED_COUNT; ++i) {  
    // Animation 1, moving color pulse.  Use position to change brightness.
    uint8_t currentStep = (millis()/speeds[speedIndex]+i)%(colorSteps*2-2);
    if (currentStep >= colorSteps) {
      currentStep = colorSteps-(currentStep-(colorSteps-2));
    }
    // Note that colors are stored in flash memory so they need to be read
    // using the pgmspace.h functions.
    uint32_t color = pgm_read_dword_near(&colorPalette[colorIndex][currentStep]);
    strip.setPixelColor(i, color);
  }
  strip.show();                // Update strip with new contents
  delay(wait);                 // Pause for a moment
  sensors_event_t  event = getAmbientConditions ();
  if (event.acceleration.z < 0){
    stripOff ();
    return 0;
  }
  return 1;

}
/*
 * Serial Debug
 */
void setupSerial () {
  Serial.begin(9600);
  // Serial.setDebugOutput(true);
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  
  // wait for serial monitor to open
  while (!Serial);
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW  
  Serial.print("Serial Com Ready!");
}


void setupAccelSensor () {

  Serial.println("LIS3DH test!");

  if (! lis.begin(0x18)) {   // change this to 0x19 for alternative i2c address
    Serial.println("Couldnt start");
    while (1) yield();
  }
  Serial.println("LIS3DH found!");

  lis.setRange(LIS3DH_RANGE_4_G);   // 2, 4, 8 or 16 G!

  Serial.print("Range = "); Serial.print(2 << lis.getRange());
  Serial.println("G");

  // lis.setDataRate(LIS3DH_DATARATE_50_HZ);
  Serial.print("Data rate set to: ");
  switch (lis.getDataRate()) {
    case LIS3DH_DATARATE_1_HZ: Serial.println("1 Hz"); break;
    case LIS3DH_DATARATE_10_HZ: Serial.println("10 Hz"); break;
    case LIS3DH_DATARATE_25_HZ: Serial.println("25 Hz"); break;
    case LIS3DH_DATARATE_50_HZ: Serial.println("50 Hz"); break;
    case LIS3DH_DATARATE_100_HZ: Serial.println("100 Hz"); break;
    case LIS3DH_DATARATE_200_HZ: Serial.println("200 Hz"); break;
    case LIS3DH_DATARATE_400_HZ: Serial.println("400 Hz"); break;

    case LIS3DH_DATARATE_POWERDOWN: Serial.println("Powered Down"); break;
    case LIS3DH_DATARATE_LOWPOWER_5KHZ: Serial.println("5 Khz Low Power"); break;
    case LIS3DH_DATARATE_LOWPOWER_1K6HZ: Serial.println("16 Khz Low Power"); break;
  }
  // 0 = turn off click detection & interrupt
  // 1 = single click only interrupt output
  // 2 = double click only interrupt output, detect single click
  // Adjust threshhold, higher numbers are less sensitive
  lis.setClick(2, CLICKTHRESHHOLD);
  delay(100);    
}

void setupNeoPixel () {
  // These lines are specifically to support the Adafruit Trinket 5V 16 MHz.
  // Any other board, you can remove this part (but no harm leaving it):
#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
  clock_prescale_set(clock_div_1);
#endif
  // END of Trinket-specific code.

  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(50); // Set BRIGHTNESS to about 1/5 (max = 255)
  Serial.println("NeoPixel Setup Complete");
}
void setup() {
  // if analog input pin 0 is unconnected, random analog
  // noise will cause the call to randomSeed() to generate
  // different seed numbers each time the sketch runs.
  // randomSeed() will then shuffle the random function.
  randomSeed(analogRead(0));
 setupSerial ();
 setupAccelSensor ();
 setupNeoPixel ();
 Serial.println("All Setup Complete");
}


void runLightingEffect () {
  static int mode = 0;
  static int on = 1;
  static int redFactor = 121;
  static int greenFactor = 121;
  static int blueFactor = 121;
  
  Serial.print("mode:  "); Serial.print(mode); Serial.print(", on:  "); Serial.print(on);
  if (on == 0){
    sensors_event_t  event = getAmbientConditions (); 
    if (event.acceleration.z >= 0){
      mode = (++mode % 12);
      on = 1;
    }
  }
  if (on == 0) return; 
 
  switch (mode) {
   case 1: 
    // Fill along the length of the strip in various colors...
    on = colorZapCycle(50);
    break;
  case 0:
    on = colorSeqCycle (50);
    break;
  case 11:
    on = colorWipeCycle(0); // Green
    break;
  case 3:
    on = colorWipe(strip.Color(  0,   0, 255), 50); // Blue
    break;
  case 2:
    on = lanternLightPulsing (50);
    break;
  case 10:
    on = lanternLightModulate (50);
    break;
  case 9:
   on = fuzzyRainbow (50);
   break;
  case 8:
    // Use Acclerometer to modify color strip
    {
      sensors_event_t  event = getAmbientConditions ();
  
      redFactor += event.acceleration.x;
      if (redFactor < 0) redFactor =1;
      if (redFactor > 122) redFactor =122;
       
      greenFactor += event.acceleration.y;
      if (greenFactor < 0) greenFactor =1;
      if (greenFactor > 122) greenFactor = 122;
       
      blueFactor += (event.acceleration.z-10);
      if (blueFactor < 0) blueFactor =1;
      if (blueFactor > 122) blueFactor = 122;
      
      Serial.print("mode:  "); Serial.print(mode);
      Serial.print(", redFactor:  "); Serial.print(redFactor);
      Serial.print(", greenFactor:  "); Serial.print(greenFactor);
      Serial.print(", blueFactor:  "); Serial.print(blueFactor);
      Serial.println(); 
      on = colorZap(strip.Color(1+(2*redFactor), 1+(2*greenFactor), 1+(2*blueFactor)), 5);
      
    }
    break;
  case 4: 
    // Do a theater marquee effect in various colors...
    on = theaterChase(strip.Color(255,   0,   0), 50); // Red, half brightness
    break;
  case 5:
    // Do a theater marquee effect in various colors...
    on = theaterChase(strip.Color(  0,   0, 127), 50); // Blue, half brightness
    break;
  case 6:
    on = rainbow(10);             // Flowing rainbow cycle along the whole strip
    break;
  case 7:
    on = theaterChaseRainbow(50); // Rainbow-enhanced theaterChase variant
    break;
  }  
}

uint8_t clickDetect (sensors_event_t & event) {
  static uint8_t mode = 0;
  static int z_orientation = 1;
  uint8_t click = lis.getClick();
  if ((click != 0) && (click & 0x30)){
    Serial.print("Click detected (0x"); Serial.print(click, HEX); Serial.print("): ");
    if (click & 0x10) {
      Serial.print(" single click");
     // ++mode;
    }
    if (click & 0x20) {
      Serial.print(" double click");
     // --mode;
    }
  } else {
    if (event.acceleration.z <0) {
      if (z_orientation > 0) {
       z_orientation = -1;     
       stripOff (); 
      }
    } else {
      if (z_orientation < 0) {
       z_orientation = 1;     
       ++mode;
      }          
    }
  }
    Serial.print("zOrientation ("); Serial.print(z_orientation); Serial.print("): ");
    // keep the mode between 0 and 7
    mode = mode %8;
    Serial.println();
  
  return mode;  
}

void loop() {

  digitalWrite(ledPin, HIGH);
  runLightingEffect ();
  // turn the ledPin on
  // turn the ledPin off:
  digitalWrite(ledPin, LOW);
 
}
