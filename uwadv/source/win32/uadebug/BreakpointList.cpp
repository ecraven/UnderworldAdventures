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
/*! \file ObjectList.cpp

   \brief code debugger breakpoint window

*/

// includes
#include "stdatl.hpp"
#include "BreakpointList.hpp"

CBreakpointListWindow::CBreakpointListWindow(unsigned int nCodeDebuggerID)
:CDockingWindowBase(idBreakpointWindow),
 m_nCodeDebuggerID(nCodeDebuggerID)
{
}

LRESULT CBreakpointListWindow::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
   CRect rcDef;
   GetClientRect(rcDef);
   m_listCtrl.Create(m_hWnd, rcDef, NULL,
      WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_EDITLABELS | LVS_SHOWSELALWAYS | LVS_OWNERDATA | LVS_SINGLESEL | LVS_SHAREIMAGELISTS | LVS_NOSORTHEADER);

   m_listCtrl.SetItemCountEx(0x400, LVSICF_NOINVALIDATEALL|LVSICF_NOSCROLL);

   return 0;
}

LRESULT CBreakpointListWindow::OnSize(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
{
   if (wParam != SIZE_MINIMIZED)
   {
      RECT rc;
      GetClientRect(&rc);
      m_listCtrl.SetWindowPos(NULL, &rc, SWP_NOZORDER | SWP_NOACTIVATE);
   }
   bHandled = FALSE;
   return 1;
}

LRESULT CBreakpointListWindow::OnSetFocus(UINT, WPARAM, LPARAM, BOOL& bHandled)
{
   if (m_listCtrl.m_hWnd != NULL && m_listCtrl.IsWindowVisible())
      m_listCtrl.SetFocus();

   bHandled = FALSE;
   return 1;
}

void CBreakpointListWindow::ReceiveNotification(CDebugWindowNotification& notify)
{
   switch (notify.m_enCode)
   {
   }
}


//virtual void CBreakpointListWindow::OnUpdatedValue(unsigned int nItem, unsigned int nSubItem, LPCTSTR pszValue);