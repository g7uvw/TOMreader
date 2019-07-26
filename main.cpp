    //
    //  main.cpp
    //  TOMReader
    //
    //  Created by David Mills on 25/07/2019.
    //  Test code to see if boost_multiarray will be easier to use / develop with than the current code
    //

#include <iostream>
#include <fstream>
#include <stdint.h>
#include <vector>
#include "boost/multi_array.hpp"

#include "TinyPngOut.hpp"

struct thead
{
    uint16_t xsize,ysize,zsize,lmarg,rmarg,tmarg,bmarg,tzmarg,bzmarg,\
    num_samples,num_proj,num_blocks,num_slices,bin,gain,speed,pepper,issue,num_frames,spare_int[13]__attribute__((packed));
    float scale,offset,voltage,current,thickness,pixel_size,distance,exposure,\
    mag_factor,filterb,correction_factor,spare_float[2]__attribute__((packed));
    uint32_t z_shift,z,theta __attribute__((packed));
    char time[26],duration[12],owner[21],user[5],specimen[32],scan[32],\
    comment[64],spare_char[192];
} TomHeader;

    // TOMfile format is 512 bytes header as descibed by the struct
    // followed by xsize x ysize x zsize of uint8_t 's
    // optional for there to be a trailer after this, but that's from old version of format and is now ignored


int main(int argc, const char * argv[]) {

    if (argc > 1)
        {
        std::cout << "argv[1] = " << argv[1] << std::endl;
        }
    else
        {
        std::cout << "No file name entered. Exiting..."<< std::endl;
        return -1;
        }

    std::ifstream tomfile(argv[1]); //open the file


    if (tomfile.is_open() && tomfile.good())
        {

        std::cout << "Success!\n";
        tomfile.seekg(0);
        tomfile.read((char*)&TomHeader, sizeof(TomHeader));

        std::cout << "XMT Size X, Y, Z : " << TomHeader.xsize << " " << TomHeader.ysize << " " << TomHeader.zsize << std::endl;
        }
    else
        {
        std::cout << "Failure!" << std::endl;
        }



        // create array to hold the image data
        // Create a 3D array that is xsize x ysize x zsize

    boost::multi_array<uint8_t,3> TomVolume;
        //TomVolume.resize(boost::extents[20][20][20]);
    TomVolume.resize(boost::extents[TomHeader.xsize][TomHeader.ysize][TomHeader.zsize]);


        // get handle to where the data actually lives and how much space we have allocated
    uint8_t *data = TomVolume.origin();
        //size_t bytes = 8000;//TomHeader.xsize * TomHeader.ysize * TomHeader.zsize;
    size_t bytes = TomHeader.xsize * TomHeader.ysize * TomHeader.zsize;
    tomfile.seekg(512); // skip the file header
    tomfile.read((char*) &data, bytes);
    tomfile.close();


        // make a RGB slice - set R=G=B
        // totally not optimal, but will do for testing.
        // write out a PNG that's just the central XY slice from the Z stack

    std::vector<uint8_t> rgbslice;
    rgbslice.resize(3 * TomHeader.xsize * TomHeader.ysize);
    for(unsigned int rgb_pixel = 0; rgb_pixel < rgbslice.size(); rgb_pixel++)
        for(unsigned int i = 0; i < TomHeader.xsize; i++)
            for(unsigned int j = 0; j < TomHeader.ysize; j++)
                {
                    //red
                rgbslice[rgb_pixel] = TomVolume[i][j][TomHeader.zsize / 2];
                rgb_pixel++;
                    //green
                rgbslice[rgb_pixel] = TomVolume[i][j][TomHeader.zsize / 2];
                rgb_pixel++;
                    //blue
                rgbslice[rgb_pixel] = TomVolume[i][j][TomHeader.zsize / 2];
                }

    uint8_t* p = rgbslice.data();

    std::ofstream out("slice.png", std::ios::binary);
    TinyPngOut pngout(static_cast<uint32_t>(TomHeader.xsize), static_cast<uint32_t>(TomHeader.ysize), out);

    pngout.write(p, static_cast<size_t>(TomHeader.xsize * TomHeader.ysize));
    
    return 0;
}
