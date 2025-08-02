#include <Windows.h>
#include <iostream>
#include <string>
#include <Psapi.h>

typedef void(WINAPI *InitDateProc)(FILETIME *);

//https://stackoverflow.com/a/10738141
std::wstring s2ws(const std::string& str)
{
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstrTo( size_needed, 0 );
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}

// Helper: write memory into remote process
BOOL WriteRemoteMemory(HANDLE hProcess, LPVOID &pRemote, LPCVOID data, SIZE_T size, DWORD Protect = PAGE_READWRITE)
{
	pRemote = VirtualAllocEx(hProcess, NULL, size, MEM_COMMIT | MEM_RESERVE, Protect);
	if (!pRemote)
		return FALSE;
	return WriteProcessMemory(hProcess, pRemote, data, size, NULL);
}

// Helper: get remote address of InitDate (trick: same offset after LoadLibrary)
/*LPVOID GetRemoteProcAddress(HANDLE hProcess, HMODULE hLocalModule, const std::wstring &dllPath, const std::string &procName)
{
	HMODULE hRemoteModule = nullptr;

	// This is naive but often works: assume LoadLibraryW loads DLL at same address
	// You can also parse remote PEB but that's more complex
	uintptr_t localBase = reinterpret_cast<uintptr_t>(hLocalModule);
	uintptr_t localProc = reinterpret_cast<uintptr_t>(GetProcAddress(hLocalModule, procName.c_str()));
	uintptr_t offset = localProc - localBase;

	return reinterpret_cast<LPVOID>(reinterpret_cast<uintptr_t>(hRemoteModule) + offset);
}*/

//other
void copy_str(wchar_t *out, const wchar_t *in)
{
	size_t len = wcslen(in);
	if ( len >= 260 )
		len = 259;
	memcpy(out, in, len * 2);
	out[len] = 0;
}

typedef HMODULE (* my_loadLibraryW)(wchar_t *lpLibFileName);
typedef FARPROC (* my_GetProcAddress)(HMODULE hModule, LPCSTR lpProcName);

struct inject_ctx
{
  my_loadLibraryW this_LoadLibraryW;
  my_GetProcAddress this_GetProcAddress;
  wchar_t dateinject_dll_path[261];
  char dll_entrypoint_funcname[9];
  FILETIME datearg;
};

// Find and call the InitDate function (only for target process)
static DWORD getInitDateFunc(inject_ctx *lpThreadParameter)
{
  HMODULE dateinj = lpThreadParameter->this_LoadLibraryW(lpThreadParameter->dateinject_dll_path);
  typedef void (* initdate)(FILETIME* datearg);
  initdate initdate_func = (initdate)lpThreadParameter->this_GetProcAddress(dateinj, lpThreadParameter->dll_entrypoint_funcname);
  if ( initdate_func )
	initdate_func(&lpThreadParameter->datearg);
  return 0;
}

int main(int argc, char* argv[])
{
	// Create fake FILETIME
	SYSTEMTIME sysTime = {2010, 1, 0, 1, 0, 0, 0, 0};
	FILETIME fakeFileTime;
	SystemTimeToFileTime(&sysTime, &fakeFileTime);

	// get MME path
	wchar_t* MMEpath;
	size_t MMEpathLength;
	errno_t err = _wdupenv_s( &MMEpath, &MMEpathLength, L"MOBICLIP_MULTICORE_ENCODER_PATH" );
	if (err) {
		std::cerr << "Failed to get MME path\n";
		return 1;
	}
	
	// MMEpath + L"\\MobiclipMulticoreEncoder.exe";
	std::wstring CmdLine(MMEpath);
	CmdLine += L"\\MobiclipMulticoreEncoder.exe ";

	// DLL path
	std::wstring dllPath = L"DateInject.dll";
	
	// Forward args to CmdLine
	for(int i = 1; i < argc; i++) {
		CmdLine += s2ws(std::string(argv[i]));
		CmdLine += ' ';
	}
	std::wcout << CmdLine << std::endl;

	// Create suspended process
	STARTUPINFOW si = {sizeof(si)};
	PROCESS_INFORMATION pi = {};
	if (!CreateProcessW(NULL, (LPWSTR)CmdLine.c_str(), NULL, NULL, FALSE,
						CREATE_SUSPENDED, NULL, NULL, &si, &pi))
	{
		std::cerr << "Failed to launch target process.\n";
		return 1;
	}

	// Get InitDate function in remote process
	inject_ctx injection;
	memset(&injection, 0, sizeof(inject_ctx));
	
	HMODULE ModuleHandleW = GetModuleHandleW(L"kernel32.dll");
	
	// Set up args for custom function
	injection.this_GetProcAddress = (my_GetProcAddress)GetProcAddress(ModuleHandleW, "GetProcAddress");
	injection.this_LoadLibraryW = (my_loadLibraryW)GetProcAddress(ModuleHandleW, "LoadLibraryW");

	copy_str(injection.dateinject_dll_path, dllPath.c_str());
	strcpy(injection.dll_entrypoint_funcname, "InitDate");
	injection.datearg = fakeFileTime;

	// Inject custom function + args to call InitDate
	LPVOID remote_getInitDateFunc = nullptr;//void*
	if (!WriteRemoteMemory(pi.hProcess, remote_getInitDateFunc, getInitDateFunc, 0x400, PAGE_EXECUTE_READWRITE))
	{
		std::cerr << "Failed to write function.\n";
		TerminateProcess(pi.hProcess, 1);
		return 1;
	}
	
	LPVOID remote_getIDFuncArg = nullptr;
	if (!WriteRemoteMemory(pi.hProcess, remote_getIDFuncArg, &injection, sizeof(inject_ctx), PAGE_EXECUTE_READWRITE))
	{
		std::cerr << "Failed to write args for function.\n";
		TerminateProcess(pi.hProcess, 1);
		return 1;
	}
	
	// Call function in new thread
	HANDLE RemoteThread = CreateRemoteThread(
		pi.hProcess,
		NULL,
		0,
		(LPTHREAD_START_ROUTINE)remote_getInitDateFunc,
		remote_getIDFuncArg,
		4,
		NULL);
	if (RemoteThread)
	{
		ResumeThread(RemoteThread);
		WaitForSingleObject(RemoteThread, INFINITE);
		CloseHandle(RemoteThread);
	}

	// Resume main thread
	ResumeThread(pi.hThread);

	// Cleanup
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	//FreeLibrary(hLocalDll);

	std::cout << "Target process started with fake date injected!\n";
	return 0;
}