#include "stdafx.h"
/* stacktrace.cpp */
/*% cl -GX -Fm stacktrace.cpp imagehlp.lib
*/
/* Dump a Win32 C++ stack trace for an error caught by the try-catch mechanism.
   Works even with non-debug executables.

   Uses Microsoft exception handling as implemented in eh.h:
       _set_se_translator
   which sets a function to be called on an error before the stack is unwound
   to the catch point.

   If inside a debug executable, use SymGetSymFromAddress
   to find function names.

   If this is a release executable--without debug information--try
   to find function names by looking them up in a map file
   (maps are created by the compiler -Fm option).
   In the map file, the function name is the name immediatly preceding
   the largest address smaller than the address being interpreted.
   Call getmapfile() before the try clause to permit this style of interpretation.

   The dump consists of:
   The exception (interpreted as text)
   The address where the exception occured
   Some bytes following the address (so you can find the place in a core dump)
   The registers at the exception (here with no floating point or segment regs)
   The stack trace:  address module function hex-offset-into-function

   The dump is created as a character string and printed with a single call
   to printf at the end of the dump.  This is to minimize interference with
   other threads that may be printing at the same time this one is.

   To use: imitate the main() as shown (in main or WinMain):

	getmapfile( executablename );
	_set_se_translator( trans_func );
	try{
		// all your code goes here
	}catch(unsigned int u){
		printf("in catch, 0x%x was caught\n", u);
	}
   To compile this program, see line 2 of this file
*/
#include "windows.h"
#include <eh.h>
#include <process.h>
#include <imagehlp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#pragma comment(lib, "DbgHelp.lib")

#define SYMSIZE 10000
#define MAXTEXT 5000
static char mapfile[256];	// A non-debug process can use a mapfile

