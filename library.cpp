#include "Windows.h"
#include "log.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include <safetyhook.hpp>

#include "JacketManager.h"
#include "D3D9HookManager.h"

#include <Psapi.h>
#include <span>

void unload(LPVOID dll_instance) {
    SPDLOG_INFO("Unloading.");

    JacketManager::UninstallHooks();
    D3D9HookManager::UninstallHooks();

    FreeLibraryAndExitThread(static_cast<HMODULE>(dll_instance), EXIT_SUCCESS);
}

DWORD WINAPI hook_init(LPVOID dll_instance) {
    auto logger = spdlog::stderr_color_mt("sdvx-rgb-buttons");
    spdlog::set_default_logger(logger);
#ifdef _DEBUG
    spdlog::set_level(spdlog::level::debug);
#endif
    spdlog::set_pattern("[%n %Y-%m-%d %T %l %s:%#] %v");
    SPDLOG_INFO("Loaded!");

    auto proc = GetCurrentProcess();
    auto module = GetModuleHandleW(L"soundvoltex.dll");
    MODULEINFO info;
    auto success = GetModuleInformation(proc, module, &info, sizeof(info));
    if (!success) {
        SPDLOG_ERROR("couldn't load sound voltex module");
        unload(dll_instance);
        return EXIT_FAILURE;
    }

    auto span = std::span<uint8_t>(reinterpret_cast<uint8_t *>(info.lpBaseOfDll), info.SizeOfImage);

    D3D9HookManager::InstallHooks();
    JacketManager::InstallHooks(span, (intptr_t)info.lpBaseOfDll);

    while (true) {
        if (GetAsyncKeyState(VK_F9)) {
            break;
        }
    }

    unload(dll_instance);
    return EXIT_SUCCESS;
}

BOOL APIENTRY DllMain(HINSTANCE dll_instance, const DWORD reason, LPVOID _) {
    if (reason == DLL_PROCESS_ATTACH) {
        CreateThread(nullptr, 0, hook_init, dll_instance, 0, nullptr);
    }

    return TRUE;
}