#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "BMPStructs.h"

// Assembly Functions
extern void greyscale(FILE* in, FILE* out, int width, int height);
extern void blur(FILE* in, FILE* out, int width, int height);

// C Functions
int readHeader(FILE* inFile, BMPHeader* header);
int readInfo(FILE* inFile, BMPImageInfo* info);

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

    // Open the file with given file name
    inFile = fopen(relativePath, "rb");
    */
    inFile = fopen("./lena.bmp", "rb");

    // Checking for error while opening the file
    if( !inFile ) {
        return -1;
    }

    // Reading header of BMP File
    if(readHeader(inFile, &header) == -1) {
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
    if(readInfo(inFile, &info) == -1) {
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


    if( info.numColors > 0 )
    {
        palette = (RGB*)malloc(sizeof(RGB) * info.numColors);
        if( fread(palette, sizeof(RGB), info.numColors, inFile) != info.numColors )
            return -1; // manage error and close file
    }

    fclose(inFile);


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
int readHeader(FILE* inFile, BMPHeader* header) {
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
        return -1; // Manage error and close file
    }

    return 1;
}

// Read Image Info from BMP File
int readInfo(FILE* inFile, BMPImageInfo* info) {
    if( fread(info, sizeof(BMPImageInfo), 1, inFile) != 1 ) {
        return -1; // Manage error and close file
    }
}


