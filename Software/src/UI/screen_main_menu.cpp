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

void ScreenMainMenu::draw(Adafruit_ST7735& tft) {
    tft.fillScreen(ST77XX_BLACK);
    tft.setTextColor(ST77XX_WHITE);
    tft.setTextSize(1);
    tft.setCursor(0, 0);
    tft.println("Main Menu");
    tft.println("1. Start");
    tft.println("2. Settings");
}

void ScreenMainMenu::handleInput(int input) {
    //empty
    // Handle input events here
}