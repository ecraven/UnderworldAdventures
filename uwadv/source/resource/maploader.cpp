/*
   Underworld Adventures - an Ultima Underworld hacking project
   Copyright (c) 2002 Michael Fink

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
/*! \file maploader.cpp

   \brief game level map loading implementation

*/

// needed includes
#include "common.hpp"
#include "underworld.hpp"
#include "level.hpp"
#include "fread_endian.hpp"
#include <string>


// tables

ua_levelmap_tiletype ua_tile_type_mapping[16] =
{
   ua_tile_solid,
   ua_tile_open,
   ua_tile_diagonal_se,
   ua_tile_diagonal_sw,
   ua_tile_diagonal_ne,
   ua_tile_diagonal_nw,
   ua_tile_slope_n,
   ua_tile_slope_s,
   ua_tile_slope_e,
   ua_tile_slope_w,
   ua_tile_solid,
   ua_tile_solid,
   ua_tile_solid,
   ua_tile_solid,
   ua_tile_solid,
   ua_tile_solid
};


// global functions

void ua_import_levelmaps(ua_settings &settings, const char *folder,
   std::vector<ua_level> &levels)
{
   unsigned int numlevels = settings.get_gametype() == ua_game_uw_demo ? 1 : 9;
   levels.reserve(numlevels);

   // open map file
   std::string mapfile(settings.get_string(ua_setting_uw_path));
   if (settings.get_gametype() == ua_game_uw_demo)
      mapfile.append("data/level13.st");
   else
   {
      mapfile.append(folder);
      mapfile.append("lev.ark");
   }

   FILE *fd = fopen(mapfile.c_str(),"rb");
   if (fd==NULL)
   {
      std::string text("could not open file ");
      text.append(mapfile);
      throw ua_exception(text.c_str());
   }

   Uint16 wall_textures[48];
   Uint16 floor_textures[10];

   // uw_demo is treated specially
   if (settings.get_gametype() == ua_game_uw_demo)
   {
      ua_level level;

      // import texture usage table
      {
         mapfile.assign(settings.get_string(ua_setting_uw_path));
         mapfile.append("data/level13.txm");

         FILE *fd2 = fopen(mapfile.c_str(),"rb");
         if (fd2==NULL)
         {
            std::string text("could not open file ");
            text.append(mapfile);
            throw ua_exception(text.c_str());
         }
         level.import_texinfo(fd2,wall_textures,floor_textures);
         fclose(fd2);
      }

      // import map
      level.import_map(fd,ua_tex_stock_floor+9,wall_textures,floor_textures);

      // import objects
      fseek(fd,0,SEEK_SET);
      level.get_mapobjects().import_objs(fd);

      levels.push_back(level);
   }
   else
   {

   // read in offsets
   std::vector<Uint32> offsets;
   Uint16 numoffsets = fread16(fd);
   offsets.resize(numoffsets,0);

   for(Uint16 n=0; n<numoffsets; n++)
      offsets[n]=fread32(fd);

   // load all levels
   for(unsigned int i=0; i<numlevels; i++)
   {
      ua_level level;

      // load texture usage table
      fseek(fd,offsets[i+18],SEEK_SET);
      level.import_texinfo(fd,wall_textures,floor_textures);

      // load level map
      fseek(fd,offsets[i],SEEK_SET);
      level.import_map(fd,ua_tex_stock_floor+15,wall_textures,floor_textures);

      // load object list
      fseek(fd,offsets[i],SEEK_SET);
      level.get_mapobjects().import_objs(fd);

      levels.push_back(level);
   }

   } // end if
   fclose(fd);
}


// ua_underworld methods

void ua_underworld::import_savegame(ua_settings &settings,const char *folder,bool initial)
{
   // load level maps
   ua_import_levelmaps(settings,folder,levels);

   // load conv globals
   {
      std::string bgname(settings.get_string(ua_setting_uw_path));
      bgname.append(folder);
      bgname.append(initial ? "babglobs.dat" : "bglobals.dat");
      conv_globals.import(bgname.c_str(),initial);
   }

   // reload level
   change_level(player.get_attr(ua_attr_maplevel));
}


// ua_level methods

void ua_level::import_map(FILE *fd, Uint16 ceil_tex, Uint16 wall_textures[48],
   Uint16 floor_textures[10])
{
   // read in map info

   // alloc memory for tiles
   tiles.clear();
   tiles.resize(64*64);

   for(Uint16 tile=0; tile<64*64; tile++)
   {
      Uint32 tileword = fread32(fd);

      // extract infos from tile word
      tiles[tile].type = ua_tile_type_mapping[tileword & 0x0000000F];

      // all size values in height units
      tiles[tile].floor = (tileword & 0xF0) >> 3;
      tiles[tile].ceiling = 32;
      tiles[tile].slope = 2; // height units per tile

      // texture indices
      Uint8 wall_index = (tileword & 0x003F0000) >> 16; // 6 bit wide
      Uint8 floor_index = (tileword & 0x00003C00) >> 10; // 4 bit wide

      if (wall_index>=48) wall_index=0;
      if (floor_index>=10) floor_index=0;

      tiles[tile].texture_wall = wall_textures[wall_index];
      tiles[tile].texture_floor = floor_textures[floor_index];
      tiles[tile].texture_ceiling = ceil_tex;
   }
}

void ua_level::import_texinfo(FILE *fd, Uint16 wall_textures[48],
   Uint16 floor_textures[10])
{
   used_textures.clear();

   Uint16 tex;
   for(tex=0; tex<48; tex++)
   {
      wall_textures[tex]  = fread16(fd)+ua_tex_stock_wall;
      used_textures.push_back(wall_textures[tex]);
   }
   for(tex=0; tex<10; tex++)
   {
      floor_textures[tex] = fread16(fd)+ua_tex_stock_floor;
      used_textures.push_back(floor_textures[tex]);
   }

//   for(tex=0; tex<6; tex++)  door_textures[tex]  = fread16(fd);
}
