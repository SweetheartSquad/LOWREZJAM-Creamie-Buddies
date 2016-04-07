#pragma once

#include <MY_Scene_Menu.h>
#include <EndScene.h>
#include <shader/ComponentShaderText.h>
#include <sweet/UI.h>
#include <Easing.h>
#include <RenderSurface.h>
#include <StandardFrameBuffer.h>
#include <RenderOptions.h>
#include <MeshFactory.h>

EndScene::EndScene(Game * _game, unsigned long int _cone, unsigned long int _face, unsigned long int _topping) :
	MY_Scene_Base(_game),
	screenSurfaceShader(new Shader("assets/RenderSurface_1", false, true)),
	screenSurface(new RenderSurface(screenSurfaceShader, true)),
	screenFBO(new StandardFrameBuffer(true)),
	ready(false),

	zoom(12.5),
	gameCamPolarCoords(0, zoom),
	orbitalSpeed(1),
	orbitalHeight(3),
	targetOrbitalHeight(orbitalHeight)
{
	
	
	bgLayer = new UILayer(0,0,0,0);
	NodeUI * bg = new NodeUI(bgLayer->world);
	bgLayer->addChild(bg);
	bg->setRationalHeight(1.f, bgLayer);
	bg->setRationalWidth(1.f, bgLayer);
	bg->background->mesh->replaceTextures(MY_ResourceManager::globalAssets->getTexture("bg")->texture);
	bg->background->mesh->setScaleMode(GL_NEAREST);

	bgLayer->resize(0, 64, 0, 64);
	uiLayer->resize(0, 64, 0, 64);


	// memory management
	screenSurface->incrementReferenceCount();
	screenSurfaceShader->incrementReferenceCount();
	screenFBO->incrementReferenceCount();
	
	screenSurface->setScaleMode(GL_NEAREST);
	screenSurface->uvEdgeMode = GL_CLAMP_TO_BORDER;



	wipe = new NodeUI(uiLayer->world);
	wipe->background->mesh->setScaleMode(GL_NEAREST);
	wipe->background->mesh->pushTexture2D(MY_ResourceManager::globalAssets->getTexture("waiting")->texture);
	uiLayer->addChild(wipe);
	wipe->setRationalHeight(1.f, uiLayer);
	wipe->setRationalWidth(1.f, uiLayer);
	wipe->marginRight.setRationalSize(0, &uiLayer->width);

	readyTimeout = new Timeout(2.f, [this](sweet::Event * _event){
		// go to next scene
		ready = true;
	});
	readyTimeout->eventManager->addEventListener("progress", [this](sweet::Event * _event){
		float p = _event->getFloatData("progress");
		p = glm::clamp(p - 0.5f, 0.f, 0.5f);
		
		wipe->marginRight.rationalSize = Easing::easeOutBounce(p, 0, 1, 0.5f);
	});
	childTransform->addChild(readyTimeout, false);
	readyTimeout->start();
	readyTimeout->name = "ready timeout";


	
	cone = new MeshEntity(MY_ResourceManager::globalAssets->getMesh("cone")->meshes.at(_cone), baseShader);
	face = new MeshEntity(MY_ResourceManager::globalAssets->getMesh("face")->meshes.at(_face), baseShader);
	topping = new MeshEntity(MY_ResourceManager::globalAssets->getMesh("topping")->meshes.at(_topping), baseShader);
	
	cone->mesh->setScaleMode(GL_NEAREST);
	face->mesh->setScaleMode(GL_NEAREST);
	topping->mesh->setScaleMode(GL_NEAREST);
	
	cone->mesh->replaceTextures(MY_ResourceManager::globalAssets->getTexture("icecream_"+std::to_string(_cone))->texture);
	face->mesh->replaceTextures(MY_ResourceManager::globalAssets->getTexture("icecream_"+std::to_string(_face))->texture);
	topping->mesh->replaceTextures(MY_ResourceManager::globalAssets->getTexture("icecream_"+std::to_string(_topping))->texture);

	childTransform->addChild(cone);
	cone->childTransform->addChild(face)->translate(0, cone->mesh->calcBoundingBox().height + cone->mesh->calcBoundingBox().y, 0);
	face->childTransform->addChild(topping)->translate(0, face->mesh->calcBoundingBox().height + face->mesh->calcBoundingBox().y, 0);
	

	orbitalCam = new PerspectiveCamera();
	childTransform->addChild(orbitalCam);
	cameras.push_back(orbitalCam);
	activeCamera = orbitalCam;


	sweet::setCursorMode(GLFW_CURSOR_NORMAL);
}

EndScene::~EndScene(){
	// memory management
	screenSurface->decrementAndDelete();
	screenSurfaceShader->decrementAndDelete();
	screenFBO->decrementAndDelete();

	delete bgLayer;
}

