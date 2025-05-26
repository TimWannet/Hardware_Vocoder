/**
 * @file screen_main_menu.cpp
 * @brief Main menu screen class
 * 
 * @details This file defines the ScreenMainMenu class, which is a subclass of ScreenBase.
 * It provides the implementation for drawing the main menu screen and handling input events.
 * 
 * @author Tim Wannet
 * @date 20-05-2025
 * @version 0.01
 */

#include "screen_main_menu.h"

void ScreenMainMenu::draw(Adafruit_ST7735& tft) 
{
    if (selectedIndex == lastSelectedIndex)
        return;

    tft.fillScreen(ST77XX_BLACK);
    tft.setTextSize(1);

    // Clamp selectedIndex
    if (selectedIndex < 0) selectedIndex = 0;
    if (selectedIndex >= itemCount) selectedIndex = itemCount - 1;

    for (int i = 0; i < itemCount; i++) {
        if (i == selectedIndex) {
            tft.setTextColor(ST77XX_BLACK, ST77XX_WHITE);
        } else {
            tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
        }
        tft.setCursor(0, i * 10 + 10);
        tft.println(menuItems[i]);
    }

    lastSelectedIndex = selectedIndex;
}

void ScreenMainMenu::handleInput(InputEvent input) 
{
    bool stateChanged = false;

    switch (input) 
    {
        case InputEvent::Left:
            if (selectedIndex >= 0) 
            {
                selectedIndex--;
                stateChanged = true;
            }
            if (selectedIndex < 0)
                selectedIndex = itemCount;
            break;

        case InputEvent::Right:
            if (selectedIndex < itemCount)
            {
                selectedIndex++;
                stateChanged = true;
            }
            if (selectedIndex >= itemCount)
                selectedIndex = 0;
            break;

        case InputEvent::Select:
            buttonState = 1;
            stateChanged = true;
            break;
    }

    if (stateChanged) {
        requestRedraw();  // <- This tells the screenManager to redraw this screen
    }
}