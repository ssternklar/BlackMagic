#include "GraphicsDevice.h"

GraphicsDevice::GraphicsDevice()
{}

GraphicsDevice::~GraphicsDevice()
{
	if (_device)
	{
		_device->Release();
	}

	if (_context)
	{
		_context->Release();
	}

	if (_swapChain)
	{
		_swapChain->Release();
	}

	if (_backBuffer)
	{
		_backBuffer->Release();
	}

	if (_depthStencil)
	{
		_depthStencil->Release();
	}
}


ID3D11Device* GraphicsDevice::Device() const
{
	return _device;
}

ID3D11DeviceContext* GraphicsDevice::Context() const
{
	return _context;
}

D3D_FEATURE_LEVEL GraphicsDevice::FeatureLevel() const
{
	return _featureLevel;
}

void GraphicsDevice::Clear(XMFLOAT4 color)
{
	_context->ClearRenderTargetView(_backBuffer, reinterpret_cast<const FLOAT*>(&color));
	_context->ClearDepthStencilView(_depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

std::shared_ptr<ID3D11SamplerState> GraphicsDevice::CreateSamplerState(D3D11_SAMPLER_DESC& desc)
{
	ID3D11SamplerState* tempSampler;
	_device->CreateSamplerState(&desc, &tempSampler);
	std::shared_ptr<ID3D11SamplerState> sampler{ tempSampler, [=](ID3D11SamplerState* sampler) {
		sampler->Release();
	} };

	return sampler;
}

void GraphicsDevice::OnResize(UINT width, UINT height)
{
	_width = width;
	_height = height;
	
	// Release existing DirectX views and buffers
	if (_depthStencil)
	{
		_depthStencil->Release();
	}
	if (_backBuffer)
	{
		_backBuffer->Release();
	}
	if (_diffuseMap)
	{
		delete _diffuseMap;
	}
	if (_specularMap)
	{
		delete _specularMap;
	}
	if (_normalMap)
	{
		delete _normalMap;
	}

	// Resize the underlying swap chain buffers
	_swapChain->ResizeBuffers(
		1,
		width,
		height,
		DXGI_FORMAT_R8G8B8A8_UNORM,
		0);

	InitBuffers();
}

void GraphicsDevice::Present(UINT interval, UINT flags)
{
	_swapChain->Present(interval, flags);
}


HRESULT GraphicsDevice::Init(HWND window, UINT width, UINT height)
{
	unsigned int deviceFlags = 0;
	_width = width;
	_height = height;

#if defined(DEBUG) || defined(_DEBUG)
	deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	DXGI_SWAP_CHAIN_DESC swapDesc = {};
	swapDesc.BufferCount = 1;
	swapDesc.BufferDesc.Width = width;
	swapDesc.BufferDesc.Height = height;
	swapDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapDesc.Flags = 0;
	swapDesc.OutputWindow = window;
	swapDesc.SampleDesc.Count = 1;
	swapDesc.SampleDesc.Quality = 0;
	swapDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapDesc.Windowed = true;

	// Result variable for below function calls
	HRESULT hr = S_OK;

	// Attempt to initialize DirectX
	hr = D3D11CreateDeviceAndSwapChain(
		0, // Video adapter (physical GPU) to use, or null for default
		D3D_DRIVER_TYPE_HARDWARE, // We want to use the hardware (GPU)
		0, // Used when doing software rendering
		deviceFlags, // Any special options
		0, // Optional array of possible verisons we want as fallbacks
		0, // The number of fallbacks in the above param
		D3D11_SDK_VERSION, // Current version of the SDK
		&swapDesc, // Address of swap chain options
		&_swapChain, // Pointer to our Swap Chain pointer
		&_device, // Pointer to our Device pointer
		&_featureLevel, // This will hold the actual feature level the app will use
		&_context); // Pointer to our Device Context pointer
	if (FAILED(hr))
		return hr;

	InitBuffers();

	_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	return S_OK;
}

void GraphicsDevice::Render(const Camera& cam, const std::vector<Renderable*>& objects, const std::vector<DirectionalLight> lights)
{
	static UINT stride = sizeof(Vertex);
	static UINT offset = 0;

	Material* currentMaterial = nullptr;

	//TODO: Sort renderables by material and texture to minimize state switches
	for(auto* object : objects)
	{
		if (object->_material.get() != currentMaterial)
		{
			currentMaterial = object->_material.get();
			object->_material->UpdateLights(lights);
		}
		
		auto cPos = cam.Position();
		object->_material->PixelShader()->SetData("cameraPos", &cPos, sizeof(XMFLOAT3));
		object->_material->PixelShader()->SetData("useNormalMap", &UseNormalMap, sizeof(unsigned int));
		object->PrepareMaterial(cam.ViewMatrix(), cam.ProjectionMatrix());
		//Upload buffers and draw
		
		auto vBuf = object->_mesh->VertexBuffer();
		_context->IASetVertexBuffers(0, 1, &vBuf, &stride, &offset);
		_context->IASetIndexBuffer(object->_mesh->IndexBuffer(), DXGI_FORMAT_R32_UINT, 0);
		_context->DrawIndexed(static_cast<UINT>(object->_mesh->IndexCount()), 0, 0);
	}
}

void GraphicsDevice::InitBuffers()
{
	// The above function created the back buffer render target
	// for us, but we need a reference to it
	ID3D11Texture2D* backBufferTexture;
	_swapChain->GetBuffer(
		0,
		__uuidof(ID3D11Texture2D),
		(void**)&backBufferTexture);

	// Now that we have the texture, create a render target view
	// for the back buffer so we can render into it.  Then release
	// our local reference to the texture, since we have the view.
	_device->CreateRenderTargetView(
		backBufferTexture,
		0,
		&_backBuffer);
	backBufferTexture->Release();

	D3D11_TEXTURE2D_DESC colorMapDesc;
	colorMapDesc.ArraySize = 1;
	colorMapDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	colorMapDesc.CPUAccessFlags = 0;
	colorMapDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	colorMapDesc.MipLevels = 1;
	colorMapDesc.MiscFlags = 0;
	colorMapDesc.SampleDesc.Count = 1;
	colorMapDesc.SampleDesc.Quality = 0;
	colorMapDesc.Usage = D3D11_USAGE_DEFAULT;
	colorMapDesc.Height = _height;
	colorMapDesc.Width = _width;

	D3D11_TEXTURE2D_DESC normalMapDesc;
	normalMapDesc.ArraySize = 1;
	normalMapDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	normalMapDesc.CPUAccessFlags = 0;
	normalMapDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	normalMapDesc.MipLevels = 1;
	normalMapDesc.MiscFlags = 0;
	normalMapDesc.SampleDesc.Count = 1;
	normalMapDesc.SampleDesc.Quality = 0;
	normalMapDesc.Usage = D3D11_USAGE_DEFAULT;
	normalMapDesc.Height = _height;
	normalMapDesc.Width = _width;

	// Set up the description of the texture to use for the depth buffer
	D3D11_TEXTURE2D_DESC depthStencilDesc;
	depthStencilDesc.Width = _width;
	depthStencilDesc.Height = _height;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;

	ID3D11Texture2D *diff, *spec, *nrm, *depth;
	
	_device->CreateTexture2D(&colorMapDesc, nullptr, &diff);
	_device->CreateTexture2D(&colorMapDesc, nullptr, &spec);
	_device->CreateTexture2D(&normalMapDesc, nullptr, &nrm);
	_device->CreateTexture2D(&depthStencilDesc, nullptr, &depth);
	_device->CreateDepthStencilView(depth, nullptr, &_depthStencil);
	depth->Release();

	// Bind the views to the pipeline, so rendering properly 
	// uses their underlying textures
	_context->OMSetRenderTargets(1, &_backBuffer, _depthStencil);

	// Lastly, set up a viewport so we render into
	// to correct portion of the window
	D3D11_VIEWPORT viewport = {};
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = static_cast<float>(_width);
	viewport.Height = static_cast<float>(_height);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	_context->RSSetViewports(1, &viewport);

}


