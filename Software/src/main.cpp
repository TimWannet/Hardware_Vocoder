
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

//defines
#define FFT_SIZE 128  // STFT window size
#define NUM_BLOCKS 1  // Number of blocks to process
#define AUDIO_MEMORY 200  // Audio memory allocation

// Audio objects
AudioInputAnalog     modulatorInput(A17); // Audio input object
AudioInputI2S        carrierInput;
AudioRecordQueue     modulatorQueue;
AudioRecordQueue     carrierQueue;
AudioOutputI2S       audioOutput;
AudioPlayQueue       queueOutput;
AudioControlSGTL5000 sgtl5000_1;

// Audio connections/patching
AudioConnection      patchCord1(modulatorInput, modulatorQueue);
AudioConnection      patchCord2(carrierInput, carrierQueue); 
AudioConnection      patchCord3(queueOutput, 0, audioOutput, 0);  // Left channel
AudioConnection      patchCord4(queueOutput, 0, audioOutput, 1); // Right channel

bool MOD_FLAG  = false;
bool CAR_FLAG  = false;
// bool Test_MODE = true;
bool Test_MODE = false;

// Buffers
float inputBuffer_modulator[FFT_SIZE]; // Time-domain samples (modulator)
float inputBuffer_carrier[FFT_SIZE];   // Time-domain samples (carrier)
float Buffer_carrier[FFT_SIZE];        // Buffer for carrier signal

// FFT output buffers (use FFT_SIZE + 2 for real-to-complex FFT)
float fftOutput_modulator[FFT_SIZE+2];  
float fftOutput_carrier[FFT_SIZE+2];   

// Magnitude and phase arrays (only half of FFT bins are useful)
float magnitude_modulator[FFT_SIZE/2];  
float magnitude_carrier[FFT_SIZE/2];    
float phase_modulator[FFT_SIZE/2];      
float phase_carrier[FFT_SIZE/2];        

float32_t hannWindow[FFT_SIZE]; // Hann window

/**
 * @brief This function applies the Hann window to the input buffer.
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
 * @param buffer The input buffer to compute the FFT on.
 * @param fftOutput The output buffer to store the FFT result.
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
        phase[i] = atan2f(imag, real);
    }
}

/**
 * @brief For now only the carrier signal is being reconstructed by the IFFT and sent to the audio queue.
 * 
 * Here the audio queue send to the audio output of the SGTL5000.
 */
void applyVocoderEffect() 
{
  for (int i = 0; i < FFT_SIZE / 2; i++) 
  {
    fftOutput_carrier[2 * i] = magnitude_carrier[i] * cos(phase_carrier[i]);  // Real
    fftOutput_carrier[2 * i + 1] = magnitude_carrier[i] * sin(phase_carrier[i]);  // Imaginary
  }
  
  // Perform Inverse FFT (IFFT) to reconstruct signal
  arm_rfft_fast_instance_f32 ifftInstance;
  arm_rfft_fast_init_f32(&ifftInstance, FFT_SIZE);
  arm_rfft_fast_f32(&ifftInstance, fftOutput_carrier, Buffer_carrier, 1);  // 1 = IFFT

  // Convert FFT output back to int16_t and send to Audio Queue
  if (queueOutput.available() >= 1)
  {
      int16_t *buffer = queueOutput.getBuffer();
      for (int i = 0; i < AUDIO_BLOCK_SAMPLES; i++) //FFT size?????????
      {
          // Scale the floating-point IFFT output to int16_t range
          int16_t sample = Buffer_carrier[i] * 32767.0f;
  
          // Clamp to avoid overflow distortion
          if (sample > 32767.0f) sample = 32767.0f;
          if (sample < -32768.0f) sample = -32768.0f;
  
          buffer[i] = sample;
      }
      queueOutput.playBuffer();
  }
}

/**
 * @brief Setup function to initialize the audio objects and buffers.
 */
void setup()
{
  Serial.begin(115200); 

  AudioMemory(AUDIO_MEMORY);  

  // Set up the audio shield
  sgtl5000_1.enable();
  sgtl5000_1.inputSelect(AUDIO_INPUT_LINEIN);  // Set carrier to Line-In
  sgtl5000_1.volume(0.5);  // Set output volume
  sgtl5000_1.lineInLevel(5); // Adjust line input gain

  modulatorQueue.begin(); // Start the audio queue
  carrierQueue.begin();  // Start the audio queue

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
 */
void loop()
{
  if(Test_MODE)
  {
    Serial.print("Modulator Blocks: ");
    Serial.println(modulatorQueue.available());
    Serial.print("Carrier Blocks: ");
    Serial.println(carrierQueue.available());
  }

  if (modulatorQueue.available() >= NUM_BLOCKS && MOD_FLAG == false) //Modulator
  {
    if(Test_MODE)
      Serial.println("Reading Modulator");

    int16_t *modulatorData;
    modulatorData = modulatorQueue.readBuffer();

    // Read Modulator samples
    for (int block = 0; block < NUM_BLOCKS; block++) 
    {
        if(Test_MODE)
        {
          Serial.print("Processing block: ");
          Serial.println(block);
        }
        for (int i = 0; i < 128; i++) 
        {
            inputBuffer_modulator[block * 128 + i] = (float32_t)modulatorData[i] / 32768.0f;
        }
    }
    modulatorQueue.clear();
    MOD_FLAG = true;

    if(Test_MODE)
      Serial.println("Finished reading Modulator");
  }

  if (carrierQueue.available() >= NUM_BLOCKS && CAR_FLAG == false)  //Carrier
  {
    if(Test_MODE)
      Serial.println("Reading Carrier");

    int16_t *carrierData;
    carrierData = carrierQueue.readBuffer();

    // Read Carrier samples
    for (int block = 0; block < NUM_BLOCKS; block++)
    {
        if(Test_MODE)
        {
          Serial.print("Processing block: ");
          Serial.println(block);
        }

        for (int i = 0; i < 128; i++)
        {
            inputBuffer_carrier[block * 128 + i] = carrierData[i] / 32768.0f;
        }
    }
    carrierQueue.clear();
    CAR_FLAG = true;
    if(Test_MODE)
      Serial.println("Finished reading Carrier");
  }

  // Clear the queue if not enough blocks are available
  if (modulatorQueue.available() >= NUM_BLOCKS && MOD_FLAG == true && CAR_FLAG == false)  
  {
    modulatorQueue.clear();
  }

  if (carrierQueue.available() >= NUM_BLOCKS && CAR_FLAG == true && MOD_FLAG == false)
  {
    carrierQueue.clear();
  }

  // Process FFT if both modulator and carrier blocks are filled
  if(MOD_FLAG && CAR_FLAG)
  {
    if(Test_MODE)
      Serial.println("Processing FFT");

    // Apply window THIS IS NOT WORKING!!!!
    // applyHannWindow(inputBuffer_modulator, FFT_SIZE);
    // applyHannWindow(inputBuffer_carrier, FFT_SIZE);

    // Compute FFTs
    computeFFT(inputBuffer_modulator, fftOutput_modulator);
    computeFFT(inputBuffer_carrier, fftOutput_carrier);

    // Get Magnitude & Phase
    getMagnitudeAndPhase(fftOutput_modulator, magnitude_modulator, phase_modulator);
    getMagnitudeAndPhase(fftOutput_carrier, magnitude_carrier, phase_carrier);

    // Print Magnitude & Phase for Serial Plotter
    if(Test_MODE)
    {
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
    }

    // Apply Vocoder Effect
    applyVocoderEffect();

    // Reset flags
    MOD_FLAG = false;
    CAR_FLAG = false;
  }
}