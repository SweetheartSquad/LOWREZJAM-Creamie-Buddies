#pragma once

#include <MY_Scene_Base.h>

class RenderSurface;
class StandardFrameBuffer;
class Slot;

class MY_Scene_Main : public MY_Scene_Base{
public:
	Shader * screenSurfaceShader;
	RenderSurface * screenSurface;
	StandardFrameBuffer * screenFBO;

	std::vector<Slot *> slots;
	MeshEntity * lever;

	float leverY;
	float targetLever;
	float leverAngle;

	Timeout * spinTimeout;

	bool spinning;


	virtual void update(Step * _step) override;
	virtual void render(sweet::MatrixStack * _matrixStack, RenderOptions * _renderOptions) override;

	virtual void unload() override;
	virtual void load() override;

	MY_Scene_Main(Game * _game);
	~MY_Scene_Main();
};