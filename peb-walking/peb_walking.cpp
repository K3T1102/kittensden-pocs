#include <windows.h>
#include <iostream>
#include <intrin.h>

using PPEB_T = struct _PEB*;
using PPEB_LDR_DATA_T = struct _PEB_LDR_DATA*;

struct UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
};

struct _PEB_LDR_DATA {
    ULONG Length;
    BOOLEAN Initialized;
    PVOID SsHandle;
    LIST_ENTRY InLoadOrderModuleList;
    LIST_ENTRY InMemoryOrderModuleList;
    LIST_ENTRY InInitializationOrderModuleList;
};

struct _PEB {
    BYTE Reserved1[2];
    BYTE BeingDebugged;
    BYTE Reserved2[1];
    PVOID Reserved3[2];
    PPEB_LDR_DATA_T Ldr;
};

struct _LDR_DATA_TABLE_ENTRY {
    LIST_ENTRY InLoadOrderLinks;
    LIST_ENTRY InMemoryOrderLinks;
    LIST_ENTRY InInitializationOrderLinks;
    PVOID DllBase;
    PVOID EntryPoint;
    ULONG SizeOfImage;
    UNICODE_STRING FullDllName;
    UNICODE_STRING BaseDllName;
};

HMODULE GetModuleHandlePeb(LPCWSTR moduleName) {
    PPEB_T pPeb = (PPEB_T)__readgsqword(0x60);

    if (!pPeb || !pPeb->Ldr)
        return nullptr;

    PPEB_LDR_DATA_T pLdr = pPeb->Ldr;

    LIST_ENTRY* head = &pLdr->InLoadOrderModuleList;
    LIST_ENTRY* current = head->Flink;

    while (current != head)
    {
        auto entry = CONTAINING_RECORD(current, _LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);

        if (_wcsicmp(entry->BaseDllName.Buffer, moduleName) == 0)
        {
            return (HMODULE)entry->DllBase;
        }

        current = current->Flink;
    };

    return nullptr;
}

FARPROC GetProcAddressPeb(HMODULE hModule, LPCSTR procName) {
    if (!hModule) return nullptr;

    auto dosHeader = (PIMAGE_DOS_HEADER)hModule;
    auto ntHeaders = (PIMAGE_NT_HEADERS)((BYTE*)hModule + dosHeader->e_lfanew);
    auto exportDirRVA = ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;

    if (!exportDirRVA) return nullptr;

    auto exportDir = (PIMAGE_EXPORT_DIRECTORY)((BYTE*)hModule + exportDirRVA);
    auto names = (DWORD*)((BYTE*)hModule + exportDir->AddressOfNames);
    auto ordinals = (WORD*)((BYTE*)hModule + exportDir->AddressOfNameOrdinals);
    auto functions = (DWORD*)((BYTE*)hModule + exportDir->AddressOfFunctions);

    for (DWORD i = 0; i < exportDir->NumberOfNames; i++)
    {
        LPCSTR name = (LPCSTR)((BYTE*)hModule + names[i]);
        if (_stricmp(name, procName) == 0)
        {
            WORD ordinal = ordinals[i];
            DWORD funcRVA = functions[ordinal];
            return (FARPROC)((BYTE*)hModule + funcRVA);
        }
    }

    return nullptr;
}

int main() {

    HMODULE hKernel32 = GetModuleHandlePeb(L"kernel32.dll");
    if (hKernel32) {
        std::wcout << L"Kernel32 encontrada em: " << hKernel32 << std::endl;
        
        // Pegar VirtualAlloc manualmente
        using VirtualAlloc_t = LPVOID(WINAPI*)(LPVOID, SIZE_T, DWORD, DWORD);
        auto myVirtualAlloc = (VirtualAlloc_t)GetProcAddressPeb(hKernel32, "VirtualAlloc");
        
        if (myVirtualAlloc) {
            std::cout << "VirtualAlloc encontrada!" << std::endl;
        }
    }

    HMODULE hNtdll = GetModuleHandlePeb(L"ntdll.dll");

    if(hNtdll) {
        std::wcout << L"Ntdll encontrada em: " << hNtdll << std::endl;

    }

    return 0;
};
