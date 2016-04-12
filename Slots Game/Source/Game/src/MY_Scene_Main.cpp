#pragma once

#include <MY_Scene_Main.h>
#include <EndScene.h>
#include <RenderSurface.h>
#include <StandardFrameBuffer.h>
#include <RenderOptions.h>


#include <shader\ShaderComponentTexture.h>
#include <shader\ShaderComponentDiffuse.h>
#include <shader\ShaderComponentMVP.h>
#include <shader\ShaderComponentUvOffset.h>
#include <shader\ShaderComponentHsv.h>

#include <Easing.h>
#include <NumberUtils.h>
#include <PointLight.h>

#include <MY_Game.h>
#include <Slot.h>

MY_Scene_Main::MY_Scene_Main(Game * _game) :
	MY_Scene_Base(_game),
	screenSurfaceShader(new Shader("assets/RenderSurface_1", false, true)),
	screenSurface(new RenderSurface(screenSurfaceShader, true)),
	screenFBO(new StandardFrameBuffer(true)),
	leverY(0),
	leverAngle(0),
	targetLever(0),
	spinning(false),
	shake(0)
{
	// memory management
	screenSurface->incrementReferenceCount();
	screenSurfaceShader->incrementReferenceCount();
	screenFBO->incrementReferenceCount();
	
	screenSurface->setScaleMode(GL_NEAREST);
	screenSurface->uvEdgeMode = GL_CLAMP_TO_BORDER;

	// GAME

	for(unsigned long int i = 1; i <= 3; ++i){
		Slot * slot = new Slot(baseShader, i);
		childTransform->addChild(slot)->translate(3, i*1.7+0.1, -3);
		slots.push_back(slot);
	}

	MeshEntity * casing = new MeshEntity(MY_ResourceManager::globalAssets->getMesh("casing")->meshes.at(0), baseShader);
	casing->mesh->setScaleMode(GL_NEAREST);
	casing->mesh->pushTexture2D(MY_ResourceManager::globalAssets->getTexture("machine")->texture);
	childTransform->addChild(casing);


	lever = new MeshEntity(MY_ResourceManager::globalAssets->getMesh("lever")->meshes.at(0), baseShader);
	lever->mesh->setScaleMode(GL_NEAREST);
	lever->mesh->pushTexture2D(MY_ResourceManager::globalAssets->getTexture("machine")->texture);
	childTransform->addChild(lever)->translate(6.056, 3.698, -2.928);
	
	

	wipe = new NodeUI(uiLayer->world);
	wipe->background->mesh->setScaleMode(GL_NEAREST);
	wipe->background->mesh->pushTexture2D(MY_ResourceManager::globalAssets->getTexture("waiting")->texture);
	uiLayer->addChild(wipe);
	wipe->setRationalHeight(1.f, uiLayer);
	wipe->setRationalWidth(1.f, uiLayer);
	wipe->setMarginLeft(1.f);

	doneTimeout = new Timeout(2.f, [this](sweet::Event * _event){
		// go to next scene
		game->scenes["end"] = new EndScene(game, slots.at(0)->selection, slots.at(1)->selection, slots.at(2)->selection);
		game->switchScene("end", true);
	});
	doneTimeout->eventManager->addEventListener("progress", [this](sweet::Event * _event){
		float p = _event->getFloatData("progress");
		p = glm::clamp(p - 0.5f, 0.f, 0.5f);
		wipe->marginLeft.rationalSize = Easing::easeOutCirc(p, 1, -1, 0.5f);
		wipe->width.rationalSize = Easing::easeOutCirc(p, 2, -1, 0.5f);
	});
	childTransform->addChild(doneTimeout, false);

	spinTimeout = new Timeout(2.f, [this](sweet::Event * _event){
		leverAngle = 0;
		doneTimeout->start();
		shake = 0;
	});

	spinTimeout->eventManager->addEventListener("start", [this](sweet::Event * _event){	
		spinning = true;
		for(unsigned long int i = 0; i < slots.size(); ++i){
			Slot * s = slots.at(i);
			s->spinTimeout->targetSeconds = spinTimeout->targetSeconds * ((float)(i+1)/slots.size() * 0.5f + 0.25f);
			s->spinTimeout->restart();
		}
		MY_ResourceManager::globalAssets->getAudio("spin")->sound->play();
	});

	spinTimeout->eventManager->addEventListener("progress", [this](sweet::Event * _event){
		float p = _event->getFloatData("progress");
		leverAngle = Easing::easeOutBounce(p, 180, -180, 1);
		shake = Easing::easeOutCubic(p, 1, -1, 0.5);
		if(shake < 0.001f){
			shake = 0;
		}
	});
	childTransform->addChild(spinTimeout, false);




	
	debugCam->controller->alignMouse();
	// UI
	debugCam->controller->rotationEnabled = false;
	debugCam->firstParent()->translate(3.6, -1.3, 27);
	debugCam->fieldOfView = 15;
	debugCam->pitch = 10;
	debugCam->yaw = 95;
	debugCam->interpolation = 1;
	debugCam->setOrientation(debugCam->calcOrientation());
	debugCam->rotateVectors(debugCam->childTransform->getOrientationQuat());
	debugCam->firstParent()->translate(debugCam->rightVectorRotated*2.5f);
}

