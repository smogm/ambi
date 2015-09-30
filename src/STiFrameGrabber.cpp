#include "STiFrameGrabber.hpp"

#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <linux/types.h>
#include <linux/fb.h>

#include <iostream>
#include <sstream>

namespace
{
   const size_t FRAME_BUFFER_SIZE = 4 * 1024 * 1024;
   const char* BPA_PART = "LMI_VID";
}

#define CLIP(X) ( (X) > 255 ? 255 : (X) < 0 ? 0 : X)

// YUV -> RGB
#define C(Y) ( (Y) - 16  )
#define D(U) ( (U) - 128 )
#define E(V) ( (V) - 128 )

#define YUV2R(Y, U, V) CLIP(( 298 * C(Y)              + 409 * E(V) + 128) >> 8)
#define YUV2G(Y, U, V) CLIP(( 298 * C(Y) - 100 * D(U) - 208 * E(V) + 128) >> 8)
#define YUV2B(Y, U, V) CLIP(( 298 * C(Y) + 516 * D(U)              + 128) >> 8)

// YCbCr -> RGB
#define CYCbCr2R(Y, Cb, Cr) CLIP( Y + ( 91881 * Cr >> 16 ) - 179 )
#define CYCbCr2G(Y, Cb, Cr) CLIP( Y - (( 22544 * Cb + 46793 * Cr ) >> 16) + 135)
#define CYCbCr2B(Y, Cb, Cr) CLIP( Y + (116129 * Cb >> 16 ) - 226 )

STiFrameGrabber::STiFrameGrabber(size_t posX, size_t posY) :
   mIsValid(false),
   mFrame(),
   //mRawFrameBuffer(new uint8_t[FRAME_BUFFER_SIZE]),
   mBPAFileDescriptor(0),
   mMappedMemoryPtr(nullptr),
   mFrameStride(0),
   mFrameRes(0),
   mBPAData(),
   mPosX(posX),
   mPosY(posY)
{
   if (initialize())
   {
      mIsValid = true;
   }
}

STiFrameGrabber::~STiFrameGrabber()
{
   // unmap and send release ioctl:
   std::cout << "unloading framegrabber..." << std::endl;
   
   if (mMappedMemoryPtr)
   {
      munmap(mMappedMemoryPtr, mBPAData.mem_size);

      if (ioctl(mBPAFileDescriptor, BPAMEMIO_UNMAPMEM))
      {
         std::cerr << "cannot unmap required mem" << std::endl;
      }
   }

   close(mBPAFileDescriptor);
}

STiFrameGrabber::operator bool() const
{
   return mIsValid;
}

bool STiFrameGrabber::getResolution()
{
   FILE* fp;
   char buf[256];
   
   fp = fopen("/proc/stb/vmpeg/0/xres","r");
   if (fp)
   {
      while (fgets(buf, sizeof(buf), fp))
      {
         sscanf(buf, "%x", &mFrameStride);
      }
      fclose(fp);
   }
   else
   {
      return false;
   }

   fp = fopen("/proc/stb/vmpeg/0/yres","r");
   if (fp)
   {
      while (fgets(buf, sizeof(buf), fp))
      {
         sscanf(buf, "%x", &mFrameRes);
      }
      fclose(fp);
   }
   else
   {
      return false;
   }
   
   if(mFrameStride == 0)
			mFrameStride = 1280;
   if(mFrameRes == 0)
			mFrameRes = 720;

   printf("res: %d x %d\n", mFrameStride, mFrameRes);

   return true;
}

bool STiFrameGrabber::getBPAMemoryDimension()
{
   FILE* fp;
   char buf[256];

   int BPAFileDescriptor = open("/dev/bpamem0", O_RDWR);
   if (BPAFileDescriptor < 0)
   {
      std::cerr << "cannot access /dev/bpamem0! err = " << BPAFileDescriptor << std::endl;
      return false;
   }

   mBPAData.bpa_part  = const_cast<char*>(BPA_PART);
   mBPAData.phys_addr = 0x00000000;
   mBPAData.mem_size  = 0;

   fp = fopen("/proc/bpa2","r");
   if (fp)
   {
      unsigned char found_part = 0;
      unsigned long mem_size = 0;
      unsigned long phys_addr = 0;
      while (fgets(buf, sizeof(buf), fp))
      {
         if(found_part || strstr(buf, mBPAData.bpa_part) != NULL)
         {
            found_part = 1;
            if (sscanf(buf, "- %lu B at %lx", &mem_size, &phys_addr) == 2)
            {
               if(mem_size > mBPAData.mem_size)
               {
                  mBPAData.mem_size  = mem_size;
                  mBPAData.phys_addr = phys_addr;
                  std::cout << "mem size: " << mem_size << " | phys_addr: " << phys_addr << std::endl;
               }
            }
         }
      }
      fclose(fp);
   }

   if (ioctl(BPAFileDescriptor, BPAMEMIO_MAPMEM, &mBPAData))
   {
      std::cerr << "cannot map required mem" << std::endl;
      return false;
   }

   close(BPAFileDescriptor);
   return true;
}

