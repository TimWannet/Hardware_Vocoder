/**
    * @file fft_utils.h
    * @brief Header file for FFT utility functions
    * 
    * @details This file contains function declarations for performing FFT operations.
    * It includes functions for getting FFT configuration, extracting magnitude and phase,
    * performing inverse FFT, and processing FFT.
    *   
    * @author Tim Wannet
    * @date 20-05-2025
    * @version 0.01
*/

// Headers
#include <arm_math.h>
#include <cmath>
#include "arm_const_structs.h"

// Function prototypes
const arm_cfft_instance_f32* getFFTConfig(int size);
void getMagnitudeAndPhase(float *buffer, float *magnitude, float *phase);
void inverseFFT(float *buffer, float *carrierMagnitude, float *carrierPhase, float *modulatorMagnitude, float *modulatorPhase);
void processFFT(float *floatBuffer, float *magnitude, float *phase);
float highpass(int16_t input);
bool SilentFrame(int16_t *buffer, int size, int threshold);

// External variables
extern const arm_cfft_instance_f32* fftConfig;
extern const int FFT_SIZE;