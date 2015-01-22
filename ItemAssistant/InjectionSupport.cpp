#include "stdafx.h"


bool InjectDLL(DWORD ProcessID, std::tstring const& dllName)
{
    if (!ProcessID)
    {
        LOG("No process ID specified.");
        return false;
    }
    LOG("Attempting to inject '" << dllName << "' into process " << ProcessID);

    // Clear error status.
    SetLastError(0);

    HANDLE Proc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, ProcessID);
    if (!Proc)
    {
        LOG("Failed to open AO client process. Error: " << GetLastError());
        return false;
    }

    std::string dllNameA = to_ascii_copy(dllName);
    LPVOID RemoteString = (LPVOID)VirtualAllocEx(Proc, NULL, dllNameA.length(), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (!RemoteString)
    {
        LOG("Failed to allocate memory in AO client. Error: " << GetLastError());
        CloseHandle(Proc);
        return false;
    }

    if (!WriteProcessMemory(Proc, (LPVOID)RemoteString, dllNameA.c_str(), dllNameA.length(), NULL))
    {
        LOG("Failed to send DLL name to AO client. Error: " << GetLastError());
        CloseHandle(Proc);
        return false;
    }

    LPVOID LoadLibAddy = (LPVOID)GetProcAddress(GetModuleHandle(_T("kernel32.dll")), "LoadLibraryA");
    if (!CreateRemoteThread(Proc, NULL, NULL, (LPTHREAD_START_ROUTINE)LoadLibAddy, (LPVOID)RemoteString, NULL, NULL))
    {
        LOG("Failed to start hook in AO client. Error: " << GetLastError());
        CloseHandle(Proc);
        return false;
    }

    CloseHandle(Proc);
    return true;
}
