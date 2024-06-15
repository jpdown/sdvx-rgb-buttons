//
// Created by pneuma on 2024-06-15.
//

#ifndef SDVX_RGB_BUTTONS_JACKETMANAGER_H
#define SDVX_RGB_BUTTONS_JACKETMANAGER_H

#include <d3d9.h>
#include <span>
#include <cstdint>
#include <safetyhook.hpp>

class JacketManager {
public:
    static bool InstallHooks(std::span<uint8_t> module, intptr_t base);
    static void UninstallHooks();
    static LPDIRECT3DTEXTURE9 GetJacket();
    static void SetJacket(LPDIRECT3DTEXTURE9 jacket);
private:
    static LPDIRECT3DTEXTURE9 _jacket;
};


#endif //SDVX_RGB_BUTTONS_JACKETMANAGER_H
