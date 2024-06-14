#include "library.h"

#include "Windows.h"
#include "log.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include <safetyhook.hpp>

#include "d3d9_hook.h"

#include "external/signature-scanner/signature.h"

#include <iostream>
#include <Psapi.h>
#include <span>

constexpr auto jacket_load_sig = "90 48 8B 00 48 89 45 ? 48 8D 55 ? 48 8B CE"_sig;

SafetyHookMid jacket_load_hook{};

void hooked_jacket_load(SafetyHookContext &ctx) {
    auto new_jacket_ptr = (LPDIRECT3DTEXTURE9*) (ctx.rbp - 0x39);
    auto new_jacket = *new_jacket_ptr;
    SPDLOG_INFO("got jacket ptr {:x}", (intptr_t) new_jacket_ptr);
    SPDLOG_INFO("got jacket {:x}", (intptr_t) new_jacket);

    new_jacket->AddRef();

    if (jacket != nullptr) {
        jacket->Release();
    }

    jacket = new_jacket;
}

void unload(LPVOID dll_instance) {
    SPDLOG_INFO("Unloading.");

    if (jacket != nullptr) {
        jacket->Release();
    }

    jacket_load_hook.reset();
    remove_hook();
    FreeLibraryAndExitThread(static_cast<HMODULE>(dll_instance), EXIT_SUCCESS);
}

DWORD WINAPI hook_init(LPVOID dll_instance) {
    auto logger = spdlog::stderr_color_mt("sdvx-rgb-buttons");
    spdlog::set_default_logger(logger);
#ifdef _DEBUG
    spdlog::set_level(spdlog::level::debug);
#endif
    spdlog::set_pattern("[%n %Y-%m-%d %T %l %s:%#:%!] %v");
    SPDLOG_INFO("Loaded!");

    init_hook();

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

    auto jacket_load_offset = jacket_load_sig.scan(span);
    if (jacket_load_offset == -1) {
        SPDLOG_ERROR("couldn't find jacket load instruction");
        unload(dll_instance);
        return EXIT_FAILURE;
    }

    jacket_load_hook = safetyhook::create_mid((intptr_t)info.lpBaseOfDll + jacket_load_offset, hooked_jacket_load);

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