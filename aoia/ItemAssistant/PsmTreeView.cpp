#include "StdAfx.h"
#include "PsmTreeView.h"

PsmTreeView::PsmTreeView(void)
{
}

PsmTreeView::~PsmTreeView(void)
{
}


void PsmTreeView::SetRootItem(PsmTreeViewItem* pItem)
{
    DeleteAllItems();
    TVINSERTSTRUCT tvis = { 0 };
    tvis.hParent = TVI_ROOT;
    tvis.hInsertAfter = TVI_ROOT;
    tvis.item.mask = TVIF_CHILDREN | TVIF_PARAM | TVIF_TEXT;
    tvis.item.pszText = LPSTR_TEXTCALLBACK;
    tvis.item.cChildren = I_CHILDRENCALLBACK;
    tvis.item.lParam = (LPARAM) pItem;
    HTREEITEM hItem = InsertItem(&tvis);
}


LRESULT PsmTreeView::OnGetDispInfo(int, LPNMHDR lParam, BOOL&)
{
    LPNMTVDISPINFO lptvdi = (LPNMTVDISPINFO) lParam;
    PsmTreeViewItem* pItem = (PsmTreeViewItem*)(lptvdi->item.lParam);
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
            StrCpy(lptvdi->item.pszText, pItem->GetLabel().c_str());
        }
    }
    return 0;
}

LRESULT PsmTreeView::OnItemExpanding(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/)
{
    bool result = true; // TRUE == Dont Expand/collapse
    LPNMTREEVIEW pnmtv = (LPNMTREEVIEW)pnmh;

    PsmTreeViewItem* pItem = (PsmTreeViewItem*)(pnmtv->itemNew.lParam);
    if (pItem)
    {
        if (pnmtv->action == TVE_COLLAPSE)
        {
            // TODO: Make this a recursive algorithm incase we have open subnodes.
            WTL::CTreeItem item(pnmtv->itemNew.hItem, this);
            WTL::CTreeItem child = item.GetChild();
            while (!child.IsNull())
            {
                PsmTreeViewItem* pChild = (PsmTreeViewItem*)child.GetData();
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
            std::vector<PsmTreeViewItem*> children = pItem->GetChildren();
            for (unsigned int i = 0; i < children.size(); ++i)
            {
                TVINSERTSTRUCT tvis = { 0 };
                tvis.hParent = pnmtv->itemNew.hItem;
                tvis.hInsertAfter = hPrevItem;
                tvis.item.mask = TVIF_CHILDREN | TVIF_PARAM | TVIF_TEXT;
                tvis.item.pszText = LPSTR_TEXTCALLBACK;

                tvis.item.cChildren = I_CHILDRENCALLBACK;
                tvis.item.lParam = (LPARAM) children[i];

                hPrevItem = InsertItem(&tvis);
            }
            result = false;
        }
    }
    return result ? TRUE : FALSE;
}

LRESULT PsmTreeView::OnSelChanged(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/)
{
    LPNMTREEVIEW pnmtv = (LPNMTREEVIEW)pnmh;

    PsmTreeViewItem* pItem = (PsmTreeViewItem*)(pnmtv->itemNew.lParam);
    if (pItem)
    {
        pItem->OnSelected();
    }

    return FALSE;
}
