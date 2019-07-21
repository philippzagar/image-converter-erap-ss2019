// JPG Functions
RGB* read_JPEG_file (FILE* infile);
void save_scanline(unsigned char* buffer, RGB* rgbValues, int actualHeight);
bool write_JPEG_file (FILE* outfile, int quality, RGB* rgbValues);
