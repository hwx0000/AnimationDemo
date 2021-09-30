#pragma once

class Application
{
public:
	virtual ~Application() {};

	virtual void Init() {};
	virtual void Update(float inDeltaTime) {};
	virtual void Render(float inAspectRatio) {};
	virtual void Shutdown() {};
};