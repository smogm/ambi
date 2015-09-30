#include "YUVFrame.hpp"

extern "C"
{
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/pixdesc.h>
#include <libavutil/samplefmt.h>
#include <libavutil/intreadwrite.h>
}

#include <iostream>

YUVFrame::YUVFrame() :
   IFrame(),
   mIsValid(false),
   mLuminance(new uint8_t[1920*300*3]),
   mChrominance(new uint8_t[1920*300*3/2])
{
   std::cout << __PRETTY_FUNCTION__ << std::endl;
   
   /*if (mLuminance && mChrominance)
   {
      decode()
   }*/
   
   /*AVPicture src;
   avpicture_alloc(&src, AV_PIX_FMT_YUV444P, aWidth, aHeight);

   if(!convert(aYUVRawFrame, aWidth, aHeight, &src))
   {
      avpicture_free(&src);
   }
   else
   {
      avpicture_free(&src);
      mIsValid = true;
   }*/
}

bool YUVFrame::setRaw(const uint8_t* const aYUVRawFrame, const size_t aWidth, const size_t aHeight)
{
   AVPicture src;
   mWidth = aWidth;
   mHeight = aHeight;
   
   avpicture_alloc(&src, AV_PIX_FMT_YUV444P, aWidth, aHeight);

   if(!convert(aYUVRawFrame, aWidth, aHeight, &src))
   {
      avpicture_free(&src);
      return false;
   }
   else
   {
      avpicture_free(&src);
      mIsValid = true;
   }
   
   return true;
}

YUVFrame::operator bool() const
{
   std::cout << __PRETTY_FUNCTION__ << std::endl;
   return mIsValid;
}

bool YUVFrame::convert(const uint8_t* ptBufferIn, int iOutWidth, int iOutHeight, AVPicture* src)
{
   std::cout << __PRETTY_FUNCTION__ << " START" << std::endl;
   SwsContext* ptImgConvertCtx; // Frame conversion context
   AVPicture ptPictureIn;

   ptImgConvertCtx = sws_getContext(mWidth, mHeight, AV_PIX_FMT_YUV420P, mWidth, mHeight, AV_PIX_FMT_YUV420P, SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);
   avpicture_fill(&ptPictureIn, ptBufferIn, AV_PIX_FMT_YUV420P, mWidth, mHeight);

   int iRes = sws_scale(ptImgConvertCtx,
                     ptPictureIn.data, //src
                     ptPictureIn.linesize,
                     0,
                     mHeight,
                     src->data,//dst
                     src->linesize);

   sws_freeContext(ptImgConvertCtx);

   std::cout << __PRETTY_FUNCTION__ << " END -> " << iRes << std::endl;
   if (iRes == iOutHeight)
     return true;

   return false;
}

/*bool YUVFrame::resize(const uint8_t* source, uint8_t* dest, int xsource, int ysource, int xdest, int ydest, int colors)
{
	const int x_ratio = (int)((xsource<<16)/xdest) ;
	const int y_ratio = (int)((ysource<<16)/ydest) ;
	int i;
	#pragma omp parallel for shared (dest, source)
	for (i=0; i<ydest; i++)
	{
		int y2_xsource = ((i*y_ratio)>>16)*xsource; // do some precalculations
		int i_xdest = i*xdest;
		int j;
		for (j=0; j<xdest; j++)
		{
			int x2 = ((j*x_ratio)>>16) ;
			int y2_x2_colors = (y2_xsource+x2)*colors;
			int i_x_colors = (i_xdest+j)*colors;
			int c;
			for (c=0; c<colors; c++)
				dest[i_x_colors + c] = source[y2_x2_colors + c] ;
		}
	}
}*/

