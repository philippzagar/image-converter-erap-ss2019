#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <setjmp.h>

//#include "./libjpeg-master/jpeglib.h"

#include "BMPStructs.h"

// Assembly Functions
extern void greyscale(RGB *rgbValuesOut, int width, int height);
extern void greyscale_simd(RGB* out, int width, int height); // Runs with greyscale for now

extern void blur(RGB* in, RGB* out, int width, int height);

// C Functions
BMPHeader* readHeader(FILE* inFile);
BMPImageInfo* readInfo(FILE* inFile);
bool checkBMPImage(BMPHeader* header, BMPImageInfo* info);
RGB* readPixels(FILE* inFile, BMPImageInfo* info);
bool writeImage(FILE* outFile, BMPHeader* header, BMPImageInfo* info, RGB* rgbValues);
RGB* convertRGBtoGreyscale(RGB* rgbValues, BMPImageInfo* info);
RGB* convolutionRGB(RGB* rgbValues, BMPImageInfo* info);
bool endsWith(char *str, char *suffix);
RGBcolorWord* convertRGBtoSIMDWord(RGB* rgbValues, BMPImageInfo* info);
RGBcolorByte* convertRGBtoSIMDByte(RGB* rgbValues, BMPImageInfo* info);

RGB* read_JPEG_file (FILE* infile);
void save_scanline(unsigned char* buffer, RGB* rgbValues, int actualHeight);

bool write_JPEG_file (FILE* outfile, int quality, RGB* rgbValues);

// Global variables
int global_image_height;
int global_image_width;
//J_COLOR_SPACE global_colorSpace;

// Main Routine
int main(int argc, char** argv) {

    // Input File
    FILE *inFile, *outFile;
    RGB* rgbValues;
    BMPHeader* header;
    BMPImageInfo* info;

    // Getting file name
    char relativePath[100];

    /*
    printf("Enter the file name: ");
    scanf("%s", &relativePath[2]);

    relativePath[0] = '.';
    relativePath[1] = '/';
    */

    // Just for testing
    strcpy(relativePath, "./lena.bmp");

    // Open the file with given file name
    inFile = fopen(relativePath, "rb");

    // Checking for error while opening the file
    if( !inFile ) {
        printf("Error while opening file\n");
        return -1;
    }

    bool isJPG = endsWith(relativePath, ".jpg") || endsWith(relativePath, ".JPG");

    // JPG File
    if(isJPG) {
        /*
        rgbValues = read_JPEG_file(inFile);

        if(!rgbValues) {
            printf("Error while reading JPG file\n");
            return -1;
        }
         */
    } else {
        // Reading header of BMP File
        header = readHeader(inFile);
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
        info = readInfo(inFile);
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
        rgbValues = readPixels(inFile, info);
        if(!rgbValues) {
            printf("Error while reading RGB values from file\n");
            return -1;
        }
    }


    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // Convert to SIMD data model in memory
    RGBcolorWord* rgbSIMD = convertRGBtoSIMDWord(rgbValues, info);


    // Convert to greyscale********************************************************************************************************************
    //greyscale(rgbValues, info->width, info->height);




    // For Blur *******************************************************************************************************************************
    // Allocate new memory space for blur, because else the algorithm doesnt work proprly
    RGB* out = (RGB*) malloc(info->width * info->height * sizeof(RGB));
    blur(rgbValues, out, info->width, info->height);

    // Free old rgbValues
    free(rgbValues);

    // Set the new values to the old pointer
    rgbValues = out;

    //RGB* out = (RGB*) malloc(info->width * info->height * sizeof(RGB));
    //blur(rgbValues, out, info->width, info->height);

    //free(rgbValues);

    //rgbValues = out;

    //rgbValues = convolutionRGB(rgbValues, info);


    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    if(isJPG) {
        /*
        outFile = fopen("./test2.jpg", "wb");

        if (!outFile) {
            printf("Error while opening JPG file to write\n");
            return -1;
        }

        if(!write_JPEG_file(outFile, 80, rgbValues)) {
            printf("Error while writing JPG file\n");
            return -1;
        }

         */
        /*
         * Write as a BMP image
        BMPHeader* header = (BMPHeader*) malloc(sizeof(BMPHeader));
        header->signature[0] = 0x42;
        header->signature[1] = 0x4D;

        int temp = 0;
        if(global_image_width * sizeof(RGB) % 4 != 0) {
            temp = 4 - (global_image_width * sizeof(RGB) % 4);
            temp *= global_image_height;
        }

        header->fileSize = 54 + global_image_width * global_image_height * sizeof(RGB) + temp;
        header->offset = 54;

        BMPImageInfo* info = (BMPImageInfo*) malloc(sizeof(BMPImageInfo));

        info->width = global_image_width;
        info->height = global_image_height;
        info->headerSize = 40;
        info->bitDepth = 24;
        info->importantColors = 0;
        info->numColors = 0;
        info->compression = 0;
        info->planeCount = 1;
        info->compressedImageSize = global_image_width * global_image_height * sizeof(RGB) + temp;
        info->horizontalResolution = 72;
        info->verticalResolution = 72;

        writeImage(fopen("./test2.bmp", "wb"), header, info, rgbVal);
        */// BMP File
    } else {
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
    }

    // Close open files
    fclose(inFile);
    fclose(outFile);

    // Clear allocated space
    if(header) {
        free(header);
    }

    if(info) {
        free(info);
    }

    if(rgbValues) {
        free(rgbValues);
    }

    return 0;
}

