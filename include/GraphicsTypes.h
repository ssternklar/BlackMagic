#pragma once

#define _BM_GRAPHICS_TYPE_INFO(X)\
void* buffer; \
X##() : buffer(nullptr) {} \
X##(void* ptr) : buffer(ptr) {} \
\
template<typename T> \
T GetAs(){return static_cast<T>(buffer);}

namespace BlackMagic
{

	struct GraphicsBuffer
	{
		enum BufferType
		{
			VERTEX_BUFFER,
			INDEX_BUFFER
		};
		_BM_GRAPHICS_TYPE_INFO(GraphicsBuffer)
	};

	struct GraphicsShader
	{
		enum ShaderType
		{
			VERTEX_SHADER,
			PIXEL_SHADER,
			COMPUTE_SHADER,
			COMBINED_SHADER
		};
		_BM_GRAPHICS_TYPE_INFO(GraphicsShader)
	};

	struct GraphicsTexture
	{
		_BM_GRAPHICS_TYPE_INFO(GraphicsTexture)
	};

	struct GraphicsRenderTarget
	{
		_BM_GRAPHICS_TYPE_INFO(GraphicsRenderTarget)
	};
}

#undef _BM_GRAPHICS_TYPE_INFO(X)