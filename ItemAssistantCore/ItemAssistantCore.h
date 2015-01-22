// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the ITEMASSISTANTCORE_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// ITEMASSISTANTCORE_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef ITEMASSISTANTCORE_EXPORTS
#define ITEMASSISTANTCORE_API __declspec(dllexport)
#else
#pragma comment( lib, "ItemAssistantCore.lib" )
#define ITEMASSISTANTCORE_API __declspec(dllimport)
#endif

// This class is exported from the ItemAssistantCore.dll
//class ITEMASSISTANTCORE_API CItemAssistantCore {
//public:
//	CItemAssistantCore(void);
//	// TODO: add your methods here.
//};
//
//extern ITEMASSISTANTCORE_API int nItemAssistantCore;
//
//ITEMASSISTANTCORE_API int fnItemAssistantCore(void);
