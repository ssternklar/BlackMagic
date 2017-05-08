#pragma once
#ifdef DXSHADER
    #define SV_POSITION SV_POSITION
    #define POSITION POSITION
    #define NORMAL NORMAL
    #define BINORMAL BINORMAL
    #define TANGENT TANGENT
    #define TEXCOORD TEXCOORD

    #define REGISTER(slot) : register(slot)
    #define FLOAT2 float2
    #define FLOAT3 float3
    #define FLOAT4 float4
    #define MAT(y,x) matrix<float, y, x>
    #define RENDERTARGET(n) : SV_TARGET##n
    #define SEMANTIC(s) : s
#endif