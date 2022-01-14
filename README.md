# save_dds
Compress and save raw pixel data as a DDS file (using stb_dxt for compression) in C++
Note: does not support mipmaps at this time.

void savedds(const char* filename, const unsigned char* pData, int width, int height, int bpp)
