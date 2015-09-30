#ifndef YUVFRAME_HPP
#define YUVFRAME_HPP

#include "IFrame.hpp"

extern "C"
{
#include <libavformat/avformat.h>
}

class YUVFrame final : public IFrame
{
   bool mIsValid;
   uint8_t* mLuminance;
   uint8_t* mChrominance;

   bool convert(const uint8_t* ptBufferIn, int iOutWidth, int iOutHeight, AVPicture* src);

   YUVFrame(const YUVFrame&) = delete;
   const YUVFrame& operator=(const YUVFrame&) = delete;

public:
   YUVFrame();
   virtual ~YUVFrame();
   
   virtual bool setRaw(const uint8_t* const aYUVRawFrame, const size_t aWidth, const size_t aHeight);

   explicit operator bool() const;
};

#endif
