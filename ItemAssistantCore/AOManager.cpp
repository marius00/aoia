#include "StdAfx.h"
#include "AOManager.h"
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/functional/hash.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <Shared/UnicodeSupport.h>
#include <Shared/FileUtils.h>
#include <Shlobj.h>
#include <iomanip>


namespace bfs = boost::filesystem;
using namespace aoia;


SINGLETON_IMPL(AOManager);


AOManager::AOManager()
{
}


AOManager::AOManager(const AOManager&)
{
}


AOManager::~AOManager()
{
}


std::tstring AOManager::getAOFolder() const
{
    assert(m_settings);

    if (m_aofolder.empty())
    {
        bfs::tpath AODir;
        bool requestFolder = true;

        // Get AO folder from settings
        std::tstring dir_setting = m_settings->getValue(_T("AOPath"));
        if (!dir_setting.empty())
        {
            AODir = dir_setting;
            if (bfs::exists(AODir / _T("anarchyonline.exe")))
            {
                requestFolder = false;
            }
        }

        if (requestFolder)
        {
            AODir = BrowseForFolder(NULL, _T("Please locate the AO directory:"));
            if (AODir.empty()) {
                return _T("");
            }

            if (!bfs::exists(AODir / _T("anarchyonline.exe"))) {
                MessageBox( NULL, _T("The specified directory doesn't contain a valid Anarchy Online installation."),
                    _T("Error - AO Item Assistant +"), MB_OK | MB_ICONERROR);
                return _T("");
            }

            // Store the new AO directory in the settings
            m_settings->setValue(_T("AOPath"), AODir.native());
        }

        m_aofolder = AODir.native();
    }

    return m_aofolder;
}


std::tstring AOManager::getAOPrefsFolder() const
{
    assert(m_settings);

    if (m_prefsfolder.empty()) {
        bool bFindFolder = true;
        bfs::tpath prefsDir;

        // Get prefs folder override from settings
        std::tstring dir_setting = m_settings->getValue(_T("AOPrefsPath"));
        if (!dir_setting.empty()) {
            prefsDir = dir_setting;
            if (bfs::exists(prefsDir / _T("Prefs.xml"))) {
                bFindFolder = false;
            }
        }

        if (bFindFolder) {
            // We intentionally do NOT store this in settings. Settings should be for manual override only.
			bfs::tpath tAOFolder = getAOFolder();
			if (tAOFolder.empty())
				 return _T("");

			TCHAR tAppData[MAX_PATH];
			SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, SHGFP_TYPE_CURRENT, tAppData);
			bfs::tpath tFCPath = tAppData;
			tFCPath /= _T("Funcom");
			tFCPath /= _T("Anarchy Online");

			boost::hash<std::string>st_hash;
			bfs::tpath tAORoot = tAOFolder.parent_path();
	        if (!tAORoot.has_relative_path())
				tAORoot = tAORoot.root_name();

			std::string tAO_Path = tAORoot.generic_string();
			std::size_t nHash = st_hash(tAO_Path);
			std::stringstream sAOHashDir;
			sAOHashDir << std::hex << nHash;
			prefsDir = tFCPath / sAOHashDir.str() / tAOFolder.filename() / _T("Prefs");
        }

        m_prefsfolder = prefsDir.native();
    }

    return m_prefsfolder;
}


std::tstring AOManager::getAOPrefsFolderManual() const
{
	assert(m_settings);

	bfs::tpath prefsDir;
	prefsDir = BrowseForFolder(NULL, _T("Please locate the AO prefs folder:"));
    if (prefsDir.empty()) {
        return _T("");
    }

    if (!bfs::exists(prefsDir / _T("Prefs.xml"))) {
        MessageBox( NULL, _T("The specified directory doesn't seem to contain valid Anarchy Online prefs."),
            _T("Error - AO Item Assistant +"), MB_OK | MB_ICONERROR);
        return _T("");
    }
    // Store the new AO Prefs directory in the settings
    m_prefsfolder = prefsDir.native();
    m_settings->setValue(_T("AOPrefsPath"), m_prefsfolder);

	return m_prefsfolder;
}


bool AOManager::getAutoPrefs() const
{
	if (m_settings->getValue(_T("AOPrefsPath")).empty())  // Get prefs folder from settings
		return true;

	return false;
}


void AOManager::setAutoPrefs(bool bAutoPrefs)
{
    m_prefsfolder = _T("");
    m_settings->setValue(_T("AOPrefsPath"), m_prefsfolder);
}


bool AOManager::getShowCreds() const
{
	if (m_showcreds.empty())
	{
		m_showcreds = m_settings->getValue(_T("ShowCreds"));   // Get ShowCreds from settings
		if (m_showcreds.empty())
		{
			m_showcreds = _T("yes");
			m_settings->setValue(_T("ShowCreds"), m_showcreds);
			return true;
		}
	}

	if (m_showcreds == _T("yes"))
		return true;
	else
		return false;
}


void AOManager::setShowCreds(bool bShowCreds)
{
	if (bShowCreds)
		m_showcreds = _T("yes");
	else
		m_showcreds = _T("no");
    
	m_settings->setValue(_T("ShowCreds"), m_showcreds);
}


bool AOManager::getMinToTaskbar() const
{
	if (m_minToTaskbar.empty())
	{
		m_minToTaskbar = m_settings->getValue(_T("Minimise To Taskbar"));   // Get ShowCreds from settings
		if (m_minToTaskbar.empty())
		{
			m_minToTaskbar = _T("yes");
			m_settings->setValue(_T("Minimise To Taskbar"), m_minToTaskbar);
			return true;
		}
	}

	if (m_minToTaskbar == _T("yes"))
		return true;
	else
		return false;
}


void AOManager::setMinToTaskbar(bool bMinToTaskbar)
{
	if (bMinToTaskbar)
		m_minToTaskbar = _T("yes");
	else
		m_minToTaskbar = _T("no");
    
	m_settings->setValue(_T("Minimise To Taskbar"), m_minToTaskbar);
}


std::vector<std::tstring> AOManager::getAccountNames() const
{
    std::vector<std::tstring> result;

    std::tstring path = AOManager::instance().getAOPrefsFolder();
    path += _T("\\*");

    WIN32_FIND_DATA findData;

    HANDLE hFind = FindFirstFileEx(path.c_str(), FindExInfoStandard, &findData, FindExSearchLimitToDirectories, NULL, 0);

    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) 
                && (findData.cFileName[0] != NULL) 
                && (findData.cFileName[0] != '.')
				&& (std::tstring(findData.cFileName) != _T("Browser")))
            {
                result.push_back(std::tstring(findData.cFileName));
            }
        }
        while (FindNextFile(hFind, &findData));
        FindClose(hFind);
    }

    return result;
}


void AOManager::SettingsManager( aoia::ISettingsPtr settings )
{
    m_settings = settings;
}
