#include "time_measurements.h"
#include "shared_global_variables.h"

// Function for time measurements - please only test the different variants seperatly because rgbValues is freed after each version
/*
void timeMeasurements(RGB* rgbValues) {
    // Time measurements
    double start, end, time = 0;
    struct timespec t;
    double factor = 1e-9;

    /********************************************************************************************/
// SIMD time measurements
/*
RGB* rgbFinalValues;
RGBcolorWord* rgbSIMD;
RGBcolorWord* rgbnewSIMD = (RGBcolorWord*) malloc(3 * global_image_width * global_image_height * sizeof(RGBcolorWord));
if(!rgbnewSIMD) {
    printf("Error allocation new memory!\n");
    return -1;
}

// Time measurement start
clock_gettime(CLOCK_MONOTONIC, &t);
start = t.tv_sec + t.tv_nsec * factor;

for(int i = 0; i < 100; i++) {
    rgbSIMD = convertRGBtoSIMDWord(rgbValues);
    if(!rgbSIMD) {
        printf("Error converting RGB array to SIMD word array!\n");
        return -1;
    }

    greyscale_simd(rgbSIMD, global_image_width, global_image_height);
    blur_simd(rgbSIMD, rgbnewSIMD, global_image_width, global_image_height);

    rgbFinalValues = convertSIMDWordtoRGB(rgbnewSIMD);
    if(!rgbFinalValues) {
        printf("Error converting SIMD word array to RGB array!\n");
        return -1;
    }

    free(rgbSIMD);
    free(rgbFinalValues);
}

// End time measurements
clock_gettime(CLOCK_MONOTONIC, &t);
end = t.tv_sec + t.tv_nsec * factor;
time = end - start;

printf("%lf\n", time);

free(rgbValues);
free(rgbnewSIMD);
*/
/********************************************************************************************/


/********************************************************************************************/
// Normal ASM time measurements (first greyscale, then blur)
/*
RGB* rgbNewValues = (RGB*) malloc(global_image_width * global_image_height * sizeof(RGB));
if(!rgbValues) {
    printf("Error allocation new memory!\n");
    return -1;
}

// Time measurement start
clock_gettime(CLOCK_MONOTONIC, &t);
start = t.tv_sec + t.tv_nsec * factor;

for(int i = 0; i < 100; i++) {
    greyscale(rgbValues, global_image_width, global_image_height);
    blur(rgbValues, rgbNewValues, global_image_width, global_image_height);
}

// End time measurements
clock_gettime(CLOCK_MONOTONIC, &t);
end = t.tv_sec + t.tv_nsec * factor;
time = end - start;

printf("%lf\n", time);

free(rgbValues);
free(rgbNewValues);
*/
/********************************************************************************************/


/********************************************************************************************/
// ASM time measurements (first blur, then greyscale)
/*
RGB* rgbNewValues = (RGB*) malloc(global_image_width * global_image_height * sizeof(RGB));
if(!rgbValues) {
    printf("Error allocation new memory!\n");
    return -1;
}

// Time measurement start
clock_gettime(CLOCK_MONOTONIC, &t);
start = t.tv_sec + t.tv_nsec * factor;

for(int i = 0; i < 100; i++) {
    blur_colour(rgbValues, rgbNewValues, global_image_width, global_image_height);
    greyscale(rgbNewValues, global_image_width, global_image_height);
}

// End time measurements
clock_gettime(CLOCK_MONOTONIC, &t);
end = t.tv_sec + t.tv_nsec * factor;
time = end - start;

printf("%lf\n", time);

free(rgbValues);
free(rgbNewValues);
*/
/********************************************************************************************/
/*
}*/

