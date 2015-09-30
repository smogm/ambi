#ifndef IFRAME_HPP
#define IFRAME_HPP

#include <cinttypes>
#include <cstdlib>

class IFrame
{
protected:
   size_t mSize;
   size_t mWidth;
   size_t mHeight;

public:
   IFrame();
   virtual ~IFrame();
   
   virtual bool setRaw(const uint8_t* const aYUVRawFrame, const size_t aWidth, const size_t aHeight) = 0;

   virtual void* getRGBBuffer() const;
   virtual size_t size() const;
   virtual size_t width() const;
   virtual size_t height() const;
};

#endif
