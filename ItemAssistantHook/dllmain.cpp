#include "stdafx.h"
#include "dllmain.h"
#include <shared/aopackets.h>
#include <detours.h>
#include <stdio.h>
#include <stdlib.h>
#include <set>
#include <boost/smart_ptr.hpp>
#include <boost/thread.hpp>
#include "DataQueue.h"

// Switches hook logging on/off
#if 1
#include "HookLog.h"
HookLog g_log;
#define LOG(streamdef) \
{ \
    std::string msg = (((std::ostringstream&)(std::ostringstream().flush() << streamdef)).str()); \
    g_log.out(msg); \
    msg += _T("\n"); \
    OutputDebugString(msg.c_str()); \
}
#else
#define LOG(streamdef) \
    __noop;
#endif

enum MessageType {
    TYPE_INCOMING = 1,
    TYPE_OUTGOIING = 2,
};

typedef void Message_t;

Message_t* ( * pOriginalDataBlockToMessage )( int _Size, void* _pDataBlock );

PVOID Func;

void LoadMessageFilter(HKEY hKeyParent, LPCTSTR lpszKeyName);

DWORD g_lastTick = 0;
DWORD g_lastThreadTick = 0;
std::set<unsigned int> g_messageFilter;
HANDLE g_hEvent;

DataQueue g_dataQueue;

HWND g_targetWnd = NULL;   // TODO: Make this a set/list that is dynamically updated

using namespace AO;

PDETOUR_TRAMPOLINE Trampoline = NULL;


/// Thread function that dispatches queued message blocks to the AOIA application.
void WorkerThreadMethod(void*)
{
    while ((g_hEvent != NULL) && (WaitForSingleObject(g_hEvent,INFINITE) == WAIT_OBJECT_0))
    {
        if (g_hEvent == NULL) {
            break;
        }

        DWORD tick = GetTickCount();
        if (tick < g_lastThreadTick) {
            // Overflow
            g_lastThreadTick = tick;
        }

        if ((tick - g_lastThreadTick > 10000) || (g_targetWnd == NULL)) {
            // We either don't have a valid window target OR it has been more than 10 sec since we last update the target.
            g_targetWnd  = FindWindow ( "ItemAssistantWindowClass", NULL ); // TODO: make the class name a list in registry
            g_lastThreadTick = GetTickCount();
            LOG("FindWindow returned: " << g_targetWnd);
        }

        while (!g_dataQueue.empty())
        {
            DataItemPtr item = g_dataQueue.pop();

            if (g_targetWnd == NULL) {
                // We have data, but no target window, so just delete the message
                continue;
            }

            COPYDATASTRUCT data;
            data.dwData = item->type();
            data.lpData = item->data();
            data.cbData = item->size();

            // To avoid blocking the main thread, we should not have a lock on the queue while we process the message.
			SendMessage( g_targetWnd, WM_COPYDATA, 0, ( LPARAM ) &data );
            LOG("After SendMessage error code is " << GetLastError());
        }
    }
}


void StartWorkerThread()
{
    g_hEvent = CreateEvent(NULL,FALSE,FALSE,"IA_Worker");
    _beginthread(WorkerThreadMethod,0,0);
}


void EndWorkerThread()
{
    SetEvent(g_hEvent);
    CloseHandle(g_hEvent);
    g_hEvent = NULL;
}


/// Hook function that is called for all incoming messages from the server to the AO client.
Message_t* DataBlockToMessageHook( int _Size, void* _pDataBlock )
{
    // Retrieves the number of milliseconds that have elapsed since the system was started, up to 49.7 days.
    DWORD time = GetTickCount();

    if (time < g_lastTick) {
        // Overflow so just reset the g_lastTick
        g_lastTick = time;
    }
    if (time - g_lastTick > 10000) {
        // 10 sec since last update
        g_lastTick = time;
        LoadMessageFilter(HKEY_CURRENT_USER, "Software\\AOMessageHook\\MessageIDs");
    }

    Header * msg = (Header*)_pDataBlock;
    unsigned int msgId = _byteswap_ulong(msg->msgid);

    if (g_messageFilter.size() == 0 || g_messageFilter.find(msgId) != g_messageFilter.end())
    {
        DataItemPtr item(new DataItem(TYPE_INCOMING, _Size, (char*)_pDataBlock));
        g_dataQueue.push(item);
        SetEvent(g_hEvent);
    }

    return pOriginalDataBlockToMessage( _Size, _pDataBlock );
}


