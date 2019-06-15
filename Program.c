#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "BMPStructs.h"

// Assembly Functions
extern void greyscale(FILE* in, FILE* out, int width, int height);
extern void blur(FILE* in, FILE* out, int width, int height);

// C Functions
bool readHeader(FILE* inFile, BMPHeader* header);
bool readInfo(FILE* inFile, BMPImageInfo* info);
bool checkBMPImage(BMPHeader header, BMPImageInfo info);
RGB* readPixels(FILE* inFile, BMPImageInfo info);
bool writeImage(FILE* outFile, BMPHeader header, BMPImageInfo info, RGB *rgbValues);
void convertRGBtoGreyscale(RGB *rgbValues, BMPImageInfo info);

// Main Routine
int main(int argc, char** argv) {

    // Input File
    FILE *inFile, *outFile;
    BMPHeader header;
    BMPImageInfo info;
    RGB *palette, *p;
    int i = 0;

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
    if(!readHeader(inFile, &header)) {
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
    if(!readInfo(inFile, &info)) {
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
        return -1;
    }

    // Read RGB values from picture
    RGB* rgbValues = readPixels(inFile, info);
    if(!rgbValues) {
        return -1;
    }

    // Convert to greyscale
    convertRGBtoGreyscale(rgbValues, info);

    // Open file to write back the picture to memory
    outFile = fopen("./lena_grey.bmp", "wb");
    if( !outFile ) {
        return -1;
    }

    // Write image with header and info back to memory
    if(!writeImage(outFile, header, info, rgbValues)) {
        return -1;
    }

    /*
    if( info.numColors > 0 )
    {
        palette = (RGB*)malloc(sizeof(RGB) * info.numColors);
        if( fread(palette, sizeof(RGB), info.numColors, inFile) != info.numColors )
            return -1; // manage error and close file
    }
    */

    // Close open files
    fclose(inFile);
    fclose(outFile);


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
bool readHeader(FILE* inFile, BMPHeader* header) {
    /*
     * if( fread(&header, sizeof(BMPHeader), 1, inFile) != 1 )
        return -1; // Manage error and close file
        */

    // Read Header data
    if( fread(&header->signature[0], sizeof(char), 1, inFile) != 1  ||
        fread(&header->signature[1], sizeof(char), 1, inFile) != 1  ||
        fread(&header->fileSize, sizeof(unsigned int), 1, inFile) != 1 ||
        fread(&header->reserved, sizeof(unsigned int), 1, inFile) != 1  ||
        fread(&header->offset, sizeof(unsigned int), 1, inFile) != 1 )
    {
        return false; // Manage error and close file
    }

    // Check valid BMP picture
    if(header->signature[0] != 0x42 || header->signature[1] != 0x4D) {
        return false;
    }

    return true;
}

// Read Image Info from BMP File
bool readInfo(FILE* inFile, BMPImageInfo* info) {
    if( fread(info, sizeof(BMPImageInfo), 1, inFile) != 1 ) {
        return false; // Manage error and close file
    }
}

// Check if BMP Image is valid
bool checkBMPImage(BMPHeader header, BMPImageInfo info) {
    // Check if the size of the header is correct -> header (14 byte) + info size is the offset, where the pixel data begins
    if(header.offset != 14 + info.headerSize) {
        printf("Error - Header size not correct!\n");
        return false;
    }

    // Check if there are multiple image planes
    if(info.planeCount == 0) {
        printf("Error - Multiple image planes!\n");
        return false;
    }

    // Check if the image is compressed
    if(info.compression != 0) {
        printf("Error - Compression is used!\n");
        return false;
    }

    // Check if numColors and importantColors is 0
    if(info.numColors != 0 && info.importantColors != 0) {
        printf("Error - Colors not correct!\n");
        return false;
    }

    // Check if Bits per pixel is 24
    if(info.bitDepth != 24) {
        printf("Error - Bits per pixel is not 24!\n");
        return false;
    }

    return true;
}

// Read RGB Pixels from BMP image
RGB* readPixels(FILE* inFile, BMPImageInfo info) {
    // Allocate memory space for RGB array of pixels
    RGB *rgbValues = (RGB*) malloc(info.width * info.height * sizeof(RGB));

    // Read values
    for(int i = 0; i < info.height; i++) {
        //printf("%i\n", i);

        // Read the values from the file to the right position in the RGB array
        if( fread(rgbValues + (i * info.width), sizeof(RGB), info.width, inFile) != info.width ) {
            printf("Error - Reading RGB values!\n");
            return NULL;
        }

        // Alignment of every new row in the picture
        if(fseek(inFile, (info.width * sizeof(RGB)) % 4, SEEK_CUR) != 0) {
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
bool writeImage(FILE* outFile, BMPHeader header, BMPImageInfo info, RGB *rgbValues) {
    // Write Header
    if( fwrite(&header.signature, sizeof(char), 2, outFile) != 2 ) {
        return false;
    }

    if( fwrite(&header.fileSize, sizeof(unsigned int), 1, outFile) != 1 ) {
        return false;
    }

    if( fwrite(&header.reserved, sizeof(unsigned int), 1, outFile) != 1 ) {
        return false;
    }

    if( fwrite(&header.offset, sizeof(unsigned int), 1, outFile) != 1 ) {
        return false;
    }

    // Write Info
    if( fwrite(&info, sizeof(BMPImageInfo), 1, outFile) != 1 ) {
        return false;
    }

    // Write RGB values row for row because of alignment
    for(int i = 0; i < info.height; i++) {
        if( fwrite(rgbValues + i * info.width, sizeof(RGB), info.width, outFile) != info.width ) {
            return false;
        }
    }

    return true;
}

// Convert RGB values to greyscale
void convertRGBtoGreyscale(RGB *rgbValues, BMPImageInfo info) {
    // Constants
    float a = 0.3;
    float b = 0.58;
    float c = 0.11;
    float sum = a + b + c;

    // Loop to convert every pixel to greyscale
    for(int i = 0; i < info.height; i++) {
        for(int j = 0; j < info.width; j++) {
            // Round down the result because RGB can only have integer numbers
            int D = (a * rgbValues[i*info.height + j].red
                    + b * rgbValues[i*info.height + j].green
                    + c * rgbValues[i*info.height + j].blue)
                    / sum;

            // Set grey color to RGB pixel
            rgbValues[i*info.height + j].red = D;
            rgbValues[i*info.height + j].green = D;
            rgbValues[i*info.height + j].blue = D;
        }
    }
}

