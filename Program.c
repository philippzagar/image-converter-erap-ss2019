#include <stdio.h>
#include <stdlib.h>

extern void greyscale(FILE* in, FILE* out, int width, int height);
extern void blur(FILE* in, FILE* out, int width, int height);

typedef struct
{
    char signature[2];
    // Alignment int 4 byte!!!
    unsigned int fileSize;
    unsigned int reserved;
    unsigned int offset;
} BMPHeader;

typedef struct
{
    unsigned int headerSize;
    unsigned int width;
    unsigned int height;
    unsigned short planeCount;
    unsigned short bitDepth;
    unsigned int compression;
    unsigned int compressedImageSize;
    unsigned int horizontalResolution;
    unsigned int verticalResolution;
    unsigned int numColors;
    unsigned int importantColors;

} BMPImageInfo;

typedef struct
{
    unsigned char blue;
    unsigned char green;
    unsigned char red;
    unsigned char reserved;
} RGB;

typedef struct
{
    BMPHeader header;
    BMPImageInfo info;
    RGB colors[256];
    unsigned short image[1];
} BMPFile;

int main(int argc, char** argv) {

    //
    FILE *inFile, *outFile;
    BMPHeader header;
    BMPImageInfo info;
    RGB *palette, *p;
    int i = 0;

    inFile = fopen("./lena.bmp", "rb");
    if( !inFile )
        return -1;

    /*
     * if( fread(&header, sizeof(BMPHeader), 1, inFile) != 1 )
        return -1; // Manage error and close file
        */

    if( fread(&header.signature[0], sizeof(char), 1, inFile) != 1 )
        return -1; // Manage error and close file

    if( fread(&header.signature[1], sizeof(char), 1, inFile) != 1 )
        return -1; // Manage error and close file

    if( fread(&header.fileSize, sizeof(unsigned int), 1, inFile) != 1 )
        return -1; // Manage error and close file

    if( fread(&header.reserved, sizeof(unsigned int), 1, inFile) != 1 )
        return -1; // Manage error and close file

    if( fread(&header.offset, sizeof(unsigned int), 1, inFile) != 1 )
        return -1; // Manage error and close file

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


    if( fread(&info, sizeof(BMPImageInfo), 1, inFile) != 1 )
        return -1; // Manage error and close file
    printf("%u", info.height);
    printf("\n");
    printf("%u", info.width);
    printf("\n");
    printf("%u", info.numColors);
    printf("\n");


    if( info.numColors > 0 )
    {
        palette = (RGB*)malloc(sizeof(RGB) * info.numColors);
        if( fread(palette, sizeof(RGB), info.numColors, inFile) != info.numColors )
            return -1; // manage error and close file
    }

    fclose(inFile);

// Binary method => if read later by another computer
    outFile = fopen("./lena_rgb", "wb");
    if( !outFile )
        return -1;

    if( fwrite(&info.numColors, sizeof(unsigned int), 1, outFile) != 1 )
        return -1; // Manage Error and close file

    if( fwrite(&palette, sizeof(RGB), info.numColors, outFile) != info.numColors )
        return -1; // Manage error and close file

    fclose(outFile);

// Text method => if read later by human
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

    return 0;
}