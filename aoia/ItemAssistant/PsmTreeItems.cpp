#include "stdafx.h"
#include "PsmTreeItems.h"
#include "PlayershopView.h"
#include <boost/filesystem.hpp>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <ItemAssistantCore/AOManager.h>

#define TIXML_USE_STL
#include <tinyxml.h>

using namespace boost::filesystem;


PsmTreeViewItemBase::PsmTreeViewItemBase(PlayershopView* pOwner)
: m_pOwner(pOwner)
{
}


PsmTreeViewItemBase::~PsmTreeViewItemBase()
{
}


void PsmTreeViewItemBase::SetOwner(PlayershopView* pOwner) 
{
    m_pOwner = pOwner;
}


unsigned int PsmTreeViewItemBase::AppendMenuCmd(HMENU hMenu, unsigned int firstID, WTL::CTreeItem item) const
{
    return firstID;
}


bool PsmTreeViewItemBase::HandleMenuCmd(unsigned int commandID, WTL::CTreeItem item)
{
    return false;
}


void PsmTreeViewItemBase::SetLabel(std::tstring const& newLabel)
{
}

void PsmTreeViewItemBase::OnSelected() 
{
}


/***************************************************************************/
/** Account Tree View Item                                                **/
/***************************************************************************/

AccountTreeViewItem::AccountTreeViewItem(
    PlayershopView* pOwner, std::tstring accountName)
    : m_label(accountName)
    , PsmTreeViewItemBase(pOwner)
{
}


AccountTreeViewItem::~AccountTreeViewItem()
{
}


void AccountTreeViewItem::OnSelected() 
{
    m_pOwner->UpdateListView(GetAllSoldItems());
}


std::vector<std::tstring> AccountTreeViewItem::GetAllSoldItems()
{
    std::vector<PsmTreeViewItem*> children = GetChildren();
    std::vector<std::tstring> v;
    for(unsigned int i=0; i<children.size(); i++)
    {
        std::vector<std::tstring> items = ((CharacterTreeViewItem1*)children[i])->GetAllSoldItems();
        for(unsigned int j=0; j<items.size(); j++)
        {
            v.push_back(items[j]);
        }
    }
    return v;
}


bool AccountTreeViewItem::CanEdit() const
{
    return false;
}


std::tstring AccountTreeViewItem::GetLabel() const
{
    return m_label;
}


bool AccountTreeViewItem::HasChildren() const
{
    return true;
}


std::vector<PsmTreeViewItem*> AccountTreeViewItem::GetChildren() const
{
    std::vector<PsmTreeViewItem*> result;

    path accountPath(AOManager::instance().getAOPrefsFolder(), boost::filesystem::native);
    accountPath = accountPath / m_label;

    directory_iterator character(accountPath), filesEnd;
    for (; character != filesEnd; ++character)
    {
        if (!is_directory(*character)) {
            continue;
        }

        try
        {
            std::tstring subfolderName = character->path().leaf().native();
            if (subfolderName.length() < 5) {
                continue;   // No point parsing this folder since it cant have a valid 'Char123123' type of name anyway.
            }
            if (subfolderName.compare(_T("Browser")) == 0) {
                continue;   // Skip the "Browser" subfolder
            }
            unsigned int charID = boost::lexical_cast<unsigned int>(subfolderName.substr(4));
            if (charID != 0) {
                result.push_back(new CharacterTreeViewItem1(m_pOwner, charID, this));
            }  
        }
        catch (boost::bad_lexical_cast &/*e*/) {
            continue;   // do nothing with this folder since it isn't a valid toon folder.
        }
    }

    return result;
}


unsigned int AccountTreeViewItem::AppendMenuCmd(HMENU hMenu, unsigned int firstID, WTL::CTreeItem item) const
{
    return firstID;
}


bool AccountTreeViewItem::HandleMenuCmd(unsigned int commandID, WTL::CTreeItem item)
{
    return false;
}


/***************************************************************************/
/** Character Tree View Item                                              **/
/***************************************************************************/

CharacterTreeViewItem1::CharacterTreeViewItem1(PlayershopView* pOwner, unsigned int charID, const AccountTreeViewItem* pParent)
: m_charid(charID)
, PsmTreeViewItemBase(pOwner)
{
    m_pParent = pParent;
    std::tstring str = g_DBManager.GetToonName(m_charid);
    if (str.empty()) {
        m_label = STREAM2STR(charID);
    } else {
        m_label = str;
    }
}


