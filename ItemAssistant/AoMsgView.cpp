#include "StdAfx.h"
#include "aomsgview.h"
#include "AOMessageParsers.h"
#include <iostream>
#include <fstream>
#include "DBManager.h"
#include "AOMessageIDs.h"


using namespace Parsers;


AoMsgView::AoMsgView(void)
{
   // m_mask.insert(AO::MSG_POS_SYNC);
    m_mask.insert(AO::MSG_UNKNOWN_1);
    m_mask.insert(AO::MSG_UNKNOWN_2);
    m_mask.insert(AO::MSG_UNKNOWN_3);
    m_mask.insert(AO::MSG_UNKNOWN_4);
}

AoMsgView::~AoMsgView(void)
{
}

BOOL AoMsgView::PreTranslateMessage(MSG* pMsg)
{
    if (pMsg->message == WM_COMMAND && LOWORD(pMsg->wParam) == ID_EDIT_CLEAR)
    {
        pMsg->hwnd = m_hWnd;
    }
    return FALSE;
}

LRESULT AoMsgView::OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    CPaintDC dc(m_hWnd);
    //RECT rect;
    //GetClientRect(&rect);
    //dc.DrawText("AO MSG VIEW", -1, &rect, NULL);
    return 0;
}

LRESULT AoMsgView::OnCreate(LPCREATESTRUCT createStruct)
{
    //m_hWndClient = m_listview.Create(m_hWnd, rcDefault, 0, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | LVS_REPORT, /*WS_EX_CLIENTEDGE*/ NULL );
    m_dlgview.Create(m_hWnd);
    m_listview = m_dlgview.GetDlgItem(IDC_LIST1);
    m_listview.ModifyStyle(0, LVS_REPORT, SWP_NOACTIVATE);
    m_listview.AddColumn(_T("Message ID"), 0);
    m_listview.AddColumn(_T("Size"), 1);
    TBBUTTON buttons[2];
    buttons[0].iBitmap = 0;
    buttons[0].idCommand = ID_EDIT_CLEAR;
    buttons[0].fsState = TBSTATE_ENABLED;
    buttons[0].fsStyle = TBSTYLE_BUTTON | BTNS_SHOWTEXT | BTNS_AUTOSIZE;
    buttons[0].dwData = NULL;
    buttons[0].iString = (INT_PTR)_T("Clear");
    buttons[1].iBitmap = 1;
    buttons[1].idCommand = ID_FILE_SAVE;
    buttons[1].fsState = TBSTATE_ENABLED;
    buttons[1].fsStyle = TBSTYLE_BUTTON | BTNS_SHOWTEXT | BTNS_AUTOSIZE;
    buttons[1].dwData = NULL;
    buttons[1].iString = (INT_PTR)_T("Save Message");

    CImageList imageList;
    imageList.CreateFromImage(IDB_AOMESSAGE_VIEW, 16, 1, CLR_DEFAULT, IMAGE_BITMAP, LR_CREATEDIBSECTION | LR_DEFAULTSIZE);
    m_toolbar.Create(GetTopLevelWindow(), NULL, _T("AOMessageViewToolBar"),
        ATL_SIMPLE_TOOLBAR_PANE_STYLE | TBSTYLE_LIST,
        TBSTYLE_EX_MIXEDBUTTONS);
    m_toolbar.SetButtonStructSize();
    m_toolbar.SetImageList(imageList);
    m_toolbar.AddButtons(ARRAYSIZE(buttons), buttons);
    m_toolbar.AutoSize();
    return 0;
}


LRESULT AoMsgView::OnSize(UINT wParam, CSize newSize)
{
    ::SetWindowPos(m_dlgview, NULL, 0, 0, newSize.cx, newSize.cy, SWP_NOZORDER | SWP_NOACTIVATE);
    return 0;
}

LRESULT AoMsgView::OnClear(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    while (m_listview.GetItemCount()) {
        DWORD_PTR data = m_listview.GetItemData(0);
        free((void*)data);
        m_listview.DeleteItem(0);
    }
    return 0;
}

