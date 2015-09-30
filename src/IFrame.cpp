#include "IFrame.hpp"

IFrame::IFrame() :
   mSize(0),
   mWidth(0),
   mHeight(0)
{
}

IFrame::~IFrame()
{
}

void* IFrame::getRGBBuffer() const
{
   return nullptr;
}

size_t IFrame::size() const
{
   return mSize;
}

size_t IFrame::width() const
{
   return mWidth;
}

size_t IFrame::height() const
{
   return mHeight;
}
