#ifndef IFRAMEGRABBER_HPP
#define IFRAMEGRABBER_HPP

#include "IFrame.hpp"

class IFrameGrabber
{
public:
   virtual ~IFrameGrabber()
   {
   }

   virtual bool grabFrame() = 0;
   virtual operator bool() const = 0;
};

#endif
