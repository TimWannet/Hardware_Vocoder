
/**
 * @file main.cpp
 * @brief Hardware Vocoder Software Implementation
 * 
 * @details This program implements a hardware vocoder using the Teensy Audio Library. 
 * It processes audio signals from a carrier input and a modulator input, performs 
 * Fast Fourier Transform (FFT) analysis, and reconstructs the signal for playback.
 * 
 * The program is structured into three main processing classes:
 * - CarrierBufferProcessor: Handles audio data from the carrier input.
 * - ModulatorProcessor: Handles audio data from the modulator input.
 * - PlaybackProcessor: Handles audio data playback after processing.
 * 
 * The FFT is performed using the CMSIS-DSP library, which provides optimized 
 * FFT routines for ARM Cortex-M processors. The program extracts magnitude and 
 * phase information from the frequency domain, reconstructs the signal, and 
 * performs an inverse FFT to return to the time domain.
 * 
 * @note The FFT size can be adjusted by modifying the `FFT_SIZE` macro. Supported 
 * sizes include 128, 256, 512, 1024, 2048, and 4096.
 * 
 * @author Tim Wannet
 * @date 26-03-2025
 * @version 0.02
 */

#include "DSP/utils.h"
#include "DSP/fft_utils.h"
#include "DSP/audio_stream_classes.h"

#include "UI/input_manager.h"
#include "UI/screen_manager.h"
#include "UI/screen_main_menu.h"

// defines/constants
#define TFT_RST   28
#define TFT_DC    29
#define TFT_CS    30
#define TFT_BLK   31  
#define TFT_MOSI  26
#define TFT_CLK   27
#define ENCODER_BUTTON 35
#define ENCODER_PIN_A 33
#define ENCODER_PIN_B 34
// #define SPI_CLOCK 24000000

const int FFT_SIZE = 1024; // Buffer size (Change this value as needed: 128, 256, 512, 1024, 2048, etc.)
const arm_cfft_instance_f32* fftConfig;

// Audio Library objects
AudioInputI2S         i2sInput;  // I2S input from Audio Shield
AudioInputAnalog      analogInput(A17); // Analog input as modulator
AudioOutputI2S        i2sOutput; // I2S output to Audio Shield
AudioControlSGTL5000  sgtl5000_1;

// variables
int16_t carrierBuffer[FFT_SIZE];
int16_t modulatorBuffer[FFT_SIZE];
int16_t fftFloatBuffer[FFT_SIZE];

float fftBuffer[FFT_SIZE * 2];
float carrierFloatBuffer[FFT_SIZE * 2];
float modulatorFloatBuffer[FFT_SIZE * 2];
float modulatorFFT[FFT_SIZE * 2];
float modulatorMagnitude[FFT_SIZE];
float carrierMagnitude[FFT_SIZE];
float smoothedModulator[FFT_SIZE] = {0}; // Initialize smoothedModulator with zeros
float modulatorPhase[FFT_SIZE];
float carrierPhase[FFT_SIZE];

volatile bool carrierBufferFull = false;
volatile bool modulatorBufferFull = false;
volatile bool playbackReady = false;


//Constructors
ILI9488 tft = ILI9488(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, -1);
// Adafruit_ST7735 tft = Adafruit_ST7735(&SPI1, TFT_CS, TFT_DC, TFT_RST);
ScreenManager* screenManager;
ScreenMainMenu mainMenu;
InputManager inputManager(ENCODER_PIN_A, ENCODER_PIN_B, ENCODER_BUTTON);

// Audio Library objects/patch connections
CarrierBufferProcessor  carrierProcessor;
ModulatorProcessor      modulatorProcessor;
PlaybackProcessor       playbackProcessor;
AudioConnection         patchCord1(i2sInput, 0, carrierProcessor, 0); 
AudioConnection         patchCord2(analogInput, 0, modulatorProcessor, 0); 
AudioConnection         patchCord3(playbackProcessor, 0, i2sOutput, 0); // left channel
AudioConnection         patchCord4(playbackProcessor, 0, i2sOutput, 1); // right channel

 
/*
* @brief Setup function
*
* @details This function initializes the audio library and the SGTL5000 audio codec.
*/
void setup()
{
    Serial.begin(115200);

    AudioMemory(30);
    sgtl5000_1.enable();
    sgtl5000_1.inputSelect(AUDIO_INPUT_LINEIN);
    sgtl5000_1.volume(0.7);

    fftConfig = getFFTConfig(FFT_SIZE);
    if (!fftConfig)
    {
        Serial.println("Invalid FFT size!");
        return;
    }
    
    

    inputManager.begin();

    // Connect input events to screen manager
    inputManager.onInput([](InputEvent event) {
        screenManager->handleInput(event);
    });

    pinMode(TFT_BLK, OUTPUT);
    digitalWrite(TFT_BLK, LOW); 
    delay(2000); 
    digitalWrite(TFT_BLK, HIGH);  // Turn backlight ON

    // Initialize the TFT display
    tft.begin();
    tft.setRotation(3);
    tft.fillScreen(ILI9488_BLACK);

    screenManager = new ScreenManager(tft);
    screenManager->setScreen(&mainMenu);

    Serial.println("Setup complete");


}

/*
* @brief Loop function
*
* @details This function is the main loop of the program.
* It checks if the buffer is full and processes the FFT when the buffer is full.
*/
void loop() 
{
    inputManager.update();  // Reads encoder, may set needsRedraw flag
    screenManager->update(); // Optional if doing per-screen updates

    screenManager->draw();  // Only draw when something changed

    if (carrierBufferFull == true && modulatorBufferFull == true)
    {
        for (int i = 0; i < FFT_SIZE; i++) 
        {
            modulatorBuffer[i] = highpass(modulatorBuffer[i]);
        }

        

        // Convert int16_t to float
        convertInt16ToFloat(carrierBuffer, carrierFloatBuffer);
        convertInt16ToFloat(modulatorBuffer, modulatorFloatBuffer);

        processFFT(carrierFloatBuffer, carrierMagnitude, carrierPhase);
        processFFT(modulatorFloatBuffer, modulatorMagnitude, modulatorPhase);

        inverseFFT(fftBuffer, carrierMagnitude, carrierPhase, modulatorMagnitude, modulatorPhase);
        
        convertFloatToInt16(fftBuffer, fftFloatBuffer);

        carrierBufferFull = false;
        modulatorBufferFull = false;
        playbackReady = true;
    }
}