Task: ADC

1. Initialise the Atmega328p's native ADC on one of its many analogue inputs. Use the controller's own voltage as the reference voltage.
Write a function that (1) initialises any port as an ADC and (2) prints out the measured value periodically.

2. To test the function, we use a Potentiometer, which is an adjustable resistor.
Connect the left input of the potentiometer to the reference voltage. 
Ground goes to the potentiometer's right input. 
The Atmega's analogue output goes into the potentiometer's middle input.

3. Now measure the analog voltage with a frequency of 125KHz, using the ATMega's supply voltage as a reference voltage. Then convert the digital value into a voltage in millivolts and output it via the serial interface.

4. At the same time, measure the temperature of the ATMega and output this in degrees Celsius via the serial interface. 
