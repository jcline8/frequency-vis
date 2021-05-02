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


LocalFileSystem local("local");
Serial pc(USBTX, USBRX);
NeoStrip leds(p18, N);

int data_idx = 0;
int samples_idx = 0;
short samples[BUFFER_SIZE];
bool full = false;

short mx[BUFFER_SIZE * 2]; // 16 bit 4 byte alligned fft input data
short my[BUFFER_SIZE * 2]; // 16 bit 4 byte alligned fft output data
float spectrum[BUFFER_SIZE/2];  // frequency spectrum
float norm_spectrum[NUM_COLS];  // frequency spectrum

int color = RED;
float brightness = 0.5;

float magnitude(short y1, short y2);
int idxConversion(int c, int r);
void norm();

void updateSamples();
void printSamples();
void calcFFT();
void printFFT();
void lightLeds();

int main() {
    leds.clear();
    leds.setBrightness(brightness);

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

void norm() {
    float mx = 0;
    for (int i = 1; i < (NUM_COLS + 1); i++) {
        if (spectrum[i] > mx) {
            mx = spectrum[i];
        }
    }
    for (int i = 0; i < NUM_COLS; i++) {
        // if (i%3 == 0) {
        //     norm_spectrum[i] = 6.0f/15.0f;
        // } else {
        //     norm_spectrum[i] = 0;
        // }
        norm_spectrum[i] = spectrum[i+1] / mx;
    }
}

void updateSamples() {
    samples[samples_idx] = (short) (sound_data[data_idx] - 0x8000);
    samples_idx++;
    if (samples_idx >= BUFFER_SIZE) {
        full = true;
        samples_idx = 0;
    }
    if ((data_idx + BUFFER_SIZE) > (NUM_ELEMENTS - 1)) {
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
    for (int i = 0; i < BUFFER_SIZE; i+=2) {
        spectrum[j] = magnitude(my[i], my[i + 1]);
        j++;
    }
}

/*
 *    void printFFT() {
 *        FILE *fp = fopen("/local/fft.csv","w");
 *        int j = 0;
 *        for (int i = 0; i < BUFFER_SIZE; i += 2) {
 *            int frequency = int(SAMPLE_RATE / BUFFER_SIZE / 2 * i);
 *            fprintf(fp, "%d,%f\n", frequency, spectrum[j]);
 *           j++;
 *        }
 *        fclose(fp);
 *    }
 */

void lightLeds() {
    leds.clear();
    norm();
    for (int i = 0; i < NUM_COLS; i++) {
        // int height = (int) ((float) NUM_ROWS * 0.5f * (norm_spectrum[2*i] + norm_spectrum[2*i+1]));
        int height = (int) ((float) NUM_ROWS * (norm_spectrum[i]));
        for (int j = 0; j < height; j++) {
            leds.setPixel(idxConversion(i, j), color);
        }
    }
    leds.write();
}
