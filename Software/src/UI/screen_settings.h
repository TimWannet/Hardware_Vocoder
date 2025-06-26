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

#ifndef SCREEN_SETTINGS_H
#define SCREEN_SETTINGS_H

#include "screen_base.h"
#include "input_manager.h"
#include "screen_manager.h"

#include "DSP/fft_utils.h"

class ScreenSettings : public ScreenBase 
{
    public:
        ScreenSettings(ScreenManager* screenManager);   
        void draw(ILI9488& tft) override;
        void handleInput(InputEvent input) override;

        void setMainMenuScreen(ScreenBase* screen);
        
    private:
        ScreenManager* screenManager = nullptr;
        ScreenBase* mainMenuScreen = nullptr;
        int selectedIndex = 0;
        int lastSelectedIndex = -1;
        bool isEditing = false;
        bool needsRedraw = true;
        static constexpr const char* menuItems[3] = {"Voiced Noise: %.2f", "Unvoiced Noise: %.2f", "Back"};
        static constexpr int itemCount = sizeof(menuItems) / sizeof(menuItems[0]);

};

#endif // SCREEN_SETTINGS_H