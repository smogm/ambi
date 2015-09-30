#include <iostream>
#include <signal.h>

#include "IFrameGrabber.hpp"
#include "STiFrameGrabber.hpp"

namespace
{
   bool keepRunning = true;
}

void signal_callback_handler(int signum)
{
   printf("Caught signal %d\n",signum);
   keepRunning = false;
}

int main(int argc, char** argv)
{
   size_t x,y;
   if (argc < 3)
   {
      x = 100;
      y = 200;
   }
   else
   {
      x = atoi(argv[1]);
      y = atoi(argv[2]);
   }
   
   signal(SIGINT, signal_callback_handler);
   signal(SIGTERM, signal_callback_handler);
   
   IFrameGrabber* frameGrabber = new STiFrameGrabber(x, y);
   if (*frameGrabber)
   {
      while(keepRunning)
      {
         frameGrabber->grabFrame();
      }
   }
   else
   {
      std::cerr << "initialization failed!" << std::endl;
   }

   delete frameGrabber;
   return 0;
}
