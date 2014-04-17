#include <Wire.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2561.h>

#define PIN 6
#define RANDOM_SEED_PIN 10
#define ONBOARD_ERROR_LED 13
#define MINIMUM_LUX 20
#define DEFAULT_BRIGHTNESS 255
#define NUMBER_OF_LEDS 16
#define MAXIMUM_DRIFT 50
#define TWEEN_CONSTANT 10

// Parameter 1 = number of pixels in strip
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMBER_OF_LEDs, PIN, NEO_GRB + NEO_KHZ800);
Adafruit_TSL2561 tsl = Adafruit_TSL2561(TSL2561_ADDR_FLOAT, 12345);
int lux = DEFAULT_BRIGHTNESS;

void configureSensor(void)
{
	tsl.setGain(TSL2561_GAIN_1X);      /* No gain ... use in bright light to avoid sensor saturation */
	//tsl.setGain(TSL2561_GAIN_16X);     /* 16x gain ... use in low light to boost sensitivity */
	//tsl.enableAutoGain(true);          /* Auto-gain ... switches automatically between 1x and 16x */
	
	/* Changing the integration time gives you better sensor resolution (402ms = 16-bit data) */
	tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_13MS);      /* fast but low resolution */
	//tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_101MS);  /* medium resolution and speed   */
	//tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_402MS);  /* 16-bit data but slowest conversions */
}

void setup() {
	//onboard LED, for problem reporting
	int blinky;
	blinky = ONBOARD_ERROR_LED;
	pinMode(blinky, OUTPUT);

	//pixels
	strip.begin();
	
	//serial port; useful only for debug output
	Serial.begin(9600);
	
	//light sensor
	if(!tsl.begin()) {
		// Give a visual indication with the onboard LED (pin 13, initialized above) if there is a probable wiring problem
		digitalWrite(blinky, HIGH);
		Serial.print("Ooops, no TSL2561 detected ... Check your wiring or I2C ADDR!");
		digitalWrite(blinky, LOW); 
		delay(10);
		while(1);
	}
	
	configureSensor();
	
	randomSeed(analogRead(RANDOM_ANALOG_PIN)); //analog read of an unconnected pin gives us good-enough entropy
 
}

void loop() {
	random_walk(MAXIMUM_DRIFT, TWEEN_CONSTANT);
}

uint8_t permute_color(uint8_t start_color, uint8_t max_change) {
	uint8_t rand;

	rand = random(1, max_change);

	//always wander away from overflow/underflow
	if(rand > start_color) {
		return start_color + rand;	
	} else if(255 - rand > start_color) {
		return start_color - rand;
	}
	if(random(1, 2) == 2) {
		return start_color - rand;
	} 
	return start_color + rand;
}
uint8_t calculate_tween(uint8_t start_color, uint8_t end_color, uint8_t this_step, uint8_t tween_constant) {
	float tweenfactor;
	float difference;
	uint8_t change;

	if(this_step == 0) return start_color;
	tweenfactor = ((float) this_step / (float) tween_constant);

	if(end_color > start_color) {
		difference = (float) end_color - (float) start_color;
		change = (uint8_t) (start_color +  difference*tweenfactor);
	} else if(start_color > end_color) {
		difference = (float) start_color - (float) end_color;
		change = (uint8_t) (start_color - difference*tweenfactor);
	} 

	return change;
}
void random_walk(uint8_t max_change, uint8_t tween) {
	uint16_t i, j;
	sensors_event_t event;
	uint8_t nextred[strip.numPixels()];
	uint8_t nextgreen[strip.numPixels()];
	uint8_t nextblue[strip.numPixels()];
	uint8_t lastred[strip.numPixels()];
	uint8_t lastgreen[strip.numPixels()];
	uint8_t lastblue[strip.numPixels()];

	//set colors separately from brightness
	for(j=0; ; j++) {
		if ((j % 5) == 0) {
			tsl.getEvent(&event);
			if(event.light) {
				lux = (int) ((double) (event.light) *(.15));
				Serial.print("Raw light reading: "); Serial.println(event.light);
				if(lux < MINIMUM_LUX) lux = MINIMUM_LUX;
				if(lux > 255) lux = 255;
	      			Serial.print("Read lux as "); Serial.println(lux);
	    		} else {
				//sensor is likely saturated; crank it
				lux = DEFAULT_BRIGHTNESS;
			}
		}
		strip.setBrightness(lux);

		if((j % tween) == 0) {
			//choose a new color for each pixel
			for(i=0; i<strip.numPixels(); i++) { 
				if(j == 0 && lastred[i] == 0 && lastgreen[i] == 0 && lastblue[i] == 0) {
					Serial.print("Randomizing initial value for pixel "); Serial.println(i);
					lastred[i] = random(0, 255);
					lastgreen[i] = random(0, 255);
					lastblue[i] = random(0, 255);
				} else {
					Serial.print("Shifting old value into previous array for pixel "); Serial.println(i);
					lastred[i] = nextred[i];
					lastgreen[i] = nextgreen[i];
					lastblue[i] = nextblue[i];
				}
			
				nextred[i] = permute_color(lastred[i], max_change);
				nextgreen[i] = permute_color(lastgreen[i], max_change);
				nextblue[i] = permute_color(lastblue[i], max_change);
			} 
		}
		for(i=0; i < strip.numPixels(); i++) { 
			uint8_t redtween;
			uint8_t greentween;
			uint8_t bluetween;

			//take tween cycles to go between lastred and nextred, etc
			redtween = calculate_tween(lastred[i], nextred[i], j%tween, tween);
			greentween = calculate_tween(lastgreen[i], nextgreen[i], j%tween, tween);
			bluetween = calculate_tween(lastblue[i], nextblue[i], j%tween, tween);

			#ifdef DEBUG_OUTPUT
			Serial.print("Setting color to "); Serial.print(redtween); Serial.print(", ");
			Serial.print(greentween); Serial.print(", ");
			Serial.print(bluetween); Serial.print(", ");
			Serial.print(" on pixel number "); Serial.print(i);
			Serial.println("");
			#endif

			strip.setPixelColor(i, redtween, greentween, bluetween);
	 	}
		strip.show();
		delay(10);
	}
}
