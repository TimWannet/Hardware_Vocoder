
/**
 * @file main.cpp
 * @author Tim Wannet
 * @brief This file contains the main program that will run on the teensy
 * 
 */

#include <Arduino.h>

/**
 * @brief Setup function to initialize the LED pin.
 * 
 * This function is called once when the program starts. It sets the built-in LED pin
 * (LED_BUILTIN) as an output.
 */
void setup()
{
  // initialize LED digital pin as an output.
  pinMode(LED_BUILTIN, OUTPUT);
}

/**
 * @brief Main loop function to toggle the LED.
 * 
 * This function is called repeatedly in an infinite loop. It turns the built-in LED on,
 * waits for one second, turns the LED off, and then waits for another second.
 */
void loop()
{
  // turn the LED on (HIGH is the voltage level)
  digitalWrite(LED_BUILTIN, HIGH);
  // wait for a second
  delay(1000); 
  // turn the LED off by making the voltage LOW
  digitalWrite(LED_BUILTIN, LOW);
   // wait for a second
  delay(1000);
} 

