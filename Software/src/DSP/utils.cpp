/**
 * @file utils.cpp
 * @brief Utility functions
 *  
 * @details This file contains utility functions for converting data formats
 * 
 * @author Tim Wannet
 * @date 20-05-2025
 * @version 0.01
 */

 // Headers
#include "utils.h"
#include <cmath>

/*
// * @brief Convert int16_t to float function
// *
// * @param[in] inputBuffer    The input audio data buffer in int16_t format
// * @param[out] outputBuffer  The output audio data buffer in float format
// *
// * @details This function converts the audio data from int16_t to float.
// * The real part is the audio data and the imaginary part is set to 0.
// */
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
        outputBuffer[i] = (int16_t)(inputBuffer[2 * i] / 16);
    }
}
