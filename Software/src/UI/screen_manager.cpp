/**
 * @file screen_manager.cpp
 * @brief Screen manager class
 * 
 * @details This file defines the ScreenManager class, which is responsible for managing different screens in the application.
 * It provides methods to set the current screen, draw the screen, update the screen, and handle input events.
 * 
 * @author Tim Wannet
 * @date 20-05-2025
 * @version 0.01
 */

#include "screen_manager.h"

ScreenManager::ScreenManager(ILI9488& display) : tft(display), currentScreen(nullptr) {}

void ScreenManager::setScreen(ScreenBase* screen) 
{
    currentScreen = screen;
    if (currentScreen) 
    {
        currentScreen->draw(tft);
        redraw = true;
    }
}

void ScreenManager::draw() 
{
    if (currentScreen && currentScreen->getRedrawRequest()) 
    {
        currentScreen->draw(tft);
        currentScreen->clearRedrawRequest();
    }
}

void ScreenManager::update()
{
    if (currentScreen) 
    {
        currentScreen->update(tft);
    }
}

void ScreenManager::handleInput(InputEvent input)
{
    if (currentScreen)
    {
        currentScreen->handleInput(input);
    }
}

bool ScreenManager::needsRedraw() 
{
    return redraw;
}
