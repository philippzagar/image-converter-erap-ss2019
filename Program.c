#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "BMPStructs.h"

// Assembly Functions
extern void greyscale(RGB* rgbValues, RGB* rgbValuesOut, int width, int height);
extern void blur(RGB* rgbValues, RGB* output, int width, int height);

// C Functions
BMPHeader* readHeader(FILE* inFile);
BMPImageInfo* readInfo(FILE* inFile);
bool checkBMPImage(BMPHeader* header, BMPImageInfo* info);
RGB* readPixels(FILE* inFile, BMPImageInfo* info);
bool writeImage(FILE* outFile, BMPHeader* header, BMPImageInfo* info, RGB* rgbValues);
void convertRGBtoGreyscale(RGB* rgbValues, BMPImageInfo* info);
void convolutionRGB(RGB* rgbValues, BMPImageInfo* info);

// Main Routine
int main(int argc, char** argv) {

    // Input File
    FILE *inFile, *outFile;

    // Getting file name
    /*char relativePath[100];
    printf("Enter the file name: ");
    scanf("%s", &relativePath[2]);

    relativePath[0] = '.';
    relativePath[1] = '/';
*/
    // Open the file with given file name
    //inFile = fopen(relativePath, "rb");

    inFile = fopen("./lena.bmp", "rb");

    // Checking for error while opening the file
    if( !inFile ) {
        printf("Error while opening file\n");
        return -1;
    }

    // Reading header and check validy (Magic Number) of BMP File
    BMPHeader* header = readHeader(inFile);
    if(!header) {
        printf("Error while reading header\n");
        return -1;
    }

    /*
    printf("%x", header.signature[0]);
    printf("\n");
    printf("%x", header.signature[1]);
    printf("\n");
    printf("%u", header.fileSize);
    printf("\n");
    printf("%08x", header.reserved);
    printf("\n");
    printf("%08x", header.offset);
    printf("\n");
    */

    // Reading image info of BMP file
    BMPImageInfo* info = readInfo(inFile);
    if(!info) {
        printf("Error while reading image info\n");
        return -1;
    }

    /*
    printf("%u", info.height);
    printf("\n");
    printf("%u", info.width);
    printf("\n");
    printf("%u", info.numColors);
    printf("\n");
     */

    // Check if BMP image is valid (no compression, etc.)
    if(!checkBMPImage(header, info)) {
        printf("Error - BMP image is not valid\n");
        return -1;
    }

    // Read RGB values from picture
    RGB* rgbValues = readPixels(inFile, info);
    if(!rgbValues) {
        printf("Error while reading RGB values from file\n");
        return -1;
    }

    // Convert to greyscale********************************************************************************************************************
    //greyscale(rgbValues, rgbValues, info->width, info->height);

    convertRGBtoGreyscale(rgbValues, info);
    //convolutionRGB(rgbValues, info);




    // Open file to write back the picture to memory
    outFile = fopen("./lena_grey.bmp", "wb");
    if( !outFile ) {
        printf("Error while opening file\n");
        return -1;
    }

    // Write image with header and info back to memory
    if(!writeImage(outFile, header, info, rgbValues)) {
        printf("Error while writing image to memory\n");
        return -1;
    }

    // Close open files
    fclose(inFile);
    fclose(outFile);

    // Clear allocated space
    free(header);
    free(info);
    free(rgbValues);

// Binary method => if read later by another computer
/*
    outFile = fopen("./lena_rgb", "wb");
    if( !outFile )
        return -1;

    if( fwrite(&info.numColors, sizeof(unsigned int), 1, outFile) != 1 )
        return -1; // Manage Error and close file

    if( fwrite(&palette, sizeof(RGB), info.numColors, outFile) != info.numColors )
        return -1; // Manage error and close file

    fclose(outFile);
*/
// Text method => if read later by human
/*
    outFile = fopen("path", "w");
    if( !outFile )
        return -1;

    for( i=0; i<info.numColors; ++i )
    {
        p = &palette[i];
        if( fprintf(outFile, "R:%d, G:%d, B:%d\n", p->red, p->green, p->blue) < 0 )
            return -1; // Manage error and close file
    }

    fclose(outFile);
*/
    return 0;
}

// Reading the Header of the BMP File
BMPHeader* readHeader(FILE* inFile) {
    /*
     * if( fread(&header, sizeof(BMPHeader), 1, inFile) != 1 )
        return -1; // Manage error and close file
        */

    BMPHeader* header = (BMPHeader*) malloc(sizeof(BMPHeader));

    // Read Header data
    if( fread(&header->signature[0], sizeof(char), 1, inFile) != 1  ||
        fread(&header->signature[1], sizeof(char), 1, inFile) != 1  ||
        fread(&header->fileSize, sizeof(unsigned int), 1, inFile) != 1 ||
        fread(&header->reserved, sizeof(unsigned int), 1, inFile) != 1  ||
        fread(&header->offset, sizeof(unsigned int), 1, inFile) != 1 )
    {
        return NULL; // Manage error and close file
    }

    // Check valid BMP picture
    if(header->signature[0] != 0x42 || header->signature[1] != 0x4D) {
        return NULL;
    }

    return header;
}

// Read Image Info from BMP File
BMPImageInfo* readInfo(FILE* inFile) {
    BMPImageInfo* info = (BMPImageInfo*) malloc(sizeof(BMPImageInfo));

    if( fread(info, sizeof(BMPImageInfo), 1, inFile) != 1 ) {
        return NULL; // Manage error and close file
    }

    return info;
}

