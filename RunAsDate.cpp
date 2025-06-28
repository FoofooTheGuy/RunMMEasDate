#include <Windows.h>
#include <minhook.h> // Include MinHook header
#include <stdio.h>

FILETIME g_fakeFileTime;
bool g_hooked = false;

// Pointer to the original function
decltype(&GetSystemTimeAsFileTime) original_GetSystemTimeAsFileTime = nullptr;

// Hooked function
void WINAPI MyGetSystemTimeAsFileTime(LPFILETIME lpSystemTimeAsFileTime)
{
    OutputDebugStringW(L"[RunAsDate] Hooked GetSystemTimeAsFileTime called");
    *lpSystemTimeAsFileTime = g_fakeFileTime;
}

// Exported function to initialize fake time and hook
extern "C" __declspec(dllexport) void InitDate(FILETIME *pFakeTime)
{
    OutputDebugStringW(L"[RunAsDate] InitDate called in target process");
    g_fakeFileTime = *pFakeTime;

    if (g_hooked)
        return;

    OutputDebugStringW(L"[RunAsDate] InitDate called in target process");
    if (MH_Initialize() != MH_OK)
        return;

    OutputDebugStringW(L"[RunAsDate] MH_CreateHook");
    if (MH_CreateHook(&GetSystemTimeAsFileTime, &MyGetSystemTimeAsFileTime,
                      reinterpret_cast<LPVOID *>(&original_GetSystemTimeAsFileTime)) != MH_OK)
    {
        return;
    }

    OutputDebugStringW(L"[RunAsDate] MH_EnableHook");
    if (MH_EnableHook(&GetSystemTimeAsFileTime) != MH_OK)
        return;

    g_hooked = true;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(hModule);

        // Auto-call InitDate with hardcoded time
        SYSTEMTIME st = {2023, 1, 0, 1, 0, 0, 0, 0};
        FILETIME ft;
        SystemTimeToFileTime(&st, &ft);
        InitDate(&ft);
    }
    return TRUE;
}
