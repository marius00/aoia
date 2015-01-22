// renamebackpackdlg.cpp : implementation of the RenameBackpackDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "renamebackpackdlg.h"
#include "version.h"


LRESULT RenameBackpackDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	CenterWindow(GetParent());

	//ATL::CWindow versionCtrl(GetDlgItem(IDC_VERSION));
	//versionCtrl.SetWindowText(STREAM2STR("Version " << g_versionNumber).c_str());

	ATL::CWindow nameCtrl(GetDlgItem(IDC_EDIT1));
	nameCtrl.SetWindowText(m_backpackName.c_str());
	success = false;
	return TRUE;
}

void RenameBackpackDlg::setCurrentName(std::tstring currentName) {
	if (currentName.length() > 2) {
		if (currentName.at(0) == '"')
			currentName = currentName.substr(1);
		if (currentName.at(currentName.length()-1) == '"')
			currentName = currentName.substr(0, currentName.length()-1);
		m_backpackName = currentName;
	}
	else
		m_backpackName = _T("");
	

}

bool RenameBackpackDlg::getSuccess() const {
	return success;
}
std::tstring RenameBackpackDlg::getBackpackName() const {
	return m_backpackName;
}
LRESULT RenameBackpackDlg::OnCloseCmd(WORD wNotifyCode, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if (wID == 1) {
		ATL::CWindow nameCtrl(GetDlgItem(IDC_EDIT1));

		TCHAR buf[256] = {0};
		nameCtrl.GetWindowText(buf, 255);
		m_backpackName = std::tstring(buf);
		success = true;
	}
	else {
		m_backpackName = _T("");
		success = false;
	}
	
	EndDialog(wID);
	return 0;
}
