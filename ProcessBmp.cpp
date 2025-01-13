// ProcessBmp.cpp : This file contains the 'main' function. Program execution begins and ends there.
// The program is to take a .bmp file and create a 2 dimensional array of pixel colours in a header (.h) file.
// This file can then be used in a WiiU program with the OSScreen APIs to draw the image on the WiiU screen.
// This is to in effect provide a way to display bit maps on the WiiU to provide usable graphic interfaces.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char fileData[65536 * 2];         // Limit the file data to 128k to keep a sensible size.
char fileHeader[56];            // The .bmp header information at the start of the file.

int main(int argc, char** argv)
{
    FILE *inFile;
    FILE *outFile;

    char outFileName[100] = "";     //Sting to give out file a different name from the input.
    unsigned int len;               // string length for output file name.

    unsigned int xsize;             // array sizes from the bitmap file.
    unsigned int ysize;
    unsigned int dataSize;          // size of the data area of the file.

    unsigned int working;           // integer for calculations

    // If there is no command line parameter then exit the program as nothing to process.
    if (argc < 2) 
    { 
        printf("No file\n");
        exit(0); 
    }
    // If bmp is not in the string it is not a bit map file.
    // Note this logic could be improved to check that bmp is the file extension.
    if (NULL == strstr(argv[1], "bmp"))
    { 
        printf("%s Not a bmp file\n", argv[1]);
        exit(0);
    }

    // If the file name seems ok try to open the file so that it can be processed.
    if (fopen_s(&inFile, argv[1], "rb") != 0)
    {
        printf("%s file could not be opened\n", argv[1]);
        exit(0);
    }
    else
    {
        // Get the bit map header information from the start of the file.
        // Note fread is used rather than fgets. String fgets will stop if it encounters a 0 (\0) end of string marker.
        fread(fileHeader, 1, 54, inFile);
        // Check the header to see that the file  is a 24-bit bmp file and the file length is less than 128k.
        if ((fileHeader[0] == 0x42) && (fileHeader[1] == 0x4D) && (fileHeader[28] == 0x18) && (fileHeader[4] <= 0x01) && (fileHeader[5] == 0x00))
        {
            // dataSize is the file size minus the header already read. Get the rest of the bytes into the fileData array, so that it can be processed.
            dataSize = (((unsigned char)fileHeader[4] * 65536) + ((unsigned char)fileHeader[3] * 256) + (unsigned char)fileHeader[2]) - 54;
            printf("Data Size %d\n", dataSize);
            // Note fread is used rather than fgets. String fgets will stop if it encounters a 0 (\0) end of string marker.
            fread(fileData, 1, dataSize, inFile);
   
            // Set the x and y sizes for processing the data array, taken from the x and y sizes in the bit map header.  
            // The X size has been allowed to go over 256 bits to process one image for Connect4.
            // In general it is probable a good idea for images to be less that 256x256 to fit sensibly on the WiiU screen.
            // If the ysize is more than 256 bits. but total size is less than 128k, the bitmap will not be processed properly.
            xsize = (unsigned char)fileHeader[19]*256 + (unsigned char)fileHeader[18];
            ysize = (unsigned char)fileHeader[22];
            printf("%i %i\n", xsize, ysize);

            // Create the output headerfile name and then open the file.
            strcat_s(outFileName, argv[1]);
            strcat_s(outFileName, ".h");
            if (fopen_s(&outFile, outFileName, "wt") != 0)
            {
                printf("out file could not be opened\n");
                exit(0);
            }
            else
            {
                // Use the input bit map filename to name the data array in the output header file.
                len = (unsigned int)(strchr(outFileName, '.') - outFileName);
                outFileName[len] = '\0';
                fprintf(outFile, "unsigned int %sImage[%d][%d] = {\n", outFileName, ysize, xsize);

                // Go through the array 3 bytes at a time (blue, green, red) to create the data array.
                // Array starts bottom left. x and y increases as expected.
                // However when displayed y counts down on the WiiU screen, which will need to be taken into account in the WiiU code.
                for (unsigned int y = 0; y < ysize; y++)
                {
                    // The rows of x data are 3 bytes per pixel, padded to 32 bits at the end of the row.
                    // The following logic is to add the padding so that the y count correctly moves through the array.
                    working = xsize * 3;
                    working = (working + (working % 4)) * y;

                    // Do the x loop to process a row of pixels.
                    fprintf(outFile, "{ ");
                    for (unsigned int x = 0; x < (xsize * 3); x = x + 3)
                    {
                        // The WiiU format is to pass in 32 bits for the 24 bit colour code 0xRRGGBBxx.
                        // RR red, GG green, BB blue, xx unused.
                        // This means the byte order must be swapped compared to the bit map file.
                        fprintf(outFile, "0x%02X%02X%02X00", (unsigned char)fileData[x + 2 + working], (unsigned char)fileData[x + 1 + working], (unsigned char)fileData[x + 0 + working]);
                        // The following logic is to ensure that a comma is not output for the end of the last line.
                        if ((x / 3) < (xsize - 1))
                        {
                            fprintf(outFile, ", ");
                        }
                    }
                    // The following logic terminates the data row with a bracket and comma, apart from the last line which just needs a bracket.
                    if (y < (ysize - 1))
                    {
                        fprintf(outFile, " },\n");
                    }
                    else
                    {
                        fprintf(outFile, " }\n");
                    }
                }
                // The data array is closed with a semi-colon.
                fprintf(outFile, " };\n");
            }
            // The files are closed as the array has been processed.
            fclose(inFile);
            fclose(outFile);
        }
        else
        {
            // If the logic decided that the file was not a bit map, display details to explain the problem.
            printf("%x %x %x %x %x\n", fileHeader[0], fileHeader[1], fileHeader[28], fileHeader[4], fileHeader[5]);
            printf("%s file is not a 24-bit bmp file of less than 128k\n", argv[1]);
            exit(0);
        }
    }
    return 0;
}

