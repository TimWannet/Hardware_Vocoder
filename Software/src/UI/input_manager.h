/**
 * @file input_manager.h
 * @brief Input manager class
 * 
 * @details This file defines the InputManager class, which is responsible for handling input events from a rotary encoder and a button.
 * It provides methods to initialize the input pins, update the input state, and set callbacks for input events.
 * 
 * @note This class is intended to be used with the Encoder library for rotary encoders.
 * 
 * @author Tim Wannet
 * @date 20-05-2025
 * @version 0.01
 */

#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

#include "Encoder.h"
#include "functional"

enum class InputEvent 
{
  Left,
  Right,
  Select
};

class InputManager 
{
    public:
        InputManager(uint8_t pinA, uint8_t pinB, uint8_t pinButton);
        void begin();
        void update();

        // Callback setter
        void onInput(std::function<void(InputEvent)> callback);

    private:
        Encoder encoder;
        uint8_t buttonPin;
        long lastPosition = 0;
        bool lastButtonState = HIGH;

    std::function<void(InputEvent)> inputCallback;
};

#endif // INPUT_MANAGER_H