void EndScene::update(Step * _step){
	// Screen shader update
	// Screen shaders are typically loaded from a file instead of built using components, so to update their uniforms
	// we need to use the OpenGL API calls
	screenSurfaceShader->bindShader(); // remember that we have to bind the shader before it can be updated
	GLint test = glGetUniformLocation(screenSurfaceShader->getProgramId(), "time");
	checkForGlError(0);
	if(test != -1){
		glUniform1f(test, _step->time);
		checkForGlError(0);
	}


	if(keyboard->keyJustDown(GLFW_KEY_L)){
		screenSurfaceShader->unload();
		screenSurfaceShader->loadFromFile(screenSurfaceShader->vertSource, screenSurfaceShader->fragSource);
		screenSurfaceShader->load();
	}




	if(ready){
		if(/*mouse->leftJustPressed() || */keyboard->keyJustDown(GLFW_KEY_ESCAPE)){
			game->switchScene("menu", true);
		}
	}
	


	// camera movement
	zoom -= mouse->getMouseWheelDelta();
	zoom = glm::clamp(zoom, 10.f, 20.f);
	gameCamPolarCoords.y += (zoom - gameCamPolarCoords.y) * 0.05f;
	gameCamPolarCoords.y = glm::clamp(gameCamPolarCoords.y, 10.f, 20.f);

	if(mouse->leftDown()){
		if(!mouse->leftJustPressed()){
			orbitalSpeed += ((mouse->mouseX(false) - mouseX) - orbitalSpeed) * 0.1f;
			targetOrbitalHeight -= ((mouse->mouseY(false) - mouseY)) * 0.025f;
		}else{
			sweet::setCursorMode(GLFW_CURSOR_DISABLED);
		}
		mouseX = mouse->mouseX(false);
		mouseY = mouse->mouseY(false);
	}

	if(mouse->leftJustReleased()){
		sweet::setCursorMode(GLFW_CURSOR_NORMAL);
	}
	
	targetOrbitalHeight = glm::clamp(targetOrbitalHeight, 1.f, 10.f);
	orbitalSpeed = glm::clamp(orbitalSpeed, -64.f, 64.f);

	orbitalHeight += (targetOrbitalHeight - orbitalHeight) * 0.1f;

	gameCamPolarCoords.x += _step->deltaTimeCorrection * 0.005f * orbitalSpeed;

	orbitalCam->firstParent()->translate(glm::vec3(glm::sin(gameCamPolarCoords.x) * gameCamPolarCoords.y, orbitalHeight, glm::cos(gameCamPolarCoords.x) * gameCamPolarCoords.y), false);



	
	bgLayer->resize(0, 64, 0, 64);
	uiLayer->resize(0, 64, 0, 64);
	MY_Scene_Base::update(_step);
	bgLayer->update(_step);
	uiLayer->update(_step);


	orbitalCam->lookAtSpot = face->childTransform->getWorldPos();
	orbitalCam->forwardVectorRotated = orbitalCam->lookAtSpot - orbitalCam->getWorldPos();
	orbitalCam->rightVectorRotated = glm::cross(orbitalCam->forwardVectorRotated, glm::vec3(0, 1, 0));
}


void EndScene::render(sweet::MatrixStack * _matrixStack, RenderOptions * _renderOptions){
	glm::uvec2 sd = sweet::getWindowDimensions();
	int max = glm::max(sd.x, sd.y);
	int min = glm::min(sd.x, sd.y);
	bool horz = sd.x == max;
	int offset = (max - min)/2;

	// keep our screen framebuffer up-to-date with the current viewport
	screenFBO->resize(64, 64);
	_renderOptions->setViewPort(0,0,64,64);
	_renderOptions->setClearColour(1,0,0,0);

	// bind our screen framebuffer
	FrameBufferInterface::pushFbo(screenFBO);
	// render the scene
	_renderOptions->clear();
	bgLayer->render(_matrixStack, _renderOptions);
	Scene::render(_matrixStack, _renderOptions);
	uiLayer->render(_matrixStack, _renderOptions);
	// unbind our screen framebuffer, rebinding the previously bound framebuffer
	// since we didn't have one bound before, this will be the default framebuffer (i.e. the one visible to the player)
	FrameBufferInterface::popFbo();

	// render our screen framebuffer using the standard render surface
	_renderOptions->setViewPort(horz ? offset : 0, horz ? 0 : offset, min, min);
	screenSurface->render(screenFBO->getTextureId());
}

void EndScene::load(){
	MY_Scene_Base::load();	

	screenSurface->load();
	screenFBO->load();
}

void EndScene::unload(){
	screenFBO->unload();
	screenSurface->unload();

	MY_Scene_Base::unload();	
}