MY_Scene_Main::~MY_Scene_Main(){
	// memory management
	screenSurface->decrementAndDelete();
	screenSurfaceShader->decrementAndDelete();
	screenFBO->decrementAndDelete();
}


void MY_Scene_Main::update(Step * _step){
	// Screen shader update
	// Screen shaders are typically loaded from a file instead of built using components, so to update their uniforms
	// we need to use the OpenGL API calls
	screenSurfaceShader->bindShader(); // remember that we have to bind the shader before it can be updated
	GLint test = glGetUniformLocation(screenSurfaceShader->getProgramId(), "time");
	checkForGlError(0);
	if(test != -1){
		glUniform1f(test, _step->time);
		checkForGlError(0);
	}test = glGetUniformLocation(screenSurfaceShader->getProgramId(), "mag");
	checkForGlError(0);
	if(test != -1){
		glUniform1f(test, shake);
		checkForGlError(0);
	}


	if(keyboard->keyJustDown(GLFW_KEY_L)){
		screenSurfaceShader->unload();
		screenSurfaceShader->loadFromFile(screenSurfaceShader->vertSource, screenSurfaceShader->fragSource);
		screenSurfaceShader->load();
	}




	// GAME
	if(!spinning){
		if(mouse->leftJustPressed()){
			// start lever
			leverY = mouse->mouseY();
			sweet::setCursorMode(GLFW_CURSOR_DISABLED);
		}else if(mouse->leftDown()){
			// pull lever
			targetLever = (mouse->mouseY(false) - leverY) / sweet::getWindowHeight();

			if(targetLever < -0.9f){
				spinTimeout->restart();
			}

		}else if(mouse->leftJustReleased()){
			// release lever
			targetLever = 0;
		}


		targetLever = glm::clamp(targetLever, -1.f, 0.f);

		leverAngle += (-targetLever*180.f - leverAngle) * 0.25f;
	}
	lever->childTransform->setOrientation(glm::angleAxis(leverAngle, glm::vec3(1,0,0)));
	if(mouse->leftJustReleased()){
		sweet::setCursorMode(GLFW_CURSOR_NORMAL);
	}
	
	// Scene update
	uiLayer->resize(0, 64, 0, 64);
	MY_Scene_Base::update(_step);
}

void MY_Scene_Main::render(sweet::MatrixStack * _matrixStack, RenderOptions * _renderOptions){
	glm::uvec2 sd = sweet::getWindowDimensions();
	int max = glm::max(sd.x, sd.y);
	int min = glm::min(sd.x, sd.y);
	bool horz = sd.x == max;
	int offset = (max - min)/2;

	// keep our screen framebuffer up-to-date with the current viewport
	screenFBO->resize(64, 64);
	_renderOptions->setViewPort(0,0,64,64);

	

	float f = (glm::sin(sweet::lastTimestamp*0.5f) + 1) * 0.5f;

	float r = 168/255.f;
	float g = 217/255.f;
	float b = 223/255.f;
	
	r += (223/255.f - r) * f;
	g += (168/255.f - g) * f;
	b += (216/255.f - b) * f;

	_renderOptions->setClearColour(1,r,g,b);

	// bind our screen framebuffer
	FrameBufferInterface::pushFbo(screenFBO);
	// render the scene
	MY_Scene_Base::render(_matrixStack, _renderOptions);
	// unbind our screen framebuffer, rebinding the previously bound framebuffer
	// since we didn't have one bound before, this will be the default framebuffer (i.e. the one visible to the player)
	FrameBufferInterface::popFbo();

	// render our screen framebuffer using the standard render surface
	_renderOptions->setViewPort(horz ? offset : 0, horz ? 0 : offset, min, min);
	screenSurface->render(screenFBO->getTextureId());
}

void MY_Scene_Main::load(){
	MY_Scene_Base::load();	

	screenSurface->load();
	screenFBO->load();
}

void MY_Scene_Main::unload(){
	screenFBO->unload();
	screenSurface->unload();

	MY_Scene_Base::unload();	
}