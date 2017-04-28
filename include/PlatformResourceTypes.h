#pragma once
#include <Windows.h>
#include <d3d11.h>
using ShaderResource = ID3D11ShaderResourceView*;
using RenderTarget = ID3D11RenderTargetView*;
using BufferHandle = ID3D11Buffer*;
using SamplerHandle = ID3D11SamplerState*;
using ResourceHandle = ID3D11DeviceChild*;
using GraphicsContext = ID3D11DeviceContext*;

#define BM_PLATFORM_ATOMIC_ADD(X, Y) InterlockedAdd(X, Y)
#define BM_PLATFORM_ATOMIC_FETCH *
using AtomicInt = volatile signed long;