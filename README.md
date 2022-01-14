# save_dds
Tiny code / single function to compress and save raw pixel data as a DDS file in C++ (compression is done with stb_dxt) 

RGB data is compressed as DXT1 and RGBA data as DXT5, then saved as a DDS file that should be compatible with all(?) DDS loaders.

Note: does not support mipmaps at this time.

void savedds(const char* filename, const unsigned char* pData, int width, int height, int bpp)