LRESULT AoMsgView::OnSave(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    int selection = m_listview.GetSelectedIndex();
    if (selection < 0) {
        return 0;
    }

    DWORD_PTR data = m_listview.GetItemData(selection);
    Parsers::AOMessageBase headerParser((char*)data, 28);    // We don't know size, but assume header size as minimum. Only used to parse size field.
    Parsers::AOMessageBase parser((char*)data, headerParser.size());

    std::ofstream ofs;
    ofs.open("c:\\temp\\message.bin", std::ios_base::out | std::ios_base::binary | std::ios_base::app);
    DumpMessageToStream(ofs, parser);

    return 0;
}

void AoMsgView::OnAOServerMessage(AOMessageBase &msg)
{
    CString msgString, sizeStr;
    unsigned int msgid = msg.messageId();

    // Search message for the playershop container ID
    //{
    //    unsigned char ids[] = { 0x00, 0xC7, 0x90, 0x19 };
    //    bool found = false;
    //    found = SearchMessageForBinarySequence(msg, ids, ARRAYSIZE(ids));
    //    if (found) {
    //        std::ofstream ofs;
    //        ofs.open("c:\\temp\\aoia_search_hits.bin", std::ios_base::out | std::ios_base::app);
    //        DumpMessageToStream(ofs, msg);
    //        ofs << "--------";
    //    }
    //}

    switch(msgid)
    {

    //case AO::MSG_MOB_SYNC:
        //{
            //AO::MobInfo* pMobInfo = (AO::MobInfo*)msg.start();
            //std::string name(&(pMobInfo->characterName.str), pMobInfo->characterName.strLen - 1);
            // Dump all character IDs and names to a file.
            //{
            //    std::ofstream ofs;
            //    ofs.open("c:\\temp\\character_names.csv", std::ios_base::out | std::ios_base::app);
            //    ofs << msg.entityId() << "\t" << name << "\r\n";
            //}
            //{
            //    std::string filename = "binfiles\\info_msg_";
            //    filename += name;
            //    filename += ".bin";
            //    std::ofstream ofs;
            //    ofs.open(filename.c_str(), std::ios_base::out | std::ios_base::binary);
            //    DumpMessageToStream(ofs, msg);
            //}
            // Search message for the playershop container ID
            //unsigned char ids[] = { 0x00, 0xC7, 0x90, 0x19 };
            //bool found = SearchMessageForBinarySequence(msg, ids, ARRAYSIZE(ids));
        //}
        //break;

	//case AO::MSG_BACKPACK:
    //    {
    //        std::basic_ofstream<TCHAR> ofs;
	//		ofs.open("c:\\temp\\msg_backpack.txt", std::ios_base::out | std::ios_base::app);
    //        DumpMessageToTextStream(ofs, msg);
    //    }
    //    break;

    //case AO::MSG_SHOP_ITEMS:
    //    {
    //        std::ofstream ofs;
    //        ofs.open("c:\\temp\\shop_content_msg.bin", std::ios_base::out | std::ios_base::binary);
    //        DumpMessageToStream(ofs, msg);
    //    }
    //    break;

    //case AO::MSG_SHOP_INFO:
    //    {
    //        AOPlayerShopInfo infoMsg(msg.start(), msg.size());
    //    }
    //    break;

    }

    //if (msgid == AO::MSG_POS_SYNC) {
    //   std::stringstream str;
    //   str << "Pos: ";
    //   for (char * payload = (char*)pMsg + AO::HeaderSize; payload <= (char*)pMsg + 55; ++payload)
    //   {
    //      str << *payload;
    //   }
    //   str << std::endl << std::flush;
    //}

    if (m_mask.find(msgid) != m_mask.end())
    {
        return;
    }

	//TEMP for easy debugging
	if (msg.entityId() != msg.characterId())
	{
		return;
	}

    msgString.Format(_T("0x%08x"), msgid);
    sizeStr.Format(_T("%d"), msg.size());
    int id = m_listview.AddItem(m_listview.GetItemCount(), 0, msgString);
    char* pdata = (char*)malloc(msg.end() - msg.start());
    memcpy(pdata, msg.start(), msg.end() - msg.start());
    m_listview.SetItemData(id, (DWORD_PTR)pdata);
    m_listview.SetItemText(id, 1, sizeStr);
    //std::ofstream ofs;
    //ofs.open("c:\\temp\\messages.bin", std::ios_base::out | std::ios_base::binary | std::ios_base::app);
    //DumpMessageToStream(ofs, msg);
    //ofs << "========" << std::flush;
}


