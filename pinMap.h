#ifndef PINMAP_H
#define PINMAP_H

#define NUM_PINS_P8 46
#define NUM_PINS_P9 46

typedef struct{
    int header_num;
    int pin_num;
} GPIO_Pin;


extern GPIO_Pin GPIO_pinList[];
/*
GPIO_Pin GPIO_pinList[] = {
    {8,  3}, {8,  4}, {8,  5}, {8,  6}, {8,  7}, {8,  8}, {8,  9}, {8, 10}, {8, 11}, {8, 12}, 
    {8, 13}, {8, 14}, {8, 15}, {8, 16}, {8, 17}, {8, 18}, {8, 19}, {8, 20}, {8, 21}, {8, 22}, 
    {8, 23}, {8, 24}, {8, 25}, {8, 26}, {8, 27}, {8, 28}, {8, 29}, {8, 30}, {8, 31}, {8, 32}, 
    {8, 33}, {8, 34}, {8, 35}, {8, 36}, {8, 37}, {8, 38}, {8, 39}, {8, 40}, {8, 41}, {8, 42}, 
    {8, 43}, {8, 44}, {8, 45}, {8, 46}, {9, 11}, {9, 12}, {9, 13}, {9, 14}, {9, 15}, {9, 16},
    {9, 19}, {9, 20}, {9, 21}, {9, 22}, {9, 23}, {9, 24}, {9, 25}, {9, 26}, {9, 27}, {9, 28},
    {9, 29}, {9, 30}, {9, 31}, {9, 41}, {9, 42}
};
*/

int getGPIO(int header_num, int pin_num);


#endif
