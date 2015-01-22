#ifndef PSMTREEVIEW_H
#define PSMTREEVIEW_H

struct PsmTreeViewItem
{
    virtual ~PsmTreeViewItem() { };

    virtual void OnSelected() = 0;
    virtual bool CanEdit() const = 0;
    virtual bool CanDelete() const = 0;
    virtual bool HasChildren() const = 0;
    virtual std::tstring GetLabel() const = 0;
    virtual void SetLabel(std::tstring const& newLabel) = 0;
    virtual std::vector<PsmTreeViewItem*> GetChildren() const = 0;
    virtual unsigned int AppendMenuCmd(HMENU hMenu, unsigned int firstID, WTL::CTreeItem item) const = 0;
    virtual bool HandleMenuCmd(unsigned int commandID, WTL::CTreeItem item) = 0;
};

class PsmTreeView : public CWindowImpl<PsmTreeView, CTreeViewCtrlEx>
{
public:
    PsmTreeView();
    virtual ~PsmTreeView();
    BEGIN_MSG_MAP_EX(PsmTreeView)
        // put your message handler entries here
        REFLECTED_NOTIFY_CODE_HANDLER(TVN_ITEMEXPANDING, OnItemExpanding)
        REFLECTED_NOTIFY_CODE_HANDLER(TVN_SELCHANGED, OnSelChanged)
        REFLECTED_NOTIFY_CODE_HANDLER(TVN_GETDISPINFO, OnGetDispInfo)
        DEFAULT_REFLECTION_HANDLER()
    END_MSG_MAP()

    LRESULT OnItemExpanding(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/);
    LRESULT OnSelChanged(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/);
    LRESULT OnGetDispInfo(int, LPNMHDR lParam, BOOL&);

    void SetRootItem(PsmTreeViewItem* pItem);
};


#endif // PSMTREEVIEW_H
