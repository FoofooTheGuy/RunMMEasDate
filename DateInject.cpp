#include <Windows.h>
#include <minhook.h> // Include MinHook header
#include <stdio.h>

FILETIME g_fakeFileTime;
SYSTEMTIME g_fakeSystemTime;
bool g_hooked = false;

// Pointer to the original function
decltype(&GetLocalTime) original_GetLocalTime = nullptr;
decltype(&GetSystemTime) original_GetSystemTime = nullptr;
decltype(&GetSystemTimeAsFileTime) original_GetSystemTimeAsFileTime = nullptr;
decltype(&GetSystemTimePreciseAsFileTime) original_GetSystemTimePreciseAsFileTime = nullptr;
//decltype(&NtQuerySystemTime) original_NtQuerySystemTime = nullptr;

// Hooked functions
void WINAPI MyGetLocalTime(LPSYSTEMTIME lpSystemTime)
{
	OutputDebugStringW(L"[RunAsDate] Hooked GetLocalTime called");
	*lpSystemTime = g_fakeSystemTime;
}
void WINAPI MyGetSystemTime(LPSYSTEMTIME lpSystemTime)
{
	OutputDebugStringW(L"[RunAsDate] Hooked GetSystemTime called");
	*lpSystemTime = g_fakeSystemTime;
}
void WINAPI MyGetSystemTimeAsFileTime(LPFILETIME lpSystemTimeAsFileTime)
{
	OutputDebugStringW(L"[RunAsDate] Hooked GetSystemTimeAsFileTime called");
	*lpSystemTimeAsFileTime = g_fakeFileTime;
}
void WINAPI MyGetSystemTimePreciseAsFileTime(LPFILETIME lpSystemTimeAsFileTime)
{
	OutputDebugStringW(L"[RunAsDate] Hooked GetSystemTimePreciseAsFileTime called");
	*lpSystemTimeAsFileTime = g_fakeFileTime;
}
/*ULONGLONG WINAPI MyNtQuerySystemTime(LPFILETIME SystemTime)
{
	OutputDebugStringW(L"[RunAsDate] Hooked GetSystemTimePreciseAsFileTime called");
	*SystemTime = g_fakeFileTime;
}*/

// Exported function to initialize fake time and hook
extern "C" __declspec(dllexport) void InitDate(FILETIME *pFakeTime)
{
	OutputDebugStringW(L"[RunAsDate] InitDate called in target process");
	g_fakeFileTime = *pFakeTime;
	FileTimeToSystemTime(&g_fakeFileTime, &g_fakeSystemTime);
	
	if (g_hooked)
		return;
	
	OutputDebugStringW(L"[RunAsDate] InitDate called in target process");
	if (MH_Initialize() != MH_OK)
		return;
	
	// GetLocalTime
	OutputDebugStringW(L"[RunAsDate] Create Hook GetLocalTime");
	if (MH_CreateHook(&GetLocalTime, &MyGetLocalTime, reinterpret_cast<LPVOID *>(&original_GetLocalTime)) != MH_OK)
		return;
	
	OutputDebugStringW(L"[RunAsDate] Enable Hook GetLocalTime");
	if (MH_EnableHook(&GetLocalTime) != MH_OK)
		return;

	// GetSystemTime
	OutputDebugStringW(L"[RunAsDate] Create Hook GetSystemTime");
	if (MH_CreateHook(&GetSystemTime, &MyGetSystemTime, reinterpret_cast<LPVOID *>(&original_GetSystemTime)) != MH_OK)
		return;
	
	OutputDebugStringW(L"[RunAsDate] Enable Hook GetSystemTime");
	if (MH_EnableHook(&GetSystemTime) != MH_OK)
		return;
	
	// GetSystemTimeAsFileTime
	OutputDebugStringW(L"[RunAsDate] Create Hook GetSystemTimeAsFileTime");
	if (MH_CreateHook(&GetSystemTimeAsFileTime, &MyGetSystemTimeAsFileTime, reinterpret_cast<LPVOID *>(&original_GetSystemTimeAsFileTime)) != MH_OK)
		return;
	
	OutputDebugStringW(L"[RunAsDate] Enable Hook GetSystemTimeAsFileTime");
	if (MH_EnableHook(&GetSystemTimeAsFileTime) != MH_OK)
		return;

	// GetSystemTimePreciseAsFileTime
	OutputDebugStringW(L"[RunAsDate] Create Hook GetSystemTimePreciseAsFileTime");
	if (MH_CreateHook(&GetSystemTimePreciseAsFileTime, &MyGetSystemTimePreciseAsFileTime, reinterpret_cast<LPVOID *>(&original_GetSystemTimePreciseAsFileTime)) != MH_OK)
		return;
	
	OutputDebugStringW(L"[RunAsDate] Enable Hook GetSystemTimePreciseAsFileTime");
	if (MH_EnableHook(&GetSystemTimePreciseAsFileTime) != MH_OK)
		return;

	// NtQuerySystemTime
	/*OutputDebugStringW(L"[RunAsDate] Create Hook NtQuerySystemTime");
	if (MH_CreateHook(&NtQuerySystemTime, &MyNtQuerySystemTime, reinterpret_cast<LPVOID *>(&original_NtQuerySystemTime)) != MH_OK)
		return;
	
	OutputDebugStringW(L"[RunAsDate] Enable Hook NtQuerySystemTime");
	if (MH_EnableHook(&NtQuerySystemTime) != MH_OK)
		return;*/
	
	g_hooked = true;
}