CharacterTreeViewItem1::~CharacterTreeViewItem1()
{
}


void CharacterTreeViewItem1::OnSelected() 
{ 
    m_pOwner->UpdateListView(GetAllSoldItems());
}


std::vector<std::tstring> CharacterTreeViewItem1::GetAllSoldItems()
{
    std::tstring filename;
    filename = STREAM2STR( AOManager::instance().getAOPrefsFolder() << m_pParent->GetLabel() << "\\Char" << m_charid << "\\PlayerShopLog.html");

    boost::filesystem::path p(to_utf8_copy(filename),boost::filesystem::native);

    std::ifstream in(p.string().c_str());
    std::string line;
    std::string text;
    std::vector<std::tstring> v;
    while(in){
        line.clear();
        std::getline(in,line);
        if(!line.empty())
        {
            text += line;
        }
    }

    // Now that we have the whole file, lets parse it

    // Text located between the two following tags is to be considered an item sold
    std::string startTag = "<div indent=wrapped>" ;
    std::string endTag   = "</div>" ;

    while(text.length() > 0)
    {

        std::string::size_type start = text.find( startTag, 0 );
        std::string::size_type end  = text.find( endTag , 0 );
        if( start != std::string::npos && end != std::string::npos)
        {
            // adding data for column 1
            v.push_back(from_ascii_copy(text.substr(start+startTag.length(),end-start-startTag.length())));

            // adding data for column 2
            std::tstring str = g_DBManager.GetToonName(m_charid);
            if(str.empty()){
                std::tstringstream ss;
                ss << m_charid;
                str = std::tstring(ss.str());
            }
            v.push_back(str);

            // remove the already processed part of the string
            text = text.substr(end+endTag.length());
        }
        else
        {
            text = "";
        }
    }
    return v;
}


bool CharacterTreeViewItem1::CanEdit() const
{
    return false;
}


std::tstring CharacterTreeViewItem1::GetLabel() const
{
    return m_label;
}


void CharacterTreeViewItem1::SetLabel(std::tstring const& newLabel)
{
    m_label = newLabel;
}


bool CharacterTreeViewItem1::HasChildren() const
{
    return false;
}


std::vector<PsmTreeViewItem*> CharacterTreeViewItem1::GetChildren() const
{
    std::vector<PsmTreeViewItem*> result;

    return result;
}


unsigned int CharacterTreeViewItem1::AppendMenuCmd(HMENU hMenu, unsigned int firstID, WTL::CTreeItem item) const
{
    return firstID;
}


bool CharacterTreeViewItem1::HandleMenuCmd(unsigned int commandID, WTL::CTreeItem item)
{
    return false;
}


/***************************************************************************/
/** Playershop Tree View Item                                             **/
/***************************************************************************/

PlayershopTreeRoot::PlayershopTreeRoot(PlayershopView* pOwner)
: PsmTreeViewItemBase(pOwner)
{
}


PlayershopTreeRoot::~PlayershopTreeRoot()
{
}


void PlayershopTreeRoot::OnSelected()
{
    std::vector<PsmTreeViewItem*> children = GetChildren();
    std::vector<std::tstring> v;
    for(unsigned int i=0; i<children.size(); i++)
    {
        std::vector<std::tstring> items = ((AccountTreeViewItem*)children[i])->GetAllSoldItems();
        for(unsigned int j=0; j<items.size(); j++)
        {
            v.push_back(items[j]);
        }
    }

    m_pOwner->UpdateListView(v);
}


bool PlayershopTreeRoot::CanEdit() const
{
    return false;
}


std::tstring PlayershopTreeRoot::GetLabel() const
{
    return _T("All Accounts");
}


bool PlayershopTreeRoot::HasChildren() const
{
    return true;
}


std::vector<PsmTreeViewItem*> PlayershopTreeRoot::GetChildren() const
{
    std::vector<PsmTreeViewItem*> result;

    std::tstring filename;
    filename = STREAM2STR( AOManager::instance().getAOPrefsFolder());
    if(filename.empty()) {
        return result;
    }

    boost::filesystem::path p(filename, boost::filesystem::native);
    boost::filesystem::directory_iterator account(p), dir_end;

    if(!is_directory(p))
    {
        return result;
    }

    for (;account != dir_end; ++account)
    {
        boost::filesystem::path acc = *account;
        if (is_directory(acc))
        {
            // we found an account ?
            result.push_back(new AccountTreeViewItem(m_pOwner, acc.leaf().native()));
        }
    }

    return result;
}
