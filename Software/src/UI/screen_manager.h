/**
 * @file screen_manager.h
 * @brief Screen manager class
 * 
 * @details This file defines the ScreenManager class, which is responsible for managing different screens in the application.
 * It provides methods to set the current screen, draw the screen, update the screen, and handle input events.
 * 
 * @note This class is intended to be used with the Adafruit ST7735 display library.
 * 
 * @author Tim Wannet
 * @date 20-05-2025
 * @version 0.01
 */

#ifndef SCREEN_MANAGER_H
#define SCREEN_MANAGER_H

#include "screen_base.h"
#include "UI/input_manager.h"

class ScreenManager 
{
    public:
        ScreenManager(ILI9488& display);
        void setScreen(ScreenBase* screen);
        void draw();
        void update();
        void handleInput(InputEvent input);
        bool needsRedraw();

    private:
        ILI9488& tft;
        ScreenBase* currentScreen;
        bool redraw = true; 
};

#endif // SCREEN_MANAGER_H