#include "JPG_functions.h"

#include "shared_global_variables.h"

// Struct for Error handeling
struct my_error_mgr {
    struct jpeg_error_mgr pub;

    jmp_buf setjmp_buffer;
};

typedef struct my_error_mgr* my_error_ptr;

// Error handeling
METHODDEF(void) my_error_exit (j_common_ptr cinfo)
{
    my_error_ptr myerr = (my_error_ptr) cinfo->err;
    (*cinfo->err->output_message) (cinfo);
    longjmp(myerr->setjmp_buffer, 1);
}

// Reading JPG File from memory and returning the RGB values
RGB* read_JPEG_file (FILE* infile)
{
    // Decompressing info object
    struct jpeg_decompress_struct cinfo;
    // Error handeling
    struct my_error_mgr jerr;
    // Buffer for a line that is read
    JSAMPARRAY buffer;
    int row_stride;

    // Set up JPEG error routines
    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = my_error_exit;

    // If error occures
    if (setjmp(jerr.setjmp_buffer)) {
        jpeg_destroy_decompress(&cinfo);
        fclose(infile);
        return NULL;
    }

    //Initialize JPEG decompression object
    jpeg_create_decompress(&cinfo);
    // Set data source (the input file)
    jpeg_stdio_src(&cinfo, infile);

    // Read header
    (void) jpeg_read_header(&cinfo, TRUE);
    // Start decompression
    (void) jpeg_start_decompress(&cinfo);

    // Count of Pixels per row
    row_stride = cinfo.output_width * cinfo.output_components;
    // Allocate the buffer for the row
    buffer = (*cinfo.mem->alloc_sarray) ((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

    // Set global variables
    global_image_height = cinfo.image_height;
    global_image_width = cinfo.image_width;
    global_colorSpace = cinfo.out_color_space;

    // Allocate space for the final RGB array
    RGB* rgbValues = (RGB*) malloc(cinfo.image_height * cinfo.image_width * sizeof(RGB));
    // Counter in which line the reader actually is
    int actualHeight = 0;

    // Reading the picture as long as there are pixel lines
    while (cinfo.output_scanline < cinfo.output_height) {
        // Read scanline
        (void) jpeg_read_scanlines(&cinfo, buffer, 1);

        // Save the scanline to the RGB array
        save_scanline((unsigned  char*) buffer[0], rgbValues, actualHeight++);
    }

    // Finish decompression
    (void) jpeg_finish_decompress(&cinfo);
    // Destroy decompression object
    jpeg_destroy_decompress(&cinfo);
    // Close file
    fclose(infile);

    return rgbValues;
}

// Method for saving the scanline to the RGB array
void save_scanline(unsigned char* buffer, RGB* rgbValues, int actualHeight) {
    for(int i = 0; i < global_image_width; i++) {
        rgbValues[i + (actualHeight * global_image_width)].red = *(buffer + (i * 3));
        rgbValues[i + (actualHeight * global_image_width)].green = *(buffer + (i * 3) + 1);
        rgbValues[i + (actualHeight * global_image_width)].blue = *(buffer + (i * 3) + 2);
    }
}

// Write JPG File to memory
bool write_JPEG_file (FILE* outfile, int quality, RGB* rgbValues)
{
    // Converting RGB Values to JSAMPLE format (needed for writing the values to the file)
    JSAMPLE* image_buffer = (JSAMPLE*) rgbValues;

    // Compressing info object
    struct jpeg_compress_struct cinfo;
    // Error handling object
    struct jpeg_error_mgr jerr;
    // Row pointer
    JSAMPROW row_pointer[1];
    int row_stride;

    // Set up JPEG error routines
    cinfo.err = jpeg_std_error(&jerr);

    // Initialize JPEG compression object
    jpeg_create_compress(&cinfo);

    // Specify destination file
    jpeg_stdio_dest(&cinfo, outfile);

    // Set parameters for compression
    cinfo.image_width = global_image_width;
    cinfo.image_height = global_image_height;
    cinfo.input_components = 3;
    cinfo.in_color_space = global_colorSpace;

    // Set rest of parameters with default values
    jpeg_set_defaults(&cinfo);
    // Set quality
    jpeg_set_quality(&cinfo, quality, TRUE);

    // Start compression
    jpeg_start_compress(&cinfo, TRUE);

    // Calculate color pixel values per scanline
    row_stride = global_image_width * 3;

    // Writing as long as there are pixels
    while (cinfo.next_scanline < cinfo.image_height) {
        // Writing the buffer to memory
        row_pointer[0] = &image_buffer[cinfo.next_scanline * row_stride];
        (void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }

    // Finish compression
    jpeg_finish_compress(&cinfo);
    // Close output file
    fclose(outfile);

    // Destroy compression object
    jpeg_destroy_compress(&cinfo);

    return true;
}