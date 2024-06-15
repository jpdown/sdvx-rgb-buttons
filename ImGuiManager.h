//
// Created by pneuma on 2024-06-15.
//

#ifndef SDVX_RGB_BUTTONS_IMGUIMANAGER_H
#define SDVX_RGB_BUTTONS_IMGUIMANAGER_H

#include <windows.h>
#include <d3d9.h>

class ImGuiManager {
public:
    static void Render();
    static void Initialize(LPDIRECT3DDEVICE9 device);
    static void Teardown();
private:
    static HWND subclassed_window;
    static bool initialized;
};


#endif //SDVX_RGB_BUTTONS_IMGUIMANAGER_H
