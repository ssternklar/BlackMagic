#pragma once
#ifdef BM_PLATFORM_WINDOWS
#   ifdef DXSHADER
#       define SV_POSITION SV_POSITION
#       define POSITION POSITION
#       define NORMAL NORMAL
#       define BINORMAL BINORMAL
#       define TANGENT TANGENT
#       define TEXCOORD TEXCOORD

#       define TEXTURE(slot) : register(t##slot);
#       define SAMPLER(slot) : register(s##slot);
#       define FLOAT(n) float##n
#       define MAT(y,x) matrix<float, y, x>
#       define MAT(n) MAT(n,n)
#       define RENDERTARGET(n) : SV_TARGET##n##;
#       define SEMANTIC(s) : s;
#   else
#       define SV_POSITION 
#       define POSITION 
#       define NORMAL 
#       define BINORMAL 
#       define TANGENT 
#       define TEXCOORD

#       define TEXTURE(slot)
#       define SAMPLER(slot)
#       define FLOAT(n) DirectX::XMFLOAT##n
#       define MAT(y,x) DirectX::XMFLOAT##y##X##x
#       define MAT(n) MAT(n,n) 
#       define RENDERTARGET(n)
#       define SEMANTIC(s)
#   endif
#endif