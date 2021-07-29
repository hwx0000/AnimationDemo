#pragma once

class Application
{
public:
	virtual ~Application() {};

	virtual void Init() = 0;
	virtual void Update() = 0;
	virtual void Render() = 0;
	virtual void Shutdown() = 0;
};