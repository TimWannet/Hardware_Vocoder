/** 
 * @file audio_stream_classes.cpp
 * @brief Audio stream classes for processing audio data
 *  
 * @details This file contains the re-implementation of audio stream classes for processing audio data.
 * It includes classes for processing audio data from the carrier input, modulator input, and playback output.
 * 
 * @author Tim Wannet
 * @date 26-03-2025
 * @version 0.01
*/

// Headers
#include "audio_stream_classes.h"

/*
* @class CarrierBufferProcessor
* @brief Processes audio data from I2S input and stores it in a buffer

* @details This class processes audio data from the I2S input, this is handled in a interrupt service routine.
* The audio data is stored in a buffer and when the buffer is full, a flag is set to signal that processing can start.
*/

    CarrierBufferProcessor::CarrierBufferProcessor() : AudioStream(1, inputQueueArray) {} 

    //override base::update()
    void CarrierBufferProcessor::update()
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


/*
* @class ModulatorProcessor
* @brief Processes audio data from the modulator and stores it in a buffer
* 
* @details This class processes audio data from the modulator, this is handled in a interrupt service routine.
* The audio data is stored in a buffer and when the buffer is full, a flag is set to signal that processing can start.
*/
        ModulatorProcessor::ModulatorProcessor() : AudioStream(1, inputQueueArray) {}

        void ModulatorProcessor::update()
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


/*
* @class PlaybackProcessor
* @brief Processes audio data from a buffer and plays it back
*
* @details This class processes audio data from the buffer and plays it back using the I2S output.
* This is handled in a interrupt service routine and the audio data is played back in chunks of 128 samples (according to the Audio Library).
*/
    PlaybackProcessor::PlaybackProcessor() : AudioStream(0, NULL) {}

    //override base::update()
    void PlaybackProcessor::update()  
    {
        if (!playbackReady) 
            return;
        
        audio_block_t *block = allocate(); 
        
        if (!block) 
            return;

        static uint16_t index = 0;

        for (int i = 0; i < AUDIO_BLOCK_SAMPLES; i++) 
        {
            block->data[i] = fftFloatBuffer[index++];
            if (index >= FFT_SIZE)
            {
                index = 0;
                playbackReady = false;
            }
        }
        transmit(block);
        release(block);
    }