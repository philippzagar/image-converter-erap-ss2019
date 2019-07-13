#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <setjmp.h>
// Libjpeg Library
//#include "./libjpeg-master/jpeglib.h"

#include "BMPStructs.h"

/***** Assembly Functions ******/
// Greyscale
extern void greyscale(RGB *rgbValuesOut, int width, int height);
extern void greyscale_simd(RGB* out, int width, int height); // Runs with greyscale for now

// Blur
extern void blur(RGB* in, RGB* out, int width, int height);
extern void blur_colour(RGB* in, RGB* out, int width, int height);

/***** C Functions ******/
// BMP Functions
BMPHeader* readHeader(FILE* inFile);
BMPImageInfo* readInfo(FILE* inFile);
bool checkBMPImage(BMPHeader* header, BMPImageInfo* info);
RGB* readBMPImage(FILE* inFile, BMPImageInfo* info);
bool writeBMPImage(FILE* outFile, BMPHeader* header, BMPImageInfo* info, RGB* rgbValues);
// JPG Functions
RGB* read_JPEG_file (FILE* infile);
void save_scanline(unsigned char* buffer, RGB* rgbValues, int actualHeight);
bool write_JPEG_file (FILE* outfile, int quality, RGB* rgbValues);
// RGB Conversion Functions
RGBcolorWord* convertRGBtoSIMDWord(RGB* rgbValues);
RGB* convertSIMDWordtoRGB(RGBcolorWord* rgbValues);
RGBcolorByte* convertRGBtoSIMDByte(RGB* rgbValues);
RGB* convertSIMDBytetoRGB(RGBcolorByte* rgbValues);
// Additional Functions
bool endsWith(char *str, char *suffix);

// C implemented Greyscale and Convolution
RGB* convertRGBtoGreyscale(RGB* rgbValues);
RGB* convolutionRGB(RGB* rgbValues);

