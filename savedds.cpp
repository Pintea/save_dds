#include <stdio.h>
#include <memory.h>

#define STB_DXT_IMPLEMENTATION
#pragma warning(push)
#pragma warning(disable: 4244) // conversion from 'int' to 'unsigned char', possible loss of data
#include "stb_dxt.h"
#pragma warning(pop)
#undef STB_DXT_IMPLEMENTATION

//
// we don't use much of the struct below, just need the correct header representation

#define DDSF_FOURCC 0x00000004l

#ifndef MAKEFOURCC
    #define MAKEFOURCC(ch0, ch1, ch2, ch3)                              \
                ((unsigned int)(unsigned char)(ch0) | ((unsigned int)(unsigned char)(ch1) << 8) |   \
                ((unsigned int)(unsigned char)(ch2) << 16) | ((unsigned int)(unsigned char)(ch3) << 24 ))
#endif //defined(MAKEFOURCC)
#define FOURCC_DXT1  (MAKEFOURCC('D','X','T','1'))
#define FOURCC_DXT5  (MAKEFOURCC('D','X','T','5'))

typedef struct _DDCOLORKEY
{
    unsigned int       dwColorSpaceLowValue;
    unsigned int       dwColorSpaceHighValue;
} DDCOLORKEY;

typedef struct _DDPIXELFORMAT
{
    unsigned int       dwSize;                 // size of structure
    unsigned int       dwFlags;                // pixel format flags
    unsigned int       dwFourCC;               // (FOURCC code)
    union
    {
        unsigned int   dwRGBBitCount;          // how many bits per pixel
        unsigned int   dwYUVBitCount;          // how many bits per pixel
        unsigned int   dwZBufferBitDepth;      // how many total bits/pixel in z buffer (including any stencil bits)
        unsigned int   dwAlphaBitDepth;        // how many bits for alpha channels
        unsigned int   dwLuminanceBitCount;    // how many bits per pixel
        unsigned int   dwBumpBitCount;         // how many bits per "buxel", total
        unsigned int   dwPrivateFormatBitCount;// Bits per pixel of private driver formats. Only valid in texture format list and if DDPF_D3DFORMAT is set
    };
    union
    {
        unsigned int   dwRBitMask;             // mask for red bit
        unsigned int   dwYBitMask;             // mask for Y bits
        unsigned int   dwStencilBitDepth;      // how many stencil bits (note: dwZBufferBitDepth-dwStencilBitDepth is total Z-only bits)
        unsigned int   dwLuminanceBitMask;     // mask for luminance bits
        unsigned int   dwBumpDuBitMask;        // mask for bump map U delta bits
        unsigned int   dwOperations;           // DDPF_D3DFORMAT Operations
    };
    union
    {
        unsigned int   dwGBitMask;             // mask for green bits
        unsigned int   dwUBitMask;             // mask for U bits
        unsigned int   dwZBitMask;             // mask for Z bits
        unsigned int   dwBumpDvBitMask;        // mask for bump map V delta bits
        struct
        {
            unsigned short    wFlipMSTypes;       // Multisample methods supported via flip for this D3DFORMAT
            unsigned short    wBltMSTypes;        // Multisample methods supported via blt for this D3DFORMAT
        } MultiSampleCaps;

    };
    union
    {
        unsigned int   dwBBitMask;             // mask for blue bits
        unsigned int   dwVBitMask;             // mask for V bits
        unsigned int   dwStencilBitMask;       // mask for stencil bits
        unsigned int   dwBumpLuminanceBitMask; // mask for luminance in bump map
    };
    union
    {
        unsigned int   dwRGBAlphaBitMask;      // mask for alpha channel
        unsigned int   dwYUVAlphaBitMask;      // mask for alpha channel
        unsigned int   dwLuminanceAlphaBitMask;// mask for alpha channel
        unsigned int   dwRGBZBitMask;          // mask for Z channel
        unsigned int   dwYUVZBitMask;          // mask for Z channel
    };
} DDPIXELFORMAT;

