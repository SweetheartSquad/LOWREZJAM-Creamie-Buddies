#pragma once

#include <MY_Scene_Base.h>

class RenderSurface;
class StandardFrameBuffer;

class EndScene : public MY_Scene_Base{
public:
	Shader * screenSurfaceShader;
	RenderSurface * screenSurface;
	StandardFrameBuffer * screenFBO;

	bool ready;
	static bool explored;
	Timeout * readyTimeout;

	NodeUI * wipe, * bg;
	UILayer * bgLayer;
	MeshEntity * cone, * face, * topping;
	
	// target zoom level
	float zoom;
	glm::vec2 gameCamPolarCoords;
	float orbitalSpeed;
	float orbitalHeight, targetOrbitalHeight;
	float mouseX;
	float mouseY;

	Camera * orbitalCam;

	EndScene(Game * _game, unsigned long int _cone, unsigned long int _face, unsigned long int _topping);
	~EndScene();

	virtual void update(Step * _step) override;
	virtual void render(sweet::MatrixStack * _matrixStack, RenderOptions * _renderOptions) override;

	virtual void unload() override;
	virtual void load() override;
};