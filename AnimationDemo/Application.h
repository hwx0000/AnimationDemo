#pragma once

class Application
{
public:
	virtual ~Application() {};

	virtual void Init() {};
	virtual void Update() {};
	virtual void Render() {};
	virtual void Shutdown() {};
};