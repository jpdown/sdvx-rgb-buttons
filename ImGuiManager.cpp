//
// Created by pneuma on 2024-06-15.
//

#include "ImGuiManager.h"
#include <commctrl.h>
#include <imgui.h>
#include <backends/imgui_impl_dx9.h>
#include <backends/imgui_impl_win32.h>

#include "log.h"
#include "JacketManager.h"

#define SUBCLASS_ID 0x2BA295

HWND ImGuiManager::subclassed_window = nullptr;
bool ImGuiManager::initialized = false;

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

void ImGuiManager::Render() {
    ImGui_ImplDX9_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImGui::GetIO().MouseDrawCursor = true;

    {
        ImGui::Begin("SDVX RGB Buttons");

        ImGui::Text("soon i will conquer the world");

        auto jacket = JacketManager::GetJacket();

        if (jacket != nullptr) {
            D3DSURFACE_DESC jacket_desc;
            jacket->GetLevelDesc(0, &jacket_desc);
            ImGui::Image((void*)jacket, ImVec2(jacket_desc.Width, jacket_desc.Height));
        } else {
            ImGui::Text("no jacket loaded");
        }

        ImGui::End();
    }

    ImGui::EndFrame();

    ImGui::Render();
    ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
}

void ImGuiManager::Initialize(LPDIRECT3DDEVICE9 device) {
    if (!initialized) {
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

        initialized = true;
    }
}

void ImGuiManager::Teardown() {
    if (initialized) {
        ImGui_ImplDX9_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();

        RemoveWindowSubclass(subclassed_window, WndProc, SUBCLASS_ID);
    }

    initialized = false;
}

