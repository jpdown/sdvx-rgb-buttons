#include "library.h"

#include "Windows.h"
#include "spdlog/spdlog.h"

#include "d3d9_hook.h"

#include <iostream>

DWORD WINAPI hook_init(LPVOID dll_instance) {
#ifdef _DEBUG
    spdlog::set_level(spdlog::level::debug);
#endif
    spdlog::set_pattern("[%n %Y-%m-%d %T %l %s:%#:%!] %v");
    SPDLOG_INFO("Loaded!");

    init_hook();

    while (true) {
        if (GetAsyncKeyState(VK_F9)) {
            break;
        }
    }

    SPDLOG_INFO("Unloading.");
    remove_hook();
    FreeLibraryAndExitThread(static_cast<HMODULE>(dll_instance), EXIT_SUCCESS);
}

BOOL APIENTRY DllMain(HINSTANCE dll_instance, const DWORD reason, LPVOID _) {
    if (reason == DLL_PROCESS_ATTACH) {
        CreateThread(nullptr, 0, hook_init, dll_instance, 0, nullptr);
    }

    return TRUE;
}