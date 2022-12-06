#include <windows.h>
#include <iostream>

BOOL file_exists(LPCWSTR file) {
	WIN32_FIND_DATAW find_file_data;
	const auto file_handle = FindFirstFileW(file, &find_file_data);

	if (file_handle == INVALID_HANDLE_VALUE) {
		return FALSE;
	}

	FindClose(file_handle);
	return TRUE;
}


int main() {
    auto np_exe = L"C:\\WINDOWS\\system32\\notepad.exe";
    const auto test_dll = L"test.dll";

    if (!file_exists(test_dll)) {
        MessageBoxW(NULL, L"test dll not found", L"Error", MB_OK);
        std::exit(0);
    }

    wchar_t test_dll_full[MAX_PATH];
    _wfullpath(test_dll_full, test_dll, MAX_PATH);

    if (!file_exists(np_exe)) {
        MessageBoxW(NULL, L"NotePad not found", L"Error", MB_OK);
        std::exit(0);
    }

    WCHAR path[MAX_PATH];
	_wsplitpath_s(np_exe, nullptr, 0, path, _countof(path), nullptr, 0, nullptr, 0);
    STARTUPINFOW si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	ZeroMemory(&pi, sizeof(pi));
	si.cb = sizeof(si);
    if (!CreateProcessW(np_exe, NULL, NULL, NULL, false, CREATE_SUSPENDED, NULL, path, &si, &pi)) {
        MessageBoxW(NULL, L"CreateProcess failed", L"Error", MB_OK);
        std::exit(0);
    };
    
    DWORD dwPid = GetProcessId(pi.hProcess);
    printf("%lu", dwPid);

    const auto ll_addr = reinterpret_cast<LPVOID>(GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "LoadLibraryW"));
    const auto ll_arg_mem = VirtualAllocEx(pi.hProcess, nullptr, wcslen(test_dll_full) * sizeof(wchar_t) + 1,
		MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    WriteProcessMemory(pi.hProcess, ll_arg_mem, test_dll_full, wcslen(test_dll_full) * sizeof(wchar_t) + 1, NULL);
    const auto h_thread = CreateRemoteThread(pi.hProcess, NULL, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(ll_addr),
		ll_arg_mem, 0, NULL);
    WaitForSingleObject(h_thread, INFINITE);
    DWORD exit_code;
	GetExitCodeThread(h_thread, &exit_code);
    Sleep(100);
	ResumeThread(pi.hThread);
}