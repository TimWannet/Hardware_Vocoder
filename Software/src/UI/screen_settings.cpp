

#include "screen_settings.h"

void ScreenSettings::draw(ILI9488& tft) 
{
    if (selectedIndex == lastSelectedIndex)
        return;

    tft.fillScreen(ILI9488_BLACK);
    tft.setTextSize(1);

    // Clamp selectedIndex
    if (selectedIndex < 0) selectedIndex = 0;
    if (selectedIndex >= itemCount) selectedIndex = itemCount - 1;

    for (int i = 0; i < itemCount; i++) 
    {
        if (i == selectedIndex) 
        {
            tft.setTextColor(ILI9488_BLACK, ILI9488_WHITE);
        } else {
            tft.setTextColor(ILI9488_WHITE, ILI9488_BLACK);
        }
        tft.setCursor(0, i * 10 + 10);
        tft.println(menuItems[i]);
    }

    lastSelectedIndex = selectedIndex;
}

void ScreenSettings::handleInput(InputEvent input) 
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

    if (stateChanged)
    {
        requestRedraw();  // <- This tells the screenManager to redraw this screen
    }
}