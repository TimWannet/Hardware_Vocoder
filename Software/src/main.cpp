#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <arm_math.h>
#include <cmath>
#include <arm_const_structs.h>

// defines
#define FFT_SIZE 256 // Buffer size (Change this value as needed: 128, 256, 512, 1024, etc.)

// Audio Library objects
AudioInputI2S         i2sInput;  // I2S input from Audio Shield
AudioInputAnalog      analogInput(A17); // Analog input as modulator
AudioOutputI2S        i2sOutput; // I2S output to Audio Shield
AudioControlSGTL5000  sgtl5000_1;

// variables
int16_t CarrierBuffer[FFT_SIZE];
int16_t modulatorBuffer[FFT_SIZE];

float fftBuffer[FFT_SIZE * 2]; // Complex FFT buffer (real + imaginary)
float modulatorFFT[FFT_SIZE * 2];
float magnitude[FFT_SIZE];
float phase[FFT_SIZE];

volatile bool bufferFull = false;
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
              CarrierBuffer[index++] = block->data[i]; // Store audio data in buffer
              if (index >= FFT_SIZE) // Buffer full
              {
                  bufferFull = true; // Signal that processing can start
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
              modulatorBuffer[index++] = block->data[i];
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
              block->data[i] = CarrierBuffer[index++];
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
CarrierBufferProcessor  bufferProcessor;
ModulatorProcessor      modProcessor;
PlaybackProcessor       playbackProcessor;
AudioConnection         patchCord1(i2sInput, 0, bufferProcessor, 0); 
AudioConnection         patchCord2(analogInput, 0, modProcessor, 0); 
AudioConnection         patchCord3(playbackProcessor, 0, i2sOutput, 0); // left channel
AudioConnection         patchCord4(playbackProcessor, 0, i2sOutput, 1); // right channel


// Function prototypes
void processFFT();

void getMagnitudeAndPhase(float *buffer)
{
  for (int i = 0; i < FFT_SIZE; i++)
  {
      float real = buffer[2 * i];
      float imag = buffer[2 * i + 1];
      magnitude[i] = sqrtf(real * real + imag * imag);
      phase[i] = atan2f(imag, real);
  }
}


/*
* @brief Process FFT function
*
* @details This function performs the FFT on the audio data in the buffer.
* The audio data is converted to float, the FFT is performed, the magnitude and phase are extracted, the signal is reconstructed, and the inverse FFT is performed.
*/
void processFFT() 
{
    // Convert int16_t to float
    for (int i = 0; i < FFT_SIZE; i++)
    {
        fftBuffer[2 * i] = (float)CarrierBuffer[i]; // Real part
        fftBuffer[2 * i + 1] = 0.0f; // Imaginary part
    }
    
    // Perform Forward FFT
    arm_cfft_f32(&arm_cfft_sR_f32_len256, fftBuffer, 0, 1);
    
    // Extract magnitude and phase
    getMagnitudeAndPhase(fftBuffer);
    
    // Reconstruct signal from magnitude and phase
    for (int i = 0; i < FFT_SIZE; i++)
    {
        fftBuffer[2 * i] = magnitude[i] * cosf(phase[i]);
        fftBuffer[2 * i + 1] = magnitude[i] * sinf(phase[i]);
    }
    
    // Perform Inverse FFT
    arm_cfft_f32(&arm_cfft_sR_f32_len256, fftBuffer, 1, 1);
    
    // Convert back to int16_t
    for (int i = 0; i < FFT_SIZE; i++)
    {
        CarrierBuffer[i] = (int16_t)(fftBuffer[2 * i] / FFT_SIZE); // Scale down
    }
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
    if (bufferFull == true) // Process FFT when buffer is full
    {
        processFFT(); // Perform FFT
        bufferFull = false;
        playbackReady = true;
    }
}

