//
// Underworld Adventures - an Ultima Underworld remake project
// Copyright (c) 2002,2003,2004,2005,2006,2019 Underworld Adventures Team
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
/// \file Inventory.hpp
/// \brief inventory
//
#pragma once

#include <vector>
#include "Object.hpp"

namespace Base
{
   class Savegame;
}

namespace Underworld
{
   class ObjectProperties;

   /// inventory position that indicates "no item"
   const Uint16 c_inventorySlotNoItem = 0xffff;

   /// inventory slot
   enum InventoryPaperdollSlot
   {
      slotTopmostFirstItem = 0, ///< topmost inventory; first item list pos

      slotLeftHand = 8,
      slotRightHand,
      slotLeftShoulder,
      slotRightShoulder,

      slotLeftFinger,
      slotRightFinger,

      slotPaperdollLegs,
      slotPaperdollChest,
      slotPaperdollHands,
      slotPaperdollFeet,
      slotPaperdollHead,

      slotMax,                // from here on indices in object list are free

      slotPlayerObjectsStart = slotLeftHand, ///< first slot with objects held by player
      slotPaperdollStart = slotPaperdollLegs ///< first slot with paperdoll objects
   };

   /// \brief Inventory
   /// The inventory is organized as a list with object infos held in an ObjectInfo
   /// struct. The first 8 entries in this list are the items in the topmost
   /// inventory view (where you usually place bags and other containers). The
   /// next entries are for the paperdoll object on the paperdoll. After these
   /// the list for objects in containers start; this area is used dynamically to
   /// store object infos.
   ///
   /// When entering containers, a stack with containers previously opened is
   /// managed to let the user return to the container above.
   ///
   /// The inventory objects of the current container is given as a "slot list"
   /// that contains all objects that are currently visible. The slot list can
   /// have various lengths. When in the top inventory, there are 8 fixed slots.
   /// When in a container that e.g. only supports n slots, the list can only get
   /// as long as n slots.
   ///
   /// The inventory also manages a single object that can float above the other
   /// icons, e.g. for reordering inventory or for dropping into the underworld.
   class Inventory
   {
   public:
      /// ctor
      Inventory();

      /// sets reference to object properties; needed for container properties
      /// and calculating weight
      void SetObjectProperties(const ObjectProperties& objectProperties)
      {
         m_objectProperties = std::cref(objectProperties);
      }

      /// adds new item combine rule
      /// \param firstItemID ID of first item
      /// \param secondItemID ID of second item
      /// \param resultItemID ID of resulting item
      /// \param itemDestroyedMask mask that determines which objects are destroyed;
      ///        1 means the first object, 2 means the second object and 3 means both objects
      void AddItemCombineRule(Uint16 firstItemID, Uint16 secondItemID,
         Uint16 resultItemID, unsigned int itemDestroyedMask);

      /// create inventory
      void Create();

      /// destroys inventory contents
      void Destroy();

      // object handling

      /// allocates a new object slot
      Uint16 Allocate();

      /// frees object slot
      void Free(Uint16 pos);

      /// retrieves an object from the list
      ObjectInfo& GetObjectInfo(Uint16 index)
      {
         UaAssert(index < m_objectList.size());
         return m_objectList[index];
      }

      /// retrieves an object from the list
      const ObjectInfo& GetObjectInfo(Uint16 index) const
      {
         UaAssert(index < m_objectList.size());
         return m_objectList[index];
      }

      // slot list handling

      /// returns number of slots in list
      size_t GetNumSlots() const { return m_slotList.size(); }

      /// returns object position of object stored in one of the slots
      Uint16 GetSlotListPos(size_t index) const;

      // container handling

      /// returns true if the given item id is a container item
      bool IsContainer(Uint16 itemID) const;

      /// returns current container object position, or c_inventorySlotNoItem when topmost
      Uint16 GetContainerPos() const;

      /// returns container object position of parent, or c_inventorySlotNoItem when topmost
      Uint16 GetParentContainerPos() const;

      /// opens a container and sets it as current container
      void OpenContainer(Uint16 pos);

      /// closes current container and enables the previous one
      void CloseContainer();

      /// adds object to container
      bool AddToContainer(Uint16& pos, Uint16 containerPos);

      /// removes object from container
      void RemoveFromContainer(Uint16 pos, Uint16 containerPos);

      // floating item support

      /// returns position of currently floating item, or c_inventorySlotNoItem
      /// when none is currently floating
      Uint16 GetFloatingObjectPos() const { return m_floatingObjectPos; }

      /// makes an object in the inventory "floating"
      void FloatObject(Uint16 pos);

      /// drops currently floating object into container, given by container
      /// position, and possibly onto an object
      bool DropFloatingObject(Uint16 containerPos, Uint16 objectIndex = c_inventorySlotNoItem);

      /// inserts new item and makes it floating; returns slot
      Uint16 InsertItem(const ObjectInfo& info);

      /// calculates current inventory weight, in 1/10 stones
      unsigned int GetInventoryWeight() const;

      // loading / saving

      /// loads inventory from savegame
      void Load(Base::Savegame& sg);

      /// saves inventory to savegame
      void Save(Base::Savegame& sg) const;

   private:
      /// builds new slot list from given container link
      void BuildSlotList(Uint16 link);

      /// drops floating object on given one
      bool DropOnObject(Uint16 containerPos, Uint16 pos);

      /// determines if an object can be dropped on the given pos, e.g. a paperdoll pos
      bool CanDropItemOnPos(const ObjectInfo& objectInfo, Uint16 pos) const;

      /// checks if object type can be placed in that slot
      bool CheckObjectTypeSlotType(Uint16 itemID, Uint16 pos) const;

      /// checks if container accepts an object with given item ID
      bool IsContainerAcceptedObjectType(Uint16 containerItemId, Uint16 objectItemId) const;

      /// tries to combine items
      bool TryCombineItems(Uint16 pos1, Uint16 pos2);

      /// swaps object with floating object; pos is contained in given container
      void SwapObjectWithFloating(Uint16 containerPos, Uint16 pos);

      /// returns weight of item (and contained items, if it's a container)
      /// in 1/10 stones
      unsigned int GetObjectWeight(Uint16 pos) const;

   private:
      /// object list
      std::vector<ObjectInfo> m_objectList;

      /// list with all objects in current slot list
      std::vector<Uint16> m_slotList;

      /// stack with currently opened containers (stored as object list position)
      std::vector<Uint16> m_containerStack;

      /// floating object position
      Uint16 m_floatingObjectPos;

      /// reference to object properties
      std::reference_wrapper<const ObjectProperties> m_objectProperties;

      /// rule for item combining
      struct ItemCombineRule
      {
         Uint16 m_itemID1;
         Uint16 m_itemID2;
         Uint16 m_resultItemID;
         unsigned int m_itemDestroyedMask;
      };

      /// list of item combine rules
      std::vector<ItemCombineRule> m_itemCombineRuleList;
   };

} // namespace Underworld
