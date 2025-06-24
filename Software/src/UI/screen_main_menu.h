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
#include "UI/input_manager.h"
#include "UI/screen_manager.h"

class ScreenMainMenu : public ScreenBase 
{
    public:
        void draw(ILI9488& tft) override;
        void handleInput(InputEvent input) override;
        

    private:
        ScreenManager* screenManager;
        int selectedIndex = 0;
        int lastSelectedIndex = -1;
        int buttonState = 0;
        bool needsRedraw = true;
        static constexpr const char* menuItems[2] = {"1. Start", "2. Settings"};
        static constexpr int itemCount = sizeof(menuItems) / sizeof(menuItems[0]);
};