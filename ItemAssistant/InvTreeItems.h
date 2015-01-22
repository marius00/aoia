#pragma once

#include "MFTreeView.h"
#include "DBManager.h"
#include <strstream>
#include <PluginSDK/IContainerManager.h>


class InventoryView;


class SqlTreeViewItemBase
    : public MFTreeViewItem
{
public:
    SqlTreeViewItemBase(InventoryView* pOwner);
    virtual ~SqlTreeViewItemBase();

    virtual void SetOwner(InventoryView* pOwner);
    virtual void SetLabel(std::tstring const& newLabel);
    virtual unsigned int AppendMenuCmd(HMENU hMenu, unsigned int firstID, WTL::CTreeItem item) const;
    virtual bool HandleMenuCmd(unsigned int commandID, WTL::CTreeItem item);
    virtual bool SortChildren() const;

protected:
    InventoryView* m_pOwner;

    enum InternalCommand
    {
        CMD_EDIT = 1,
        CMD_DELETE,
        CMD_EXPORT,
		CMD_RENAME,
    };

    mutable std::map<unsigned int, InternalCommand> m_commands;
};


class ContainerTreeViewItem
    : public SqlTreeViewItemBase
{
public:
    ContainerTreeViewItem(sqlite::IDBPtr db, aoia::IContainerManagerPtr containerManager, InventoryView* pOwner, unsigned int charid, unsigned int containerid, std::tstring const& constraints = std::tstring(), std::tstring const& label = std::tstring());
    virtual ~ContainerTreeViewItem();

    virtual void OnSelected();
    virtual bool CanEdit() const;
    virtual bool CanDelete() const;
    virtual std::tstring GetLabel() const;
    virtual std::vector<MFTreeViewItem*> GetChildren() const;
    virtual bool SortChildren() const;
    virtual bool HasChildren() const;
    virtual unsigned int AppendMenuCmd(HMENU hMenu, unsigned int firstID, WTL::CTreeItem item) const;
    virtual bool HandleMenuCmd(unsigned int commandID, WTL::CTreeItem item);
	virtual void HandleMenuCmdRename(WTL::CTreeItem item);

private:
    sqlite::IDBPtr m_db;
    aoia::IContainerManagerPtr m_containerManager;
    unsigned int m_charid;
    unsigned int m_containerid;
    std::tstring m_label;
    std::tstring m_constraints;
};


class CharacterTreeViewItem
    : public SqlTreeViewItemBase
{
public:
    CharacterTreeViewItem(sqlite::IDBPtr db, aoia::IContainerManagerPtr containerManager, InventoryView* pOwner, unsigned int charid);
    virtual ~CharacterTreeViewItem();

    virtual void OnSelected();
    virtual bool CanEdit() const;
    virtual bool CanDelete() const;
    virtual std::tstring GetLabel() const;
    virtual void SetLabel(std::tstring const& newLabel);
    virtual std::vector<MFTreeViewItem*> GetChildren() const;
    virtual bool HasChildren() const;
    virtual unsigned int AppendMenuCmd(HMENU hMenu, unsigned int firstID, WTL::CTreeItem item) const;
    virtual bool HandleMenuCmd(unsigned int commandID, WTL::CTreeItem item);

private:
    sqlite::IDBPtr m_db;
    aoia::IContainerManagerPtr m_containerManager;
    unsigned int m_charid;
    std::tstring m_label;
};

class AccountTreeViewItem
	: public SqlTreeViewItemBase
{
public:
   AccountTreeViewItem(sqlite::IDBPtr db, aoia::IContainerManagerPtr containerManager, InventoryView* pOwner, std::tstring accountName);
   virtual ~AccountTreeViewItem();

   virtual void OnSelected();
   virtual bool CanEdit() const;
   virtual bool CanDelete() const { return true; }
   virtual std::tstring GetLabel() const;
   virtual std::vector<MFTreeViewItem*> GetChildren() const;
   virtual bool HasChildren() const;
   virtual unsigned int AppendMenuCmd(HMENU hMenu, unsigned int firstID, WTL::CTreeItem item) const;
   virtual bool HandleMenuCmd(unsigned int commandID, WTL::CTreeItem item);
private:
   std::tstring m_label;
   sqlite::IDBPtr m_db;
   aoia::IContainerManagerPtr m_containerManager;
};

class AccountRootTreeViewItem
	: public SqlTreeViewItemBase
{
public:
   AccountRootTreeViewItem(sqlite::IDBPtr db, aoia::IContainerManagerPtr containerManager, InventoryView* pOwner);
   virtual ~AccountRootTreeViewItem();

   virtual void OnSelected();
   virtual bool CanEdit() const;
   virtual bool CanDelete() const { return true; }
   virtual std::tstring GetLabel() const;
   virtual std::vector<MFTreeViewItem*> GetChildren() const;
   virtual bool HasChildren() const;
   virtual unsigned int AppendMenuCmd(HMENU hMenu, unsigned int firstID, WTL::CTreeItem item) const;
   virtual bool HandleMenuCmd(unsigned int commandID, WTL::CTreeItem item);
private:
	virtual unsigned __int64 DoesAccountHaveToons(boost::filesystem::path account_prefs) const;
	sqlite::IDBPtr m_db;
	aoia::IContainerManagerPtr m_containerManager;
};
