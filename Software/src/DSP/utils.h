/**
 * @file utils.h
 * @brief Header file for utility functions
 * 
 * @details This file contains function declarations for converting data formats
 * 
 * @author Tim Wannet
 * @date 20-05-2025
 * @version 0.01
 */

// Headers
#include <cstdint>

// Function prototypes
void convertInt16ToFloat(int16_t *inputBuffer, float *outputBuffer);
void convertFloatToInt16(float *inputBuffer, int16_t *outputBuffer);

// External variables
extern const int FFT_SIZE;