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
/*! \file uwscript.cpp

   \brief underworld Lua script bindings

*/

// needed includes
#include "common.hpp"
#include "uwscript.hpp"
#include "underworld.hpp"


// constants

//! name of lua userdata variable containing the "this" pointer
const char* ua_uwscript_lua_thisptr_name = "self";


// ua_underworld_script_bindings methods

void ua_underworld_script_bindings::init(ua_underworld *uw)
{
   // init lua state
   L = lua_open(256);

   // open lualib libraries
   lua_baselibopen(L);
   lua_strlibopen(L);
   lua_mathlibopen(L);

   // load all lua scripts
   ua_files_manager &fmgr = uw->get_game_core()->get_filesmgr();

   // read init text file
   std::string initlist_str;
   {
      SDL_RWops *uwinit = fmgr.get_uadata_file("uw1/scripts/uwinit.txt");

      if (uwinit==NULL)
         throw ua_exception("could not find underworld Lua scripts init file");

      // read in all of the file
      std::vector<char> initlist;

      SDL_RWseek(uwinit,0,SEEK_END);
      int size = SDL_RWtell(uwinit);
      SDL_RWseek(uwinit,0,SEEK_SET);

      if (size<=0)
         throw ua_exception("could not read underworld Lua scripts init file");

      initlist.resize(size+1,0);
      SDL_RWread(uwinit,&initlist[0],size,1);
      initlist[size]=0;

      initlist_str.assign(reinterpret_cast<char *>(&initlist[0]));
   }

   std::string::size_type pos=0;

   // remove '\r' chars
   do
   {
      pos = initlist_str.find_first_of('\r');
      if (pos != std::string::npos)
         initlist_str.erase(pos,1);

   } while (pos != std::string::npos);

   // load all scripts mentioned in init text file
   do
   {
      pos = initlist_str.find_first_of('\n');

      if (pos != std::string::npos)
      {
         // load single script
         std::string basename;
         basename.assign(initlist_str.c_str(),pos);

         if (basename.size()>0)
         {
            int ret = fmgr.load_lua_script(L,basename.c_str());
            if (ret!=0)
               throw ua_exception("could not execute underworld Lua script");
         }

         // remove from list
         initlist_str.erase(0,pos+1);
      }

   } while (pos != std::string::npos);

   register_functions();

   // set "this" pointer userdata
   lua_pushuserdata(L,uw);
   lua_setglobal(L,ua_uwscript_lua_thisptr_name);

   // call init function lua_init_script()
   lua_getglobal(L,"lua_init_script");
   checked_lua_call(0,0);
}

void ua_underworld_script_bindings::register_functions()
{
// template:
//   lua_register(L,"func_name",func_name);

   // ua_player access
   lua_register(L,"player_get_attr",player_get_attr);
   lua_register(L,"player_set_attr",player_set_attr);
   lua_register(L,"player_get_skill",player_get_skill);
   lua_register(L,"player_set_skill",player_set_skill);
   lua_register(L,"player_get_pos",player_get_pos);
   lua_register(L,"player_set_pos",player_set_pos);
   lua_register(L,"player_get_angle",player_get_angle);
   lua_register(L,"player_set_angle",player_set_angle);

   lua_register(L,"tilemap_get_tile",tilemap_get_tile);
   lua_register(L,"tilemap_get_type",tilemap_get_type);
   lua_register(L,"tilemap_set_type",tilemap_set_type);
   lua_register(L,"tilemap_get_floor",tilemap_get_floor);
   lua_register(L,"tilemap_set_floor",tilemap_set_floor);
   lua_register(L,"tilemap_get_automap_visible",tilemap_get_automap_visible);
   lua_register(L,"tilemap_set_automap_visible",tilemap_set_automap_visible)  ;
   lua_register(L,"tilemap_get_objlist_start",tilemap_get_objlist_start);
}

void ua_underworld_script_bindings::checked_lua_call(int params, int retvals)
{
   int ret = lua_call(L,params,retvals);
   if (ret!=0)
      ua_trace("Lua function call error code: %u\n",ret);
//      throw ua_exception("");
}

double ua_underworld_script_bindings::get_lua_constant(const char *name)
{
   lua_getglobal(L,name);

   double val = 0.0;
   if (lua_isnumber(L,-1)) val = lua_tonumber(L,-1);

   lua_pop(L,1);

   return val;
}

void ua_underworld_script_bindings::done()
{
   // call function lua_done_script()
   lua_getglobal(L,"lua_done_script");
   checked_lua_call(0,0);

   lua_close(L);
}

void ua_underworld_script_bindings::load_game(ua_savegame &sg)
{
   sg.begin_section("luascript");

   sg.end_section();
}

void ua_underworld_script_bindings::save_game(ua_savegame &sg)
{
   sg.begin_section("luascript");

   sg.end_section();
}


// Lua functions callable from C

