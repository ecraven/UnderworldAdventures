/*
   Underworld Adventures - an Ultima Underworld hacking project
   Copyright (c) 2003 Underworld Adventures Team

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

   $Id$

*/
/*! \file levark.cpp

   \brief level archive decoding implementation

*/

// needed includes
#include "common.hpp"
#include "levark.hpp"
#include <algorithm>
#include <bitset>


// ua_dump_level_archive member

void ua_dump_level_archive::start(std::string& basepath,std::string& param)
{
   printf("level archive dumping\nprocessing file %s%s\n",
      basepath.c_str(),param.c_str());

   // load game strings
   {
      std::string strpak(basepath);
      strpak.append("data/strings.pak");
      try
      {
         gstr.load(strpak.c_str());
      }
      catch(...)
      {
      }
   }

   // try to open file
   std::string filename(basepath);
   filename.append(param);

   FILE* fd = fopen(filename.c_str(),"rb");

   if (fd==NULL)
   {
      printf("could not open file!\n");
      return;
   }

   // do file blocks overview
   {
      // get file length
      fseek(fd,0L,SEEK_END);
      Uint32 filelen = ftell(fd);
      fseek(fd,0L,SEEK_SET);

      Uint16 noffsets = fread16(fd);

      printf("file length: %08x bytes, %u slots\n\n",filelen,noffsets);

      // read in all offsets
      for(Uint16 n=0; n<noffsets; n++)
      {
         Uint32 offset = fread32(fd);
         if (offset != 0)
         {
            offsets_ordered.push_back(offset);
            offsets_list.push_back(offset);
         }
      }

      std::sort(offsets_ordered.begin(),offsets_ordered.end());
      offsets_ordered.push_back(filelen);

      // dump blocks
      printf("block  offset    size  description\n");
      unsigned int max = offsets_list.size()-1;
      for(unsigned int i=0; i<max; i++)
      {
         Uint32 offset = offsets_list[i];

         // search offset
         unsigned int a,max2 = offsets_ordered.size();
         for(a=0; a<max2 && offsets_ordered[a] != offset; a++);

         Uint32 next_offset = offsets_ordered[i+1];
         if (a==max2-1)
            next_offset = offsets_ordered[i];

         next_offset -= offsets_ordered[i];

         printf("%4u   %08x  %04x  %s\n",
            i,offsets_ordered[i],next_offset,
            next_offset == 0x7c08 ? "tile map + master object list" :
            next_offset == 0x0180 ? "animation overlay info" :
            next_offset == 0x007a ? "texture mapping" :
            next_offset == 0x1000 ? "automap info" : ""
            );
      }
      printf("\n");
   }

   unsigned int nlevels = !is_uw2 ? 9 : 50;

   // process all levels
   for(level=0; level<nlevels; level++)
   {
      fseek(fd,level*4+2,SEEK_SET);
      Uint32 offset = fread32(fd);
      fseek(fd,offset,SEEK_SET);

      objinfos.clear();
      npcinfos.clear();

      linkcount.clear();
      linkcount.resize(0x400,0);
      linkref.clear();
      linkref.resize(0x400,0);
      tilemap_links.clear();
      item_positions.clear();
      item_positions.resize(0x400,std::make_pair<Uint8,Uint8>(0xff,0xff));
      freelist.clear();
      freelist.insert(0);

      if (is_uw2)
      {
         // TODO load uw2 file
      }
      else
      {
         // read in tilemap
         {
            for(unsigned int i=0; i<64*64; i++)
            {
               Uint16 tileword = fread16(fd);
               tileword = fread16(fd);

               Uint16 index = get_bits(tileword,6,10);
               tilemap_links.push_back(index);
            }
         }

         // read in master object list
         for(unsigned int i=0; i<0x400; i++)
         {
            // read object info
            objinfos.push_back(fread16(fd));
            objinfos.push_back(fread16(fd));
            objinfos.push_back(fread16(fd));
            objinfos.push_back(fread16(fd));

            // read npc extra info
            if (i<0x100)
            {
               for(unsigned int j=0; j<27-8; j++)
                  npcinfos.push_back(fgetc(fd));
            }
         }

         // read in free list
         fseek(fd,0x7c02-0x7300,SEEK_CUR);
         Uint16 mobile_items = fread16(fd);
         Uint16 static_items = fread16(fd);

         fseek(fd,0x7300-0x7c06,SEEK_CUR);
         for(unsigned int j=0; j<mobile_items+1; j++)
         {
            Uint16 pos = fread16(fd);
            freelist.insert(pos);
         }

         fseek(fd,0x01fc-(mobile_items+1)*2,SEEK_CUR);
         for(unsigned int k=0; k<static_items+1; k++)
         {
            Uint16 pos = fread16(fd);
            freelist.insert(pos);
         }

         // texture mapping
         fseek(fd,(level+nlevels*2)*4+2,SEEK_SET);
         offset = fread32(fd);
         fseek(fd,offset,SEEK_SET);

         {
            for(unsigned int i=0; i<48; i++)
               tex_wall[i] = fread16(fd);

            for(unsigned int j=0; j<10; j++)
               tex_floor[j] = fread16(fd);
         }
      }

      objinfos[4] |= 0x007f; // an_adventurer item_id

      process_level();
      dump_infos();
   }

   fclose(fd);
}

