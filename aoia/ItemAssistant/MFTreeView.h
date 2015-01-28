#ifndef MFTREEVIEW_H
#define MFTREEVIEW_H

#include <string>
#include <map>
#include <vector>


struct MFTreeViewItem
{
    virtual ~MFTreeViewItem() { };

    virtual void OnSelected() = 0;
    virtual bool CanEdit() const = 0;
    virtual bool CanDelete() const = 0;
    virtual bool HasChildren() const = 0;
    virtual std::tstring GetLabel() const = 0;
    virtual void SetLabel(std::tstring const& newLabel) = 0;
    virtual std::vector<MFTreeViewItem*> GetChildren() const = 0;
    virtual bool SortChildren() const = 0;
    virtual unsigned int AppendMenuCmd(HMENU hMenu, unsigned int firstID, WTL::CTreeItem item) const = 0;
    virtual bool HandleMenuCmd(unsigned int commandID, WTL::CTreeItem item) = 0;
};


class MFTreeView
    : public CWindowImpl<MFTreeView, WTL::CTreeViewCtrlEx>
{
public:
    MFTreeView();
    virtual ~MFTreeView();

    BEGIN_MSG_MAP_EX(MFTreeView)
        REFLECTED_NOTIFY_CODE_HANDLER(TVN_ITEMEXPANDING, OnItemExpanding)
        REFLECTED_NOTIFY_CODE_HANDLER(TVN_SELCHANGED, OnSelChanged)
        REFLECTED_NOTIFY_CODE_HANDLER(TVN_GETDISPINFO, OnGetDispInfo)
        REFLECTED_NOTIFY_CODE_HANDLER(NM_RCLICK, OnRClick)
		REFLECTED_NOTIFY_CODE_HANDLER(TVN_KEYDOWN, OnKeydown)
        REFLECTED_NOTIFY_CODE_HANDLER(TVN_BEGINLABELEDIT, OnLabelEditBegin)
        REFLECTED_NOTIFY_CODE_HANDLER(TVN_ENDLABELEDIT, OnLabelEditEnd)
        DEFAULT_REFLECTION_HANDLER()
    END_MSG_MAP()

    void addRootItem(MFTreeViewItem* pItem);

protected:
    LRESULT OnItemExpanding(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/);
    LRESULT OnSelChanged(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/);
    LRESULT OnGetDispInfo(int, LPNMHDR lParam, BOOL&);
    LRESULT OnRClick(int /*idCtrl*/, LPNMHDR pnmh, BOOL& bHandled);
    LRESULT OnLabelEditBegin(int /*idCtrl*/, LPNMHDR pnmh, BOOL& bHandled);
    LRESULT OnLabelEditEnd(int /*idCtrl*/, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT OnKeydown(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
};


#endif // MFTREEVIEW_H
