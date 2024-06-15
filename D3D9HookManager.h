//
// Created by pneuma on 2024-06-15.
//

#ifndef SDVX_RGB_BUTTONS_D3D9HOOKMANAGER_H
#define SDVX_RGB_BUTTONS_D3D9HOOKMANAGER_H

#include <span>
#include <cstdint>
#include <safetyhook.hpp>
#include <d3d9.h>

class D3D9HookManager {
public:
    static bool InstallHooks();
    static void UninstallHooks();
};


#endif //SDVX_RGB_BUTTONS_D3D9HOOKMANAGER_H
