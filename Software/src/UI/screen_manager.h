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