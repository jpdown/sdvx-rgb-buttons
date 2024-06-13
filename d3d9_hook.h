//
// Created by pneuma on 5/27/2024.
//

#ifndef CHUNI_SCALER_D3D9_HOOK_H
#define CHUNI_SCALER_D3D9_HOOK_H

#include <d3d9.h>

void init_hook();
void remove_hook();

extern LPDIRECT3DTEXTURE9 jacket;

#endif //CHUNI_SCALER_D3D9_HOOK_H
