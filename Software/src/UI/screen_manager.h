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

#include "screen_base.h"

class ScreenManager 
{
    public:
        ScreenManager(Adafruit_ST7735& display);
        void setScreen(ScreenBase* screen);
        void draw();
        void update();
        void handleInput(int input);

    private:
        Adafruit_ST7735& tft;
        ScreenBase* currentScreen;
};