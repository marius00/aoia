#include "stdafx.h"
#include "MFTreeView.h"


MFTreeView::MFTreeView()
{
}


MFTreeView::~MFTreeView() 
{ 
}


LRESULT MFTreeView::OnItemExpanding(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/)
{
   bool result = true; // TRUE == Dont Expand/collapse
   LPNMTREEVIEW pnmtv = (LPNMTREEVIEW)pnmh;

   MFTreeViewItem* pItem = (MFTreeViewItem*)(pnmtv->itemNew.lParam);
   if (pItem)
   {
      if (pnmtv->action == TVE_COLLAPSE)
      {
         // TODO: Make this a recursive algorithm incase we have open subnodes.
         WTL::CTreeItem item(pnmtv->itemNew.hItem, this);
         WTL::CTreeItem child = item.GetChild();
         while (!child.IsNull())
         {
            MFTreeViewItem* pChild = (MFTreeViewItem*)child.GetData();
            child.SetData(NULL);
            if (pChild)
            {
               delete pChild;
            }
            child = GetNextSiblingItem(child);
         }

         SendMessage(m_hWnd, TVM_EXPAND, TVE_COLLAPSE | TVE_COLLAPSERESET, (LPARAM) pnmtv->itemNew.hItem);

         result = false;
      }
      else if (pnmtv->action == TVE_EXPAND && pItem->HasChildren())
      {
         HTREEITEM hPrevItem = pnmtv->itemNew.hItem;
         std::vector<MFTreeViewItem*> children = pItem->GetChildren();
         for (unsigned int i = 0; i < children.size(); ++i)
         {
            TVINSERTSTRUCT tvis = { 0 };
            tvis.hParent = pnmtv->itemNew.hItem;
            tvis.hInsertAfter = pItem->SortChildren() ? TVI_SORT : hPrevItem;
            tvis.item.mask = TVIF_CHILDREN | TVIF_PARAM | TVIF_TEXT;
            tvis.item.pszText = LPSTR_TEXTCALLBACK;
            //tvis.item.iImage = nImage;
            //tvis.item.iSelectedImage = nSelectedImage;
            //tvis.item.state = nState;
            //tvis.item.stateMask = nStateMask;
            tvis.item.cChildren = I_CHILDRENCALLBACK;
            tvis.item.lParam = (LPARAM) children[i];

            hPrevItem = InsertItem(&tvis);
         }
         result = false;
      }
   }
   return result ? TRUE : FALSE;
}


LRESULT MFTreeView::OnSelChanged(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/)
{
   LPNMTREEVIEW pnmtv = (LPNMTREEVIEW)pnmh;

   MFTreeViewItem* pItem = (MFTreeViewItem*)(pnmtv->itemNew.lParam);
   if (pItem)
   {
      pItem->OnSelected();
   }

   return FALSE;
}


LRESULT MFTreeView::OnGetDispInfo(int, LPNMHDR lParam, BOOL&)
{
   LPNMTVDISPINFO lptvdi = (LPNMTVDISPINFO) lParam;
   MFTreeViewItem* pItem = (MFTreeViewItem*)(lptvdi->item.lParam);
   if (pItem)
   {
      if (lptvdi->item.mask & TVIF_CHILDREN)
      {
         lptvdi->item.cChildren = pItem->HasChildren() ? 1 : 0;
      }
      if (lptvdi->item.mask & TVIF_IMAGE)
      {
      }
      if (lptvdi->item.mask & TVIF_SELECTEDIMAGE)
      {
      }
      if (lptvdi->item.mask & TVIF_TEXT)
      {
         StrCpyN(lptvdi->item.pszText, pItem->GetLabel().c_str(), lptvdi->item.cchTextMax);
      }
   }
   return 0;
}


LRESULT MFTreeView::OnRClick(int /*idCtrl*/, LPNMHDR pnmh, BOOL& bHandled)
{
   HMENU ctxMenu = CreatePopupMenu();

   WTL::CTreeItem item = GetDropHilightItem();
   if (item.IsNull())
   {
      item = GetSelectedItem();
   }
   MFTreeViewItem* pItem = (MFTreeViewItem*)item.GetData();

   if (pItem == NULL)
   {
      DestroyMenu(ctxMenu);
      return 0;
   }

   unsigned int nextID = pItem->AppendMenuCmd(ctxMenu, 1, item);

   if (GetMenuItemCount(ctxMenu) > 0)
   {
      AppendMenu(ctxMenu, MF_SEPARATOR, 0, NULL);
   }
   unsigned int refreshID = nextID++;
   nextID = AppendMenu(ctxMenu, MF_STRING, refreshID, _T("Refresh"));

   if (GetMenuItemCount(ctxMenu) == 0)
   {
      DestroyMenu(ctxMenu);
      return 0;
   }

   POINT pos;
   GetCursorPos(&pos);

   int command = TrackPopupMenuEx(ctxMenu, TPM_NONOTIFY | TPM_RETURNCMD, pos.x, pos.y, pnmh->hwndFrom, NULL);

   if (!pItem->HandleMenuCmd(command, item))
   {
      if (command == refreshID)
      {
         WTL::CTreeItem parent = item.GetParent();
         parent.Expand(TVE_COLLAPSE | TVE_COLLAPSERESET);
         parent.Expand();
      }
   }

   DestroyMenu(ctxMenu);

   return 1;
}


LRESULT MFTreeView::OnLabelEditBegin(int /*idCtrl*/, LPNMHDR pnmh, BOOL& bHandled)
{
   NMTVDISPINFO* ptvdi = (LPNMTVDISPINFO) pnmh;

   MFTreeViewItem* pItem = (MFTreeViewItem*)(ptvdi->item.lParam);

   if (pItem != NULL && pItem->CanEdit())
   {
      return FALSE;  // Allow edit
   }

   return TRUE;   // cancel edit
}


LRESULT MFTreeView::OnLabelEditEnd(int /*idCtrl*/, LPNMHDR pnmh, BOOL& bHandled)
{
   LPNMTVDISPINFO ptvdi = (LPNMTVDISPINFO) pnmh;

   MFTreeViewItem* pItem = (MFTreeViewItem*)(ptvdi->item.lParam);

   if (pItem != NULL && ptvdi->item.pszText != NULL)
   {
      pItem->SetLabel(ptvdi->item.pszText);
   }

   return FALSE;  // Reject
}


void MFTreeView::addRootItem(MFTreeViewItem* pItem)
{
   TVINSERTSTRUCT tvis = { 0 };
   tvis.hParent = TVI_ROOT;
   tvis.hInsertAfter = TVI_ROOT;
   tvis.item.mask = TVIF_CHILDREN | TVIF_PARAM | TVIF_TEXT;
   tvis.item.pszText = LPSTR_TEXTCALLBACK;
   //tvis.item.iImage = nImage;
   //tvis.item.iSelectedImage = nSelectedImage;
   //tvis.item.state = nState;
   //tvis.item.stateMask = nStateMask;
   tvis.item.cChildren = I_CHILDRENCALLBACK;
   tvis.item.lParam = (LPARAM) pItem;

   HTREEITEM hItem = InsertItem(&tvis);
}
