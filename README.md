# ProcessBmp
Process a bitmap image to a C header file for use on the Wii U.

This tool has been developed to work with Wii U OSScreen API. The tool creates a two dimensional array of 32-bit words containing 0xRRGGBB00 for 24-bit colour in the form needed by the OSScreen OSScreenPutPixelEx API. This header file can then be used with some simple code in the Wii U to display an image on the screen.

This tool was quickly developed to support some Wii U Homebrew simple games development. It should operate correctly for 24-bit colour bitmap files of up to 256x256 pixels. Microsoft paint can be used to draw images, or convert images to 24-bit .bmp format.
