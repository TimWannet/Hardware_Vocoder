
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
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <arm_math.h>
#include <cmath>
#include <arm_const_structs.h>

// defines/constants
const int FFT_SIZE = 256; // Buffer size (Change this value as needed: 128, 256, 512, 1024, etc.)
const arm_cfft_instance_f32* fftConfig;

// Audio Library objects
AudioInputI2S         i2sInput;  // I2S input from Audio Shield
AudioInputAnalog      analogInput(A17); // Analog input as modulator
AudioOutputI2S        i2sOutput; // I2S output to Audio Shield
AudioControlSGTL5000  sgtl5000_1;

// variables
int16_t carrierBuffer[FFT_SIZE];
int16_t modulatorBuffer[FFT_SIZE];

float fftBuffer[FFT_SIZE * 2];
float carrierFloatBuffer[FFT_SIZE * 2];
float modulatorFloatBuffer[FFT_SIZE * 2];
float modulatorFFT[FFT_SIZE * 2];
float modulatorMagnitude[FFT_SIZE];
float carrierMagnitude[FFT_SIZE];
float modulatorPhase[FFT_SIZE];
float carrierPhase[FFT_SIZE];

volatile bool carrierBufferFull = false;
volatile bool modulatorBufferFull = false;
volatile bool playbackReady = false;



/*
* @class CarrierBufferProcessor
* @brief Processes audio data from I2S input and stores it in a buffer

* @details This class processes audio data from the I2S input, this is handled in a interrupt service routine.
* The audio data is stored in a buffer and when the buffer is full, a flag is set to signal that processing can start.
*/
class CarrierBufferProcessor : public AudioStream 
{
  public:
      CarrierBufferProcessor() : AudioStream(1, inputQueueArray) {} 

      //override base::update()
      void update() override
      {
          audio_block_t *block;
          block = receiveReadOnly(0); // Receive audio data from I2S input
          if (!block)
            return;

          static uint16_t index = 0;
          for (int i = 0; i < AUDIO_BLOCK_SAMPLES && index < FFT_SIZE; i++)
          {
              carrierBuffer[index++] = block->data[i]; // Store audio data in buffer
              if (index >= FFT_SIZE) // Buffer full
              {
                  carrierBufferFull = true; // Signal that processing can start
                  index = 0;
              }
          }
          release(block);
      }

  private:
      audio_block_t *inputQueueArray[1]; 
};

/*
* @class ModulatorProcessor
* @brief Processes audio data from the modulator and stores it in a buffer
* 
* @details This class processes audio data from the modulator, this is handled in a interrupt service routine.
* The audio data is stored in a buffer and when the buffer is full, a flag is set to signal that processing can start.
*/
class ModulatorProcessor : public AudioStream 
{
    public:
        ModulatorProcessor() : AudioStream(1, inputQueueArray) {}

        void update() override
        {
            audio_block_t *block;
            block = receiveReadOnly(0);
            if (!block)
                return;

            static uint16_t index = 0;
            for (int i = 0; i < AUDIO_BLOCK_SAMPLES && index < FFT_SIZE; i++)
            {
                modulatorBuffer[index++] = block->data[i]; //32767
            }

            if (index >= FFT_SIZE) // Buffer full
            {
                modulatorBufferFull = true; // Signal that processing can start
                index = 0;
            }
            release(block);
        }

    private:
        audio_block_t *inputQueueArray[1];
};

/*
* @class PlaybackProcessor
* @brief Processes audio data from a buffer and plays it back
*
* @details This class processes audio data from the buffer and plays it back using the I2S output.
* This is handled in a interrupt service routine and the audio data is played back in chunks of 128 samples (according to the Audio Library).
*/
class PlaybackProcessor : public AudioStream 
{
  public:
      PlaybackProcessor() : AudioStream(0, NULL) {}

      //override base::update()
      void update() override  
      {
          if (!playbackReady) 
            return;
          
          audio_block_t *block = allocate(); 
          
          if (!block) 
            return;

          static uint16_t index = 0;

          for (int i = 0; i < AUDIO_BLOCK_SAMPLES; i++) 
          {
              block->data[i] = carrierBuffer[index++];
              if (index >= FFT_SIZE)
              {
                  index = 0;
                  playbackReady = false;
              }
          }
          transmit(block);
          release(block);
      }
};

// Audio Library objects/patch connections
CarrierBufferProcessor  carrierProcessor;
ModulatorProcessor      modulatorProcessor;
PlaybackProcessor       playbackProcessor;
AudioConnection         patchCord1(i2sInput, 0, carrierProcessor, 0); 
AudioConnection         patchCord2(analogInput, 0, modulatorProcessor, 0); 
AudioConnection         patchCord3(playbackProcessor, 0, i2sOutput, 0); // left channel
AudioConnection         patchCord4(playbackProcessor, 0, i2sOutput, 1); // right channel

// Function prototypes
void processFFT(arm_cfft_instance_f32* fftConfig, float *floatBuffer, float *magnitude, float *phase);
void getMagnitudeAndPhase(float *buffer, float *magnitude, float *phase);
void convertInt16ToFloat(int16_t *inputBuffer, float *outputBuffer);
void convertFloatToInt16(float *inputBuffer, int16_t *outputBuffer);
void inverseFFT(float *buffer, float *carrierMagnitude, float *carrierPhase, float *modulatorMagnitude, float *modulatorPhase);
const arm_cfft_instance_f32* getFFTConfig(int size);