// Check if BMP Image is valid
bool checkBMPImage(BMPHeader* header, BMPImageInfo* info) {
    // Check if the size of the header is correct -> header (14 byte) + info size is the offset, where the pixel data begins
    if(header->offset != 14 + info->headerSize) {
        printf("Error - Header size not correct!\n");
        return false;
    }

    // Check if there are multiple image planes
    if(info->planeCount == 0) {
        printf("Error - Multiple image planes!\n");
        return false;
    }

    // Check if the image is compressed
    if(info->compression != 0) {
        printf("Error - Compression is used!\n");
        return false;
    }

    // Check if numColors and importantColors is 0
    if(info->numColors != 0 && info->importantColors != 0) {
        printf("Error - Colors not correct!\n");
        return false;
    }

    // Check if Bits per pixel is 24
    if(info->bitDepth != 24) {
        printf("Error - Bits per pixel is not 24!\n");
        return false;
    }

    return true;
}

// Read RGB Pixels from BMP image
RGB* readPixels(FILE* inFile, BMPImageInfo* info) {
    // Allocate memory space for RGB array of pixels
    RGB *rgbValues = (RGB*) malloc(info->width * info->height * sizeof(RGB));

    // Read values
    for(int i = 0; i < info->height; i++) {
        //printf("%i\n", i);

        // Read the values from the file to the right position in the RGB array
        if( fread(rgbValues + (i * info->width), sizeof(RGB), info->width, inFile) != info->width ) {
            printf("Error - Reading RGB values!\n");
            return NULL;
        }

        // Alignment of every new row in the picture
        if(fseek(inFile, (info->width * sizeof(RGB)) % 4, SEEK_CUR) != 0) {
            printf("Error - Moving offset!\n");
            return NULL;
        }

    }

    // Output loop
    /*
    for(int x = 250000; x < 262144; x++) {
        printf("%i: ", x);
        printf("%02x ", rgbValues[x].blue);
        printf("%02x ", rgbValues[x].green);
        printf("%02x    ", rgbValues[x].red);
    }
    */

    return rgbValues;
}

// Write Image with the header
bool writeImage(FILE* outFile, BMPHeader* header, BMPImageInfo* info, RGB* rgbValues) {
    // Write Header
    if( fwrite(&header->signature, sizeof(char), 2, outFile) != 2 ) {
        return false;
    }

    if( fwrite(&header->fileSize, sizeof(unsigned int), 1, outFile) != 1 ) {
        return false;
    }

    if( fwrite(&header->reserved, sizeof(unsigned int), 1, outFile) != 1 ) {
        return false;
    }

    if( fwrite(&header->offset, sizeof(unsigned int), 1, outFile) != 1 ) {
        return false;
    }

    // Write Info
    if( fwrite(info, sizeof(BMPImageInfo), 1, outFile) != 1 ) {
        return false;
    }

    // Write RGB values row for row because of alignment
    for(int i = 0; i < info->height; i++) {
        if( fwrite(rgbValues + i * info->width, sizeof(RGB), info->width, outFile) != info->width ) {
            return false;
        }
    }

    return true;
}

// Convert RGB values to greyscale
void convertRGBtoGreyscale(RGB* rgbValues, BMPImageInfo* info) {
    // Constants
    float a = 0.3;
    float b = 0.3;
    float c = 0.3;
    float sum = a + b + c;

    // Loop to convert every pixel to greyscale
    for(int i = 0; i < info->height; i++) {
        for(int j = 0; j < info->width; j++) {
            // Round down the result because RGB can only have integer numbers
            int D = (a * rgbValues[i*info->height + j].red
                    + b * rgbValues[i*info->height + j].green
                    + c * rgbValues[i*info->height + j].blue)
                    / sum;

            // Set grey color to RGB pixel
            rgbValues[i*info->height + j].red = D;
            rgbValues[i*info->height + j].green = D;
            rgbValues[i*info->height + j].blue = D;
        }
    }
}

// Convert RGB values to greyscale
void convolutionRGB(RGB* rgbValues, BMPImageInfo* info) {
    // Constants
    char kernel[3][3] = {
            {1, 2, 1},
            {2, 4, 2},
            {1, 2, 1}
    };

    // Variables
    int sumRed = 0;
    int sumGreen = 0;
    int sumBlue = 0;
    char sumRedChar;
    char sumGreenChar;
    char sumBlueChar;

    // Loop every pixel of image
    for(int i = 0; i < info->height; i++) {
        for(int j = 0; j < info->width; j++) {
            // Calculate pointer to the current pixel
            RGB* pointer = rgbValues + (i * info->width) + (j);

            printf("i: %i j: %i         ", i, j);

            // Loop through kernel
            for(int kerneli = -1; kerneli <= 1; kerneli++) {
                for(int kernelj = -1; kernelj <= 1; kernelj++) {
                    // Check if the kernel is inside of the picture
                    if(i + kerneli >= 0 && j + kernelj >= 0 &&
                        i + kerneli < info->height && j + kernelj < info->width) {

                        printf("kerneli: %i kernelj: %i         ", kerneli, kernelj);

                        RGB* helpPointer = pointer + kerneli * info->width + kernelj;

                        // Sum up
                        sumRed += helpPointer->red * kernel[kerneli+1][kernelj+1];
                        sumGreen += helpPointer->green * kernel[kerneli+1][kernelj+1];
                        sumBlue += helpPointer->blue * kernel[kerneli+1][kernelj+1];
                    }
                }
            }

            // Devide
            sumRed /= 16;
            sumGreen /= 16;
            sumBlue /= 16;

            // Parse int to char
            sumRedChar = sumRed;
            sumGreenChar = sumRed;
            sumBlueChar = sumRed;

            // Write back to RGB value
            pointer->red = sumRedChar;
            pointer->green = sumGreenChar;
            pointer->blue = sumBlueChar;

            // Set sums to 0
            sumRed = 0; sumGreen = 0; sumBlue = 0;
        }
    }
}
