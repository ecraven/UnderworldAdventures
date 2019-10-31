//
// Underworld Adventures - an Ultima Underworld remake project
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
/// \file LuaScripting.hpp
/// \brief Lua scripting support
//
#pragma once

#include "Base.hpp"
#include "IScripting.hpp"
#include "IDebugServer.hpp"
#include <vector>
#include <map>
#include <set>

class IGame;
struct lua_State;
struct lua_Debug;

/// lua scripting class
class LuaScripting : public IScripting, public ICodeDebugger
{
public:
   /// ctor
   LuaScripting();
   virtual ~LuaScripting() {}

   /// returns lua state info struct
   lua_State* GetLuaState() { return L; }

   /// lua function call
   void CheckedCall(int numArgs, int numResults);

   // virtual methods from IScripting
   virtual void Init(IBasicGame* game) override;
   virtual bool LoadScript(const char* basename) override;
   virtual void Done() override;
   virtual void InitNewGame() override;
   virtual void EvalCritter(Uint16 pos) override;
   virtual void TriggerSetOff(Uint16 pos) override;
   virtual void UserAction(UserInterfaceUserAction action,
      unsigned int param) override;
   virtual void OnChangingLevel() override;

private:
   /// loads a script
   int LoadScript(Base::SDL_RWopsPtr rwops, const char* chunkname);

   /// returns scripting class from Lua state
   static LuaScripting& GetScriptingFromSelf(lua_State* L);

   /// debug hook called from Lua
   static void DebugHook(lua_State* L, lua_Debug* ar);

   /// translate Lua debug hook event codes to readable text
   static const char* LuaDebugEventNameFromInt(int event);

   /// debug hook called from Lua; now with actual scripting object
   void DebugHook(lua_Debug* ar);

   /// checks if a breakpoint was reached
   void CheckBreakpoints();

   /// waits for the debugger to continue debugging
   void WaitDebuggerContinue();

   /// registers all callable functions
   void RegisterFunctions();

   // virtual methods from ICodeDebugger
   virtual DebugServerCodeDebuggerType GetDebuggerType() override;
   virtual void PrepareDebugInfo() override;
   virtual DebugServerCodeDebuggerState GetDebuggerState() const override;
   virtual void SetDebuggerState(DebugServerCodeDebuggerState state) override;
   virtual DebugServerCodeDebuggerCommand GetDebuggerCommand() const override;
   virtual void SetDebuggerCommand(DebugServerCodeDebuggerCommand command) override;
   virtual void GetCurrentPos(size_t& sourcefileIndex, size_t& sourcefileLine,
      size_t& codePosition, bool& isSourcefileValid) override;
   virtual size_t GetNumSourcefiles() const override;
   virtual size_t GetSourcefileName(size_t index, char* buffer, size_t length) override;
   virtual size_t GetNumBreakpoints() const override;
   virtual void GetBreakpointInfo(size_t breakpointIndex,
      size_t& sourcefileIndex, size_t& sourcefileLine,
      size_t& codePosition, bool& visible) const override;

private:
   /// lua state information
   lua_State* L;

   /// ptr to basic game interface
   IBasicGame* m_game;

   /// name for 'self' global in Lua
   static const char* s_selfName;

   /// array with all loaded script files
   std::vector<std::string> m_loadedScriptFiles;

   /// map with all source files and line numbers
   std::map<std::string, std::set<unsigned int>> m_allLineNumbers;

   /// current debugger state
   DebugServerCodeDebuggerState m_debuggerState;

   /// current debugger command
   DebugServerCodeDebuggerCommand m_debuggerCommand;

   /// current position sourcefile index
   size_t m_currentPositionSourcefileIndex;

   /// current position sourcefile line
   size_t m_currentPositionSourcefileLine;

   /// function call depth when doing "step over"
   size_t m_stepOverFunctionCallDepth;

   /// list with all breakpoints
   std::vector<DebugCodeBreakpointInfo> m_breakpointsList;

private:
   // registered C functions callable from Lua
   // prototype: static int xyz(lua_State* L);
   static int uw_print(lua_State* L);        ///< Lua function uw.print
   static int uw_get_string(lua_State* L);   ///< uw.get_string
   static int uw_change_level(lua_State* L); ///< uw.change_level
   static int uw_start_conv(lua_State* L);   ///< uw.start_conv
   static int uw_show_cutscene(lua_State* L);      ///< uw.show_cutscene
   static int uw_show_ingame_anim(lua_State* L);   ///< uw.show_ingame_anim
   static int uw_cursor_use_item(lua_State* L);    ///< uw.cursor_use_item
   static int uw_cursor_target(lua_State* L);      ///< uw.cursor_target

   static int player_get_info(lua_State* L); ///< player.get_info
   static int player_set_info(lua_State* L); ///< player.set_info
   static int player_get_attr(lua_State* L); ///< player.get_attr
   static int player_get_skill(lua_State* L);///< player.get_skill
   static int player_set_attr(lua_State* L); ///< player.SetAttribute
   static int player_set_skill(lua_State* L);///< player.SetSkill

   static int objectlist_get_info(lua_State* L);///< objectlist.get_info
   static int objectlist_set_info(lua_State* L);///< objectlist.set_info
   static int objectlist_delete(lua_State* L);  ///< objectlist.delete
   static int objectlist_insert(lua_State* L);  ///< objectlist.insert

   static int inventory_get_info(lua_State* L);  ///< inventory.get_info
   static int inventory_get_floating_item(lua_State* L);  ///< inventory.get_floating_item
   static int inventory_float_add_item(lua_State* L);  ///< inventory.float_add_item

   static int tilemap_get_info(lua_State* L);///< tilemap.get_info
   static int tilemap_set_info(lua_State* L);///< tilemap.set_info
   static int tilemap_get_floor_height(lua_State* L); ///< tilemap.get_floor_height
   static int tilemap_get_object_list_link(lua_State* L); ///< tilemap.get_object_list_link

   static int runebag_set(lua_State* L);       ///< runebag.set
   static int runebag_test(lua_State* L);      ///< runebag.test

   static int conv_is_avail(lua_State* L);   ///< conv.is_avail
   static int conv_get_global(lua_State* L); ///< conv.GetGlobal
   static int conv_set_global(lua_State* L); ///< conv.SetGlobal

   static int quest_get_flag(lua_State* L);  ///< quest.get_flag
   static int quest_set_flag(lua_State* L);  ///< quest.set_flag

   static int prop_get_common(lua_State* L); ///< prop.get_common
   static int prop_get_special(lua_State* L);///< prop.get_special

   /// translates from rune type Integer value to RuneType
   static Underworld::RuneType GetRuneTypeFromInteger(unsigned int rune);

   /// adds ObjectInfo to the table currently on top of the stack
   static void AddObjectInfoTableFields(lua_State* L, const Underworld::ObjectInfo& info);
};
