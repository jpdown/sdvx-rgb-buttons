//
// Created by pneuma on 2024-06-15.
//

#include "JacketManager.h"

#include "external/signature-scanner/signature.h"
#include "log.h"

constexpr auto gameplay_jacket_load_sig = "90 48 8B 00 48 89 45 ? 48 8D 55 ? 48 8B CE"_sig;

LPDIRECT3DTEXTURE9 JacketManager::_jacket = nullptr;
static SafetyHookMid gameplay_jacket_load = {};

void GameplayJacketLoadHook(SafetyHookContext &ctx) {
    auto new_jacket_ptr = (LPDIRECT3DTEXTURE9*) (ctx.rbp - 0x39);
    auto new_jacket = *new_jacket_ptr;
    SPDLOG_DEBUG("got jacket ptr {:x}", (intptr_t) new_jacket_ptr);
    SPDLOG_DEBUG("got jacket {:x}", (intptr_t) new_jacket);

    new_jacket->AddRef();

    auto existing_jacket = JacketManager::GetJacket();
    if (existing_jacket != nullptr) {
        existing_jacket->Release();
    }

    JacketManager::SetJacket(new_jacket);
}


bool JacketManager::InstallHooks(std::span<uint8_t> module, intptr_t base) {
    auto jacket_load_offset = gameplay_jacket_load_sig.scan(module);
    if (jacket_load_offset == -1) {
        SPDLOG_ERROR("couldn't find gameplay jacket load instruction");
        UninstallHooks();
        return false;
    }

    gameplay_jacket_load = safetyhook::create_mid(base + jacket_load_offset, GameplayJacketLoadHook);
    SPDLOG_INFO("Installed gameplay jacket load hook.");

    return true;
}

LPDIRECT3DTEXTURE9 JacketManager::GetJacket() {
    return _jacket;
}

void JacketManager::SetJacket(LPDIRECT3DTEXTURE9 jacket) {
    _jacket = jacket;
    // TODO generate jacket colour palette
}

void JacketManager::UninstallHooks() {
    if (_jacket != nullptr) {
        _jacket->Release();
        _jacket = nullptr;
    }
    gameplay_jacket_load.reset();
}



