glow-cloud
==========

Make pretty colors with Arduino and NeoPixels.

Software Requirements
------------

The project depends on headers for the Adafruit NeoPixel, available at 
[the @adafruit GitHub repository](https://github.com/adafruit/Adafruit_NeoPixel), 
and (for light sensing) the Adafruit TSL2561 library 
[also provided by @adafruit](https://github.com/adafruit/Adafruit_TSL2561).

Hardware Requirements
---------------------

This code was written for the Adafruit FLORA wearable Arduino, but it should work on 
any Arduino capable of addressing the NeoPixel PCBs - see 
[the Adafruit docuemntation](https://learn.adafruit.com/adafruit-neopixel-uberguide/arduino-library) 
for up-to-date information.

This code expects NeoPixels to be connected on pin 6.  If a different output pin is desired,
simply change the definition of PIN at the top of `coruscate.ino`.

If brightness sensing is desired, a [TSL2561](http://www.adafruit.com/products/1246) module should 
be connected to the I2C SCL and SDA pins; see your Arduino documentation for the locations of these pins.
If no TSL2561 is detected, `coruscate.ino` will set the NeoPixel brightness to `DEFAULT_BRIGHTNESS` .

Tunable Constants
-----------------

`PIN` is the digital output to which the strip of NeoPixels is connected.  Set this according to your physical hardware setup.

`ONBOARD_ERROR_LED` will blink if there is a problem detecting the TSL2561.  Set this to an unused pin if TSL2561 
is not in use or debug output is not desired.

`RANDOM_SEED_PIN` will be read on startup to seed the random number generator.  Any ungrounded analog pin will 
work well.

`MINIMUM_LUX` sets a floor on the brightness of the NeoPixel LEDs - even if very little ambient light is detected, 
the LEDs will always be at least `MINIMUM_LUX` bright.

`DEFAULT_BRIGHTNESS` sets the brightness for NeoPixels if there is no TSL2561, or the TSL2561 can't be read.  
(Failure to read the TSL2561 often results from either total darkness or complete saturation, situations 
which are unfortunately indistinguishable; I chose to handle this by setting `DEFAULT_BRIGHTNESS` relatively high.)
