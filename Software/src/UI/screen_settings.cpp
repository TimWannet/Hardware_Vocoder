

#include "screen_settings.h"

ScreenSettings::ScreenSettings(ScreenManager* manager) : screenManager(manager) {}

void ScreenSettings::setMainMenuScreen(ScreenBase* screen) 
{
    mainMenuScreen = screen;
}

void ScreenSettings::draw(ILI9488& tft) 
{
    if (selectedIndex == lastSelectedIndex && !isEditing)
        return;

    tft.fillScreen(ILI9488_BLACK);
    tft.setTextSize(1);

    // Clamp selectedIndex
    if (selectedIndex < 0) selectedIndex = 0;
    if (selectedIndex >= itemCount) selectedIndex = itemCount - 1;

   for (int i = 0; i < itemCount; i++) 
    {
        if (i == selectedIndex) 
            tft.setTextColor(ILI9488_BLACK, ILI9488_WHITE);
        else
            tft.setTextColor(ILI9488_WHITE, ILI9488_BLACK);

        tft.setCursor(0, i * 10 + 10);

        char buffer[40];

        if (i == 0)
        {    
            snprintf(buffer, sizeof(buffer), "%sVoiced: %.2f", (isEditing && i == selectedIndex) ? "[*] " : "", voicedNoiseStrength);
        } 
        
        else if (i == 1)
        {
            snprintf(buffer, sizeof(buffer), "%sUnvoiced: %.2f", (isEditing && i == selectedIndex) ? "[*] " : "", unvoicedNoiseStrength);
        }
        
        else if (i == 2) 
        {
            snprintf(buffer, sizeof(buffer), "Back");
        }

        tft.println(buffer);
    }
    lastSelectedIndex = selectedIndex;
}

void ScreenSettings::handleInput(InputEvent input) 
{
    // bool stateChanged = false;

    // switch (input) 
    // {
    //     case InputEvent::Left:
    //         stateChanged = true;
    //         if (selectedIndex >= 0)
    //         {
    //             selectedIndex--;    
    //         }
    //         if (selectedIndex < 0)
    //             selectedIndex = itemCount;
    //         break;

    //     case InputEvent::Right:
    //         stateChanged = true;
    //         if (selectedIndex < itemCount)
    //         {
    //             selectedIndex++;
    //         }
    //         if (selectedIndex >= itemCount)
    //             selectedIndex = 0;
    //         break;

    //     case InputEvent::Select:
    //     stateChanged = true;     
    //         switch(selectedIndex)
    //         {                
    //             case 0:
    //                 voicedNoiseStrength += 0.1f;
    //                 if (voicedNoiseStrength > 1.0f)
    //                     voicedNoiseStrength = 0.0f;
    //             break;
                
    //             case 1:
    //                 unvoicedNoiseStrength += 0.1f;
    //                 if (unvoicedNoiseStrength > 1.0f)
    //                     unvoicedNoiseStrength = 0.0f;
    //             break;

    //             case 2:
    //                     screenManager->setScreen(mainMenuScreen);
    //                     selectedIndex = 0;  
    //             break;
    //         }
    //         break;
    // }

    // if (stateChanged)
    // {
    //     requestRedraw();  // <- This tells the screenManager to redraw this screen
    // }

     bool stateChanged = false;

    switch (input) 
    {
        case InputEvent::Left:
        case InputEvent::Right:
            stateChanged = true;

            if (isEditing)
            {
                // We are editing a value
                float delta = (input == InputEvent::Right) ? 0.05f : -0.05f;

                switch (selectedIndex)
                {

                    case 0:
                        voicedNoiseStrength += delta;
                        if (voicedNoiseStrength < 0.0f) voicedNoiseStrength = 0.0f;
                        if (voicedNoiseStrength > 1.0f) voicedNoiseStrength = 1.0f;
                        
                        break;
                    case 1:
                        unvoicedNoiseStrength += delta;
                        if (unvoicedNoiseStrength < 0.0f) unvoicedNoiseStrength = 0.0f;
                        if (unvoicedNoiseStrength > 1.0f) unvoicedNoiseStrength = 1.0f;
                        break;
                }
            }
            else
            {
                // We're navigating, not editing
                if (input == InputEvent::Right)
                    selectedIndex = (selectedIndex + 1) % itemCount;
                else
                    selectedIndex = (selectedIndex - 1 + itemCount) % itemCount;
            }
            break;

        case InputEvent::Select:
            stateChanged = true;

            if (selectedIndex == 2)
            {
                if (!isEditing)
                {
                    // "Back" action
                    screenManager->setScreen(mainMenuScreen);
                    selectedIndex = 0; 
                    return;
                }
            }
            else
            {
                // Toggle editing mode for value fields
                isEditing = !isEditing;
            }
            break;
    }

    if (stateChanged) 
        requestRedraw();
}