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
/*! \file levark.hpp

   \brief lev.ark level archive decoding

*/

// include guard
#ifndef uwdump_levark_hpp_
#define uwdump_levark_hpp_

// needed includes
#include "uwdump.hpp"
#include "gamestrings.hpp"
#include <set>
#include <bitset>


// classes

//! dumps level archives
class ua_dump_level_archive
{
public:
   void start(std::string& basepath,std::string& param);
   void process_level();
   void dump_infos();
   void dump_special_link_chain(std::bitset<0x400>& visited, unsigned int pos,unsigned int indent=0);
   void dump_item(Uint16 pos);
   void dump_npcinfos(Uint16 pos);

   void follow_link(Uint16 link, unsigned int tilepos, bool special);

protected:
   // current underworld level
   unsigned int level;

   // recursion level for follow_link()
   unsigned int follow_level;

   // link chain index from tilemap, 64*64 bytes long
   std::vector<Uint16> tilemap_links;

   // lev.ark offsets in map/ordered form
   std::vector<Uint32> offsets_list;
   std::vector<Uint32> offsets_ordered;

   std::set<Uint16> freelist;

   // common object info bytes, 0x400*8 bytes long
   std::vector<Uint16> objinfos;

   // npc extra info bytes, 0x100*(0x1b-8) bytes long
   std::vector<Uint8> npcinfos;

   // reference count for all objects, 0x400 long
   std::vector<unsigned int> linkcount;

   // link reference index, 0x400 long
   std::vector<Uint16> linkref;

   // tilemap position for all objects, 0x400 long
   std::vector<std::pair<Uint8,Uint8> > item_positions;

   Uint16 tex_wall[48];
   Uint16 tex_floor[10];

   // game strings
   ua_gamestrings gstr;
};

#endif
