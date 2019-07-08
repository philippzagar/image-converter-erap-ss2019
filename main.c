#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <setjmp.h>

#include "./libjpeg-master/jpeglib.h"

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

boolean write_JPEG_file (FILE* outfile, int quality, RGB* rgbValues);

// Global variables
int global_image_height;
int global_image_width;
J_COLOR_SPACE global_colorSpace;

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

    boolean isJPG = endsWith(relativePath, ".jpg") || endsWith(relativePath, ".JPG");

    // JPG File
    if(isJPG) {
        rgbValues = read_JPEG_file(inFile);

        if(!rgbValues) {
            printf("Error while reading JPG file\n");
            return -1;
        }
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
        outFile = fopen("./test2.jpg", "wb");

        if (!outFile) {
            printf("Error while opening JPG file to write\n");
            return -1;
        }

        if(!write_JPEG_file(outFile, 80, rgbValues)) {
            printf("Error while writing JPG file\n");
            return -1;
        }

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

/******************** JPEG DECOMPRESSION SAMPLE INTERFACE *******************/

/* This half of the example shows how to read data from the JPEG decompressor.
 * It's a bit more refined than the above, in that we show:
 *   (a) how to modify the JPEG library's standard error-reporting behavior;
 *   (b) how to allocate workspace using the library's memory manager.
 *
 * Just to make this example a little different from the first one, we'll
 * assume that we do not intend to put the whole image into an in-memory
 * buffer, but to send it line-by-line someplace else.  We need a one-
 * scanline-high JSAMPLE array as a work buffer, and we will let the JPEG
 * memory manager allocate it for us.  This approach is actually quite useful
 * because we don't need to remember to deallocate the buffer separately: it
 * will go away automatically when the JPEG object is cleaned up.
 */


/*
 * ERROR HANDLING:
 *
 * The JPEG library's standard error handler (jerror.c) is divided into
 * several "methods" which you can override individually.  This lets you
 * adjust the behavior without duplicating a lot of code, which you might
 * have to update with each future release.
 *
 * Our example here shows how to override the "error_exit" method so that
 * control is returned to the library's caller when a fatal error occurs,
 * rather than calling exit() as the standard error_exit method does.
 *
 * We use C's setjmp/longjmp facility to return control.  This means that the
 * routine which calls the JPEG library must first execute a setjmp() call to
 * establish the return point.  We want the replacement error_exit to do a
 * longjmp().  But we need to make the setjmp buffer accessible to the
 * error_exit routine.  To do this, we make a private extension of the
 * standard JPEG error handler object.  (If we were using C++, we'd say we
 * were making a subclass of the regular error handler.)
 *
 * Here's the extended error handler struct:
 */

struct my_error_mgr {
    struct jpeg_error_mgr pub;	/* "public" fields */

    jmp_buf setjmp_buffer;	/* for return to caller */
};

typedef struct my_error_mgr * my_error_ptr;

/*
 * Here's the routine that will replace the standard error_exit method:
 */

METHODDEF(void)
my_error_exit (j_common_ptr cinfo)
{
    /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
    my_error_ptr myerr = (my_error_ptr) cinfo->err;

    /* Always display the message. */
    /* We could postpone this until after returning, if we chose. */
    (*cinfo->err->output_message) (cinfo);

    /* Return control to the setjmp point */
    longjmp(myerr->setjmp_buffer, 1);
}


/*
 * Sample routine for JPEG decompression.  We assume that the source file name
 * is passed in.  We want to return 1 on success, 0 on error.
 */


RGB* read_JPEG_file (FILE* infile)
{
    /* This struct contains the JPEG decompression parameters and pointers to
     * working space (which is allocated as needed by the JPEG library).
     */
    struct jpeg_decompress_struct cinfo;
    /* We use our private extension JPEG error handler.
     * Note that this struct must live as long as the main JPEG parameter
     * struct, to avoid dangling-pointer problems.
     */
    struct my_error_mgr jerr;
    /* More stuff */
    JSAMPARRAY buffer;		/* Output row buffer */
    int row_stride;		/* physical row width in output buffer */

    /* In this example we want to open the input file before doing anything else,
     * so that the setjmp() error recovery below can assume the file is open.
     * VERY IMPORTANT: use "b" option to fopen() if you are on a machine that
     * requires it in order to read binary files.
     */

    /* Step 1: allocate and initialize JPEG decompression object */

    /* We set up the normal JPEG error routines, then override error_exit. */
    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = my_error_exit;
    /* Establish the setjmp return context for my_error_exit to use. */
    if (setjmp(jerr.setjmp_buffer)) {
        /* If we get here, the JPEG code has signaled an error.
         * We need to clean up the JPEG object, close the input file, and return.
         */
        jpeg_destroy_decompress(&cinfo);
        fclose(infile);
        return NULL;
    }
    /* Now we can initialize the JPEG decompression object. */
    jpeg_create_decompress(&cinfo);

    /* Step 2: specify data source (eg, a file) */

    jpeg_stdio_src(&cinfo, infile);

    /* Step 3: read file parameters with jpeg_read_header() */

    (void) jpeg_read_header(&cinfo, TRUE);
    /* We can ignore the return value from jpeg_read_header since
     *   (a) suspension is not possible with the stdio data source, and
     *   (b) we passed TRUE to reject a tables-only JPEG file as an error.
     * See libjpeg.txt for more info.
     */

    /* Step 4: set parameters for decompression */

    /* In this example, we don't need to change any of the defaults set by
     * jpeg_read_header(), so we do nothing here.
     */

    /* Step 5: Start decompressor */

    (void) jpeg_start_decompress(&cinfo);
    /* We can ignore the return value since suspension is not possible
     * with the stdio data source.
     */

    /* We may need to do some setup of our own at this point before reading
     * the data.  After jpeg_start_decompress() we have the correct scaled
     * output image dimensions available, as well as the output colormap
     * if we asked for color quantization.
     * In this example, we need to make an output work buffer of the right size.
     */
    /* JSAMPLEs per row in output buffer */
    row_stride = cinfo.output_width * cinfo.output_components;
    /* Make a one-row-high sample array that will go away when done with image */
    buffer = (*cinfo.mem->alloc_sarray)
            ((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

    global_image_height = cinfo.image_height;
    global_image_width = cinfo.image_width;
    global_colorSpace = cinfo.out_color_space;

    // Allocate space for the final RGB array
    RGB* rgbValues = (RGB*) malloc(cinfo.image_height * cinfo.image_width * sizeof(RGB));
    // Counter in which line the reader actually is
    int actualHeight = 0;

    /* Step 6: while (scan lines remain to be read) */
    /*           jpeg_read_scanlines(...); */

    /* Here we use the library's state variable cinfo.output_scanline as the
     * loop counter, so that we don't have to keep track ourselves.
     */


    while (cinfo.output_scanline < cinfo.output_height) {
        /* jpeg_read_scanlines expects an array of pointers to scanlines.
         * Here the array is only one element long, but you could ask for
         * more than one scanline at a time if that's more convenient.
         */
        (void) jpeg_read_scanlines(&cinfo, buffer, 1);

        /* Assume put_scanline_someplace wants a pointer and sample count. */
        save_scanline((unsigned  char*) buffer[0], rgbValues, actualHeight++);
    }



    /* Step 7: Finish decompression */

    (void) jpeg_finish_decompress(&cinfo);
    /* We can ignore the return value since suspension is not possible
     * with the stdio data source.
     */

    /* Step 8: Release JPEG decompression object */

    /* This is an important step since it will release a good deal of memory. */
    jpeg_destroy_decompress(&cinfo);

    /* After finish_decompress, we can close the input file.
     * Here we postpone it until after no more JPEG errors are possible,
     * so as to simplify the setjmp error logic above.  (Actually, I don't
     * think that jpeg_destroy can do an error exit, but why assume anything...)
     */
    fclose(infile);

    /* At this point you may want to check to see whether any corrupt-data
     * warnings occurred (test whether jerr.pub.num_warnings is nonzero).
     */

    /* And we're done! */
    return rgbValues;
}


/*
 * SOME FINE POINTS:
 *
 * In the above code, we ignored the return value of jpeg_read_scanlines,
 * which is the number of scanlines actually read.  We could get away with
 * this because we asked for only one line at a time and we weren't using
 * a suspending data source.  See libjpeg.txt for more info.
 *
 * We cheated a bit by calling alloc_sarray() after jpeg_start_decompress();
 * we should have done it beforehand to ensure that the space would be
 * counted against the JPEG max_memory setting.  In some systems the above
 * code would risk an out-of-memory error.  However, in general we don't
 * know the output image dimensions before jpeg_start_decompress(), unless we
 * call jpeg_calc_output_dimensions().  See libjpeg.txt for more about this.
 *
 * Scanlines are returned in the same order as they appear in the JPEG file,
 * which is standardly top-to-bottom.  If you must emit data bottom-to-top,
 * you can use one of the virtual arrays provided by the JPEG memory manager
 * to invert the data.  See wrbmp.c for an example.
 *
 * As with compression, some operating modes may require temporary files.
 * On some systems you may need to set up a signal handler to ensure that
 * temporary files are deleted if the program is interrupted.  See libjpeg.txt.
 */


/******************** JPEG COMPRESSION SAMPLE INTERFACE *******************/

/* This half of the example shows how to feed data into the JPEG compressor.
 * We present a minimal version that does not worry about refinements such
 * as error recovery (the JPEG code will just exit() if it gets an error).
 */


/*
 * IMAGE DATA FORMATS:
 *
 * The standard input image format is a rectangular array of pixels, with
 * each pixel having the same number of "component" values (color channels).
 * Each pixel row is an array of JSAMPLEs (which typically are unsigned chars).
 * If you are working with color data, then the color values for each pixel
 * must be adjacent in the row; for example, R,G,B,R,G,B,R,G,B,... for 24-bit
 * RGB color.
 *
 * For this example, we'll assume that this data structure matches the way
 * our application has stored the image in memory, so we can just pass a
 * pointer to our image buffer.  In particular, let's say that the image is
 * RGB color and is described by:
 */

//extern JSAMPLE * image_buffer;	/* Points to large array of R,G,B-order data */
//extern int image_height;	/* Number of rows in image */
//extern int image_width;		/* Number of columns in image */


/*
 * Sample routine for JPEG compression.  We assume that the target file name
 * and a compression quality factor are passed in.
 */

// Quality 0 - 100
boolean write_JPEG_file (FILE* outfile, int quality, RGB* rgbValues)
{
    JSAMPLE* image_buffer = (JSAMPLE*) rgbValues;

    /* This struct contains the JPEG compression parameters and pointers to
     * working space (which is allocated as needed by the JPEG library).
     * It is possible to have several such structures, representing multiple
     * compression/decompression processes, in existence at once.  We refer
     * to any one struct (and its associated working data) as a "JPEG object".
     */
    struct jpeg_compress_struct cinfo;
    /* This struct represents a JPEG error handler.  It is declared separately
     * because applications often want to supply a specialized error handler
     * (see the second half of this file for an example).  But here we just
     * take the easy way out and use the standard error handler, which will
     * print a message on stderr and call exit() if compression fails.
     * Note that this struct must live as long as the main JPEG parameter
     * struct, to avoid dangling-pointer problems.
     */
    struct jpeg_error_mgr jerr;
    /* More stuff */
    JSAMPROW row_pointer[1];	/* pointer to JSAMPLE row[s] */
    int row_stride;		/* physical row width in image buffer */

    /* Step 1: allocate and initialize JPEG compression object */

    /* We have to set up the error handler first, in case the initialization
     * step fails.  (Unlikely, but it could happen if you are out of memory.)
     * This routine fills in the contents of struct jerr, and returns jerr's
     * address which we place into the link field in cinfo.
     */
    cinfo.err = jpeg_std_error(&jerr);
    /* Now we can initialize the JPEG compression object. */
    jpeg_create_compress(&cinfo);

    /* Step 2: specify data destination (eg, a file) */
    /* Note: steps 2 and 3 can be done in either order. */

    /* Here we use the library-supplied code to send compressed data to a
     * stdio stream.  You can also write your own code to do something else.
     * VERY IMPORTANT: use "b" option to fopen() if you are on a machine that
     * requires it in order to write binary files.
     */
    jpeg_stdio_dest(&cinfo, outfile);

    /* Step 3: set parameters for compression */

    /* First we supply a description of the input image.
     * Four fields of the cinfo struct must be filled in:
     */
    cinfo.image_width = global_image_width; 	/* image width and height, in pixels */
    cinfo.image_height = global_image_height;
    cinfo.input_components = 3;		/* # of color components per pixel */
    cinfo.in_color_space = global_colorSpace; 	/* colorspace of input image */
    /* Now use the library's routine to set default compression parameters.
     * (You must set at least cinfo.in_color_space before calling this,
     * since the defaults depend on the source color space.)
     */
    jpeg_set_defaults(&cinfo);
    /* Now you can set any non-default parameters you wish to.
     * Here we just illustrate the use of quality (quantization table) scaling:
     */
    jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);

    /* Step 4: Start compressor */

    /* TRUE ensures that we will write a complete interchange-JPEG file.
     * Pass TRUE unless you are very sure of what you're doing.
     */
    jpeg_start_compress(&cinfo, TRUE);

    /* Step 5: while (scan lines remain to be written) */
    /*           jpeg_write_scanlines(...); */

    /* Here we use the library's state variable cinfo.next_scanline as the
     * loop counter, so that we don't have to keep track ourselves.
     * To keep things simple, we pass one scanline per call; you can pass
     * more if you wish, though.
     */
    row_stride = global_image_width * 3;	/* JSAMPLEs per row in image_buffer */

    while (cinfo.next_scanline < cinfo.image_height) {
        /* jpeg_write_scanlines expects an array of pointers to scanlines.
         * Here the array is only one element long, but you could pass
         * more than one scanline at a time if that's more convenient.
         */
        row_pointer[0] = & image_buffer[cinfo.next_scanline * row_stride];
        (void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }

    /* Step 6: Finish compression */

    jpeg_finish_compress(&cinfo);
    /* After finish_compress, we can close the output file. */
    fclose(outfile);

    /* Step 7: release JPEG compression object */

    /* This is an important step since it will release a good deal of memory. */
    jpeg_destroy_compress(&cinfo);

    /* And we're done! */
    return true;
}


/*
 * SOME FINE POINTS:
 *
 * In the above loop, we ignored the return value of jpeg_write_scanlines,
 * which is the number of scanlines actually written.  We could get away
 * with this because we were only relying on the value of cinfo.next_scanline,
 * which will be incremented correctly.  If you maintain additional loop
 * variables then you should be careful to increment them properly.
 * Actually, for output to a stdio stream you needn't worry, because
 * then jpeg_write_scanlines will write all the lines passed (or else exit
 * with a fatal error).  Partial writes can only occur if you use a data
 * destination module that can demand suspension of the compressor.
 * (If you don't know what that's for, you don't need it.)
 *
 * If the compressor requires full-image buffers (for entropy-coding
 * optimization or a multi-scan JPEG file), it will create temporary
 * files for anything that doesn't fit within the maximum-memory setting.
 * (Note that temp files are NOT needed if you use the default parameters.)
 * On some systems you may need to set up a signal handler to ensure that
 * temporary files are deleted if the program is interrupted.  See libjpeg.txt.
 *
 * Scanlines MUST be supplied in top-to-bottom order if you want your JPEG
 * files to be compatible with everyone else's.  If you cannot readily read
 * your data in that order, you'll need an intermediate array to hold the
 * image.  See rdtarga.c or rdbmp.c for examples of handling bottom-to-top
 * source data using the JPEG code's internal virtual-array mechanisms.
 */

void save_scanline(unsigned char* buffer, RGB* rgbValues, int actualHeight) {
    for(int i = 0; i < global_image_width; i++) {
        rgbValues[i + (actualHeight * global_image_width)].red = *(buffer + (i * 3));
        rgbValues[i + (actualHeight * global_image_width)].green = *(buffer + (i * 3) + 1);
        rgbValues[i + (actualHeight * global_image_width)].blue = *(buffer + (i * 3) + 2);
    }
}

// -ljpeg