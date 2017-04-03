#pragma once

#include <d3d11.h>

#if defined(_WIN32) || defined(_WIN64)
using ShaderResource = ID3D11ShaderResourceView;
using RenderTarget = ID3D11RenderTargetView;
using BufferHandle = ID3D11Buffer;
using SamplerHandle = ID3D11SamplerState;
using ResourceHandle = ID3D11DeviceChild;
#endif