void ua_underworld_script_bindings::lua_game_tick(double curtime)
{
   // call Lua function
   lua_getglobal(L,"lua_game_tick");
   lua_pushnumber(L,curtime);

   checked_lua_call(1,0);
}

bool ua_underworld_script_bindings::lua_inventory_is_container(Uint16 item_id)
{
   // call Lua function
   lua_getglobal(L,"lua_inventory_is_container");
   lua_pushnumber(L,static_cast<double>(item_id));

   checked_lua_call(1,1);

   // returns nil when no container
   bool ret = lua_isnumber(L,-1)!=0;

   // remove return parameters
   lua_pop(L,1);
   return ret;
}

ua_inv_item_category ua_underworld_script_bindings::lua_inventory_categorize_item(Uint16 item_id)
{
   // call Lua function
   lua_getglobal(L,"lua_inventory_categorize_item");
   lua_pushnumber(L,static_cast<double>(item_id));

   checked_lua_call(1,1);

   // get return value
   ua_inv_item_category cat = ua_inv_cat_normal;

   if (lua_isnumber(L,-1))
   {
      cat = static_cast<ua_inv_item_category>(
         static_cast<unsigned int>(lua_tonumber(L,-1)));
   }

   lua_pop(L,1);
   return cat;
}

void ua_underworld_script_bindings::lua_inventory_look(Uint16 item_pos)
{
}

void ua_underworld_script_bindings::lua_inventory_use(Uint16 item_pos)
{
}


ua_obj_combine_result ua_underworld_script_bindings::lua_obj_combine(
   Uint16 item_id1, Uint16 item_id2, Uint16 &result_id)
{
   // call Lua function
   lua_getglobal(L,"lua_obj_combine");
   lua_pushnumber(L,static_cast<double>(item_id1));
   lua_pushnumber(L,static_cast<double>(item_id2));

   checked_lua_call(2,2);

   // retrieve return values
   ua_obj_combine_result res = ua_obj_cmb_failed;

   if (lua_isnumber(L,-2))
   {
      res = static_cast<ua_obj_combine_result>(
         static_cast<unsigned int>(lua_tonumber(L,-2)));
   }

   result_id = 0xffff;
   if (lua_isnumber(L,-1))
   {
      result_id = static_cast<Uint16>(lua_tonumber(L,-1));
   }
   else
      res = ua_obj_cmb_failed;

   lua_pop(L,2);

   return res;
}

/*! params is the number of parameters the current registered function
    expects. it is needed to determine where the "self" userdata value is
    stored. */
ua_underworld& ua_underworld_script_bindings::get_underworld_from_self(lua_State* L)
{
   ua_underworld* self = NULL;

   lua_getglobal(L,ua_uwscript_lua_thisptr_name);
   if (!lua_isuserdata(L,-1))
      throw ua_exception("'self' parameter wasn't set by ua_underworld Lua script");

   // get pointer to underworld
   self = reinterpret_cast<ua_underworld*>(lua_touserdata(L,-1));

   if (self->get_scripts().L != L)
      throw ua_exception("wrong 'self' parameter in ua_underworld Lua script");

   lua_pop(L,1);

   return *self;
}


// registered C functions callable from Lua

// player_* functions

int ua_underworld_script_bindings::player_get_attr(lua_State *L)
{
   ua_player &pl = get_underworld_from_self(L).get_player();

   ua_player_attributes attr_type =
      static_cast<ua_player_attributes>(static_cast<int>(lua_tonumber(L,-1)));

   unsigned int attr_value = pl.get_attr(attr_type);
   lua_pushnumber(L,static_cast<double>(attr_value));

   return 1; // one return parameter
}

int ua_underworld_script_bindings::player_set_attr(lua_State *L)
{
   ua_player &pl = get_underworld_from_self(L).get_player();

   ua_player_attributes attr_type =
      static_cast<ua_player_attributes>(static_cast<int>(lua_tonumber(L,-2)));
   unsigned int attr_value = static_cast<unsigned int>(lua_tonumber(L,-1));

   pl.set_attr(attr_type,attr_value);

   return 0;
}

int ua_underworld_script_bindings::player_get_skill(lua_State *L)
{
   ua_player &pl = get_underworld_from_self(L).get_player();

   ua_player_skills skill_type =
      static_cast<ua_player_skills>(static_cast<int>(lua_tonumber(L,-1)));

   unsigned int skill_value = pl.get_skill(skill_type);
   lua_pushnumber(L,static_cast<double>(skill_value));

   return 1;
}

int ua_underworld_script_bindings::player_set_skill(lua_State *L)
{
   ua_player &pl = get_underworld_from_self(L).get_player();

   ua_player_skills skill_type =
      static_cast<ua_player_skills>(static_cast<int>(lua_tonumber(L,-2)));
   unsigned int skill_value = static_cast<unsigned int>(lua_tonumber(L,-1));

   pl.set_skill(skill_type,skill_value);

   return 0;
}

