/**
 * @file main.cpp
 * @author J. Harper Cline (james.h.cline@gmail.com)
 * @brief Code to visualize the frequency spectrum of an audio file.
 * @version 0.1
 * @date 2021-05-02
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "mbed.h"
#include "NeoStrip.h"
#include "audio.h"

extern "C" void fftR4(short *y, short *x, int N);

#define BUFFER_SIZE 1024
#define SAMPLE_RATE 10000
#define NUM_ROWS 15
#define NUM_COLS 26
#define N 390
#define RED     0xFF0000
#define GREEN   0x00FF00
#define BLUE    0x0000FF

Serial pc(USBTX, USBRX);
NeoStrip leds(p18, N);
DigitalIn change_color(p8);
DigitalIn brightness_up(p11);
DigitalIn brightness_down(p12);
Ticker color_ticker;
Ticker brightness_ticker;

int data_idx = 0;
int samples_idx = 0;
short samples[BUFFER_SIZE];
bool full = false;

short mx[BUFFER_SIZE * 2];
short my[BUFFER_SIZE * 2];
float spectrum[BUFFER_SIZE/2];
float output_data[NUM_COLS];

int colors[] = {RED, GREEN, BLUE};
int color_idx = 0;
float brightness = 0.5;

float magnitude(short y1, short y2);
int idxConversion(int c, int r);
void spectrumToOutput();
void updateSamples();
void calcFFT();
void printFFT();
void lightLeds();
void updateColor();
void updateBrightness();

/**
 * @brief Program main routine.
 * 
 * @return int No return expected.
 */
int main() {
    leds.clear();
    leds.setBrightness(brightness);

    change_color.mode(PullUp);
    brightness_down.mode(PullUp);
    brightness_down.mode(PullUp);

    color_ticker.attach(&updateColor, 0.1);
    brightness_ticker.attach(&updateBrightness, 0.1);

    while (1) {
        updateSamples();
        if (full) {
            calcFFT();
            lightLeds();
            full = false;
        }
        wait(1.0/SAMPLE_RATE);
    }
}

/**
 * @brief Get the magnitude of a complex number.
 * 
 * @param y1 The real component of the complex number. 
 * @param y2 The imaginary component of the complex number.
 * @return float The magnitude of the real and imaginary component.
 */
float magnitude(short y1, short y2) {
    return sqrt(float(y1 * y1 + y2 * y2));
}

/**
 * @brief Convert a 2-D array index to a 1-D array index.
 * @details If the column index is even, index the row from the top to bottom to compensate for the strip being reversed.
 * If the column index is odd, index the row from the bottom to top because the strip is in the propper orientation.
 * @param c The column index value.
 * @param r The row index value.
 * @return int The 1-D NeoPixel strip array index.
 */
int idxConversion(int c, int r) {
    int idx;
    if (c % 2 == 0) {
        idx = (c + 1) * NUM_ROWS - r - 1;
    } else {
        idx = c * NUM_ROWS + r;
    }
    return idx;
}

/**
 * @brief Converts the FFT spectrum to an array output for the NeoPixel strip.
 * @details spectrum[] has a length of 512, so divide spectrum[] by the NUM_COLS in the display to get 19 spectrum[] values per bin.
 * Average 19 spectrum[] values and put store that average in output_data[].
 * Normalize output_data[] to be left with values from 0.0f - 1.0f.
 */
void spectrumToOutput() {
    float max = 0.0;
    for (int i = 0; i < NUM_COLS; i++) {
        output_data[i] = 0.0;
        for (int j = 0; j < 19; j++) {
            output_data[i] += spectrum[i * 19 + j];
        }
        output_data[i] /= 19;
        if (output_data[i] > max) {
            max = output_data[i];
        }
    }
    for (int i = 0; i < NUM_COLS; i++) {
        output_data[i] /= max;
    }
}

/**
 * @brief Fills the sample buffer with a slice of the audio data. When out of audio data, restart from beginning.
 * @details Fill samples[] with data from sound_data[] until it is full.
 * When samples[] is full, set the full flag and reset the samples_idx to base.
 * When out sound_data[], reset the data_idx to base.
 */
void updateSamples() {
    samples[samples_idx] = sound_data[data_idx];
    samples_idx++;
    if (samples_idx >= BUFFER_SIZE) {
        full = true;
        samples_idx = 0;
    }
    if (data_idx + BUFFER_SIZE > NUM_ELEMENTS - 1) {
        data_idx = 0;
    } else {
        data_idx += BUFFER_SIZE;
    }
}

/**
 * @brief Computes the FFT of a set of audio samples.
 * @details Clear the input (mx[]) and output (my[]) arrays to the fftR4() method.
 * Populate mx[] with audio sample values.
 * Compute the FFT using fftR4().
 * Populate spectrum[] with the magnitude of the real and imaginary components of my[].
 */
void calcFFT() {
    for (int i = 0; i < 2 * BUFFER_SIZE; i++) {
        my[i] = 0;
        mx[i] = 0;
    }
    for (int i = 0; i < BUFFER_SIZE; i++) {
        mx[i * 2] = samples[i];
    }
    fftR4(my, mx, BUFFER_SIZE);
    int j = 0;
    for (int i = 0; i < BUFFER_SIZE; i += 2) {
        spectrum[j] = magnitude(my[i], my[i + 1]);
        j++;
    }
}

/**
 * @brief Displays the FFT spectrum on the the NeoPixel strip.
 * @details Clear the NeoPixel LED strip.
 * Convert the current sample frequency spectrum to a valide NeoPixel output.
 * Populate each column with the output_data[] array.
 * Update the LED strip brightness.
 * Write the brightness and values to the NeoPixel LED strip.
 */
void lightLeds() {
    leds.clear();
    spectrumToOutput();
    for (int i = 0; i < NUM_COLS; i++) {
        int height = (int) (((float) NUM_ROWS) * output_data[i]);
        for (int j = 0; j < height; j++) {
            leds.setPixel(idxConversion(i, j), colors[color_idx]);
        }
    }
    leds.setBrightness(brightness);
    leds.write();
}

/**
 * @brief Change the current color.
 * @details If the button is pushed (PullUp mode so value 0), increment the color_idx value to select the next color.
 * Mod 3 to perform the wrap around calculation. (Eg. 2 -> 0, not 2 -> 3)
 */
void updateColor() {
    if (!change_color) {
        pc.printf("Button 1 triggered.\n\r");
        color_idx = (color_idx + 1) % 3;
    }
}

/**
 * @brief Change the current brightness.
 * @details If the brightness_up button is pushed and the brightness is not yet at max, increment the brightness by 0.1.
 * If the brightness_down button is pushed and the brightness is not yet at min, decrement the brightness by 0.1.
 */
void updateBrightness() {
    if (!brightness_up && brightness < 1.0) {
        pc.printf("Button 2 triggered.\n\r");
        brightness += 0.1;
        if (brightness > 1.0) {
            brightness = 1.0;
        }
    } 
    if (!brightness_down && brightness > 0.0) {
        pc.printf("Button 3 triggered.\n\r");
        brightness -= 0.1;
        if (brightness < 0.0) {
            brightness = 0.0;
        }
    }
}
