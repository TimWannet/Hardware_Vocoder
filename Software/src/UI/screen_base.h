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

#include "Adafruit_GFX.h"
// #include "Adafruit_ST7735.h"
#include <ILI9488.h>
// #include <TFT_eSPI.h>
#include <SPI.h>

#include "UI/input_manager.h"

class ScreenBase 
{
    public:
        virtual void draw(ILI9488& tft) = 0;
        virtual void update(ILI9488& tft) {};
        virtual void handleInput(InputEvent input) {};
        virtual ~ScreenBase() {}

        bool getRedrawRequest() const { return redrawRequested; }
        void clearRedrawRequest() { redrawRequested = false; }
        void requestRedraw() { redrawRequested = true; }

    protected:
        bool redrawRequested = true;
};

#endif // SCREEN_BASE_H