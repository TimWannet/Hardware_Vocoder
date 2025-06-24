/**
 * @file fft_utils.cpp
 * @brief FFT utility functions
 *  
 * @details This file contains utility functions for performing FFT operations.
 * It includes functions for getting FFT configuration, extracting magnitude and phase,
 * performing inverse FFT, and processing FFT.
 *  
 * @author Tim Wannet
 * @date 20-05-2025
 * @version 0.01
 */

// Headers
#include "fft_utils.h"

// Variables
const float sampleRate = 44100.0; // Sample rate in Hz
const float cutoffFreq = 100.0; // Cutoff frequency for highpass filter

float prev_input = 0.0;
float prev_output = 0.0;
float RC = 1.0 / (2 * PI * cutoffFreq);  // e.g., 200 Hz
float dt = 1.0 / sampleRate;
float alpha = RC / (RC + dt);

float noiseUnvoiced = static_cast<float>(rand()) / RAND_MAX - 0.5f; // -0.5 to +0.5
float unvoicedNoiseStrength = 0.9f; // scale to taste
float noiseVoiced = static_cast<float>(rand()) / RAND_MAX - 0.5f;
float voicedNoiseStrength = 0.4f;
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

    // // Debug print: Print first 8 bins of magnitude and phase
    // Serial.println("FFT Debug Output:");
    // for (int i = 0; i < 8 && i < FFT_SIZE; i++) {
    //     Serial.print("Bin ");
    //     Serial.print(i);
    //     Serial.print(": Magnitude = ");
    //     Serial.print(magnitude[i]);
    //     Serial.print(", Phase = ");
    //     Serial.println(phase[i]);
    // }
}

bool isUnvoiced(const float* magnitude) 
{
    constexpr float SAMPLE_RATE = 44100.0f;
    constexpr int FFT_SIZE = 2048;
    constexpr float BIN_WIDTH = SAMPLE_RATE / FFT_SIZE;

    // Define band ranges (you can tune these further)
    const int lowStart = (int)(80 / BIN_WIDTH);
    const int lowEnd   = (int)(500 / BIN_WIDTH);

    const int highStart = (int)(3000 / BIN_WIDTH);
    const int highEnd   = (int)(8000 / BIN_WIDTH);

    float lowEnergy = 0.0f;
    float highEnergy = 0.0f;

    for (int i = lowStart; i < lowEnd; i++) {
        lowEnergy += magnitude[i];
    }

    for (int i = highStart; i < highEnd; i++) {
        highEnergy += magnitude[i];
    }

    // Avoid divide-by-zero
    if (lowEnergy < 1e-5f) lowEnergy = 1e-5f;

    float ratio = highEnergy / lowEnergy;

    // You can tune this threshold — try values between 3.0 and 6.0
    return (ratio > 4.0f);
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

    float lowEnergy = 0.0f;
    float highEnergy = 0.0f;

    for (int i = 0; i < FFT_SIZE / 2; i++) {
        if (i < FFT_SIZE / 16) { // Low frequencies (e.g. ~0–500Hz)
            lowEnergy += modulatorMagnitude[i];
        } else if (i > FFT_SIZE / 10) { // High frequencies (e.g. >1kHz)
            highEnergy += modulatorMagnitude[i];
        }
    }

    // bool isUnvoiced = (highEnergy > lowEnergy * 1.0f) && (highEnergy > 0.01f);
    bool is_unvoiced = isUnvoiced(modulatorMagnitude);

    if (is_unvoiced) 
    {
        for (int i = 0; i < FFT_SIZE / 2; i++) 
        {
            buffer[2 * i] = noiseUnvoiced * unvoicedNoiseStrength; // real
            buffer[2 * i + 1] = noiseUnvoiced * unvoicedNoiseStrength; // imaginary
        }
    }
    else
    {
        for (int i = 0; i < FFT_SIZE; i++) 
        {
            float normalizedMod = modulatorMagnitude[i] / 32768.0f;  // Assuming 16-bit range
            float normalizedCarrier = carrierMagnitude[i] / 32768.0f;
            float fftMagnitude = normalizedCarrier * pow(normalizedMod, 0.4f) * 30768.0f; // Scale back
            fftMagnitude += noiseVoiced * voicedNoiseStrength * 30768.0f;; // Add noise to voiced signal 

            buffer[2 * i] = fftMagnitude * cosf(carrierPhase[i]); // Real part
            buffer[2 * i + 1] = fftMagnitude * sinf(carrierPhase[i]); // Imaginary part
        }
    }
    // Perform Inverse FFT
    arm_cfft_f32(fftConfig, buffer, 1, 1);

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
* @brief Highpass filter function
*
* @param[in] input The input audio data
* @return The filtered output
*
* @details This function implements a simple highpass filter using a single pole IIR filter.
* The filter is designed to remove low-frequency components from the audio signal.
*/
float highpass(int16_t input)
{
    float output = alpha * (prev_output + input - prev_input);
    prev_input = input;
    prev_output = output;
    return output;
}
