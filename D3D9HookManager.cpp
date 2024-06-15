//
// Created by pneuma on 2024-06-15.
//

#include "D3D9HookManager.h"

#include "log.h"
#include "ImGuiManager.h"

static SafetyHookInline end_scene{};

HRESULT SAFETYHOOK_NOINLINE SAFETYHOOK_STDCALL EndSceneHook(LPDIRECT3DDEVICE9 device) {
    ImGuiManager::Initialize(device);
    ImGuiManager::Render();
    return end_scene.stdcall<HRESULT>(device);
}

bool D3D9HookManager::InstallHooks() {
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
        return false;
    }

    // The 42nd index into the table is the EndScene function (you can verify by looking at d3d9.h)
    void ***table = reinterpret_cast<void ***>(dummy);
    // Copy this address to a value we own
    void *endscene_func = (*table)[42];

    // remove the device so we don't have anything dangling
    dummy->Release();
    d3d->Release();

    // install the hook
    end_scene = safetyhook::create_inline(endscene_func, EndSceneHook);
    SPDLOG_INFO("end scene hook installed");
    return true;
}

void D3D9HookManager::UninstallHooks() {
    ImGuiManager::Teardown();
    end_scene.reset();
}