// Reading the Header of the BMP File
BMPHeader* readHeader(FILE* inFile) {
    /*
     * if( fread(&header, sizeof(BMPHeader), 1, inFile) != 1 )
        return -1; // Manage error and close file
        */

    BMPHeader* header = (BMPHeader*) malloc(sizeof(BMPHeader));

    // Read Header data one by one because of alignment
    if( fread(&header->signature[0], sizeof(char), 1, inFile) != 1  ||
        fread(&header->signature[1], sizeof(char), 1, inFile) != 1  ||
        fread(&header->fileSize, sizeof(unsigned int), 1, inFile) != 1 ||
        fread(&header->reserved, sizeof(unsigned int), 1, inFile) != 1  ||
        fread(&header->offset, sizeof(unsigned int), 1, inFile) != 1 )
    {
        return NULL; // Manage error and close file
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
    // Check valid BMP picture
    if(header->signature[0] != 0x42 || header->signature[1] != 0x4D) {
        return false;
    }

    // Check if the header size is correct, we assume the header is a 40 byte info header
    if(info->headerSize != 40) {
        printf("Error - Header size not correct!\n");
        printf("Header size is %d\n", info->headerSize);
        //return false;
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

    return true;
}

// Read RGB Pixels from BMP image
RGB* readPixels(FILE* inFile, BMPImageInfo* info) {
    // Allocate memory space for RGB array of pixels
    RGB *rgbValues = (RGB*) malloc(info->width * info->height * sizeof(RGB));

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

    if(((info->width * sizeof(RGB)) % 4) == 0) {
        alignment = 0;
    } else {
        alignment = 4 - (info->width * sizeof(RGB)) % 4;
    }

    // Read values
    for(unsigned int i = 0; i < info->height; i++) {
        // Read the values from the file to the right position in the RGB array
        if( fread(rgbValues + (i * info->width), sizeof(RGB), info->width, inFile) != info->width ) {
            printf("Error - Reading RGB values!\n");
            return NULL;
        }

        if(fseek(inFile, alignment, SEEK_CUR) != 0) {
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
        alignment = 4 - (info->width * sizeof(RGB)) % 4;
    }

    // Write RGB values row for row because of alignment
    for(unsigned int i = 0; i < info->height; i++) {
        if( fwrite(rgbValues + i * info->width, sizeof(RGB), info->width, outFile) != info->width ) {
            return false;
        }

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
RGB* convertRGBtoGreyscale(RGB* rgbValues, BMPImageInfo* info) {
    // Constants
    float a = 0.3;
    float b = 0.3;
    float c = 0.3;
    float sum = a + b + c;

    // Loop to convert every pixel to greyscale
    for(unsigned int i = 0; i < info->height; i++) {
        for(unsigned int j = 0; j < info->width; j++) {
            // Round down the result because RGB can only have integer numbers
            int D = (a * rgbValues[i*info->width + j].red
                    + b * rgbValues[i*info->width + j].green
                    + c * rgbValues[i*info->width + j].blue)
                    / sum;

            // Set grey color to RGB pixel
            rgbValues[i*info->width + j].red = D;
            rgbValues[i*info->width + j].green = D;
            rgbValues[i*info->width + j].blue = D;
        }
    }

    return rgbValues;
}

// Convert RGB values to greyscale
RGB* convolutionRGB(RGB* rgbValues, BMPImageInfo* info) {
    // Constants
    char kernel[3][3] = {
            {1, 2, 1},
            {2, 4, 2},
            {1, 2, 1}
    };

    // Allocate new RGB array -> Values of old array are not overwritten (because of needing those pixels for convolution)
    RGB* convolutionRGBValues = (RGB*) malloc(info->width * info->height * sizeof(RGB));

    // Variables
    int sumRed = 0;
    int sumGreen = 0;
    int sumBlue = 0;
    char sumRedChar;
    char sumGreenChar;
    char sumBlueChar;

    // Loop every pixel of image
    for(unsigned int currentHeigh = 0; currentHeigh < info->height; currentHeigh++) {
        for(unsigned int currentWidth = 0; currentWidth < info->width; currentWidth++) {

            int helpPointer = (currentHeigh*info->width + currentWidth);

           // Loop through kernel
            for(int kerneli = -1; kerneli <= 1; kerneli++) {
                for(int kernelj = -1; kernelj <= 1; kernelj++) {
                    // Check if the kernel is inside of the picture
                    if(currentHeigh + kerneli >= 0 && currentWidth + kernelj >= 0 && // Skip the pixel otherwise --> value of 0
                       currentHeigh + kerneli < info->height && currentWidth + kernelj < info->width) {

                        int pixelNavigator = helpPointer +  kerneli*info->width + kernelj;

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
RGBcolorWord* convertRGBtoSIMDWord(RGB* rgbValues, BMPImageInfo* info) {
    long countPixels = info->width * info->height;

    // Allocate memory space for each color of RGB array of pixels - 2 byte for one color -> SIMD
    RGBcolorWord *rgbNewValuesRed = (RGBcolorWord*) malloc(countPixels * 2 * sizeof(unsigned char));
    RGBcolorWord *rgbNewValuesGreen = (RGBcolorWord*) malloc(countPixels * 2 * sizeof(unsigned char));
    RGBcolorWord *rgbNewValuesBlue = (RGBcolorWord*) malloc(countPixels * 2 * sizeof(unsigned char));

    // Allocate new array for all RGB values
    RGBcolorWord *rgbNewValues = (RGBcolorWord*) malloc(3 * countPixels * 2 * sizeof(unsigned char));

    for(long i = 0; i < countPixels; i++) {
        rgbNewValuesRed[i].color = rgbValues[i].red;
        rgbNewValuesGreen[i].color = rgbValues[i].green;
        rgbNewValuesBlue[i].color = rgbValues[i].blue;
    }

    // Copy the single color arrays into one final array -> rrrrrrr/gggggggg/bbbbbbb
    memcpy(rgbNewValues, rgbNewValuesRed, countPixels * 2 * sizeof(unsigned char));
    memcpy(rgbNewValues + (2 * sizeof(unsigned char) * countPixels), rgbNewValuesGreen, countPixels * 2 * sizeof(unsigned char));
    memcpy(rgbNewValues + 2 * (2 * sizeof(unsigned char) * countPixels), rgbNewValuesBlue, countPixels * 2 * sizeof(unsigned char));

    return rgbNewValues;
}

// Converts the SIMD Word RGB values to normal RGB values
RGB* convertSIMDWordtoRGB(RGBcolorWord* rgbValues, BMPImageInfo* info) {
    // Allocate new array for all RGB values
    RGB* rgbNewValues = (RGB*) malloc(info->width * info->height * sizeof(RGB));

    RGBcolorWord* redPointer = rgbValues;
    RGBcolorWord* greenPointer = rgbValues + (info->width * info->height * 2 * sizeof(unsigned char));
    RGBcolorWord* bluePointer = rgbValues + 2 * (info->width * info->height * 2 * sizeof(unsigned char));

    for(long i = 0; i < info->width * info->height; i++) {
        rgbNewValues[i].red = redPointer[i].color;
        rgbNewValues[i].green = greenPointer[i].color;
        rgbNewValues[i].blue = bluePointer[i].color;
    }

    return rgbNewValues;
}

// Converts the RGB values to a different data model in memory, because SIMD in assembly needs it
RGBcolorByte* convertRGBtoSIMDByte(RGB* rgbValues, BMPImageInfo* info) {
    long countPixels = info->width * info->height;

    // Allocate memory space for each color of RGB array of pixels - 2 byte for one color -> SIMD
    RGBcolorByte *rgbNewValuesRed = (RGBcolorByte*) malloc(countPixels * sizeof(unsigned char));
    RGBcolorByte *rgbNewValuesGreen = (RGBcolorByte*) malloc(countPixels * sizeof(unsigned char));
    RGBcolorByte *rgbNewValuesBlue = (RGBcolorByte*) malloc(countPixels * sizeof(unsigned char));

    // Allocate new array for all RGB values
    RGBcolorByte *rgbNewValues = (RGBcolorByte*) malloc(3 * countPixels * sizeof(unsigned char));

    for(long i = 0; i < countPixels; i++) {
        rgbNewValuesRed[i].color = rgbValues[i].red;
        rgbNewValuesGreen[i].color = rgbValues[i].green;
        rgbNewValuesBlue[i].color = rgbValues[i].blue;
    }

    // Copy the single color arrays into one final array -> rrrrrrr/gggggggg/bbbbbbb
    memcpy(rgbNewValues, rgbNewValuesRed, countPixels * sizeof(unsigned char));
    memcpy(rgbNewValues + (sizeof(unsigned char) * countPixels), rgbNewValuesGreen, countPixels * sizeof(unsigned char));
    memcpy(rgbNewValues + 2 * (sizeof(unsigned char) * countPixels), rgbNewValuesBlue, countPixels * sizeof(unsigned char));

    return rgbNewValues;
}

// Converts the SIMD Byte values to normal RGB values
RGB* convertSIMDBytetoRGB(RGBcolorByte* rgbValues, BMPImageInfo* info) {
    // Allocate new array for all RGB values
    RGB* rgbNewValues = (RGB*) malloc(info->width * info->height * sizeof(RGB));

    RGBcolorByte* redPointer = rgbValues;
    RGBcolorByte* greenPointer = rgbValues + (info->width * info->height * sizeof(unsigned char));
    RGBcolorByte* bluePointer = rgbValues + 2 * (info->width * info->height * sizeof(unsigned char));

    for(long i = 0; i < info->width * info->height; i++) {
        rgbNewValues[i].red = redPointer[i].color;
        rgbNewValues[i].green = greenPointer[i].color;
        rgbNewValues[i].blue = bluePointer[i].color;
    }

    return rgbNewValues;
}

// Check if string ends with a certain suffix
bool endsWith(char *str, char *suffix)
{
    if (!str || !suffix)
        return false;
    size_t lenstr = strlen(str);
    size_t lensuffix = strlen(suffix);
    if (lensuffix > lenstr)
        return false;
    return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}

// #include "./JPG_stuff.c"

// -ljpeg