int ua_underworld_script_bindings::player_get_pos(lua_State *L)
{
   ua_player &pl = get_underworld_from_self(L).get_player();

   lua_pushnumber(L,pl.get_xpos());
   lua_pushnumber(L,pl.get_ypos());

   return 2;
}

int ua_underworld_script_bindings::player_set_pos(lua_State *L)
{
   ua_player &pl = get_underworld_from_self(L).get_player();

   double xpos = lua_tonumber(L,-2);
   double ypos = lua_tonumber(L,-1);

   pl.set_pos(xpos,ypos);

   return 0;
}

int ua_underworld_script_bindings::player_get_angle(lua_State *L)
{
   ua_player &pl = get_underworld_from_self(L).get_player();

   lua_pushnumber(L,pl.get_angle_rot());

   return 1;
}

int ua_underworld_script_bindings::player_set_angle(lua_State *L)
{
   ua_player &pl = get_underworld_from_self(L).get_player();

   double angle = lua_tonumber(L,-1);
   pl.set_angle_rot(angle);

   return 0;
}

// tilemap_* functions

ua_levelmap_tile& ua_underworld_script_bindings::get_tile_per_handle(
   ua_underworld& uw, unsigned int tile_handle)
{
   // decode the tile_handle hash, as seen from tilemap_get_tile()
   unsigned int xpos = tile_handle & 63;
   unsigned int ypos = (tile_handle >> 6) & 63;
   unsigned int level = tile_handle >> 12;

   ua_level& levelmap = uw.get_level(level);
   return levelmap.get_tile(xpos,ypos);
}

int ua_underworld_script_bindings::tilemap_get_tile(lua_State* L)
{
   ua_underworld &uw = get_underworld_from_self(L);

   int level = static_cast<int>(lua_tonumber(L,-3));
   unsigned int xpos = static_cast<unsigned int>(lua_tonumber(L,-1));
   unsigned int ypos = static_cast<unsigned int>(lua_tonumber(L,-2));

   // get current level, when level is negative
   if (level<0)
      level = uw.get_player().get_attr(ua_attr_maplevel);

   // the tile_handle is just a hash of level, xpos and ypos, but don't tell
   // the Lua programmers :-)
   unsigned int tile_handle = (level << 12) | (ypos << 6) | xpos;

   lua_pushnumber(L,static_cast<double>(tile_handle));

   return 1;
}

int ua_underworld_script_bindings::tilemap_get_type(lua_State* L)
{
   ua_underworld& uw = get_underworld_from_self(L);

   unsigned int tile_handle = static_cast<unsigned int>(lua_tonumber(L,-1));
   ua_levelmap_tile& tile = get_tile_per_handle(uw,tile_handle);

   lua_pushnumber(L,static_cast<double>(tile.type));

   return 1;
}

int ua_underworld_script_bindings::tilemap_set_type(lua_State* L)
{
   ua_underworld &uw = get_underworld_from_self(L);

   unsigned int tile_handle = static_cast<unsigned int>(lua_tonumber(L,-2));
   ua_levelmap_tile& tile = get_tile_per_handle(uw,tile_handle);

   tile.type = static_cast<ua_levelmap_tiletype>(
      static_cast<unsigned int>(lua_tonumber(L,-1) ));

   return 0;
}

int ua_underworld_script_bindings::tilemap_get_floor(lua_State* L)
{
   ua_underworld &uw = get_underworld_from_self(L);

   unsigned int tile_handle = static_cast<unsigned int>(lua_tonumber(L,-1));
   ua_levelmap_tile& tile = get_tile_per_handle(uw,tile_handle);

   lua_pushnumber(L,static_cast<double>(tile.floor));
   return 1;
}

int ua_underworld_script_bindings::tilemap_set_floor(lua_State* L)
{
   ua_underworld &uw = get_underworld_from_self(L);

   unsigned int tile_handle = static_cast<unsigned int>(lua_tonumber(L,-2));
   ua_levelmap_tile& tile = get_tile_per_handle(uw,tile_handle);

   tile.floor = static_cast<ua_levelmap_tiletype>(
      static_cast<unsigned int>(lua_tonumber(L,-1) ));

   return 0;
}

int ua_underworld_script_bindings::tilemap_get_automap_visible(lua_State* L)
{
   ua_underworld &uw = get_underworld_from_self(L);
   // TODO
   return 1;
}

int ua_underworld_script_bindings::tilemap_set_automap_visible(lua_State* L)
{
   ua_underworld &uw = get_underworld_from_self(L);
   // TODO
   return 0;
}

int ua_underworld_script_bindings::tilemap_get_objlist_start(lua_State* L)
{
   ua_underworld &uw = get_underworld_from_self(L);
   // TODO
   return 1;
}
