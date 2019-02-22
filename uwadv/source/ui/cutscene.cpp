//
// Underworld Adventures - an Ultima Underworld hacking project
// Copyright (c) 2002,2003,2004,2019 Underworld Adventures Team
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
/// \file cutscene.cpp
/// \brief cutscene frame extraction
//
#include "common.hpp"
#include "cutscene.hpp"
#include "cutsloader.hpp"

/*
void ua_cutscene::load(Base::Settings& settings, unsigned int main,
   unsigned int sub)
{
   char buffer[32];
   sprintf(buffer,"cuts/cs%03o.n%02o",main,sub);

   load(settings,buffer);
}
*/

void ua_cutscene::load(Base::Settings& settings, const char* relfilename)
{
   std::string filename(settings.GetString(Base::settingUnderworldPath));
   filename.append(relfilename);

   load(filename.c_str());
}

void ua_cutscene::load(const char* filename)
{
   Import::CutsceneLoader::LoadCutscene(filename, image, lpdarray, lpages, records);

   curframe = (unsigned int)-1;
}

void ua_cutscene::update_frame(unsigned int framenum)
{
   if (curframe != framenum)
   {
      // when we are at the max, start again from the first
      if (curframe > framenum)
      {
         curframe = 0;
         decode_frame(0);
      }

      // decode all frames between the current and the wanted frame
      while (curframe < framenum)
      {
         curframe++;
         decode_frame(curframe);
      }
   }

   // update image quad
   update();
}

void ua_cutscene::decode_frame(unsigned int framenum)
{
   unsigned int largepages = lpdarray.size();

   // first, search large page to use
   unsigned int i = 0;
   for (; i < largepages; i++)
      if (lpdarray[i].base <= framenum && unsigned(lpdarray[i].base + lpdarray[i].records) > framenum)
         break;

   if (i >= largepages)
      throw Base::Exception("could not find frame in large pages");

   // calculate large page pointer
   Uint16* curlp16 = reinterpret_cast<Uint16*>(&lpages[0x10000 * i]);
   Uint8* curlp = reinterpret_cast<Uint8*>(curlp16);

   unsigned int destframe = framenum - lpdarray[i].base;

   // calculate offset to proper record
   unsigned int offset = 0;
#if (SDL_BYTEORDER==SDL_BIG_ENDIAN)
   for (unsigned int j = 0; j < destframe; j++)
      offset += ua_endian_convert16(curlp16[j + 4]);
#else
   for (unsigned int j = 0; j < destframe; j++)
      offset += curlp16[j + 4];
#endif

   // calculate start of "record" struct
   Uint8* src = curlp + 8 + lpdarray[i].records * 2 + offset;
   Uint16* src16 = reinterpret_cast<Uint16*>(src);

   // add extra offset
   if (src[1])
      src += (src16[1] + (src16[1] & 1));

   // extract the pixel data
   Import::CutsceneLoader::extract_cutscene_data(&src[4], &image.get_pixels()[0],
      image.get_xres()*image.get_yres());
}


#ifdef PROFILING_CUTSCENE

#include <time.h>

int main(int argc, char**argv)
{
   time_t begin, stop;
   int i, max;

   // benchmark ctor
   double ctor_time;
   {
      begin = time(0);
      max = 200000;
      for (i = 0; i < max; i++)
      {
         ua_cutscene cuts;
      }
      stop = time(0);

      ctor_time = double(stop - begin)*1000.0 / double(max);
      printf("ctor : %1.2f ms\n", ctor_time);
   }

   // benchmark loading
   double load_time;
   {
      begin = time(0);
      max = 20;
      for (i = 0; i < max; i++)
      {
         ua_cutscene cuts;
         cuts.load(argv[1]);
      }
      stop = time(0);

      load_time = double(stop - begin)*1000.0 / double(max) - ctor_time;
      printf("load : %3.1f ms\n", load_time);
   }

   // benchmark extracting first frame
   double frame1_time;
   {
      ua_texture tex;

      begin = time(0);
      max = 100;
      for (i = 0; i < max; i++)
      {
         ua_cutscene cuts;
         cuts.load(argv[1]);
         cuts.get_frame(tex, 0);
      }
      stop = time(0);

      frame1_time = double(stop - begin)*1000.0 / double(max) - load_time;
      printf("first frame : %2.1f ms\n", frame1_time);
   }

   // benchmark extracting first frame 100 times
   double frame2_time;
   {
      ua_texture tex;
      ua_cutscene cuts;
      cuts.load(argv[1]);
      cuts.get_frame(tex, 0);

      begin = time(0);
      max = 100;
      for (i = 0; i < max; i++)
      {
         cuts.get_frame(tex, 0);
      }
      stop = time(0);

      frame2_time = double(stop - begin)*1000.0 / double(max);
      printf("first frame, %u times : %2.1f ms\n", max, frame2_time);
   }

   // benchmark extracting all frames of the cutscene
   double frame3_time;
   {
      ua_texture tex;
      ua_cutscene cuts;
      cuts.load(argv[1]);

      begin = time(0);
      max = 100;
      int max2 = cuts.get_maxframes();

      for (i = 0; i < max; i++)
      {
         for (int j = 0; j < max2; j++)
            cuts.get_frame(tex, j);
      }
      stop = time(0);

      frame3_time = double(stop - begin)*1000.0 / double(max*max2);
      printf("all frames : %2.1f ms\n", frame3_time);
   }

   return 0;
}

#endif
