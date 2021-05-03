# **Audio Frequency Visualizer**

## **Contributors**
Ike Gugel, Tolga Ustuner, James Cline

## **Parts List**
### **LED Board**:
| Part | Description |
| ----------- | ----------- |
| Frost Glaze Plastic Overlay | Forefront plastic piece used to blur LEDs. |
| #10-32 x 2 in Phillips Screws | Used to attach the Frost Glaze Plastic Overlay to the board with LED strips. |
| 3/16 Washers | Used to further secure the Plastic Overlay to the board. |
| Alitove LED Strip | Roll of 150 WS2812b LEDs, three rolls were soldered onto the board. |
### **MBed**: 
| Part | Description |
| ----------- | ----------- |
| Arm Mbed LPC1768 | Main microcontroller for managing pushbottons and light control. |
| 5V Barrel Jack and Power Supply | Used to provide input power for the entire board. |
| Pushbuttons | Used as PullUps to change the color settings of the LEDs. |
| Resistors | 330 ohm resistors used to implement pullup buttons. |

## **Schematic**
![image](https://drive.google.com/uc?export=view&id=<FILE_ID>)

## **Code Documentation**
### **Libraries**:
#### `mbed.h`
The C++ library mbed.h contains the MBed SDK tools required to define inputs and outputs on the MBed LCP1768 microcontroller.
#### `NeoStrip.h`
The C++ library NeoStrip.h contains the classes and methods required to set the hue and brightness of an LED in a strip of AdaFruit NeoPixel RGB LEDs. Publically available at: https://os.mbed.com/users/yanndouze/code/NeoStrip/annotate/662aa70e768f/NeoStrip.h/
#### `FFTCM3.s`
The ARM assembly library FFTCM3.s contains the methods required to compute the FFT of a set of samples to obtain a frequency analysis. Publically available at: https://github.com/gabonator/DS203/blob/master/Source/HwLayer/ArmM3/bios/FFTCM3.s
### **Methods**:
#### `float magnitude(short y1, short y2)`
| Category | Description |
| ----------- | ----------- |
| Summary | Get the magnitude of a complex number. |
| y1 | The real component of the complex number. |
| y2 | The imaginary component of the complex number. |
| return | The magnitude of the real and imaginary component. |
#### `int idxConversion(int c, int r)`
| Category | Description |
| ----------- | ----------- |
| Summary | Convert a 2-D array index to a 1-D array index. |
| Details | If the column index is even, index the row from the top to bottom to compensate for the strip being reversed. If the column index is odd, index the row from the bottom to top because the strip is in the propper orientation. |
| c | The column index value. |
| r | The row index value. |
| return | The 1-D NeoPixel strip array index. |
#### `void spectrumToOutput()`
| Category | Description |
| ----------- | ----------- |
| Summary | Converts the FFT spectrum to an array output for the NeoPixel strip. |
| Details | spectrum[] has a length of 512, so divide 512 by the NUM_COLS in the display to get 19 spectrum[] values per bin. Average 19 spectrum[] values and store that average in output_data[]. Normalize output_data[] to be left with values from 0.0f - 1.0f. |
#### `void updateSamples()`
| Category | Description |
| ----------- | ----------- |
| Summary | Fills the sample buffer with a slice of the audio data. When out of audio data, restart from beginning. |
| Details | Fill samples[] with data from sound_data[] until it is full. When samples[] is full, set the full flag and reset the samples_idx to base. When out sound_data[], reset the data_idx to base. |
#### `void calcFFT()`
| Category | Description |
| ----------- | ----------- |
| Summary | Computes the FFT of a set of audio samples. |
| Details | Clear the input (mx[]) and output (my[]) arrays to the fftR4() method. Populate mx[] with audio sample values. Compute the FFT using fftR4(). Populate spectrum[] with the magnitude of the real and imaginary components of my[]. |
#### `void lightLeds()`
| Category | Description |
| ----------- | ----------- |
| Summary | Displays the FFT spectrum on the the NeoPixel strip. |
| Details | Clear the NeoPixel LED strip. Convert the current sample frequency spectrum to a valide NeoPixel output. Populate each column with the output_data[] array. Update the LED strip brightness. Write the brightness and values to the NeoPixel LED strip. |
#### `void updateColor()`
| Category | Description |
| ----------- | ----------- |
| Summary | Change the current color. |
| Details | If the button is pushed (PullUp mode so value 0), increment the color_idx value to select the next color. Mod 3 to perform the wrap around calculation. (Eg. 2 -> 0, not 2 -> 3) |
#### `void updateBrightness()`
| Category | Description |
| ----------- | ----------- |
| Summary | Change the current brightness. |
| Details | If the brightness_up button is pushed and the brightness is not yet at max, increment the brightness by 0.1. If the brightness_down button is pushed and the brightness is not yet at min, decrement the brightness by 0.1. |

## **Additional Photos and Videos**
TODO
