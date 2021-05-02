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
DigitalIn brightness_up(p9);
DigitalIn brightness_down(p10);
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

int main() {
    leds.clear();
    leds.setBrightness(brightness);

    change_color.mode(PullUp);
    brightness_down.mode(PullUp);
    brightness_down.mode(PullUp);

    color_ticker.attach(&updateColor, 1.0/SAMPLE_RATE);
    // brightness_ticker.attach(&updateBrightness, 1.0/SAMPLE_RATE);

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

float magnitude(short y1, short y2) {
    return sqrt(float(y1 * y1 + y2 * y2));
}

int idxConversion(int c, int r) {
    int idx;
    if (c % 2 == 0) {
        idx = (c + 1) * NUM_ROWS - r - 1;
    } else {
        idx = c * NUM_ROWS + r;
    }
    return idx;
}

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

void updateColor() {
    if (change_color == 0) {
        color_idx = (color_idx + 1) % 3;
    }
}

void updateBrightness() {
    if (brightness_up == 0 && brightness < 1.0) {
        brightness += 0.1;
    } else if (brightness_down == 0 && brightness > 0.0) {
        brightness -= 0.1;
    }
}