/*
* @brief Convert int16_t to float function
*
* @param[in] inputBuffer    The input audio data buffer in int16_t format
* @param[out] outputBuffer  The output audio data buffer in float format
*
* @details This function converts the audio data from int16_t to float.
* The real part is the audio data and the imaginary part is set to 0.
*/
void convertInt16ToFloat(int16_t *inputBuffer, float *outputBuffer)
{
  for (int i = 0; i < FFT_SIZE; i++)
  {
    outputBuffer[2 * i] = (float)inputBuffer[i]; // Real part
    outputBuffer[2 * i + 1] = 0.0f; // Imaginary part
  }
}

/*
* @brief Convert float to int16_t function
*
* @param[in] inputBuffer    The input audio data buffer in float format
* @param[out] outputBuffer  The output audio data buffer in int16_t format
*
* @details This function converts the audio data from float to int16_t.
* The audio data is scaled down by dividing by the FFT size.
*/
void convertFloatToInt16(float *inputBuffer, int16_t *outputBuffer)
{
    for (int i = 0; i < FFT_SIZE; i++)
    {
        outputBuffer[i] = (int16_t)(inputBuffer[2 * i] / FFT_SIZE);
    }
}

/*
* @brief Get Magnitude and Phase function
*
* @param[in] buffer         The audio data buffer
* @param[out] magnitude     The magnitude information
* @param[out] phase         The phase information
*
* @details This function extracts the magnitude and phase from the buffer in the frequency domain.
* The magnitude is calculated as the square root of the sum of the squares of the real and imaginary parts.
* The phase is calculated as the arctangent of the imaginary part divided by the real part.
*/
void getMagnitudeAndPhase(float *buffer, float *magnitude, float *phase)
{
  for (int i = 0; i < FFT_SIZE; i++)
  {
      float real = buffer[2 * i];
      float imag = buffer[2 * i + 1];
      magnitude[i] = sqrtf((real * real) + (imag * imag));
      phase[i] = atan2f(imag, real);
  }
}

/*
* @brief Get FFT Configuration function
*
* @param[in] size The FFT size
*
* @details This function returns the FFT configuration based on the FFT size.
*/
const arm_cfft_instance_f32* getFFTConfig(int size) 
{
    switch (size) 
    {
        case 128:  return &arm_cfft_sR_f32_len128;
        case 256:  return &arm_cfft_sR_f32_len256;
        case 512:  return &arm_cfft_sR_f32_len512;
        case 1024: return &arm_cfft_sR_f32_len1024;
        case 2048: return &arm_cfft_sR_f32_len2048;
        case 4096: return &arm_cfft_sR_f32_len4096;
        default:   return nullptr; // Handle error
    }
}

/*
* @brief Process FFT function
*
* @param[in] buffer         The audio data buffer
* @param[in] floatBuffer    The float buffer
* @param[out] magnitude     The magnitude information
* @param[out] phase         The phase information 
*
* @details This function performs the FFT on the audio data in the buffer.
* It converts the audio data to float, performs the FFT, and extracts the magnitude and phase information.
*/
void processFFT(float *floatBuffer, float *magnitude, float *phase)
{
    // Perform FFT
    arm_cfft_f32(fftConfig, floatBuffer, 0, 1);
    
    // Extract magnitude and phase
    getMagnitudeAndPhase(floatBuffer, magnitude, phase);
}

/*
* @brief Inverse FFT function
*
* @param[in] buffer             The audio data buffer
* @param[in] carrierMagnitude   The carrier magnitude information
* @param[in] carrierPhase       The carrier phase information
* @param[in] modulatorMagnitude The modulator magnitude information
* @param[in] modulatorPhase     The modulator phase information
*
* @details This function reconstructs the signal from the magnitude and phase information.
* It then performs an inverse FFT to return to the time domain.
*/
void inverseFFT(float *buffer, float *carrierMagnitude, float *carrierPhase, float *modulatorMagnitude, float *modulatorPhase)
{
    // Reconstruct signal from magnitude and phase
    for (int i = 0; i < FFT_SIZE; i++)
    {
        buffer[2 * i] = carrierMagnitude[i] * cosf(carrierPhase[i]);
        buffer[2 * i + 1] = carrierMagnitude[i] * sinf(carrierPhase[i]);
    }

    // Perform Inverse FFT
    arm_cfft_f32(fftConfig, fftBuffer, 1, 1);
}

/*
* @brief Setup function
*
* @details This function initializes the audio library and the SGTL5000 audio codec.
*/
void setup()
{
    Serial.begin(115200);

    AudioMemory(10);
    sgtl5000_1.enable();
    sgtl5000_1.inputSelect(AUDIO_INPUT_LINEIN);
    sgtl5000_1.volume(0.5);

    fftConfig = getFFTConfig(FFT_SIZE);
    if (!fftConfig) 
    {
        Serial.println("Invalid FFT size!");
        return;
    }

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
    if (carrierBufferFull && modulatorBufferFull)
    {
        // Convert int16_t to float
        convertInt16ToFloat(carrierBuffer, carrierFloatBuffer);

        processFFT(carrierFloatBuffer, carrierMagnitude, carrierPhase);
        processFFT(modulatorFloatBuffer, modulatorMagnitude, modulatorPhase);
        
        inverseFFT(fftBuffer, carrierMagnitude, carrierPhase, modulatorMagnitude, modulatorPhase);
        
        convertFloatToInt16(fftBuffer, carrierBuffer);
        
        carrierBufferFull = false;
        modulatorBufferFull = false;
        playbackReady = true;
    }
}