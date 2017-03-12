#pragma once

template <class T>
class Singleton
{
public:
	static T& Instance()
	{
		static InstanceClass instance_object;
		return instance_object;
	}

protected:
	Singleton& operator=(const Singleton&) = delete;

private:
	virtual void SingletonHook() = 0;
	class InstanceClass : public T
	{
	private:
		void SingletonHook() {}
		InstanceClass& operator=(const InstanceClass&) = delete;
	};
};