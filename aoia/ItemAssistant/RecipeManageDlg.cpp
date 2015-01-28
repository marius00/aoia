// RecipeManageDlg.cpp : implementation of the RecipeManageDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "RecipeManageDlg.h"
#include "version.h"


LRESULT RecipeManageDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	CenterWindow(GetParent());
	/*
	ATL::CWindow versionCtrl(GetDlgItem(IDC_VERSION));
	versionCtrl.SetWindowText(STREAM2STR("Version " << g_versionNumber).c_str());
	*/
	return TRUE;
}

LRESULT RecipeManageDlg::OnImportCmd(WORD /*wNotifyCode*/, WORD wID, HWND hWndCtl, BOOL& /*bHandled*/) {
	//std::tstring filter = _T("Template XML Files (*.xml)|*.xml|");
	
	std::tstring xml = _T("xml");
	
	CFileDialog fOpenDlg(TRUE, 
		xml.c_str(), 
		NULL, 
		OFN_DONTADDTORECENT|OFN_FILEMUSTEXIST|OFN_NONETWORKBUTTON, 
		NULL, //filter.c_str(), 
		hWndCtl);



	
	TCHAR buf[256] = {0};	
	if(fOpenDlg.DoModal() == IDOK) {
		//fOpenDlg.GetFilePath(buf, 255);
		fOpenDlg.GetFolderPath(buf, 255);
		if ( CopyFile(buf, _T("lists\\unknown.xml"), TRUE) != 0) {
			MessageBox(_T("Could not import template"), _T("Error"));
		}
	}
	return 0;
}

LRESULT RecipeManageDlg::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	EndDialog(wID);
	return 0;
}
