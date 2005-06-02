/*
   Underworld Adventures Debugger - a debugger tool for Underworld Adventures
   Copyright (c) 2004,2005 Michael Fink

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
/*! \file PlayerInfo.hpp

   \brief player info docking window

*/
//! \ingroup uadebug

//@{

// include guard
#pragma once

// includes

// classes

//! player info docking window
class CPlayerInfoWindow :
   public CDockingWindowBase,
   public IEditListViewCallback
{
   typedef CPlayerInfoWindow thisClass;
   typedef CDockingWindowBase baseClass;
public:
   //! ctor
   CPlayerInfoWindow():baseClass(idPlayerInfoWindow){}

   DECLARE_DOCKING_WINDOW(_T("Player Info"), CSize(200,100)/*docked*/, CSize(200,600)/*floating*/, dockwins::CDockingSide::sRight)

   DECLARE_WND_CLASS(_T("PlayerInfoWindow"))
   BEGIN_MSG_MAP(thisClass)
      MESSAGE_HANDLER(WM_CREATE, OnCreate)
      MESSAGE_HANDLER(WM_SIZE, OnSize)
      MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
      CHAIN_MSG_MAP(baseClass)
      REFLECT_NOTIFICATIONS()
   END_MSG_MAP()

protected:
   LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
   LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
   LRESULT OnSetFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

   // virtual methods from CDockingWindowBase
   void ReceiveNotification(CDebugWindowNotification& notify);

   //! updates player info data
   void UpdateData();

   // virtual methods from IEditListViewCallback
   virtual void OnUpdatedValue(unsigned int nItem, unsigned int nSubItem, LPCTSTR pszValue);

protected:
   //! player info list 
   CEditListViewCtrl m_listCtrl;

   //! indicates if control is read-only
   bool m_bReadonly;
};

//@}
