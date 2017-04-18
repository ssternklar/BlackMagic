#pragma once

#if defined(BM_PLATFORM_WINDOWS)
#include <d3d11.h>

using ShaderResource = ID3D11ShaderResourceView;
using RenderTarget = ID3D11RenderTargetView;
using BufferHandle = ID3D11Buffer;
using SamplerHandle = ID3D11SamplerState;
using ResourceHandle = ID3D11DeviceChild;
#endif