void ua_dump_level_archive::process_level()
{
   // follow all links and special links, starting from the tilemap indices
   for(unsigned int i=0; i<64*64; i++)
   {
      Uint16 index = tilemap_links[i];

      if (index != 0)
      {
         follow_level = 0;
         linkref[index] = 0xffff; // from tilemap
         follow_link(index,i,false);
      }
   }
}

void ua_dump_level_archive::dump_infos()
{
   printf("dumping infos for level %u\n\n",level);

   process_level();

   // dump every object in list
   {
      printf("dumping master object list:\n");
      for(unsigned int i=0; i<0x400; i++)
         dump_item(i);

      printf("\n");
   }

   // dumping npc infos
   {
      printf("dumping npc infos for mobile objects:\n");

      for(unsigned int i=0; i<0x100; i++)
      {
         bool is_free = freelist.find(i)!=freelist.end();
         if (is_free) continue;

         dump_npcinfos(i);
      }
      printf("\n");
   }

   // dumping npc inventories
   {
      printf("dumping npc inventory:\n");
      for(unsigned int i=0; i<0x100; i++)
      {
         bool is_free = freelist.find(i)!=freelist.end();
         if (is_free) continue;

         // check if inventory
         bool is_link = get_bits(objinfos[i*4+0],15,1) == 0;
         Uint16 special = get_bits(objinfos[i*4+3],6,10);

         Uint16 item_id = get_bits(objinfos[i*4+0],0,9);

         if (is_link && special != 0 && item_id>=0x0040 && item_id<0x0080)
         {
            printf("\ninventory for npc:\n");
            dump_npcinfos(i);

            // follow inventory object chain
            Uint16 link = special;
            do
            {
               bool is_free = freelist.find(link)!=freelist.end();
               if (is_free) break;

               dump_item(link);
               link = get_bits(objinfos[link*4+2],6,10);
            }
            while(link != 0);
         }
      }
      printf("\n");
   }

   // dumping traps
   printf("dumping traps:\n");

   for(unsigned int j=0x0180; j<0x0191; j++)
   {
      for(unsigned int pos=0; pos<0x400; pos++)
      {
         bool is_free = freelist.find(pos)!=freelist.end();
         Uint16 item_id = get_bits(objinfos[pos*4],0,9);

         if (!is_free && j==item_id)
            dump_item(pos);
      }
   }

   // following buttons/triggers, etc.
   printf("\ndumping special link chains:\n");
   {
      std::bitset<0x400> visited;
      visited.reset();

      //for(int m=0x03ff; m>=0; m--)
      for(unsigned int pos=0; pos<0x400; pos++)
      {
         Uint16 item_id = get_bits(objinfos[pos*4],0,9);

         // don't start with non-tilemap objects or a teleport trap
         if ((item_positions[pos].first == 0xff && item_positions[pos].second == 0xff) ||
            item_id == 0x0181)
            continue;

         dump_special_link_chain(visited,pos,0);
      }
   }

   // dumping texture mapping
   printf("\ndumping texture mapping:\nwall textures:");
   {
      unsigned int i;
      for(i=0; i<48; i++)
         printf("%c%04x",(i%8)==0? '\n' : ' ',tex_wall[i]);

      printf("\n\nceiling textures:");
      for(i=0; i<10; i++)
         printf("%c%04x",(i%8)==0? '\n' : ' ',tex_floor[i]);
   }

   printf("\n\n");
}