typedef struct _DDSURFACEDESC2
{
    unsigned int               dwSize;                 // size of the DDSURFACEDESC structure
    unsigned int               dwFlags;                // determines what fields are valid
    unsigned int               dwHeight;               // height of surface to be created
    unsigned int               dwWidth;                // width of input surface
    union
    {
        int					   lPitch;                 // distance to start of next line (return value only)
        unsigned int           dwLinearSize;           // Formless late-allocated optimized surface size
    };
    union
    {
        unsigned int           dwBackBufferCount;      // number of back buffers requested
        unsigned int           dwDepth;                // the depth if this is a volume texture 
    };
    union
    {
        unsigned int           dwMipMapCount;          // number of mip-map levels requestde
                                                // dwZBufferBitDepth removed, use ddpfPixelFormat one instead
        unsigned int           dwRefreshRate;          // refresh rate (used when display mode is described)
        unsigned int           dwSrcVBHandle;          // The source used in VB::Optimize
    };
    unsigned int               dwAlphaBitDepth;        // depth of alpha buffer requested
    unsigned int               dwReserved;             // reserved
    unsigned int                lpSurface;              // pointer to the associated surface memory
    union
    {
        DDCOLORKEY			   ddckCKDestOverlay;      // color key for destination overlay use
        unsigned int           dwEmptyFaceColor;       // Physical color for empty cubemap faces
    };
    DDCOLORKEY                 ddckCKDestBlt;          // color key for destination blt use
    DDCOLORKEY                 ddckCKSrcOverlay;       // color key for source overlay use
    DDCOLORKEY                 ddckCKSrcBlt;           // color key for source blt use
    union
    {
        DDPIXELFORMAT		   ddpfPixelFormat;        // pixel format description of the surface
        unsigned int           dwFVF;                  // vertex format description of vertex buffers
    };

    struct _DDSCAPS2
    {
        unsigned int       dwCaps;
        unsigned int       dwCaps2;
        unsigned int       dwCaps3;
        union
        {
            unsigned int       dwCaps4;
            unsigned int       dwVolumeDepth;
        };
    } ddsCaps;// direct draw surface capabilities

    unsigned int               dwTextureStage;         // stage in multitexture cascade
} DDSURFACEDESC2;


unsigned char* compress_to_dxt(const unsigned char* pData, int width, int height, int bpp, int* outDstSize)
{
    const int blockSize = bpp == 24 ? 8 : 16;
    const int dstSize = ((width + 3) / 4) * ((height + 3) / 4) * blockSize;
    unsigned char* dst = new unsigned char[dstSize];
    if (!dst)
    {
        *outDstSize = 0;
        return NULL;
    }
    *outDstSize = dstSize;

    const int c = bpp / 8; // num channels
    const int stride = width * c;
    unsigned char* pd = dst;
    for (int j = 0; j < height; j += 4)
    {
        for (int i = 0; i < width; i += 4)
        {
            const unsigned char* row[4] = {
                &pData[(j + 0) * stride + i * c],
                &pData[(j + 1) * stride + i * c],
                &pData[(j + 2) * stride + i * c],
                &pData[(j + 3) * stride + i * c]
            };

            unsigned char block[16][4];
            for (int b = 0; b < 16; ++b)
            {
                block[b][0] = row[b / 4][(b % 4) * c + 0];
                block[b][1] = row[b / 4][(b % 4) * c + 1];
                block[b][2] = row[b / 4][(b % 4) * c + 2];
                block[b][3] = bpp == 24 ? 0 : row[b / 4][(b % 4) * c + 3];
            }

            stb_compress_dxt_block(pd, &block[0][0], bpp == 24 ? 0 : 1, STB_DXT_HIGHQUAL /* or STB_DXT_NORMAL */);
            pd += blockSize;
        }
    }
    return dst;
}

void savedds(const char* filename, const unsigned char* pData, int width, int height, int bpp)
{
    if (bpp != 24 && bpp != 32)
        return;

    if ((width % 4) || (height % 4))
        return;

    FILE* f = fopen(filename, "wb");
    if (!f)
        return;

    char filecode[5] = "DDS ";
    fwrite(filecode, 4, 1, f);

    DDSURFACEDESC2 ddsd;
    memset(&ddsd, 0, sizeof(ddsd));
    ddsd.dwSize = sizeof(ddsd);
    ddsd.dwFlags = 0;
    ddsd.dwWidth = width;
    ddsd.dwHeight = height;
    ddsd.lPitch = width * height;
    ddsd.dwMipMapCount = 0;
    ddsd.ddpfPixelFormat.dwSize = sizeof(ddsd.ddpfPixelFormat);
    ddsd.ddpfPixelFormat.dwFlags = DDSF_FOURCC;
    ddsd.ddpfPixelFormat.dwFourCC = bpp == 24 ? FOURCC_DXT1 : FOURCC_DXT5;
    fwrite(&ddsd, sizeof(ddsd), 1, f);

    int dstSize = 0;
    unsigned char* dst = compress_to_dxt(pData, width, height, bpp, &dstSize);
    if (dst)
    {
        fwrite(dst, dstSize, 1, f);
        delete[] dst;
    }

    fclose(f);
}
