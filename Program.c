#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
//#include "./jpeg-6b/jpeglib.h"
#include "BMPStructs.h"

/*
// we will be using this uninitialized pointer later to store raw, uncompressd image
unsigned char *raw_image = NULL;

// dimensions of the image we want to write
int _width;
int _height;
int _bytes_per_pixel;   // or 1 for GRACYSCALE images
int _color_space; // or JCS_GRAYSCALE for grayscale images
*/

// Assembly Functions
extern void greyscale(RGB *rgbValuesOut, int width, int height);
void greyscale_simd(RGB* out, int width, int height); // Runs with greyscale for now


extern void blur(FILE* in, FILE* out, int width, int height);

// C Functions
BMPHeader* readHeader(FILE* inFile);
BMPImageInfo* readInfo(FILE* inFile);
bool checkBMPImage(BMPHeader* header, BMPImageInfo* info);
RGB* readPixels(FILE* inFile, BMPImageInfo* info);
bool writeImage(FILE* outFile, BMPHeader* header, BMPImageInfo* info, RGB* rgbValues);
RGB* convertRGBtoGreyscale(RGB* rgbValues, BMPImageInfo* info);
RGB* convolutionRGB(RGB* rgbValues, BMPImageInfo* info);

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

    // Reading header of BMP File
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
    //greyscale(rgbValues, info->width, info->height);

    rgbValues = convertRGBtoGreyscale(rgbValues, info);
    rgbValues = convolutionRGB(rgbValues, info);

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
    for(int i = 0; i < info->height; i++) {
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
    for(int i = 0; i < info->height; i++) {
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
    for(int i = 0; i < info->height; i++) {
        for(int j = 0; j < info->width; j++) {
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
    for(int currentHeigh = 0; currentHeigh < info->height; currentHeigh++) {
        for(int currentWidth = 0; currentWidth < info->width; currentWidth++) {

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
            sumGreenChar = sumRed;
            sumBlueChar = sumRed;

            // Write back to RGB value
            convolutionRGBValues[helpPointer].red = sumRedChar;
            convolutionRGBValues[helpPointer].green = sumGreenChar;
            convolutionRGBValues[helpPointer].red = sumBlueChar;

            // Set sums to 0
            sumRed = 0; sumGreen = 0; sumBlue = 0;
        }
    }

    return convolutionRGBValues;
}

/**
 * read_jpeg_file Reads from a jpeg file on disk specified by filename and saves into the
 * raw_image buffer in an uncompressed format.
 *
 * \returns positive integer if successful, -1 otherwise
 * \param *filename char string specifying the file name to read from
 *
 */

/*
int read_jpeg_file( char *filename )
{
    // these are standard libjpeg structures for reading(decompression)
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    // libjpeg data structure for storing one row, that is, scanline of an image
    JSAMPROW row_pointer[1];

    FILE *infile = fopen( filename, "rb" );
    unsigned long location = 0;
    int i = 0;

    if ( !infile )
    {
        printf("Error opening jpeg file %s\n!", filename );
        return -1;
    }
    //  here we set up the standard libjpeg error handler
    cinfo.err = jpeg_std_error( &jerr );
    // setup decompression process and source, then read JPEG header
    jpeg_create_decompress( &cinfo );
    // this makes the library read from infile
    jpeg_stdio_src( &cinfo, infile );
    // reading the image header which contains image information
    jpeg_read_header( &cinfo, TRUE );
    // Uncomment the following to output image information, if needed.

    printf( "JPEG File Information: \n" );
    printf( "Image width and height: %d pixels and %d pixels.\n", width=cinfo.image_width, height=cinfo.image_height );
    printf( "Color components per pixel: %d.\n", bytes_per_pixel = cinfo.num_components );
    printf( "Color space: %d.\n", cinfo.jpeg_color_space );

    // Start decompression jpeg here
    jpeg_start_decompress( &cinfo );

    // allocate memory to hold the uncompressed image
    raw_image = (unsigned char*)malloc( cinfo.output_width*cinfo.output_height*cinfo.num_components );
    // now actually read the jpeg into the raw buffer
    row_pointer[0] = (unsigned char *)malloc( cinfo.output_width*cinfo.num_components );
    // read one scan line at a time
    while( cinfo.output_scanline < cinfo.image_height )
    {
        jpeg_read_scanlines( &cinfo, row_pointer, 1 );
        for( i=0; i<cinfo.image_width*cinfo.num_components;i++)
            raw_image[location++] = row_pointer[0][i];
    }
    // wrap up decompression, destroy objects, free pointers and close open files
    jpeg_finish_decompress( &cinfo );
    jpeg_destroy_decompress( &cinfo );
    free( row_pointer[0] );
    fclose( infile );

    return 1;
}


int main(int argc,char **argv)
{
    int x,y;

    if(argc != 3){ printf("Usage: %s source.jpg dest.bmp",argv[0]); return -1; }
    x=y=0;
    // Try opening a jpeg
    if( read_jpeg_file( argv[1] ) > 0 )   write_bmp_file( argv[2] );

    else return -1;

    free(raw_image);
    return 0;
}
*/