// Global variables
unsigned int global_image_height;
unsigned int global_image_width;
//J_COLOR_SPACE global_colorSpace;

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

    /*
    // Read File Name from User
    printf("Enter the file name: ");
    !!!! fgets verwenden !!!!
    fgets(&relativePath[2], 100, stdin);
    //scanf("%s", &relativePath[2]);

    // Make Filename to relative path
    relativePath[0] = '.';
    relativePath[1] = '/';
    */

    // Just for testing
    strcpy(relativePath, "./lena.bmp");

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

    // JPG File
    if(isJPG) {
        /*
        // Read JPG File
        rgbValues = read_JPEG_file(inFile);

        // Check for error while reading the JPG File
        if(!rgbValues) {
            printf("Error while reading JPG file\n");
            return -1;
        }
         */
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

    // Assembly Functions
    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // Convert to SIMD data model in memory

    RGBcolorWord* rgbSIMD = convertRGBtoSIMDWord(rgbValues);

    greyscale_simd(rgbSIMD, info->width, info->height);

    RGB* rgb = convertSIMDWordtoRGB(rgbSIMD);

    free(rgbValues);

    rgbValues = rgb;



    // Convert to greyscale********************************************************************************************************************
    //greyscale(rgbValues, info->width, info->height);


    // For Blur *******************************************************************************************************************************
    // Allocate new memory space for blur, because else the algorithm doesnt work proprly
    //RGB* out = (RGB*) malloc(global_image_width * global_image_height * sizeof(RGB));
    //blur(rgbValues, out, global_image_width, global_image_height);

    // Free old rgbValues



    // For Flo***with rrrgggbbb*************************************************************************************************************************************
    // RGBcolorWord* rgbSIMD = convertRGBtoSIMDWord(rgbValues);
    // blur_colour (rgbSIMD, rgbSIMD, info->width, info->height);
    //
    // RGB* rgb = convertSIMDWordtoRGB(rgbSIMD);
    // free(rgbValues);
    //
    // rgbValues = rgb;


    // RGB* out = (RGB*) malloc(global_image_width * global_image_height * sizeof(RGB));
    //
    //
    // blur_colour (rgbValues, out, info->width, info->height);
    //
    //
    //
    // free(rgbValues);
    //
    // rgbValues = out;





    // Set the new values to the old pointer


    //RGB* out = (RGB*) malloc(info->width * info->height * sizeof(RGB));
    //blur(rgbValues, out, info->width, info->height);

    //free(rgbValues);

    //rgbValues = out;

    //rgbValues = convolutionRGB(rgbValues, info);


    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    // JPG File
    if(isJPG) {
        /*
        // Open output file
        outFile = fopen("./test2.jpg", "wb");

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

        */
    // BMP File
    } else {
        // Open file to write back the picture to memory
        outFile = fopen("./lena_grey.bmp", "wb");
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

// Reading the Header of the BMP File
BMPHeader* readHeader(FILE* inFile) {
    BMPHeader* header = (BMPHeader*) malloc(sizeof(BMPHeader));

    // Read Header data one by one because of alignment
    if( fread(&header->signature[0], sizeof(char), 1, inFile) != 1  ||
        fread(&header->signature[1], sizeof(char), 1, inFile) != 1  ||
        fread(&header->fileSize, sizeof(unsigned int), 1, inFile) != 1 ||
        fread(&header->reserved, sizeof(unsigned int), 1, inFile) != 1  ||
        fread(&header->offset, sizeof(unsigned int), 1, inFile) != 1 )
    {
        return NULL;
    }

    return header;
}

// Read Image Info from BMP File
BMPImageInfo* readInfo(FILE* inFile) {
    BMPImageInfo* info = (BMPImageInfo*) malloc(sizeof(BMPImageInfo));

    if( fread(info, sizeof(BMPImageInfo), 1, inFile) != 1 ) {
        return NULL;
    }

    return info;
}

// Check if BMP Image is valid
bool checkBMPImage(BMPHeader* header, BMPImageInfo* info) {
    // Check valid BMP picture
    if(header->signature[0] != 0x42 || header->signature[1] != 0x4D) {
        return false;
    }

    // Check if the header size is correct, we assume the header is a 40 byte info header
    if(info->headerSize != 40) {
        //printf("Error - Header size not correct!\n");
        printf("Header size is %d\n", info->headerSize);
    }

    // Check if the size of the header is correct -> header (14 byte) + info size is the offset, where the pixel data begins
    if(header->offset != 14 + info->headerSize) {
        printf("Error - Header size not correct!\n");
        return false;
    }

    // Check if there are multiple image planes
    if(info->planeCount != 1) {
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

    // Set global variables
    global_image_width = info->width;
    global_image_height = info->height;

    return true;
}

// Read RGB Pixels from BMP image
RGB* readBMPImage(FILE* inFile, BMPImageInfo* info) {
    // Allocate memory space for RGB array of pixels
    RGB *rgbValues = (RGB*) malloc(global_image_width * global_image_height * sizeof(RGB));

    // Moving offset of the info header because the size can vary
    if(info->headerSize != 40) {
        printf("Moving header %d\n", info->headerSize - 40);

        if(fseek(inFile, info->headerSize - 40, SEEK_CUR) != 0) {
            printf("Error - Moving offset of header!\n");
            return NULL;
        }
    }

    // Alignment of every new row in the picture
    int alignment;

    // Calculate alignment
    if(((global_image_width * sizeof(RGB)) % 4) == 0) {
        alignment = 0;
    } else {
        alignment = 4 - (global_image_width * sizeof(RGB)) % 4;
    }

    // Read values
    for(unsigned int i = 0; i < global_image_height; i++) {
        // Read the values from the file to the right position in the RGB array
        if( fread(rgbValues + (i * global_image_width), sizeof(RGB), global_image_width, inFile) != global_image_width ) {
            printf("Error - Reading RGB values!\n");
            return NULL;
        }

        // Moving read pointer because of alignment
        if(fseek(inFile, alignment, SEEK_CUR) != 0) {
            printf("Error - Moving offset!\n");
            return NULL;
        }

    }

    return rgbValues;
}

// Write Image with the header
bool writeBMPImage(FILE* outFile, BMPHeader* header, BMPImageInfo* info, RGB* rgbValues) {
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

    // Moving offset of the info header because the size can vary
    if(info->headerSize != 40) {
        printf("Moving header %d\n", info->headerSize - 40);

        if(fseek(outFile, info->headerSize - 40, SEEK_CUR) != 0) {
            printf("Error - Moving offset of header!\n");
            return false;
        }
    }

    // Alignment of every new row in the picture
    int alignment;

    if(((info->width * sizeof(RGB)) % 4) == 0) {
        alignment = 0;
    } else {
        alignment = 4 - (global_image_width * sizeof(RGB)) % 4;
    }

    // Write RGB values row for row because of alignment
    for(unsigned int i = 0; i < global_image_height; i++) {
        if( fwrite(rgbValues + i * global_image_width, sizeof(RGB), global_image_width, outFile) != global_image_width ) {
            return false;
        }

        // Move write pointer because of alignment
        if(i != info->height-1) {
            if(fseek(outFile, alignment, SEEK_CUR) != 0) {
                printf("Error - Moving offset!\n");
                return false;
            }
        }
    }

    // Add last alignment of file
    char zero = 0;
    for(int i = 0; i < alignment; i++) {
        if( fwrite(&zero, 1, 1, outFile) != 1 ) {
            return false;
        }
    }

    return true;
}

// Convert RGB values to greyscale
RGB* convertRGBtoGreyscale(RGB* rgbValues) {
    // Constants
    float a = 0.3;
    float b = 0.3;
    float c = 0.3;
    float sum = a + b + c;

    // Loop to convert every pixel to greyscale
    for(unsigned int i = 0; i < global_image_height; i++) {
        for(unsigned int j = 0; j < global_image_width; j++) {
            // Round down the result because RGB can only have integer numbers
            int D = (a * rgbValues[i*global_image_width + j].red
                    + b * rgbValues[i*global_image_width + j].green
                    + c * rgbValues[i*global_image_width + j].blue)
                    / sum;

            // Set grey color to RGB pixel
            rgbValues[i*global_image_width + j].red = D;
            rgbValues[i*global_image_width + j].green = D;
            rgbValues[i*global_image_width + j].blue = D;
        }
    }

    return rgbValues;
}

// Convert RGB values to greyscale
RGB* convolutionRGB(RGB* rgbValues) {
    // Constants
    char kernel[3][3] = {
            {1, 2, 1},
            {2, 4, 2},
            {1, 2, 1}
    };

    // Allocate new RGB array -> Values of old array are not overwritten (because of needing those pixels for convolution)
    RGB* convolutionRGBValues = (RGB*) malloc(global_image_width * global_image_height * sizeof(RGB));

    // Variables
    int sumRed = 0;
    int sumGreen = 0;
    int sumBlue = 0;
    char sumRedChar;
    char sumGreenChar;
    char sumBlueChar;

    // Loop every pixel of image
    for(unsigned int currentHeigh = 0; currentHeigh < global_image_height; currentHeigh++) {
        for(unsigned int currentWidth = 0; currentWidth < global_image_width; currentWidth++) {

            int helpPointer = (currentHeigh*global_image_width + currentWidth);

           // Loop through kernel
            for(int kerneli = -1; kerneli <= 1; kerneli++) {
                for(int kernelj = -1; kernelj <= 1; kernelj++) {
                    // Check if the kernel is inside of the picture
                    if(currentHeigh + kerneli >= 0 && currentWidth + kernelj >= 0 && // Skip the pixel otherwise --> value of 0
                       currentHeigh + kerneli < global_image_height && currentWidth + kernelj < global_image_width) {

                        int pixelNavigator = helpPointer +  kerneli*global_image_width + kernelj;

                        // Sum up
                        sumRed += rgbValues[pixelNavigator].red * kernel[kerneli+1][kernelj+1];
                        sumGreen += rgbValues[pixelNavigator].green * kernel[kerneli+1][kernelj+1];
                        sumBlue += rgbValues[pixelNavigator].blue * kernel[kerneli+1][kernelj+1];
                    }
                }
            }

            // Devide for average
            sumRed /= 16;
            sumGreen /= 16;
            sumBlue /= 16;

            // Parse int to char
            sumRedChar = sumRed;
            sumGreenChar = sumGreen;
            sumBlueChar = sumBlue;

            // Write back to RGB value
            convolutionRGBValues[helpPointer].red = sumRedChar;
            convolutionRGBValues[helpPointer].green = sumGreenChar;
            convolutionRGBValues[helpPointer].blue = sumBlueChar;

            // Set sums to 0
            sumRed = 0; sumGreen = 0; sumBlue = 0;
        }
    }

    return convolutionRGBValues;
}

// Converts the RGB values to a different data model in memory, because SIMD in assembly needs it
// Color is of type word
// Memory model rrrrr/ggggg/bbbbbb
RGBcolorWord* convertRGBtoSIMDWord(RGB* rgbValues) {
    long countPixels = global_image_width * global_image_height;

    // Allocate new array for all RGB values (color is a word)
    RGBcolorWord* rgbNewValues = (RGBcolorWord*) malloc(3 * (countPixels * 2 * sizeof(unsigned char)));

    // Write Pixels to the new array, where a color is a word
    for(long i = 0; i < countPixels; i++) {
        rgbNewValues[i].color = rgbValues[i].red;
        rgbNewValues[i + countPixels].color = rgbValues[i].green;
        rgbNewValues[i + 2*countPixels].color = rgbValues[i].blue;
    }

    return rgbNewValues;
}

// Converts the SIMD Word RGB values to normal RGB values
RGB* convertSIMDWordtoRGB(RGBcolorWord* rgbValues) {
    long countPixels = global_image_width * global_image_height;

    // Allocate new array for all RGB values
    RGB* rgbNewValues = (RGB*) malloc(countPixels * sizeof(RGB));

    // Convert the Word values back to the normal RGB format, where a color is a char
    for(long i = 0; i < countPixels; i++) {
        rgbNewValues[i].red = rgbValues[i].color;
        rgbNewValues[i].green = rgbValues[i + countPixels].color;
        rgbNewValues[i].blue = rgbValues[i + 2*countPixels].color;
    }

    return rgbNewValues;
}

// Converts the RGB values to a different data model in memory, because SIMD in assembly needs it
// Color is of type byte
// Memory model rrrrr/ggggg/bbbbbb
RGBcolorByte* convertRGBtoSIMDByte(RGB* rgbValues) {
    long countPixels = global_image_width * global_image_height;

    // Allocate new array for all RGB values
    RGBcolorByte* rgbNewValues = (RGBcolorByte*) malloc(3 * (countPixels * sizeof(unsigned char)));

    // Write Pixels to the new array, where a color is a byte
    for(long i = 0; i < countPixels; i++) {
        rgbNewValues[i].color = rgbValues[i].red;
        rgbNewValues[i + countPixels].color = rgbValues[i].green;
        rgbNewValues[i + 2*countPixels].color = rgbValues[i].blue;
    }

    return rgbNewValues;
}

// Converts the SIMD Byte values to normal RGB values
RGB* convertSIMDBytetoRGB(RGBcolorByte* rgbValues) {
    long countPixels = global_image_width * global_image_height;

    // Allocate new array for all RGB values
    RGB* rgbNewValues = (RGB*) malloc(countPixels * sizeof(RGB));

    // Convert the byte values back to the normal RGB format, where a color is a char
    for(long i = 0; i < countPixels; i++) {
        rgbNewValues[i].red = rgbValues[i].color;
        rgbNewValues[i].green = rgbValues[i + countPixels].color;
        rgbNewValues[i].blue = rgbValues[i + 2*countPixels].color;
    }

    return rgbNewValues;
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

// JPEG stuff
// #include "./JPG_stuff.c"

// -ljpeg
