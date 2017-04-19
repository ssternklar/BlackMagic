#pragma once

#if defined(BM_PLATFORM_WINDOWS)
#include <d3d11.h>
#endif

using ShaderResource = BM_PLATFORM_TEXTURE;
using RenderTarget = BM_PLATFORM_RENDERTARGET;
using BufferHandle = BM_PLATFORM_BUFFER;
using SamplerHandle = BM_PLATFORM_SAMPLER;
using ResourceHandle = BM_PLATFORM_RESOURCE;