void ua_dump_level_archive::dump_special_link_chain(std::bitset<0x400>& visited, unsigned int pos,
                                                    unsigned int indent)
{
   unsigned int step=0;

   Uint16 item_id = get_bits(objinfos[pos*4],0,9);
   bool is_free = freelist.find(pos)!=freelist.end();

   Uint16 special = get_bits(objinfos[pos*4+3],6,10);
   bool is_link = get_bits(objinfos[pos*4+0],15,1) == 0;

   while((pos!=0 && !visited.test(pos) && !is_free &&
      is_link && special != 0 && (item_id < 0x0040 || item_id >= 0x0080))
      || (item_id >= 0x01a0 && item_id <= 0x01a6) // all triggers have is_quantity set
      )
   {
      if (step==0 && indent==0)
         printf("\nnext chain:\n");

      { for(unsigned int i=0; i<indent; i++) printf("     "); }

      printf("{%02x} ",step);
      if (pos<0x100)
         dump_npcinfos(pos);
      else
         dump_item(pos);

      if (item_id >= 0x0080 && item_id < 0x008e)
      {
         // dump container items
         { for(unsigned int i=0; i<indent; i++) printf("     "); }
         printf("container items:\n");

         Uint16 link = get_bits(objinfos[pos*4+3],6,10);
         do
         {
            // check if linked item has a special link
            bool is_link2 = get_bits(objinfos[link*4+0],15,1) == 0;
            Uint16 special2 = get_bits(objinfos[link*4+3],6,10);

            if (is_link2 && special2 != 0)
               dump_special_link_chain(visited,link,indent+1);
            else
            {
               { for(unsigned int i=0; i<indent; i++) printf("     "); }

               // no special link, just dump item
               dump_item(link);
            }


            link = get_bits(objinfos[link*4+2],6,10);
         } while(link != 0);

         return;
      }

      visited.set(pos);

      // get next object
      pos = special;

      if (pos != 0)
      {
         special = get_bits(objinfos[pos*4+3],6,10);
         is_link = get_bits(objinfos[pos*4+0],15,1) == 0;
         is_free = freelist.find(pos)!=freelist.end();
         item_id = get_bits(objinfos[pos*4+0],0,9);
      }

      step++;
   }

   if (step>0 && pos !=0)
   {
      { for(unsigned int i=0; i<indent; i++) printf("     "); }

      // print target item
      printf("{--} ");
      dump_item(pos);
   }
}

void ua_dump_level_archive::dump_item(Uint16 pos)
{
   Uint16* objptr = &objinfos[pos*4];

   printf("%04x: ",pos);

   bool is_free = freelist.find(pos)!=freelist.end();

   printf("[%04x %04x %04x %04x] ",
      objptr[0],objptr[1],objptr[2],objptr[3]);

   if (is_free)
   {
      printf("[free slot]\n");
      return;
   }

   Uint16 item_id = get_bits(objptr[0],0,9);
   Uint16 quality = get_bits(objptr[2],0,6);
   Uint16 owner = get_bits(objptr[3],0,6);

   printf("id=%04x ",item_id);
   printf("link=%04x ",get_bits(objptr[2],6,10));

   printf("flags=%x invis=%x ench=%x is_quant=%x ",
      get_bits(objptr[0],9,4), get_bits(objptr[0],14,1),
      get_bits(objptr[0],12,1),get_bits(objptr[0],15,1));

   printf("ref=%02x ",linkcount[pos]);
   if (linkref[pos]==0xffff)
      printf("linkref=tile ");
   else
      printf("linkref=%04x ",linkref[pos]);

   printf("tile=%02x/%02x ",item_positions[pos].first,
      item_positions[pos].second);

   bool is_link = get_bits(objptr[0],15,1) == 0;
   Uint16 special = get_bits(objptr[3],6,10);

   printf("[xpos=%u ypos=%u heading=%u zpos=%02x] ",
      get_bits(objptr[1],13,3),
      get_bits(objptr[1],10,3),
      get_bits(objptr[1],7,2),
      get_bits(objptr[1],0,7) );

   switch(item_id)
   {
   case 0x016e: // special tmap object
      printf("[texidx=%02x tex=%04x texname=\"%s\"]",owner,tex_wall[owner],
         gstr.get_string(10,tex_wall[owner]).c_str());
      break;

   case 0x0181: // a_teleport trap
      printf("[level=%02x dest=%02x/%02x",
         get_bits(objptr[1],0,7),
         get_bits(objptr[2],0,6),
         get_bits(objptr[3],0,6)
         );

      if (is_link)
         printf(" sp_link=%04x",special);

      printf("] ");
      break;

   case 0x0190: // a_text string trap
      {
         Uint16 id = (quality << 5) | owner;
         std::string text(gstr.get_string(9,id));

         std::string::size_type pos;
         while( std::string::npos != (pos=text.find("\n")))
            text.replace(pos,1,"\\n");

         printf("[string=%04x text=\"%s\"] ",id,text.c_str());
      }
      break;

   case 0x018d: // a_set variable trap
      {
         Uint16 value = get_bits(objptr[2],0,6) << 5; // quality
         value |= get_bits(objptr[3],0,6); // owner
         value <<= 3;
         value |= get_bits(objptr[1],10,3); // ypos

         Uint16 heading = get_bits(objptr[1],7,3);
         Uint16 zpos = get_bits(objptr[1],0,7);

         if (zpos==0)
         {
            // modify bits in bit array
            if (heading==1)
            {
               // mask out bit
               printf("[modify bit, heading=%u, clear bit %u] ",heading,value);
            }
            else
            if (heading==5)
            {
               printf("[modify bit, heading=%u, flip bit %u] ",heading,value);
            }
            else
            {
               printf("[modify bit, heading=%u, set bit %u] ",heading,value);
            }
         }
         else
         {
            // modify variable
            printf("[modify var, type=%u, var[%02x] ",heading,zpos);
            switch(heading)
            {
            case 0: printf("add %04x",value); break;
            case 1: printf("sub %04x",value); break;
            case 2: printf("set %04x",value); break;
            case 3: printf("and %04x",value); break;
            case 4: printf("or  %04x",value); break;
            case 5: printf("xor %04x",value); break;
            case 6: printf("shl %04x",value); break;
            default:
               printf("unknown operand");
            }
            printf("] ");
         }

      }
      break;

   default:
      printf("[quality=%02x ",quality);
      printf("owner=%02x ",owner);

      printf("%-8s=%04x] ",
         is_link? "sp_link" : special < 0x200 ? "quantity" : "special",
         special);
      break;
   }

   printf("name=%s",gstr.get_string(4,item_id).c_str());
   printf("\n");
}

