
/**
 * @file main.cpp
 * @author Tim Wannet
 * @brief This file contains the main program that will run on the teensy.
 * @version 0.1
 * 
 */
#include <Arduino.h>
#include <Audio.h>
#include <arm_math.h>  // CMSIS-DSP for FFT

#define FFT_SIZE 512  // STFT window size
#define OVERLAP 256   // Overlap size (50%)
#define SAMPLE_RATE 44100 // Audio sample rate (44.1 kHz)
#define HOP_SIZE (FFT_SIZE - OVERLAP)

// Audio input setup
// AudioInputI2S        audioInput; // Audio input object
AudioInputAnalog     modulatorInput(A17); // Audio input object
AudioInputI2S        carrierInput;
AudioRecordQueue     modulatorQueue;
AudioRecordQueue     carrierQueue;
AudioOutputI2S       audioOutput;
AudioControlSGTL5000 sgtl5000_1;

// AudioConnection      patchCord1(modulatorInput, modulatorQueue); // Connect audio input to queue
// AudioConnection      patchCord2(carrierInput, carrierQueue);

AudioConnection      patchCord1(modulatorInput, modulatorQueue); // Connect modulator input to queue
AudioConnection      patchCord2(carrierInput, carrierQueue);
// AudioConnection      patchCord3(carrierInput, 1, audioOutput, 0); // Monitor left line-in to output

bool MOD_FLAG = false;
bool CAR_FLAG = false;

// Buffers
float inputBuffer_modulator[FFT_SIZE]; // Modulator input buffer
float inputBuffer_carrier[FFT_SIZE]; // Carrier input buffer
float fftOutput_modulator[FFT_SIZE];
float fftOutput_carrier[FFT_SIZE];
float magnitude_modulator[FFT_SIZE / 2];  // Magnitude spectrum
float magnitude_carrier[FFT_SIZE / 2];  // Magnitude spectrum
float phase_modulator[FFT_SIZE / 2];
float phase_carrier[FFT_SIZE / 2];

float32_t hannWindow[FFT_SIZE]; // Hann window

/**
 * @brief Apply the Hann window to the input buffer.
 * 
 * This function applies the Hann window to the input buffer.
 * 
 * @param buffer The input buffer to apply the Hann window to.
 * @param size The size of the input buffer.
 */
void applyHannWindow(float32_t* buffer, int size) 
{
  for (int i = 0; i < size; i++) 
  {
      buffer[i] *= hannWindow[i];
  }
}

/**
 * @brief Compute the FFT of the input buffer.
 * 
 * This function computes the FFT of the input buffer and stores the result in the fftOutput_modulator buffer.
 * 
 * @param buffer The input buffer to compute the FFT on.
 */
void computeFFT(float32_t* buffer, float32_t* fftOutput)
{
  arm_rfft_fast_instance_f32 fftInstance;
  arm_rfft_fast_init_f32(&fftInstance, FFT_SIZE);
  arm_rfft_fast_f32(&fftInstance, buffer, fftOutput, 0);
}

/**
 * @brief Get the magnitude and phase of the FFT output.
 * 
 * This function computes the magnitude and phase of the FFT output and stores the result in the magnitude and phase buffers.
 * 
 * @param fftOutput The FFT output buffer.
 * @param magnitude The magnitude buffer.
 * @param phase The phase buffer.
 */
void getMagnitudeAndPhase(float32_t* fftOutput, float32_t* magnitude, float32_t* phase)
{
    for (int i = 0; i < FFT_SIZE / 2; i++)
    {
        float real = fftOutput[2 * i];
        float imag = fftOutput[2 * i + 1];
        magnitude[i] = sqrtf(real * real + imag * imag);
        phase[i] = atan2f(imag, real);  // Compute phase
    }
}

/**
 * @brief Setup function to initialize the audio objects and buffers.
 * 
 * This function initializes the audio memory and sets up the hann window.
 */
