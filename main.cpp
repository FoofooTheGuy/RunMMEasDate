#include <Windows.h>
#include <iostream>
#include <string>
#include <Psapi.h>

typedef void(WINAPI *InitDateProc)(FILETIME *);

// Helper: write memory into remote process
BOOL WriteRemoteMemory(HANDLE hProcess, LPVOID &pRemote, LPCVOID data, SIZE_T size)
{
    pRemote = VirtualAllocEx(hProcess, NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!pRemote)
        return FALSE;
    return WriteProcessMemory(hProcess, pRemote, data, size, NULL);
}

// Helper: get remote address of InitDate (trick: same offset after LoadLibrary)
LPVOID GetRemoteProcAddress(HANDLE hProcess, HMODULE hLocalModule, const std::wstring &dllPath, const std::string &procName)
{
    HMODULE hRemoteModule = nullptr;

    // This is naive but often works: assume LoadLibraryW loads DLL at same address
    // You can also parse remote PEB but that's more complex
    uintptr_t localBase = reinterpret_cast<uintptr_t>(hLocalModule);
    uintptr_t localProc = reinterpret_cast<uintptr_t>(GetProcAddress(hLocalModule, procName.c_str()));
    uintptr_t offset = localProc - localBase;

    return reinterpret_cast<LPVOID>(reinterpret_cast<uintptr_t>(hRemoteModule) + offset);
}

int main()
{
    // 1. Create fake FILETIME
    SYSTEMTIME sysTime = {2023, 1, 0, 1, 0, 0, 0, 0};
    FILETIME fakeFileTime;
    SystemTimeToFileTime(&sysTime, &fakeFileTime);

    // 2. Target EXE and DLL path
    std::wstring targetExe = L"..\\bin\\Debug\\GetTime.exe";
    std::wstring dllPath = L"RunAsDate.dll";

    // 3. Create suspended process
    STARTUPINFOW si = {sizeof(si)};
    PROCESS_INFORMATION pi = {};
    if (!CreateProcessW(NULL, (LPWSTR)targetExe.c_str(), NULL, NULL, FALSE,
                        CREATE_SUSPENDED, NULL, NULL, &si, &pi))
    {
        std::cerr << "Failed to launch target process.\n";
        return 1;
    }

    // 4. Inject DLL
    LPVOID remoteDllPath = nullptr;
    if (!WriteRemoteMemory(pi.hProcess, remoteDllPath, dllPath.c_str(), (dllPath.length() + 1) * sizeof(wchar_t)))
    {
        std::cerr << "Failed to write DLL path.\n";
        TerminateProcess(pi.hProcess, 1);
        return 1;
    }

    auto loadLibraryAddr = GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "LoadLibraryW");
    HANDLE hThread = CreateRemoteThread(pi.hProcess, NULL, 0,
                                        (LPTHREAD_START_ROUTINE)loadLibraryAddr, remoteDllPath, 0, NULL);
    WaitForSingleObject(hThread, INFINITE);
    CloseHandle(hThread);
    VirtualFreeEx(pi.hProcess, remoteDllPath, 0, MEM_RELEASE);

    // 5. Load DLL locally to calculate InitDate offset
    HMODULE hLocalDll = LoadLibraryW(dllPath.c_str());
    if (!hLocalDll)
    {
        std::cerr << "Failed to load local DLL.\n";
        TerminateProcess(pi.hProcess, 1);
        return 1;
    }

    // 6. Get InitDate address offset
    FARPROC localInitDate = GetProcAddress(hLocalDll, "InitDate");
    if (!localInitDate)
    {
        std::cerr << "Failed to find InitDate.\n";
        TerminateProcess(pi.hProcess, 1);
        return 1;
    }

    uintptr_t offset = (uintptr_t)localInitDate - (uintptr_t)hLocalDll;

    // 7. Calculate InitDate address in remote process
    HMODULE hRemoteDll = nullptr;
    EnumProcessModules(pi.hProcess, (HMODULE *)&hRemoteDll, sizeof(HMODULE), NULL); // risky
    LPVOID remoteInitDate = (LPBYTE)hRemoteDll + offset;

    // 8. Write FILETIME into remote process
    LPVOID remoteFileTime = nullptr;
    if (!WriteRemoteMemory(pi.hProcess, remoteFileTime, &fakeFileTime, sizeof(FILETIME)))
    {
        std::cerr << "Failed to write FILETIME.\n";
        TerminateProcess(pi.hProcess, 1);
        return 1;
    }

    // 9. Call InitDate(FILETIME*) in remote process
    HANDLE hInitThread = CreateRemoteThread(pi.hProcess, NULL, 0,
                                            (LPTHREAD_START_ROUTINE)remoteInitDate, remoteFileTime, 0, NULL);
    WaitForSingleObject(hInitThread, INFINITE);
    CloseHandle(hInitThread);

    // 10. Resume main thread
    ResumeThread(pi.hThread);

    // Cleanup
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    FreeLibrary(hLocalDll);

    std::cout << "Target process started with fake date injected!\n";
    return 0;
}