/*bool YUVFrame::decode()
{
   //
	// decode luma & chroma plane or lets say sort it
	//
	unsigned int x,y,luna_mem_pos = 0, chroma_mem_pos = 0, dat1 = 0;         

   int blocksize   = 0x80;
   int skip        = (m_stb.chr_luma_stride)*skiplines; // Skip mem position
	int skipx		= skiplines;
	
	//
	// Fix for some strange resolution, from Oktay
	//
	if((stride/2)%2==1)
		stride-=2;

	for (x = 0; x < stride; x += m_stb.chr_luma_stride)
    {
        // check if we can still copy a complete block.
        if ((stride - x) <= m_stb.chr_luma_stride)
            blocksize = stride-x;

        dat1 = x;    // 1088    16 (68 x)
        for (y = 0; y < ofs; y+=skiplines)
        {
            int z1=0;
            int skipofs=0;
            for(int y1=0;y1<blocksize;y1+=skipx)
            {
                *(bitmap->m_luma + (dat1/skipx)+(y1/skipx))=*(y1+memory_tmp + pageoffset + luna_mem_pos);

                if (y < ofs2 && z1%2==0)
                {
                    skipofs=1;
                    bitmap->m_chroma[(dat1/skipx)+(y1/skipx)]=*(y1+memory_tmp + pageoffset + offset + chroma_mem_pos);
                    bitmap->m_chroma[(dat1/skipx)+(y1/skipx)+1]=*(1+y1+memory_tmp + pageoffset + offset + chroma_mem_pos);     
                }
                z1++;
            }

            if(skipofs==1)
                chroma_mem_pos += skip;
                
            skipofs=0;
            dat1 += stride;
            luna_mem_pos += skip;
                      
        }

        //Skipping invisble lines
        if ( (xres_orig == 1280 && yres_orig == 1080) ) luna_mem_pos += (ofs - yres_orig) * m_stb.chr_luma_stride;
    }

	if (yres_orig%2 == 1)
		yres_orig--;	// drop one line to make the numer even again

   return true;
}*/

/*bool YUVFrame::convert(const uint8_t* ptBufferIn, int iOutWidth, int iOutHeight, AVPicture* src)
{
   ///////////////////////////////////////////////////////////////////
   //
   //          Covert YUV to RGB image
   //
   ///////////////////////////////////////////////////////////////////

   int  posb, x, y;
   long pos_chr, pos_luma, pos_rgb, rgbskip; // relative position in chroma- and luma-map
   int  Y, U, V, RU, GU, GV, BV;             // YUV-Coefficients
   int  U2, V2, RU2, GU2, GV2, BV2;          // YUV-Coefficients
  
   // yuv2rgb conversion (4:2:0)
   rgbskip = m_xres * 3;
  
   pos_rgb = 0;

   for (y=0; y < m_yres; y+=2)
   {     
      for (x=0; x < m_xres; x+=2)
      {
         pos_luma = x + (y * m_xres);
         pos_chr  = x + (y * m_xres / 2);

         // chroma contains both U and V data

         U2=m_chroma[pos_chr+1]; //2
         V2=m_chroma[pos_chr+0]; //3 litte->big endian :)

         RU2=yuv2rgbtable_ru[V2]; 
         GU2=yuv2rgbtable_gu[U2];
         GV2=yuv2rgbtable_gv[V2];
         BV2=yuv2rgbtable_bv[U2];

         // now we do 4*2 pixels on each iteration this is more code but much faster 
         Y=yuv2rgbtable_y[m_luma[pos_luma]]; 
         m_data[pos_rgb+0]=CLAMP((Y + RU2)>>16);
         m_data[pos_rgb+1]=CLAMP((Y - GV2 - GU2)>>16);
         m_data[pos_rgb+2]=CLAMP((Y + BV2)>>16);

         Y=yuv2rgbtable_y[m_luma[m_xres+pos_luma]];
         m_data[pos_rgb+0+rgbskip]=CLAMP((Y + RU2)>>16);
         m_data[pos_rgb+1+rgbskip]=CLAMP((Y - GV2 - GU2)>>16);
         m_data[pos_rgb+2+rgbskip]=CLAMP((Y + BV2)>>16);
               
         pos_rgb  +=3; 

         Y=yuv2rgbtable_y[m_luma[pos_luma+1]];
         m_data[pos_rgb+0]=CLAMP((Y + RU2)>>16);
         m_data[pos_rgb+1]=CLAMP((Y - GV2 - GU2)>>16);
         m_data[pos_rgb+2]=CLAMP((Y + BV2)>>16);

         Y=yuv2rgbtable_y[m_luma[m_xres+pos_luma+1]];
         m_data[pos_rgb+0+rgbskip]=CLAMP((Y + RU2)>>16);
         m_data[pos_rgb+1+rgbskip]=CLAMP((Y - GV2 - GU2)>>16);
         m_data[pos_rgb+2+rgbskip]=CLAMP((Y + BV2)>>16);

         pos_rgb  +=3; // skip forward for the next group of 4 pixels
      }
      pos_rgb+=rgbskip; // skip a complete line
   }
  
  return true;
}*/

YUVFrame::~YUVFrame()
{
}