void setup()
{
  Serial.begin(115200);

  AudioMemory(8);

  sgtl5000_1.enable();
  sgtl5000_1.inputSelect(AUDIO_INPUT_LINEIN);  // Set carrier to Line-In
  sgtl5000_1.volume(0.8);  // Set output volume
  sgtl5000_1.lineInLevel(5); // Adjust line input gain

  modulatorQueue.begin();
  carrierQueue.begin();

//   while (modulatorQueue.available() < 4 || carrierQueue.available() < 4) {
//     Serial.print("Modulator Blocks: ");
//     Serial.print(modulatorQueue.available());
//     Serial.print(" | Carrier Blocks: ");
//     Serial.println(carrierQueue.available());
//     delay(1000);
// }


  // Generate Hann window
  for (int i = 0; i < FFT_SIZE; i++) 
  {
      hannWindow[i] = 0.5 * (1 - cosf(2 * PI * i / (FFT_SIZE - 1)));
  }


  Serial.println("Setup done");
}

/**
 * @brief Main loop function
 * 
 * This function reads the audio input samples, applies the Hann window, computes the FFT and prints the FFT magnitude.
 */
void loop()
{
  Serial.print("Modulator Blocks: ");
  Serial.println(modulatorQueue.available());
  Serial.print("Carrier Blocks: ");
  Serial.println(carrierQueue.available());
  delay(10);
  if (modulatorQueue.available() >= 4 && MOD_FLAG == false)  // Wait for 4 blocks
  {
    Serial.println("Reading Modulator");
    // delay(1000);
    int16_t *modulatorData;
    modulatorData = modulatorQueue.readBuffer();

    // Read Modulator samples
    for (int block = 0; block < 4; block++) 
    {
        Serial.print("Processing block: ");
        Serial.println(block);
        
        for (int i = 0; i < 128; i++) 
        {
            inputBuffer_modulator[block * 128 + i] = (float32_t)modulatorData[i] / 32768.0f;
        }
    }
    // modulatorQueue.freeBuffer();
    modulatorQueue.clear();
    MOD_FLAG = true;
    Serial.println("Finished reading Modulator");
  }

  if (carrierQueue.available() >= 4 && CAR_FLAG == false)  // Wait for 4 blocks
  {
    Serial.println("Reading Carrier");
    // delay(1000);
    int16_t *carrierData;
    carrierData = carrierQueue.readBuffer();

    // Read Carrier samples
    for (int block = 0; block < 4; block++)
    {
        Serial.print("Processing block: ");
        Serial.println(block);
        
        for (int i = 0; i < 128; i++)
        {
            inputBuffer_carrier[block * 128 + i] = carrierData[i] / 32768.0f;
        }
    }
    // carrierQueue.freeBuffer();
    carrierQueue.clear();
    CAR_FLAG = true;
    Serial.println("Finished reading Carrier");
  }

  if (modulatorQueue.available() >= 4 && MOD_FLAG == true && CAR_FLAG == false)  // Wait for 4 blocks
  {
    modulatorQueue.clear();
  }

  if (carrierQueue.available() >= 4 && CAR_FLAG == true && MOD_FLAG == false)  // Wait for 4 blocks
  {
    carrierQueue.clear();
  }

  if(MOD_FLAG && CAR_FLAG)
  {
    Serial.println("Processing FFT");
    // Apply window
    applyHannWindow(inputBuffer_modulator, FFT_SIZE);
    applyHannWindow(inputBuffer_carrier, FFT_SIZE);

    // Compute FFTs
    computeFFT(inputBuffer_modulator, fftOutput_modulator);
    computeFFT(inputBuffer_carrier, fftOutput_carrier);

    // Get Magnitude & Phase
    getMagnitudeAndPhase(fftOutput_modulator, magnitude_modulator, phase_modulator);
    getMagnitudeAndPhase(fftOutput_carrier, magnitude_carrier, phase_carrier);

    // Print Magnitude & Phase for Serial Plotter
    for (int i = 0; i < FFT_SIZE / 2; i++) 
    {
        Serial.print(magnitude_modulator[i]);  // Modulator Magnitude
        Serial.print(",");
        Serial.print(phase_modulator[i]);      // Modulator Phase
        Serial.print(",");
        Serial.print(magnitude_carrier[i]);    // Carrier Magnitude
        Serial.print(",");
        Serial.println(phase_carrier[i]);      // Carrier Phase
    }
    MOD_FLAG = false;
    CAR_FLAG = false;
  }
}