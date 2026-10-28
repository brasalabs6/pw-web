//#include "stdafx.h"
#include <windows.h>
#include "infocollection.h"
#include <vector>
#include "process.h"
#include <stdio.h>
using namespace Collector;

//	The original's un-suffixed GetModuleHandle maps to the W variant under
//	UNICODE (LPCTSTR is wide here); the seam exposes only the A/W pair.
#define GetModuleHandle GetModuleHandleW

//	The seam's PE types stop before the export/IAT directory entries (the
//	port's module handles are NULL, so ReplaceAPI returns before the walk
//	below can ever run); local extensions restore the directory surface so
//	the original code still type-checks.
#define IMAGE_DIRECTORY_ENTRY_EXPORT 0
#define IMAGE_DIRECTORY_ENTRY_IAT 12
#define PAGE_READWRITE 0x04
typedef struct _PW153_IMAGE_DATA_DIRECTORY { DWORD VirtualAddress; DWORD Size; } PW153_IMAGE_DATA_DIRECTORY;
struct PW153_IMAGE_OPTIONAL_HEADER32 : IMAGE_OPTIONAL_HEADER32
{
	PW153_IMAGE_DATA_DIRECTORY DataDirectory[16];
};
struct PW153_IMAGE_NT_HEADERS32
{
	DWORD Signature;
	IMAGE_FILE_HEADER FileHeader;
	PW153_IMAGE_OPTIONAL_HEADER32 OptionalHeader;
};
typedef PW153_IMAGE_NT_HEADERS32 * PW153_PIMAGE_NT_HEADERS32;
#define PIMAGE_NT_HEADERS32 PW153_PIMAGE_NT_HEADERS32
typedef struct _PW153_IMAGE_EXPORT_DIRECTORY {
	DWORD Characteristics;
	DWORD TimeDateStamp;
	WORD MajorVersion;
	WORD MinorVersion;
	DWORD Name;
	DWORD Base;
	DWORD NumberOfFunctions;
	DWORD NumberOfNames;
	DWORD AddressOfFunctions;
	DWORD AddressOfNames;
	DWORD AddressOfNameOrdinals;
} PW153_IMAGE_EXPORT_DIRECTORY, * PW153_PIMAGE_EXPORT_DIRECTORY;
#define PIMAGE_EXPORT_DIRECTORY PW153_PIMAGE_EXPORT_DIRECTORY
//	VirtualProtect/PAGE_* live in the platform umbrella (Windows.h) - the
//	contract owner; the same explicit failure semantics.

DWORD ReplaceAPI(LPCTSTR dllName, LPCSTR apiName, DWORD newEntry)
{
	PIMAGE_DOS_HEADER mz_header;
	PIMAGE_NT_HEADERS32 pe_header;
	LPDWORD entry;
	DWORD rva_base, entry_count, i, oldProtect;
	HMODULE hDll = GetModuleHandle(dllName);
	DWORD oldEntry = (DWORD)GetProcAddress(hDll, apiName);

	if( NULL == hDll || NULL == oldEntry )
		return oldEntry;

	if( 0 )//Engine::GetInstance().IsNT() )
	{
		rva_base  = (DWORD)hDll;
		mz_header = (PIMAGE_DOS_HEADER)hDll;
		pe_header = (PIMAGE_NT_HEADERS32)(rva_base + mz_header->e_lfanew);
		PIMAGE_EXPORT_DIRECTORY exp_dir = (PIMAGE_EXPORT_DIRECTORY)(rva_base + pe_header->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
		entry = (LPDWORD)(rva_base + exp_dir->AddressOfFunctions);
		entry_count = exp_dir->NumberOfFunctions;
		for ( i = 0; i < entry_count; i++ )
		{
			if ( oldEntry - rva_base == entry[i] )
			{
				VirtualProtect( entry + i, 4, PAGE_READWRITE, &oldProtect);
				entry[i] = newEntry - rva_base;
				VirtualProtect( entry + i, 4, oldProtect, &oldProtect);
				break;
			}
		}
		if ( i == entry_count )	return 0;
	}

	rva_base = (DWORD)GetModuleHandle( NULL );
	mz_header = (PIMAGE_DOS_HEADER)rva_base;
	pe_header = (PIMAGE_NT_HEADERS32)(rva_base + mz_header->e_lfanew);
	entry = (LPDWORD)(rva_base + pe_header->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IAT].VirtualAddress);
	entry_count = pe_header->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IAT].Size / sizeof(DWORD);
	for ( i = 0; i < entry_count; i++ )
	{
		if ( oldEntry == entry[i] )
		{
			VirtualProtect( entry + i, 4, PAGE_READWRITE, &oldProtect);
			entry[i] = newEntry;
			VirtualProtect( entry + i, 4, oldProtect, &oldProtect);
			break;
		}
	}
	return oldEntry;
}