void ua_dump_level_archive::dump_npcinfos(Uint16 pos)
{
   Uint16* objptr = &objinfos[pos*4];
   Uint8* infoptr = &npcinfos[pos*19];

   printf("%04x: ",pos);

   Uint16 item_id = get_bits(objptr[0],0,9);
   printf("id=%04x ",item_id);

   // print inventory link
   bool is_link = get_bits(objptr[0],15,1) == 0;
   Uint16 special = get_bits(objptr[3],6,10);
   printf(is_link ? "inv=%04x " : "inv=no   ",special);

   printf("[hp=%02x ",infoptr[0]);

   Uint16 word_b = infoptr[6] | (infoptr[7]<<8);
   printf("goal=%x ",get_bits(word_b,0,4));
   printf("gtarg=%02x ",get_bits(word_b,4,8));

   Uint16 word_d = infoptr[8] | (infoptr[9]<<8);
   printf("level=%x ",get_bits(word_d,0,4));
   printf("talkedto=%x ",get_bits(word_d,13,1));
   printf("attitude=%x] ",get_bits(word_d,14,2));

   printf("pos=%02x/%02x ",
      item_positions[pos].first,item_positions[pos].second);

   Uint16 home = infoptr[15];
   home = (home << 8) | infoptr[14];
   printf("home=%02x/%02x ",get_bits(home,10,6),get_bits(home,4,6));

   printf("[");
   for(unsigned int i=0; i<19; i++)
   {
      printf("%02x%c",infoptr[i],i==18?0:' ');
   }
   printf("] ");

   // npc type and name
   printf("type=%-20.20s ",gstr.get_string(4,item_id).c_str());

   printf("whoami=%02x ",infoptr[18]);
   if (infoptr[18]!=0)
      printf("name=%s",gstr.get_string(7,infoptr[18]+16).c_str());
   else
      printf("name=n/a");

   printf("\n");
}

void ua_dump_level_archive::follow_link(Uint16 link, unsigned int tilepos, bool special)
{
   if (++follow_level>32)
   {
      printf("follow_link(): recursion level too high\n");
      return;
   }

   do
   {
      Uint16* objptr = &objinfos[link*4];

      // ref count
      linkcount[link]++;

      // must be occupied
      //freelist.erase(link);

      if (!special)
      {
         // store tilemap pos for that link
         item_positions[link] =
            std::make_pair<Uint8,Uint8>(tilepos&63,tilepos>>6);
      }

      // check if we should follow special links
      bool is_link = get_bits(objptr[0],15,1) == 0;
      if (is_link)
      {
         Uint16 special = get_bits(objptr[3],6,10);
         if (special != 0)
         {
            linkref[special] = link;
            follow_link(special,0,true);
         }
      }

      Uint16 oldlink = link;

      // get next item in chain
      link = get_bits(objptr[2],6,10);

      if (link!=0)
         linkref[link] = oldlink;
   }
   while(link != 0);

   --follow_level;
}
