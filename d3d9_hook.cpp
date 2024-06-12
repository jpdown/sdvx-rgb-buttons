//
// Created by pneuma on 5/27/2024.
//

#include <cstdio>
#include "d3d9_hook.h"
#include "spdlog/spdlog.h"
#include <safetyhook.hpp>

#include <imgui.h>
#include <backends/imgui_impl_dx9.h>
#include <backends/imgui_impl_win32.h>

void *endscene_func;

bool imgui_initialized = false;
bool show_demo_window = false;
bool show_another_window = false;
ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

SafetyHookInline end_scene_hook{};

void initialize_imgui(LPDIRECT3DDEVICE9 device) {
    D3DDEVICE_CREATION_PARAMETERS creation_params;
    auto result = device->GetCreationParameters(&creation_params);

    if (result == D3D_OK) {
        SPDLOG_INFO("got window");
        SPDLOG_INFO((void *)creation_params.hFocusWindow);
    } else {
        SPDLOG_ERROR("got error %l", result);
        return;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(creation_params.hFocusWindow);
    ImGui_ImplDX9_Init(device);

    imgui_initialized = true;


    // TODO need to receive window messages and pass to imgui
    // I believe I can do this through subclassing the window
}

HRESULT SAFETYHOOK_NOINLINE SAFETYHOOK_STDCALL end_scene(LPDIRECT3DDEVICE9 device) {

    if (!imgui_initialized) {
        initialize_imgui(device);
    }

    ImGui_ImplDX9_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
    if (show_demo_window)
        ImGui::ShowDemoWindow(&show_demo_window);

    // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
    {
        static float f = 0.0f;
        static int counter = 0;

        ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

        ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
        ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
        ImGui::Checkbox("Another Window", &show_another_window);

        ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
        ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

        if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
            counter++;
        ImGui::SameLine();
        ImGui::Text("counter = %d", counter);

        ImGui::End();
    }

    // 3. Show another simple window.
    if (show_another_window)
    {
        ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
        ImGui::Text("Hello from another window!");
        if (ImGui::Button("Close Me"))
            show_another_window = false;
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
        SPDLOG_ERROR("error with d3d %ld", result);
        SPDLOG_ERROR("is devicelost %d", result == D3DERR_DEVICELOST);
        SPDLOG_ERROR("is invalidcall %d", result == D3DERR_INVALIDCALL);
        SPDLOG_ERROR("is notavailable %d", result == D3DERR_NOTAVAILABLE);
        SPDLOG_ERROR("is outofvideomemory %d", result == D3DERR_OUTOFVIDEOMEMORY);
    }

    // The 42nd index into the table is the EndScene function (you can verify by looking at d3d9.h)
    void ***table = reinterpret_cast<void ***>(dummy);
    SPDLOG_DEBUG(std::format("dummy pointer {:p} table pointer {:p} 42 pointer {:p}", (void*)dummy, (void*)table, (*table)[42]));
    // Copy this address to a value we own
    endscene_func = (*table)[42];

    // remove the device so we don't have anything dangling
    dummy->Release();
    d3d->Release();

    SPDLOG_DEBUG("installing hook...");
    // install the hook
    end_scene_hook = safetyhook::create_inline(endscene_func, reinterpret_cast<void *>(end_scene));
    SPDLOG_INFO("end scene hook installed");
}

void remove_hook() {
    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    // uninstall the hook, safetyhook makes this easy
    end_scene_hook.reset();
    SPDLOG_INFO("end scene hook uninstalled");
}
