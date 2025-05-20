/**
 * @file screen_main_menu.h
 * @brief Main menu screen class
 * 
 * @details This file defines the ScreenMainMenu class, which is a subclass of ScreenBase.
 * It provides the implementation for drawing the main menu screen and handling input events.
 * 
 * @note This class is intended to be used with the Adafruit ST7735 display library.
 * 
 * @author Tim Wannet
 * @date 20-05-2025
 * @version 0.01
*/

#include "screen_base.h"

class ScreenMainMenu : public ScreenBase 
{
    public:
        void draw(Adafruit_ST7735& tft) override;
        void handleInput(int input) override;
};