#ifndef FRAMEGRABBER_HPP
#define FRAMEGRABBER_HPP

#include "IFrameGrabber.hpp"
#include "YUVFrame.hpp"

#include <bpamem.h>

class STiFrameGrabber final : public IFrameGrabber
{
   bool mIsValid;
   YUVFrame mFrame;
   int mBPAFileDescriptor;
   uint8_t* mMappedMemoryPtr;
   size_t mFrameStride, mFrameRes;
   BPAMemMapMemData mBPAData;
   size_t mPosX, mPosY;

   STiFrameGrabber(const STiFrameGrabber&) = delete;
   const STiFrameGrabber& operator=(const STiFrameGrabber&) = delete;
   
   bool getResolution();
   bool getBPAMemoryDimension();
   bool initialize();
   void getPixel(const size_t x, const size_t y, uint8_t* rgb);

   public:
      STiFrameGrabber(size_t posX, size_t posY);
      virtual ~STiFrameGrabber();

      virtual bool grabFrame();
      explicit operator bool() const;
};

#endif
