// BMP Functions
BMPHeader* readHeader(FILE* inFile);
BMPImageInfo* readInfo(FILE* inFile);
bool checkBMPImage(BMPHeader* header, BMPImageInfo* info);
RGB* readBMPImage(FILE* inFile, BMPImageInfo* info);
bool writeBMPImage(FILE* outFile, BMPHeader* header, BMPImageInfo* info, RGB* rgbValues);

// RGB Conversion Functions
RGBcolorWord* convertRGBtoSIMDWord(RGB* rgbValues);
RGB* convertSIMDWordtoRGB(RGBcolorWord* rgbValues);

// C implemented Greyscale and Convolution
RGB* convertRGBtoGreyscale(RGB* rgbValues);
RGB* convolutionRGB(RGB* rgbValues);
