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
/*! \file underworld.cpp

   \brief underworld object

*/

// needed includes
#include "common.hpp"
#include "underworld.hpp"
#include "uamath.hpp"


// ua_underworld methods

void ua_underworld::init(ua_game_core_interface *thecore)
{
   core=thecore;

   curlevel=0;

   levels.clear();

   physics.init(this);

   inventory.init();

   player.init();
   player.set_pos(32.0,2.0);
   player.set_angle(90.0);
   player.set_attr(ua_attr_gender,1);
}

void ua_underworld::walk_player(double angle)
{
   // speed: 1.8 tiles per second
   double speed = 1.8 / core->get_tickrate();

   ua_vector2d dir;
   dir.set_polar(speed,angle);

   physics.walk_player(dir);
}

double ua_underworld::get_player_height()
{
   // TODO: ask physics model about player height
   return get_current_level().get_floor_height(player.get_xpos(),player.get_ypos());
}

void ua_underworld::render(ua_frustum &fr)
{
   get_current_level().render(core->get_texmgr(),fr);
}

ua_level &ua_underworld::get_current_level()
{
   return levels[curlevel];
}

void ua_underworld::change_level(unsigned int level)
{
   if (level>levels.size())
      throw ua_exception("game wanted to change to unknown level");

   curlevel = level;
   get_current_level().prepare_textures(core->get_texmgr());
}
