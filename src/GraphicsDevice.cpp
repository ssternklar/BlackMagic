#include "GraphicsDevice.h"
#include "Vertex.h"

using DirectX::XMFLOAT2;
using DirectX::XMFLOAT4;
using DirectX::XMFLOAT4X4;

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
		_depthStencilTexture->Release();
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

	if (_positionMap)
	{
		delete _positionMap;
	}

	if (_quad)
	{
		_quad->Release();
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
	FLOAT black[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	_context->ClearRenderTargetView(_backBuffer, reinterpret_cast<const FLOAT*>(&color));
	_context->ClearRenderTargetView(*_diffuseMap, reinterpret_cast<const FLOAT*>(&color));
	_context->ClearRenderTargetView(*_specularMap, black);
	_context->ClearRenderTargetView(*_normalMap, black);
	_context->ClearRenderTargetView(*_positionMap, black);
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

ID3D11Buffer* GraphicsDevice::CreateBuffer(const D3D11_BUFFER_DESC& desc, const D3D11_SUBRESOURCE_DATA& data)
{
	ID3D11Buffer* buf;
	_device->CreateBuffer(&desc, &data, &buf);
	return buf;
}

void GraphicsDevice::OnResize(UINT width, UINT height)
{
	_width = width;
	_height = height;
	
	// Release existing DirectX views and buffers
	if (_depthStencil)
	{
		_depthStencil->Release();
		_depthStencilTexture->Release();
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
	if (_positionMap)
	{
		delete _positionMap;
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

HRESULT GraphicsDevice::InitDx(HWND window, UINT width, UINT height)
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
	
	if (!FAILED(hr))
		_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	return hr;
}

void GraphicsDevice::Init(ContentManager* content)
{
	//Initialize the GBuffer and depth buffer
	InitBuffers();
	
	//Quad vertex buffer for the second pass
	XMFLOAT2 quad[6] = {
		{ -1, 1 },
		{ 1, 1 },
		{ 1, -1 },
		{ -1, 1 },
		{ 1, -1 },
		{ -1, -1 }
	};

	D3D11_BUFFER_DESC vbDesc = { 0 };
	vbDesc.ByteWidth = sizeof(quad);
	vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbDesc.Usage = D3D11_USAGE_IMMUTABLE;

	D3D11_SUBRESOURCE_DATA vbData = { 0 };
	vbData.pSysMem = quad;

	_device->CreateBuffer(&vbDesc, &vbData, &_quad);

	//Load lightpass shaders
	_lightPassVS = content->Load<VertexShader>(L"/shaders/LightPassVS.cso");
	_lightPassPS = content->Load<PixelShader>(L"/shaders/LightPassPS.cso");

	//Set up g-buffer sampler
	D3D11_SAMPLER_DESC sampDesc = {};
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	sampDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	sampDesc.MaxAnisotropy = D3D11_MAX_MAXANISOTROPY;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	_gBufferSampler = CreateSamplerState(sampDesc);
	
}

void GraphicsDevice::Render(const Camera& cam, const std::vector<ECS::Entity*>& objects, const std::vector<DirectionalLight>& lights)
{
	static UINT stride = sizeof(Vertex);
	static UINT quadStride = sizeof(XMFLOAT2);
	static UINT offset = 0;

	Material* currentMaterial = nullptr;

	//TODO: Sort renderables by material and texture to minimize state switches
	ID3D11RenderTargetView* rts[4] = { *_diffuseMap, *_specularMap, *_positionMap, *_normalMap };
	_context->OMSetRenderTargets(4, rts, _depthStencil);

	//Load object attributes into the g-buffer (geometry pass)
	for(auto* object : objects)
	{
		auto renderable = object->get<Renderable>();
		auto transform = object->get<Transform>();

		if (renderable->_material.get() != currentMaterial)
		{
			currentMaterial = renderable->_material.get();
			renderable->_material->Apply(cam.ViewMatrix(), cam.ProjectionMatrix());
		}
		
		//Update per-object constant buffer
		renderable->_material->VertexShader()->SetData("world", transform->Matrix(), sizeof(XMFLOAT4X4));
		
		//Upload buffers and draw
		renderable->_material->Upload();

		auto vBuf = renderable->_mesh->VertexBuffer();
		_context->IASetVertexBuffers(0, 1, &vBuf, &stride, &offset);
		_context->IASetIndexBuffer(renderable->_mesh->IndexBuffer(), DXGI_FORMAT_R32_UINT, 0);
		_context->DrawIndexed(static_cast<UINT>(renderable->_mesh->IndexCount()), 0, 0);
	}


	//Combine g-buffer and render to backbuffer
	_context->OMSetRenderTargets(1, &_backBuffer, nullptr);
	auto cPos = cam.Position();
	size_t padding = (16 - (sizeof(DirectionalLight) % 16))*(lights.size() - 1);

	_lightPassVS->SetShader();
	_lightPassPS->SetShader();
	_lightPassPS->SetFloat3("cameraPosition", cPos);
	_lightPassPS->SetData("directionalLights", &lights[0], sizeof(DirectionalLight)*lights.size() + padding);
	_lightPassPS->SetSamplerState("mainSampler", _gBufferSampler.get());
	_lightPassPS->SetShaderResourceView("diffuseMap", *_diffuseMap);
	_lightPassPS->SetShaderResourceView("specularMap", *_specularMap);
	_lightPassPS->SetShaderResourceView("positionMap", *_positionMap);
	_lightPassPS->SetShaderResourceView("normalMap", *_normalMap);
	_lightPassPS->CopyAllBufferData();

	_context->IASetVertexBuffers(0, 1, &_quad, &quadStride, &offset);
	_context->Draw(6, 0);

	//Can't have SRVs and RTVs that are pointing to the same texture bound at the same time, so unset them
	ID3D11ShaderResourceView* srvs[4] = { 0 };
	_context->PSSetShaderResources(0, 4, srvs);
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
	normalMapDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
	normalMapDesc.MipLevels = 1;
	normalMapDesc.MiscFlags = 0;
	normalMapDesc.SampleDesc.Count = 1;
	normalMapDesc.SampleDesc.Quality = 0;
	normalMapDesc.Usage = D3D11_USAGE_DEFAULT;
	normalMapDesc.Height = _height;
	normalMapDesc.Width = _width;

	D3D11_TEXTURE2D_DESC posMapDesc;
	posMapDesc.ArraySize = 1;
	posMapDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	posMapDesc.CPUAccessFlags = 0;
	posMapDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	posMapDesc.MipLevels = 1;
	posMapDesc.MiscFlags = 0;
	posMapDesc.SampleDesc.Count = 1;
	posMapDesc.SampleDesc.Quality = 0;
	posMapDesc.Usage = D3D11_USAGE_DEFAULT;
	posMapDesc.Height = _height;
	posMapDesc.Width = _width;


	D3D11_TEXTURE2D_DESC depthStencilDesc;
	depthStencilDesc.Width = _width;
	depthStencilDesc.Height = _height;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;

	D3D11_DEPTH_STENCIL_VIEW_DESC dsDesc;
	dsDesc.Flags = 0;
	dsDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsDesc.Texture2D.MipSlice = 0;

	D3D11_SHADER_RESOURCE_VIEW_DESC depthSRVDesc;
	depthSRVDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	depthSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	depthSRVDesc.Texture2D.MostDetailedMip = 0;
	depthSRVDesc.Texture2D.MipLevels = -1;

	_diffuseMap = createEmptyTexture(colorMapDesc);
	_specularMap = createEmptyTexture(colorMapDesc);
	_normalMap = createEmptyTexture(normalMapDesc);
	_positionMap = createEmptyTexture(posMapDesc);

	ID3D11Texture2D* depth;
	_device->CreateTexture2D(&depthStencilDesc, nullptr, &depth);
	_device->CreateDepthStencilView(depth, &dsDesc, &_depthStencil);
	_device->CreateShaderResourceView(depth, &depthSRVDesc, &_depthStencilTexture);
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

Texture* GraphicsDevice::createEmptyTexture(D3D11_TEXTURE2D_DESC& desc)
{
	ID3D11Texture2D* tex;
	ID3D11RenderTargetView* rtv;
	ID3D11ShaderResourceView* srv;

	_device->CreateTexture2D(&desc, nullptr, &tex);
	_device->CreateRenderTargetView(tex, nullptr, &rtv);
	_device->CreateShaderResourceView(tex, nullptr, &srv);

	return new Texture(tex, srv, rtv);
}


