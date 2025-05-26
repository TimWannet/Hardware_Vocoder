/**
 * @file input_manager.cpp
 * @brief Input manager class
 * 
 * @details This file defines the InputManager class, which is responsible for handling input events from a rotary encoder and a button.
 * It provides methods to initialize the input pins, update the input state, and set callbacks for input events.
 * 
 * @author Tim Wannet
 * @date 20-05-2025
 * @version 0.01
 */

#include "input_manager.h"
// #include "Arduino.h"

InputManager::InputManager(uint8_t pinA, uint8_t pinB, uint8_t pinButton): encoder(pinA, pinB), buttonPin(pinButton) {}

void InputManager::begin()
{
  pinMode(buttonPin, INPUT_PULLUP);
  lastPosition = encoder.read();
  lastButtonState = digitalRead(buttonPin);
}

void InputManager::onInput(std::function<void(InputEvent)> callback)
{
  inputCallback = callback;
}

void InputManager::update()
{
  // --- Rotary encoder movement ---
  long newPosition = encoder.read()/4;
  if (newPosition != lastPosition) 
  {
    if (inputCallback)
    {
      inputCallback(newPosition > lastPosition ? InputEvent::Right : InputEvent::Left);
    }
    lastPosition = newPosition;
  }

  // --- Button press ---
  bool currentState = digitalRead(buttonPin);
  if (lastButtonState == HIGH && currentState == LOW) 
  {
    if (inputCallback) 
    {
        inputCallback(InputEvent::Select);
    }
}
  lastButtonState = currentState;
}
