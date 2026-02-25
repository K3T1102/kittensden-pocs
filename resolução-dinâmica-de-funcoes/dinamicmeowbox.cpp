#include <windows.h>
#include <iostream>

int main() {
    HMODULE user32_module = LoadLibraryA("user32.dll");
    if (user32_module == NULL) {
        std::cerr << "Falha ao carregar user32.dll" << std::endl;
        return 1;
    }
    std::cout << "user32.dll carregada com sucesso" << std::endl;

    using MeowBoxA_t = int (WINAPI*)(HWND, LPCSTR, LPCSTR, UINT);
    MeowBoxA_t MeowBoxA = (MeowBoxA_t)GetProcAddress(user32_module, "MessageBoxA");
    if (MeowBoxA == NULL) {
        std::cerr << "Falha ao obter o endereço de MessageBoxA" << std::endl;
        FreeLibrary(user32_module);
        return 1;
    }
    std::cout << "Endereço de MessageBoxA obtido com sucesso" << std::endl;
    MeowBoxA(NULL, "meowmeow", "Kitten's den", MB_OK | MB_ICONINFORMATION);
    FreeLibrary(user32_module);
    return 0;
}