#include <Windows.h>
#include <Psapi.h>
#include <time.h>

#include "CMem.h"
#include "PatternScan.h"

void WINAPI Load(); 
void FpsHook();
DWORD jmpBack = NULL;

DWORD* DrunkLevel = 0;
int fpsStep = 60;
int fpsVariation = 3;

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

int RandomVariation()
{
	// Get random value between -fpsVariation and fpsVariation (range is fpsVariation * 2)
	return (rand() % 2 == 1 ? (rand() % fpsVariation) : -(rand() % fpsVariation));
}

void WINAPI Load() {
	// http://ugbase.eu/Thread-Checking-is-game-fully-loaded-or-not
	srand((unsigned int)time(NULL));

	while(*(bool*)0xA444A0 == false)
	{
		Sleep(1000);
	}

	DWORD fps = FindPattern("\x89\x86\xC9\x02\x00\x00", "xxxxxx");
	jmpBack = fps + 6;

	CMem::ApplyJmp((BYTE*)fps, (DWORD)FpsHook, 6);

	while (DrunkLevel == 0)
	{
		Sleep(1000);
	}

	while (DrunkLevel != 0) 
	{
		if (*DrunkLevel <= 2000)
		{
			(*DrunkLevel) = max((*DrunkLevel) - (fpsStep + RandomVariation()), 0);
		}
		Sleep(1000);
	}
}

__declspec(naked) void FpsHook()
{
	__asm
	{
		pushad
	}

	if (DrunkLevel == 0)
	{
		__asm
		{
			popad
			add esi, 000002C9h
			mov[DrunkLevel], esi
			sub esi, 000002C9h

			inc eax
			jmp[jmpBack]
		}
	}
	else
	{
		__asm
		{
			popad
			inc eax
			jmp[jmpBack]
		}
	}
}