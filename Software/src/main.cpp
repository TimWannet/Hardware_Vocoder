
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
AudioInputAnalog     audioInput(A2); // Audio input object
AudioRecordQueue     recordQueue;
AudioConnection      patchCord1(audioInput, 0, recordQueue, 0); // Connect audio input to FFT

// Buffers
float inputBuffer_modulator[FFT_SIZE]; // Modulator input buffer
float fftOutput_modulator[FFT_SIZE];
float magnitude_modulator[FFT_SIZE / 2];  // Magnitude spectrum
float phase_modulator[FFT_SIZE / 2];

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
void computeFFT(float32_t* buffer) 
{
  arm_rfft_fast_instance_f32 fftInstance;
  arm_rfft_fast_init_f32(&fftInstance, FFT_SIZE);
  arm_rfft_fast_f32(&fftInstance, buffer, fftOutput_modulator, 0);
}

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

  AudioMemory(12);
  recordQueue.begin();

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
  // Serial.println("Waiting...");
  // Serial.println(recordQueue.available());
  if (recordQueue.available() >= 4) 
  {
    // Get raw audio samples
    Serial.println("Getting audio samples...");
    int16_t *audioData = recordQueue.readBuffer();

    // Convert to float
    for (int i = 0; i < FFT_SIZE; i++) 
    {
        inputBuffer_modulator[i] = (float32_t)audioData[i] / 32768.0f;  // Normalize to -1.0 to 1.0
    }
    recordQueue.freeBuffer();

    // Apply window function
    applyHannWindow(inputBuffer_modulator, FFT_SIZE);

    // Compute FFT
    computeFFT(inputBuffer_modulator);

    // Get magnitude spectrum
    getMagnitudeAndPhase(fftOutput_modulator, magnitude_modulator, phase_modulator);
    
    // Print magnitude spectrum
    for (int i = 0; i < FFT_SIZE / 2; i++)
    {
      Serial.print(magnitude_modulator[i]);
      Serial.print(" ");
    }
    Serial.println();
  }
}