//
// Created by pneuma on 5/27/2024.
//

#include <cstdio>
#include "d3d9_hook.h"
#include "log.h"
#include <safetyhook.hpp>

#include <imgui.h>
#include <backends/imgui_impl_dx9.h>
#include <backends/imgui_impl_win32.h>
#include <windows.h>
#include <commctrl.h>

void *endscene_func;

bool imgui_initialized = false;

#define SUBCLASS_ID 0x2BA295
HWND subclassed_window;

SafetyHookInline end_scene_hook{};

LPDIRECT3DTEXTURE9 jacket = nullptr;

// Forward declaration of helpers
void initialize_imgui(LPDIRECT3DDEVICE9 device);
LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubClass, DWORD_PTR dwRefData);


HRESULT SAFETYHOOK_NOINLINE SAFETYHOOK_STDCALL end_scene(LPDIRECT3DDEVICE9 device) {
    if (!imgui_initialized) {
        initialize_imgui(device);
    }

    ImGui_ImplDX9_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImGui::GetIO().MouseDrawCursor = true;

    {
        ImGui::Begin("SDVX RGB Buttons");

        ImGui::Text("soon i will conquer the world");

        if (jacket != nullptr) {
            D3DSURFACE_DESC my_image_desc;
            jacket->GetLevelDesc(0, &my_image_desc);
            ImGui::Image((void*)jacket, ImVec2(my_image_desc.Width, my_image_desc.Height));
        } else {
            ImGui::Text("no jacket loaded");
        }

        ImGui::End();
    }

    ImGui::EndFrame();

    ImGui::Render();
    ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

    return end_scene_hook.stdcall<HRESULT>(device);
}

void init_hook() {
    IDirect3D9 *d3d = Direct3DCreate9(D3D_SDK_VERSION);
    IDirect3DDevice9 *dummy;

    // want to create a new d3d9 device that we will not actually use
    D3DPRESENT_PARAMETERS params = {};
    params.Windowed = true;
    params.SwapEffect = D3DSWAPEFFECT_DISCARD;

    // we can pass in a bogus window, so just use the default desktop window
    HRESULT result = d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, GetDesktopWindow(), D3DCREATE_HARDWARE_VERTEXPROCESSING, &params, &dummy);

    if (result != D3D_OK) {
        SPDLOG_ERROR("error with d3d %ld, not hooking dx9", result);
        return;
    }

    // The 42nd index into the table is the EndScene function (you can verify by looking at d3d9.h)
    void ***table = reinterpret_cast<void ***>(dummy);
    // Copy this address to a value we own
    endscene_func = (*table)[42];

    // remove the device so we don't have anything dangling
    dummy->Release();
    d3d->Release();

    // install the hook
    end_scene_hook = safetyhook::create_inline(endscene_func, reinterpret_cast<void *>(end_scene));
    SPDLOG_INFO("end scene hook installed");
}

void remove_hook() {
    if (imgui_initialized) {
        ImGui_ImplDX9_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();

        RemoveWindowSubclass(subclassed_window, WndProc, SUBCLASS_ID);
    }

    // uninstall the hook, safetyhook makes this easy
    end_scene_hook.reset();
    SPDLOG_INFO("end scene hook uninstalled");
}



void initialize_imgui(LPDIRECT3DDEVICE9 device) {
    D3DDEVICE_CREATION_PARAMETERS creation_params;
    auto result = device->GetCreationParameters(&creation_params);

    if (result != D3D_OK) {
        SPDLOG_ERROR("couldn't get window for imgui, error %l", result);
        return;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.IniFilename = nullptr; // Disable writing an imgui.ini to the game folder

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(creation_params.hFocusWindow);
    ImGui_ImplDX9_Init(device);

    SetWindowSubclass(creation_params.hFocusWindow, WndProc, SUBCLASS_ID, reinterpret_cast<DWORD_PTR>(nullptr));
    // Need to store this to undo the subclass on unload
    subclassed_window = creation_params.hFocusWindow;

    imgui_initialized = true;
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
        return true;

    if (uMsg == WM_NCDESTROY) {
        RemoveWindowSubclass(hWnd, WndProc, uIdSubclass);
    }

    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}
