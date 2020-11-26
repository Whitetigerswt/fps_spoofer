#include <Windows.h>
#include <Psapi.h>

#include "CMem.h"
#include "PatternScan.h"

void WINAPI Load(); 
void FpsHook();
DWORD jmpBack = NULL;

// Tell the server we have (real FPS / x) frames
int x = 3;

// e.g: if we have 300 fps, tell the server we actually have 100. (300 / 3 = 100)
// If we have 100 fps, tell the server we have 33. Change at your own discretion.

HMODULE hMod = NULL;
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		hMod = hModule;
		if ( CreateThread( 0, 0, (LPTHREAD_START_ROUTINE)Load, NULL, 0, 0) == NULL ) {
			ExitProcess(GetLastError());
			return FALSE; 
		}
		break;
	}
	return TRUE;
}

void WINAPI Load() {
	// http://ugbase.eu/Thread-Checking-is-game-fully-loaded-or-not
	while(*(bool*)0xA444A0 == false)
	{
		Sleep(1000);
	}

	DWORD fps = FindPattern("\x89\x86\xC9\x02\x00\x00", "xxxxxx");
	jmpBack = fps + 6;

	CMem::ApplyJmp((BYTE*)fps, (DWORD)FpsHook, 6);
}

int Random()
{
	return rand() % x;
}

__declspec(naked) void FpsHook()
{
	__asm
	{
		pushad
	}
	// one in x chance we calculate a frame loss

	if (Random() == 0)
	{
		__asm
		{
			popad
			mov[esi + 0x00002C9], eax
			jmp[jmpBack]
		}
	}

	__asm
	{
		popad
		inc eax
		jmp[jmpBack]
	}
}