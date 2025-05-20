#include "screen_manager.h"

ScreenManager::ScreenManager(Adafruit_ST7735& display) : tft(display), currentScreen(nullptr) {}

void ScreenManager::setScreen(ScreenBase* screen) 
{
    currentScreen = screen;
    if (currentScreen) 
    {
        currentScreen->draw(tft);
    }
}

void ScreenManager::draw() 
{
    if (currentScreen) 
    {
        currentScreen->draw(tft);
    }
}

void ScreenManager::update() 
{
    if (currentScreen) 
    {
        currentScreen->update(tft);
    }
}

void ScreenManager::handleInput(int input) 
{
    if (currentScreen) 
    {
        currentScreen->handleInput(input);
    }
}