/**
 * @file screen_base.h
 * @brief Base class for screen management
 *  
 * This file defines the ScreenBase class, which serves as a base class for all screens in the application.
 * It provides a common interface for drawing, updating, and handling input events.
 * 
 * @note This class is intended to be subclassed for specific screen implementations.
 * 
 * @author Tim Wannet
 * @date 20-05-2025
 * @version 0.01
*/

#ifndef SCREEN_BASE_H
#define SCREEN_BASE_H

#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

class ScreenBase 
{
    public:
        virtual void draw(Adafruit_ST7735& tft) = 0;
        virtual void update(Adafruit_ST7735& tft) {};
        virtual void handleInput(int input) {};
        virtual ~ScreenBase() {}
};

#endif // SCREEN_BASE_H