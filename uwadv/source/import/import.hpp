//
// Underworld Adventures - an Ultima Underworld remake project
// Copyright (c) 2003,2004,2005,2006,2019 Michael Fink
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
/// \file import.hpp
/// \brief game data import
//
#pragma once

#include "base.hpp"
#include "file.hpp"
#include <vector>
#include <set>
#include <map>

namespace Base
{
   class ResourceManager;
   class Settings;
   class File;
}

namespace Underworld
{
   class ObjectProperties;
   class LevelList;
   class ObjectList;
   class Tilemap;
   class Player;
   class ConvGlobals;
   class Underworld;
}

namespace Conv
{
   class CodeVM;
}

/// import classes
namespace Import
{
   /// imports all object properties
   void ImportProperties(Base::ResourceManager& resourceManager,
      Underworld::ObjectProperties& properties);

   /// Loads conversation globals from bglobals.dat or babglobs.dat
   void LoadConvGlobals(Underworld::ConvGlobals& globals,
      Base::Settings& settings, const char* folder, bool initial);

   /// Loads conversation code into virtual machine
   bool LoadConvCode(Conv::CodeVM& vm, const char* cnvfile, Uint16 conv);

   /// Loads imported functions list
   void LoadConvCodeImportedFunctions(Conv::CodeVM& vm, Base::File& file);

#if 0
   /// imports common to uw1 and uw2
   class ua_uw_import
   {
   public:
      // underworld objects loading

      /// loads underworld objects from folder
      void load_underworld(Underworld& underw, Base::Settings& settings,
         const char* folder, bool initial);
   };

#endif

   // inline functions

   /// retrieves bits from given value
   inline Uint8 GetBits(Uint8 uiValue, unsigned int start, unsigned int count)
   {
      UaAssert(start < 8 && start + count <= 8);
      return static_cast<Uint8>(uiValue >> start) & static_cast<Uint8>((1 << count) - 1);
   }

   /// retrieves bits from given value
   inline Uint16 GetBits(Uint16 uiValue, unsigned int start, unsigned int count)
   {
      UaAssert(start < 16 && start + count <= 16);
      return static_cast<Uint16>(uiValue >> start) & static_cast<Uint16>((1 << count) - 1);
   }

   /// retrieves bits from given value
   inline Uint32 GetBits(Uint32 uiValue, unsigned int start, unsigned int count)
   {
      UaAssert(start < 32 && start + count <= 32);
      return (uiValue >> start) & ((1 << count) - 1);
   }

} // namespace Import
