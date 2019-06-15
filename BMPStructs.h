//
// Created by Philipp on 14.06.2019.
//

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
} RGB;

typedef struct
{
    BMPHeader header;
    BMPImageInfo info;
    RGB colors[256];
    unsigned short image[1];
} BMPFile;