void AoMsgView::OnAOClientMessage(AOClientMessageBase &msg)
{
    //std::ofstream ofs;
    //ofs.open("c:\\temp\\client_messages.bin", std::ios_base::out | std::ios_base::binary | std::ios_base::app);
    //DumpMessageToStream(ofs, msg);
    //ofs << "========" << std::flush;
}


/// Dump a message to stream
void AoMsgView::DumpMessageToStream(std::ostream &out, Parser &msg)
{
    char* p = msg.start();
    while (p < msg.end())
	{
        out << *p;
        ++p;
    }
}

void AoMsgView::DumpMessageToTextStream(std::basic_ofstream<TCHAR> &out, Parser &msg)
{
    char* p = msg.start();
	//out.width(2);
	//out.fill('0');
    while (p < msg.end())
	{
		//short pp = _byteswap_ushort((*p << 8) + *(p+1));
		short pp = _byteswap_ushort((*p << 8));
		//<< std::showbase gives 0x first
		out.width(2);
		out.fill('0');
		out << std::hex << pp << _T(" ");
        ++p;
    }
	out << std::endl;
}


/// Search message for a binary pattern
bool AoMsgView::SearchMessageForBinarySequence(AOMessageBase &msg, unsigned char* pArray, unsigned int arraySize)
{
    bool retval = false;
    char* p = msg.start();

    while (p + arraySize < msg.end())
    {
        for (unsigned int i = 0; i < arraySize; ++i)
		{
            if (p[i] != pArray[i])
			{
                retval = false;
                break;  // No match.
            }
            retval = true;
        }
        if (retval)
		{
            break;
        }
        ++p;
    }
    return retval;
}


LRESULT DlgView::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    this->SetWindowText(_T("Dialog View"));
    HICON hIcon = (HICON)::LoadImage(
        _Module.GetResourceInstance(),
        MAKEINTRESOURCE(IDR_MAINFRAME),
        IMAGE_ICON, 16, 16, LR_SHARED);
    this->SetIcon(hIcon, ICON_SMALL);
    DlgResize_Init(false, true, WS_CLIPCHILDREN);
    return 0;
}

LRESULT DlgView::OnForwardMsg(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
    LPMSG pMsg = (LPMSG)lParam;
    return this->PreTranslateMessage(pMsg);
}

LRESULT DlgView::OnNMClickList1(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/)
{
    WTL::CListViewCtrl listview(GetDlgItem(IDC_LIST1));
    int index = listview.GetSelectedIndex();
    if (index >= 0)
    {
        AO::Header *pMsg = (AO::Header*)listview.GetItemData(index);
        Native::AOMessageHeader msg(pMsg, true); //TODO: We only show server messages in this view
        char* pData = (char*)pMsg;
        unsigned int size = _byteswap_ushort(pMsg->msgsize);
        WTL::CString str;
        std::tstring text;
        unsigned char * p = (unsigned char*)pData;
        int linebreak = 0;
        text += msg.print();
        for (unsigned int offset = 0; offset < size; offset += 4)
        {
            p = (unsigned char*)(pData + offset);
            for (int i = 0; i < 4; i++)
            {
                str.Format(_T("%02X"), p[i]);
                text += str;
            }
            if (linebreak < 4)
            {
                text += _T("\t");
                linebreak++;
            }
            else
            {
                text += _T("\r\n");
                linebreak = 0;
            }
        }
        GetDlgItem(IDC_EDIT2).SetWindowText(text.c_str());
    }
    return 0;
}

BOOL DlgView::PreTranslateMessage(MSG* pMsg)
{
    return IsDialogMessage(pMsg);
}
