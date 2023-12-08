#ifndef LED_MATRIX_H
#define LED_MATRIX_H


void displayInteger(int value); //displays the given integer on the LED matrix
void displayDouble(double value); //displays the given decimal number on the LED matrix
void set_i2cFileDesc(); // initialize I2C
void turnOnDisplay(); //turn on the LED matrix
void turnOffDisplay(); // turn off the LED matrix
void displayFromArr(const int LED_Arr[]); //display the array onto the matrix
void reset_Display(); // resets the matrix

#endif