bool STiFrameGrabber::initialize()
{
   if (!getResolution())
   {
      std::cout << "getResolution() failed!" << std::endl;
      return false;
   }
   
   if (!getBPAMemoryDimension())
   {
      std::cout << "getBPAMemoryDimension() failed!" << std::endl;
      return false;
   }
   
   //if stride and res is zero than this is most probably a stillpicture
   if(mFrameStride == 0)
      mFrameStride = 1280;
   if(mFrameRes == 0)
      mFrameRes = 720;

   std::stringstream bpaMemoryDevice;
   bpaMemoryDevice << "/dev/bpamem" << mBPAData.device_num;
   std::cout << "dev: " << bpaMemoryDevice << std::endl;

   mBPAFileDescriptor = open(bpaMemoryDevice.str().c_str(), O_RDWR);
   if (mBPAFileDescriptor < 0)
   {
      std::cerr << "cannot access " << bpaMemoryDevice << "! err = " << mBPAFileDescriptor << std::endl;
      return false;
   }

   mMappedMemoryPtr = reinterpret_cast<uint8_t *>(mmap(0, mBPAData.mem_size, PROT_WRITE|PROT_READ, MAP_SHARED, mBPAFileDescriptor, 0));
   if(mMappedMemoryPtr == MAP_FAILED)
   {
      std::cerr << "could not map bpa mem" << std::endl;
      close(mBPAFileDescriptor);
      return false;
   }

   return true;
}

void STiFrameGrabber::getPixel(const size_t xPos, const size_t yPos, uint8_t* const rgb)
{
   // DVB is using YUV 4:2:0
   // see: https://en.wikipedia.org/wiki/YUV#Y.27UV420p_.28and_Y.27V12_or_YV12.29_to_RGB888_conversion
   
   // for each pixel of the current picture (e.g. fullhd at 1920 x 1080), there is one Y value (so we have 1920 x 1080 Y values)
   // for each 2x2 Y values there is one U and one V value, stored behind the Y value block
   
   /*
    *  Y1 Y2 Y3 Y4  Y5  Y6
    *  Y7 Y8 Y9 Y10 Y11 Y12
    *  U1 U2 U3 U4  U5  U6
    *  V1 V2 V3 V4  V5  V6
    */ 

   size_t sizeOfYBlock = mFrameStride /*width -> x*/ * mFrameRes /*height -> y*/; // e.g. 1920 * 1080
   size_t sizeOfUBlock = (sizeOfYBlock) / 4; // (width / 2) * (height / 2)

   size_t offsetUBlock = sizeOfYBlock;
   size_t offsetVBlock = sizeOfYBlock + sizeOfUBlock;

   size_t posInsideUVBlock = ((yPos * mFrameStride) / 4) + (xPos / 2);
   size_t posOfYValue = yPos * mFrameStride + xPos;
   
   // YUV 4:2:0:
   size_t posOfUValue = offsetUBlock + posInsideUVBlock;
   size_t posOfVValue = offsetVBlock + posInsideUVBlock;
   
   // on YV12 the V is located directly behind the Y block:
   //size_t posOfUValue = offsetVBlock + posInsideUVBlock;
   //size_t posOfVValue = offsetUBlock + posInsideUVBlock;

   uint8_t y = mMappedMemoryPtr[posOfYValue];
   uint8_t u = mMappedMemoryPtr[posOfUValue];
   uint8_t v = mMappedMemoryPtr[posOfVValue];
   
   /*
   *rgb     = 1164*(y - 16) + 1596*(v - 128);
   *(rgb+1) = 1164*(y - 16) - 813*(v - 128) - 391*(u - 128);
   *(rgb+2) = 1164*(y - 16) + 2018*(u - 128);
   
   *rgb     /= 1000;
   *(rgb+1) /= 1000;
   *(rgb+2) /= 1000;*/

//   *rgb     = YUV2R(y, u, v);
//   *(rgb+1) = YUV2G(y, u, v);
//   *(rgb+2) = YUV2B(y, u, v);

   *rgb     = CYCbCr2R(y, u, v);
   *(rgb+1) = CYCbCr2G(y, u, v);
   *(rgb+2) = CYCbCr2B(y, u, v);
}

bool STiFrameGrabber::grabFrame()
{
   //std::cout << __PRETTY_FUNCTION__ << " START" << std::endl;
   // copy to prevent race conditions during conversation:
   //memset(mRawFrameBuffer, 0, FRAME_BUFFER_SIZE);
   //memcpy(mRawFrameBuffer, mMappedMemoryPtr, FRAME_BUFFER_SIZE);
   uint8_t rgb[3];
   getPixel(mPosX, mPosY, rgb);
   printf("px %d x %d -> r: %02X, g: %02X, b: %02X\n", mPosX, mPosY, rgb[0], rgb[1], rgb[2]);

   //std::cout << __PRETTY_FUNCTION__ << " END" << std::endl;
   //mFrame.setRaw(mRawFrameBuffer, mFrameStride, mFrameRes);

   return true; //mFrame;
}
