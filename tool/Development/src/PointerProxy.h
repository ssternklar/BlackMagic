#pragma once

template <typename T>
struct proxy_ctr
{
	T* ptr = nullptr;
	struct proxy_ptr
	{
		proxy_ptr() {}
		proxy_ptr(proxy_ctr<T>* c):ctr(c) {}
		T* ptr() { return ctr->ptr; }
		T* operator->() { return ctr->ptr; }
		T& operator *() { return *ctr->ptr; } // are additional operators actually a bad idea?
	private:
		proxy_ctr<T>* ctr;
	};
	proxy_ptr handle;
	proxy_ctr() { handle = proxy_ptr(this); }
};

template<typename T>
class ProxyVector
{
public:
	ProxyVector();
	~ProxyVector();

	typename proxy_ctr<T>::proxy_ptr track(T* ptr);
	void relinquish(T* ptr);
	void relinquish(T* ptr, size_t count);
	void move(T* from, T* to);
	void move(T* from, T* to, size_t count);

private:
	void expand();

	template<typename T>
	struct proxy_index
	{
		size_t line = 0;
		size_t index = 0;
		proxy_ctr<T>* proxy = nullptr;
	};
	proxy_index<T> find(T* ptr);

	const size_t lineWidth = 64 / sizeof(proxy_ctr<T>);
	size_t lineCount;
	proxy_ctr<T>** proxies;

	proxy_index<T> nextProxy;
};

template<typename T>
ProxyVector<T>::ProxyVector()
{
	proxies = nullptr;
	lineCount = 0;

	expand();

	nextProxy.proxy = proxies[0];
}

template<typename T>
ProxyVector<T>::~ProxyVector()
{
	for (size_t i = 0; i < lineCount; ++i)
		delete proxies[i];
	delete[] proxies;
}

template<typename T>
void ProxyVector<T>::expand()
{
	proxy_ctr<T>** newArr = new proxy_ctr<T>*[++lineCount];

	if (proxies)
	{
		memcpy_s(newArr, lineWidth, proxies, lineWidth - 1);
		delete[] proxies;
	}

	newArr[lineCount - 1] = new proxy_ctr<T>[lineWidth];
	proxies = newArr;
}

template<typename T>
typename ProxyVector<T>::proxy_index<T> ProxyVector<T>::find(T* ptr)
{
	proxy_index<T> index;

	for (size_t i = 0; i < lineCount; ++i)
		for (size_t j = 0; j < lineWidth; ++j)
			if (proxies[i][j].ptr == ptr)
			{
				index.line = i;
				index.index = j;
				index.proxy = &(proxies[i][j]);
				break;
			}

	return index;
}

template<typename T>
typename proxy_ctr<T>::proxy_ptr ProxyVector<T>::track(T* ptr)
{
	proxy_ctr<T>* proxy = nextProxy.proxy;
	proxy->ptr = ptr;

	do
	{
		++nextProxy.index;
		nextProxy.line += nextProxy.index == lineWidth;
		nextProxy.index %= lineWidth;

		if (nextProxy.line < lineCount)
			nextProxy.proxy = &(proxies[nextProxy.line][nextProxy.index]);
	} while (nextProxy.line < lineCount && nextProxy.proxy->ptr);

	return proxy->handle;
}

template<typename T>
void ProxyVector<T>::relinquish(T* ptr)
{
	proxy_index<T> proxy = find(ptr);

	if (proxy.proxy)
	{
		proxy.proxy->ptr = nullptr;

		if (proxy.line * lineWidth + proxy.index < nextProxy.line * lineWidth + nextProxy.index)
			nextProxy = proxy;
	}
}

template<typename T>
void ProxyVector<T>::relinquish(T* ptr, size_t count)
{
	proxy_index<T> proxy;

	for (size_t i = 0; i < count; ++i)
	{
		proxy = find(ptr + i);

		if (proxy.proxy)
		{
			proxy.proxy->ptr = nullptr;

			if (proxy.line * lineWidth + proxy.index < nextProxy.line * lineWidth + nextProxy.index)
				nextProxy = proxy;
		}
	}
}

template<typename T>
void ProxyVector<T>::move(T* from, T* to)
{
	proxy_index<T> proxy = find(from);

	if (proxy.proxy)
		proxy.proxy->ptr = to;
}

template<typename T>
void ProxyVector<T>::move(T* from, T* to, size_t count)
{
	proxy_index<T> proxy;

	for (size_t i = 0; i < count; ++i)
	{
		proxy = find(from + i);

		if (proxy.proxy)
			proxy.proxy->ptr = to + i;
	}
}