/// Loads a list of message IDs that should be passed on to the AOIA application
void LoadMessageFilter(HKEY hKeyParent, LPCTSTR lpszKeyName)
{
    g_messageFilter.empty();

    ATL::CRegKey reg;
    if (reg.Open(hKeyParent, lpszKeyName, KEY_READ) == ERROR_SUCCESS)
    {
        TCHAR subkey[256];
        DWORD skLength = 256;
        DWORD dw;
        int index = 0;

        while (true)
        {
            if (reg.EnumKey(index, subkey, &skLength) == ERROR_SUCCESS)
            {
                index++;
                if (reg.QueryDWORDValue(subkey, dw) == ERROR_SUCCESS)
                {
                    g_messageFilter.insert(dw);
                }
            }
            else
            {
                break;
            }
        }
    }
    else
    {
        LOG("Unable to open key: " << lpszKeyName)
    }
}


#pragma pack(push, 1)

typedef struct {
    DWORD edi;
    DWORD esi;
    DWORD ebp;
    DWORD esp;
    DWORD ebx;
    DWORD edx;
    DWORD ecx;
    DWORD eax;
} CPU_CONTEXT;

#pragma pack(pop)


/// Hook function that is called before a message is sent from the AO client to the server.
void OnConnectionSend(void * connection, unsigned char * _msgData, unsigned int len)
{
	Header * msg = (Header*)_msgData;
    unsigned int msgId = _byteswap_ulong(msg->msgid);

	if (msgId == 0x54111123 || msgId == 0x02)//move and keepalive
	{
		return;
	}
    LOG("OnConnectionSend( len: " << len << ", msgId: 0x" << std::hex << msgId);

    DataItemPtr item(new DataItem(TYPE_OUTGOIING, len, (char*)_msgData));
    g_dataQueue.push(item);
    SetEvent(g_hEvent);
}


__declspec(naked) void SendConnectionHook (CPU_CONTEXT saved_regs, void * ret_addr, DWORD arg1, DWORD arg2, DWORD arg3)
{
    DWORD edi_value;
    DWORD old_last_error;

    __asm
    {
        pushad;   /* first "argument", which is also used to store registers */
        push ecx; /* padding so that ebp+8 refers to the first "argument" */

        /* set up standard prologue */
        push ebp;
        mov ebp, esp;
        sub esp, __LOCAL_SIZE;
    }

    edi_value = saved_regs.edi;
    old_last_error = GetLastError();
    OnConnectionSend((void *) saved_regs.ecx, (unsigned char *) arg3, arg2);
    SetLastError(old_last_error);

    __asm
    {
        /* standard epilogue */
        mov esp, ebp;
        pop ebp;

        pop ecx; /* clear padding */
        popad; /* clear first "argument" */
        jmp [Trampoline];
    }
}


int ProcessAttach( HINSTANCE _hModule )
{
    // Hook ::DataBlockToMessage() (incoming)
    pOriginalDataBlockToMessage = (Message_t *(__cdecl*)(int,void*))::GetProcAddress(::GetModuleHandle("MessageProtocol.dll"), "?DataBlockToMessage@@YAPAVMessage_t@@IPAX@Z");

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach((PVOID*)&pOriginalDataBlockToMessage, DataBlockToMessageHook);
    DetourTransactionCommit();

    // Hook Connection::Send() (outgoing)
    Func =  (int (__stdcall *)(unsigned int, unsigned short, void const *))::GetProcAddress(::GetModuleHandle("Connection.dll"), "?Send@Connection_t@@QAEHIIPBX@Z");
    PVOID DetourPtr;
    PVOID TargetPtr;
    DetourTransactionBegin();
    DetourAttachEx(&Func, SendConnectionHook, &Trampoline, &TargetPtr, &DetourPtr );
    DetourTransactionCommit();

    StartWorkerThread();

    return TRUE;
}


int ProcessDetach( HINSTANCE _hModule )
{
	#ifdef DEBUG
		OutputDebugString("ProcessDetach");
	#endif

    LONG res1 = DetourTransactionBegin();
    LONG res2 = DetourUpdateThread(GetCurrentThread());
    LONG res3 = DetourDetach((PVOID*)&pOriginalDataBlockToMessage, DataBlockToMessageHook);
    LONG res4 = DetourTransactionCommit();
    LONG res5 = DetourTransactionBegin();
    LONG res6 = DetourDetach(&Func, SendConnectionHook);

	//#ifdef DEBUG
	//	std::ostringstream s;
	//	s << "ProcessDetach( r1: " << res1 << ", r2: " << res2 << ", r3: " << res3 << ", r4: " << res4 << ", r5: " << res5 << ", r6: " << res6 << std::endl;
	//	OutputDebugString(s.str().c_str());
	//#endif

    LONG res7 = DetourTransactionCommit();
    LOG("ProcessDetach r7: " << res7);

    EndWorkerThread();

    return TRUE;
}


BOOL APIENTRY DllMain(HINSTANCE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        return ProcessAttach( hModule );
    case DLL_PROCESS_DETACH:
        return ProcessDetach( hModule );
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    }
    return TRUE;
}
