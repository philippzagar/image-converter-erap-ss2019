#include "BMP_functions.h"

#include "shared_global_variables.h"

// Reading the Header of the BMP File
BMPHeader* readHeader(FILE* inFile) {
    BMPHeader* header = (BMPHeader*) malloc(sizeof(BMPHeader));

    if(!header) {
        printf("Error allocation new memory!\n");
        return NULL;
    }

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

    if(!info) {
        printf("Error allocation new memory!\n");
        return NULL;
    }

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
        printf("Header size is %d byte\n", info->headerSize);
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

    // If picture is really large
    if(info->width > 20000 || info->height > 20000) {
        printf("Error - Picture is too large!\n");
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

    if(!rgbValues) {
        printf("Error allocation new memory!\n");
        return NULL;
    }

    // Moving offset of the info header because the size can vary
    if(info->headerSize != 40) {
        printf("Moving header %d byte\n", info->headerSize - 40);

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
        printf("Moving header %d byte\n", info->headerSize - 40);

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

// Converts the RGB values to a different data model in memory, because SIMD in assembly needs it
// Color is of type word
// Memory model rrrrr/ggggg/bbbbbb
RGBcolorWord* convertRGBtoSIMDWord(RGB* rgbValues) {
    long countPixels = global_image_width * global_image_height;

    // Allocate new array for all RGB values (color is a word)
    RGBcolorWord* rgbNewValues = (RGBcolorWord*) malloc(3 * (countPixels * 2 * sizeof(unsigned char)));

    if(!rgbNewValues) {
        printf("Error allocation new memory!\n");
        return NULL;
    }

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

    if(!rgbNewValues) {
        printf("Error allocation new memory!\n");
        return NULL;
    }

    // Convert the Word values back to the normal RGB format, where a color is a char
    for(long i = 0; i < countPixels; i++) {
        rgbNewValues[i].red = rgbValues[i].color;
        rgbNewValues[i].green = rgbValues[i + countPixels].color;
        rgbNewValues[i].blue = rgbValues[i + 2*countPixels].color;
    }

    return rgbNewValues;
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

    if(!convolutionRGBValues) {
        printf("Error allocation new memory!\n");
        return NULL;
    }

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

