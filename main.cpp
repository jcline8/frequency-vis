#include "mbed.h"
#include "audiodata.h"

extern "C" void fftR4(short *y, short *x, int N);
Serial pc(USBTX, USBRX);

#define BUFFER_SIZE 1024
#define SAMPLE_RATE 10000

LocalFileSystem local("local");

int data_idx = 0;
int samples_idx = 0;

short samples[BUFFER_SIZE];
short mx[BUFFER_SIZE * 2]; // 16 bit 4 byte alligned ff input data
short my[BUFFER_SIZE * 2]; // 16 bit 4 byte alligned fft output data
float spectrum[BUFFER_SIZE/2];  // frequency spectrum

bool full = false;

float magnitude(short y1, short y2);
void updateSamples();
void printSamples();
void calcFFT();
void printFFT();

int main() {
    // Setup
    while (true) {
        // Always update samples
        updateSamples();
        if (full) {
            // Do FFT
            calcFFT();
            // Display FFT
            // printFFT();
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
    samples[samples_idx] = (short) (sound_data[data_idx] - 0x8000);
    samples_idx++;

    if (samples_idx >= BUFFER_SIZE) {
        // pc.printf("Samples full.\r\n");
        full = true;
        samples_idx = 0;

        if ((data_idx + BUFFER_SIZE) > NUM_ELEMENTS) {
            data_idx = 0;
        } else {
            data_idx += BUFFER_SIZE;
        }
    }

    pc.printf("Sample index: %d\n\r", samples_idx);
    pc.printf("Data index: %d\n\r", data_idx);
    wait(0.1);
}

void calcFFT() {
    // pc.printf("Calculating FFT.\n\r");
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

void printFFT() {
    pc.printf("Printing FFT.\n\r");
    FILE *fp = fopen("/local/fft.csv","w");
    int j = 0;
    for (int i = 0; i < BUFFER_SIZE; i += 2) {
        int frequency = int(SAMPLE_RATE / BUFFER_SIZE / 2 * i);
        fprintf(fp, "%d,%f\n", frequency, spectrum[j]);
        j++;
    }
    fclose(fp);
}
