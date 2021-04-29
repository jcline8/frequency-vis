#include "mbed.h"
#include "audiodata.h"

extern "C" void fftR4(short *y, short *x, int N);

#define BUFFER_SIZE 1024
#define SAMPLE_RATE 10000

LocalFileSystem local("local");

int data_idx = 0;
int samples_idx = 0;

short samples[BUFFER_SIZE];
short mx[BUFFER_SIZE * 2]; // input data 16 bit, 4 byte aligned  x0r,x0i,x1r,x1i,....
short my[BUFFER_SIZE * 2]; // output data 16 bit,4 byte aligned  y0r,y0i,y1r,y1i,....
float spectrum[BUFFER_SIZE/2];  // frequency spectrum

bool full = false;

float magnitude(short y1, short y2);
void updateSamples();
void printSamples();
void calcFFT();
void printFFT();

int main() {
    // Setup
    while (data_idx < NUM_ELEMENTS) {
        // Always update samples
        updateSamples();
        if (full) {
            // Do FFT
            calcFFT();
            // Display FFT
            printFFT();
            // Reset full flag
            full = false;
        }
        wait(1.0/SAMPLE_RATE);
    }
}

float magnitude(short y1, short y2)
{
    return sqrt(float(y1 * y1 + y2 * y2));
}

void updateSamples() {
    samples[samples_idx] = (short) (data[data_idx] - 0x8000);
    samples_idx++;

    if (samples_idx >= BUFFER_SIZE) {
        full = true;
        samples_idx = 0;
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
    for (int i = 0; i < BUF_LEN; i+=2) {
        spectrum[j] = magnitude(my[i], my[i + 1]);
        j++;
    }
}

void printFFT() {
    FILE *fp = fopen("/local/fft.csv","w");
    int j = 0;
    for (int i = 0; i < BUFFER_SIZE; i += 2) {
        int frequency = int(SAMPLE_RATE / BUFFER_SIZE / 2 * i);
        fprintf(fp, "%d,%f\n", frequency, spectrum[j]);
        j++;
    }
    fclose(fp);
}