static char *prregs(char *str, PCONTEXT cr){
	char *cp= str;
	sprintf(cp, "\t%s:%08x", "Eax", cr->Eax); cp += strlen(cp);
	sprintf(cp, " %s:%08x",  "Ebx", cr->Ebx); cp += strlen(cp);
	sprintf(cp, " %s:%08x",  "Ecx", cr->Ecx); cp += strlen(cp);
	sprintf(cp, " %s:%08x\n","Edx", cr->Edx); cp += strlen(cp);
	sprintf(cp, "\t%s:%08x", "Esi", cr->Esi); cp += strlen(cp);
	sprintf(cp, " %s:%08x",  "Edi", cr->Edi); cp += strlen(cp);
	sprintf(cp, " %s:%08x",  "Esp", cr->Esp); cp += strlen(cp);
	sprintf(cp, " %s:%08x\n","Ebp", cr->Ebp); cp += strlen(cp);
	return str;
}
static char *ConvertSimpleException(DWORD dwExcept){
	switch (dwExcept){
	case EXCEPTION_ACCESS_VIOLATION: return "Access Violation"; break;
 	case EXCEPTION_DATATYPE_MISALIGNMENT: return "Datatype Misalignment"; break;
 	case EXCEPTION_BREAKPOINT: return "Breakpoint"; break;
 	case EXCEPTION_SINGLE_STEP: return "Single Step"; break;
 	case EXCEPTION_ARRAY_BOUNDS_EXCEEDED: return "Array Bounds Exceeded"; break;
 	case EXCEPTION_FLT_DENORMAL_OPERAND: return "Flt Denormal Operand"; break;
 	case EXCEPTION_FLT_DIVIDE_BY_ZERO: return "Flt Divide By ZERO"; break;
 	case EXCEPTION_FLT_INEXACT_RESULT: return "Flt Inexact Result"; break;
 	case EXCEPTION_FLT_INVALID_OPERATION: return "Flt Invalid Operation"; break;
 	case EXCEPTION_FLT_OVERFLOW: return "Flt Overflow"; break;
 	case EXCEPTION_FLT_STACK_CHECK: return "Flt Stack Check"; break;
 	case EXCEPTION_FLT_UNDERFLOW: return "Flt Underflow"; break;
 	case EXCEPTION_INT_DIVIDE_BY_ZERO: return "Int Divide By Zero"; break;
 	case EXCEPTION_INT_OVERFLOW: return "Int Overflow"; break;
 	case EXCEPTION_PRIV_INSTRUCTION: return "Priv Instruction"; break;
 	case EXCEPTION_IN_PAGE_ERROR: return "In Page Error"; break;
 	case EXCEPTION_ILLEGAL_INSTRUCTION: return "Illegal Instruction"; break;
 	case EXCEPTION_NONCONTINUABLE_EXCEPTION: return "Noncontinuable Exception"; break;
	case EXCEPTION_STACK_OVERFLOW: return "Stack Overflow"; break;
 	case EXCEPTION_INVALID_DISPOSITION: return "Invalid Disposition"; break;
 	case EXCEPTION_GUARD_PAGE: return "Guard Page"; break;
 	case EXCEPTION_INVALID_HANDLE: return "Invalid Handle"; break;
	default: break;
	}
	return 0;
}
static int getword(FILE *fp, char *word, int n){
	int c;
	int k= 0;
	while((c= getc(fp)) != EOF){
		if(c<'0' || c>'z'){
			if(k>0)
				break;
		}else if(--n<=0)
			break;
		else{
			++k;
			*word++= c;
		}
	}
	*word++= 0;
	return c!=EOF;
}
static int isnum(char *cp){
	int k;
	for(k= 0; cp[k]; k++)
		if(!isxdigit(cp[k]))
			return 0;
	return k==8;
}
static char *trmap(unsigned long addr, char *mapname, unsigned long *offset){
	char word[512];
	char lastword[512];
	static char wbest[512];
	unsigned long ubest= 0;
	FILE *fp= fopen(mapname, "rt");
	if(!fp)
		return 0;
	while(getword(fp, word, sizeof(word))){
		if(isnum(word)){
			char *endc;
			unsigned long u= strtoul(word, &endc, 16);
			if(u<=addr && u>ubest){
				strcpy(wbest, lastword);
				ubest= u;
			}
		}else
			strcpy(lastword, word);
	}
	fclose(fp);
	if(offset)
		*offset= addr - ubest;
	return wbest[0]? wbest:0;
}
void getmapfile(char *prog){
	/* try to find a mapfile to use */
	char *cp;
	FILE *fp;
	strcpy(mapfile, prog);
	if((cp= strrchr(mapfile, '.')) && !stricmp(cp, ".exe"))
		strcpy(cp+1, "map");
	else
		strcat(mapfile, ".map");
	if(fp= fopen(mapfile, "rt"))
		fclose(fp);
	else
		mapfile[0]= 0;
}
void trans_func(unsigned int u, EXCEPTION_POINTERS *ExInfo){
	STACKFRAME sf;
	BOOL ok;
	PIMAGEHLP_SYMBOL pSym;
	IMAGEHLP_MODULE Mod;
	DWORD Disp;
	BYTE *bp;
	char *cexcept;
	char *ou, *outstr;
	static BOOL entered;

	if(entered)
		throw u;
	pSym= (PIMAGEHLP_SYMBOL)GlobalAlloc(GMEM_FIXED, SYMSIZE+MAXTEXT);
	if(!pSym)
		throw u;
	ou= outstr= ((char *)pSym)+SYMSIZE;
	entered= TRUE;
	if(cexcept= ConvertSimpleException(u))
		strcpy(ou, cexcept);
	else
		sprintf(ou, "Exception %x", u);
	ou += strlen(ou);
	sprintf(ou, " at %08x:", bp= (BYTE *)(ExInfo->ExceptionRecord->ExceptionAddress));
	try{
		for(int k= 0; k<12; k++){
			ou += strlen(ou);
			sprintf(ou, " %02x", *bp++);
		}
	}catch(unsigned int w){
		/* Fails if bp is invalid address */
		w= 0;
		ou += strlen(ou);
		sprintf(ou, " Invalid Address");
	}
	ou += strlen(ou);
	sprintf(ou, "\n");
	prregs(ou+strlen(ou), ExInfo->ContextRecord);
	Mod.SizeOfStruct = sizeof(Mod);

	ZeroMemory(&sf, sizeof(sf));
	sf.AddrPC.Offset = ExInfo->ContextRecord->Eip;
	sf.AddrStack.Offset = ExInfo->ContextRecord->Esp;
	sf.AddrFrame.Offset = ExInfo->ContextRecord->Ebp;
	sf.AddrPC.Mode = AddrModeFlat;
	sf.AddrStack.Mode = AddrModeFlat;
	sf.AddrFrame.Mode = AddrModeFlat;

	ok= SymInitialize(GetCurrentProcess(), NULL, TRUE);
	ou += strlen(ou);
	sprintf(ou, "Stack Trace:\n");
	for(;;) {
		ok = StackWalk(
			IMAGE_FILE_MACHINE_I386,
			GetCurrentProcess(),
			GetCurrentThread(),
			&sf,
			ExInfo->ContextRecord,
			NULL,
			SymFunctionTableAccess,
			SymGetModuleBase,
			NULL);

		if(!ok || sf.AddrFrame.Offset == 0)
			break;
		ou += strlen(ou);
		if(ou>outstr + MAXTEXT-1000){
			sprintf(ou, "No more room for trace\n");
			break;
		}
		sprintf(ou, "%08x:", sf.AddrPC.Offset);
		DWORD dwModBase= SymGetModuleBase (GetCurrentProcess( ), sf.AddrPC.Offset);
		ou += strlen(ou);
		if(dwModBase && SymGetModuleInfo(GetCurrentProcess(), sf.AddrPC.Offset, &Mod)){
			sprintf(ou, " %s", Mod.ModuleName);
			ou += strlen(ou);
		}
		pSym->SizeOfStruct = SYMSIZE;
		pSym->MaxNameLength = SYMSIZE - sizeof(IMAGEHLP_SYMBOL);
		if(SymGetSymFromAddr(GetCurrentProcess(), sf.AddrPC.Offset, &Disp, pSym))
			sprintf(ou, " %s() + 0x%x\n", pSym->Name, Disp);
		else if(mapfile[0]){
			char *cp= trmap(sf.AddrPC.Offset, mapfile, &Disp);
			if(*cp=='?'){
				UnDecorateSymbolName(cp, pSym->Name, pSym->MaxNameLength, UNDNAME_COMPLETE);
				sprintf(ou, " %s + 0x%x\n", pSym->Name, Disp);
			}else
				sprintf(ou, " %s() + 0x%x\n", cp, Disp);
		}else if(dwModBase)
			sprintf(ou, ": 0x%x\n", sf.AddrPC.Offset - dwModBase);
	}
	printf("%s", outstr);
	SymCleanup(GetCurrentProcess());
	GlobalFree(pSym);
	entered= FALSE;
	throw u;
}