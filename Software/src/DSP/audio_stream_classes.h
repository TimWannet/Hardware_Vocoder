/**
 * @file audio_stream_classes.h
 * @brief Header file for audio stream classes
 * 
 * @details This file contains class declarations for processing audio data streams.
 * It includes classes for processing audio data from the carrier input, modulator input, and playback output.
 * 
 * @author Tim Wannet
 * @date 26-03-2025
 * @version 0.01
 */

// Headers
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>

// External variables
extern const int FFT_SIZE;

extern volatile bool carrierBufferFull;
extern volatile bool modulatorBufferFull;
extern volatile bool playbackReady;

extern int16_t carrierBuffer[];
extern int16_t modulatorBuffer[];
extern int16_t fftFloatBuffer[];

/*
* @class CarrierBufferProcessor
* @brief Processes audio data from I2S input and stores it in a buffer

* @details This class processes audio data from the I2S input, this is handled in a interrupt service routine.
* The audio data is stored in a buffer and when the buffer is full, a flag is set to signal that processing can start.
*/
class CarrierBufferProcessor : public AudioStream 
{
    public:
        CarrierBufferProcessor();

        //override base::update()
        void update() override;


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
        ModulatorProcessor();

        void update() override;

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
    PlaybackProcessor();

    //override base::update()
    void update() override;
};