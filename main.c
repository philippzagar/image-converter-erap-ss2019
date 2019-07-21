#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <setjmp.h>
// For time measurements
#include <time.h>
// Libjpeg Library
#include "./libjpeg-master/jpeglib.h"

/***** C Functions ******/
// BMP Structs
#include "includes/BMPStructs.h"
// BMP Functions
#include "includes/BMP_functions.h"
#include "includes/BMP_functions.c"
// JPG Functions
#include "includes/JPG_functions.h"
#include "includes/JPG_functions.c"
// Time Measurements
#include "includes/time_measurements.h"
#include "includes/time_measurements.c"

// Additional Functions
bool endsWith(char *str, char *suffix);

/***** Assembly Functions ******/
// Greyscale
extern void greyscale(RGB *rgbValuesOut, int width, int height);
// Greyscale SIMD
extern void greyscale_simd(RGBcolorWord* out, int width, int height);

// Blur
extern void blur(RGB* in, RGB* out, int width, int height);
// Blur colour - for test blur first, then grey
extern void blur_colour(RGB* in, RGB* out, int width, int height);
// Blur SIMD
extern void blur_simd(RGBcolorWord* in, RGBcolorWord* out, int width, int height);

// Global variables
unsigned int global_image_height;
unsigned int global_image_width;
J_COLOR_SPACE global_colorSpace;

// Main Routine
int main(int argc, char** argv) {

    // Input/Output File
    FILE *inFile, *outFile;
    // BMP Header
    BMPHeader* header;
    BMPImageInfo* info;
    // RGB Values
    RGB* rgbValues;
    // File path
    char relativePath[100];
    char relativePathCopy[100];
    // 105 because -conv (the new ending of the file, without type extension) is +5 chars
    char newRelativePath[105];

    // Read File Name from User
    printf("Enter the file name: ");
    if(fgets(relativePath, 100, stdin) == NULL) {
        printf("Error while reading filename\n");
        return -1;
    }

    // Remove \n of input string
    size_t ln = strlen(relativePath) - 1;
    if (*relativePath && relativePath[ln] == '\n') {
        relativePath[ln] = '\0';
    }

    // Copy string because the original String is modified through the split operation
    strcpy(relativePathCopy, relativePath);

    // Split string at a .
    char delimiter[] = ".";
    char* fileName = strtok(relativePathCopy, delimiter);

    // Check File format
    bool isJPG = endsWith(relativePath, ".jpg") || endsWith(relativePath, ".JPG");
    bool isBMP = endsWith(relativePath, ".bmp") || endsWith(relativePath, ".BMP");

    // Open the file with given file name
    inFile = fopen(relativePath, "rb");

    // Checking for error while opening the file
    if( !inFile ) {
        printf("Error while opening file\n");
        return -1;
    }

    /******** Read picture ***********/
    // JPG File
    if(isJPG) {
        // Read JPG File
        rgbValues = read_JPEG_file(inFile);

        // Check for error while reading the JPG File
        if(!rgbValues) {
            printf("Error while reading JPG file\n");
            return -1;
        }
    // BMP File
    } else if(isBMP){
        // Reading header of BMP File
        header = readHeader(inFile);
        if(!header) {
            printf("Error while reading header\n");
            return -1;
        }

        // Reading image info of BMP file
        info = readInfo(inFile);
        if(!info) {
            printf("Error while reading image info\n");
            return -1;
        }

        // Check if BMP image is valid (no compression, etc.)
        if(!checkBMPImage(header, info)) {
            printf("Error - BMP image is not valid\n");
            return -1;
        }

        // Read RGB values from picture
        rgbValues = readBMPImage(inFile, info);
        if(!rgbValues) {
            printf("Error while reading RGB values from file\n");
            return -1;
        }
    // Wrong File Format
    } else {
        printf("Wrong file format - only BMP and JPG supported!\n");
        return -1;
    }

    // Close open file
    if(inFile != NULL) {
        fclose(inFile);
    }

    // Function for time measurements - every measurement in there
    // timeMeasurements(rgbValues);

    /******** Conversion of picture ***********/
    // Helper variables for picture conversion
    RGB* rgbFinalValues;
    RGBcolorWord* rgbSIMD;
    // Allocate new SIMD Array for blur function (else the results would be not entirly accurate)
    RGBcolorWord* rgbnewSIMD = (RGBcolorWord*) malloc(3 * global_image_width * global_image_height * sizeof(RGBcolorWord));
    if(!rgbnewSIMD) {
        printf("Error allocation new memory!\n");
        return -1;
    }

    // Conversion from RGB array to Word array (rrr/ggg/bbb), that can be used for SIMD
    rgbSIMD = convertRGBtoSIMDWord(rgbValues);
    if(!rgbSIMD) {
        printf("Error converting RGB array to SIMD word array!\n");
        return -1;
    }

    // Greyscale SIMD Assembly Function
    greyscale_simd(rgbSIMD, global_image_width, global_image_height);
    // Blur SIMD Assembly Function
    blur_simd(rgbSIMD, rgbnewSIMD, global_image_width, global_image_height);

    // Conversion from Word array for SIMD to RGB array
    rgbFinalValues = convertSIMDWordtoRGB(rgbnewSIMD);
    if(!rgbFinalValues) {
        printf("Error converting SIMD word array to RGB array!\n");
        return -1;
    }

    // Free allocated memory
    free(rgbValues);
    free(rgbSIMD);
    free(rgbnewSIMD);

    // Set final values to input values (that is now written back to memory)
    rgbValues = rgbFinalValues;

    /******** Write picture ***********/
    // JPG File
    if(isJPG) {
        // New name of file
        strcat(newRelativePath, fileName);
        strcat(newRelativePath, "-conv.jpg");

        // Open output file
        outFile = fopen(newRelativePath, "wb");

        if (!outFile) {
            printf("Error while opening JPG file to write\n");
            return -1;
        }

        // Writing JPEG file to memory
        if(!write_JPEG_file(outFile, 80, rgbValues)) {
            printf("Error while writing JPG file\n");
            return -1;
        }

        if(outFile != NULL) {
            fclose(outFile);
        }
    // BMP File
    } else {
        // New name of file
        strcat(newRelativePath, fileName);
        strcat(newRelativePath, "-conv.bmp");

        // Open file to write back the picture to memory
        outFile = fopen(newRelativePath, "wb");
        if( !outFile ) {
            printf("Error while opening file\n");
            return -1;
        }

        // Write image with header and info back to memory
        if(!writeBMPImage(outFile, header, info, rgbValues)) {
            printf("Error while writing image to memory\n");
            return -1;
        }

        // Clear allocated space
        if(header != NULL) {
            free(header);
        }

        if(info != NULL) {
            free(info);
        }

        if(outFile != NULL) {
            fclose(outFile);
        }
    }

    // Clear allocated space
    if(rgbValues != NULL) {
        free(rgbValues);
    }

    return 0;
}

// Check if string ends with a certain suffix
bool endsWith(char *str, char *suffix)
{
    if (!str || !suffix) {
        return false;
    }

    size_t lenstr = strlen(str);
    size_t lensuffix = strlen(suffix);

    if (lensuffix > lenstr) {
        return false;
    }

    return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}
