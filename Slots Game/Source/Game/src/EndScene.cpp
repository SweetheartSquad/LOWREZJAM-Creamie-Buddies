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

bool EndScene::explored = false;

EndScene::EndScene(Game * _game, unsigned long int _cone, unsigned long int _face, unsigned long int _topping) :
	MY_Scene_Base(_game),
	screenSurfaceShader(new Shader("assets/RenderSurface_1", false, true)),
	screenSurface(new RenderSurface(screenSurfaceShader, true)),
	screenFBO(new StandardFrameBuffer(true)),
	ready(false),
	clickingUI(false),

	zoom(5.f),
	gameCamPolarCoords(0, zoom),
	orbitalSpeed(0),
	orbitalHeight(3),
	targetOrbitalHeight(orbitalHeight)
{
	
	bgLayer = new UILayer(0,0,0,0);
	
	tileBg = new NodeUI(bgLayer->world);
	bgLayer->addChild(tileBg);
	tileBg->setRationalHeight(1.f, bgLayer);
	tileBg->setRationalWidth(1.f, bgLayer);
	tileBg->background->mesh->replaceTextures(MY_ResourceManager::globalAssets->getTexture("tilebg")->texture);
	tileBg->background->mesh->setScaleMode(GL_NEAREST);

	bg = new NodeUI(bgLayer->world);
	bgLayer->addChild(bg);
	bg->setRationalHeight(1.f, bgLayer);
	bg->setRationalWidth(1.f, bgLayer);
	bg->background->mesh->replaceTextures(MY_ResourceManager::globalAssets->getTexture("bg_1")->texture);
	bg->background->mesh->setScaleMode(GL_NEAREST);

	bgLayer->resize(0, 64, 0, 64);
	uiLayer->resize(0, 64, 0, 64);


	// memory management
	screenSurface->incrementReferenceCount();
	screenSurfaceShader->incrementReferenceCount();
	screenFBO->incrementReferenceCount();
	
	screenSurface->setScaleMode(GL_NEAREST);
	screenSurface->uvEdgeMode = GL_CLAMP_TO_BORDER;
	
	controls = new NodeUI(uiLayer->world);
	controls->background->setVisible(false);
	controls->setRationalHeight(1.f, uiLayer);
	controls->setRationalWidth(1.f, uiLayer);
	controls->boxSizing = kCONTENT_BOX;
	controls->marginBottom.setRationalSize(0.f, &uiLayer->height);
	uiLayer->addChild(controls);

	NodeUI * redoButt = new NodeUI(uiLayer->world);
	redoButt->background->mesh->setScaleMode(GL_NEAREST);
	redoButt->background->mesh->pushTexture2D(MY_ResourceManager::globalAssets->getTexture("redoButt")->texture);
	redoButt->setRationalHeight(10/64.f, controls);
	redoButt->setRationalWidth(10/64.f, controls);
	controls->addChild(redoButt);
	redoButt->boxSizing = kCONTENT_BOX;
	redoButt->setMarginLeft(1/64.f);
	redoButt->setMarginBottom(1/64.f);
	redoButt->setMouseEnabled(true);

	NodeUI * picButt = new NodeUI(uiLayer->world);
	picButt->background->mesh->setScaleMode(GL_NEAREST);
	picButt->background->mesh->pushTexture2D(MY_ResourceManager::globalAssets->getTexture("picButt")->texture);
	picButt->setRationalHeight(10/64.f, controls);
	picButt->setRationalWidth(10/64.f, controls);
	controls->addChild(picButt);
	picButt->boxSizing = kCONTENT_BOX;
	picButt->setMarginLeft(12/64.f);
	picButt->setMarginBottom(1/64.f);
	picButt->setMouseEnabled(true);


	wipe = new NodeUI(uiLayer->world);
	wipe->background->mesh->setScaleMode(GL_NEAREST);
	wipe->background->mesh->pushTexture2D(MY_ResourceManager::globalAssets->getTexture("waiting")->texture);
	uiLayer->addChild(wipe);
	wipe->setRationalHeight(1.f, uiLayer);
	wipe->setRationalWidth(1.f, uiLayer);
	wipe->marginLeft.setRationalSize(0, &uiLayer->width);

	
	
	
	NodeUI * fade = new NodeUI(uiLayer->world);
	fade->setRationalHeight(1.f, uiLayer);
	fade->setRationalWidth(1.f, uiLayer);
	fade->setVisible(false);
	uiLayer->addChild(fade);

	Timeout * zoomTimeout = new Timeout(1.5f, [this](sweet::Event * _event){
		orbitalSpeed = -6;
	});
	zoomTimeout->eventManager->addEventListener("progress", [this](sweet::Event * _event){
		zoom = Easing::easeInOutCubic(_event->getFloatData("progress"), 5, 15, 1.f);
		gameCamPolarCoords.x += glm::sin(_event->getFloatData("progress")*glm::pi<float>()) * -10/60.f;
		orbitalSpeed = Easing::easeInOutCubic(_event->getFloatData("progress"), 0, -6, 1.f);
	});
	zoomTimeout->eventManager->addEventListener("start", [this](sweet::Event * _event){
		MY_ResourceManager::globalAssets->getAudio("tada")->sound->play();
	});
	childTransform->addChild(zoomTimeout, false);

	Timeout * fadeTimeout = new Timeout(0.5f, [fade](sweet::Event * _event){
		fade->setVisible(false);
		fade->setBackgroundColour(1,1,1,0);

		// open an explorer window pointed to the screenshots folder
		if(!explored){
			explored = true;
				
			TCHAR buffer[MAX_PATH];
			DWORD dwRet = GetCurrentDirectory(MAX_PATH, buffer);

			std::wstringstream ss;
			ss << buffer << L"\\data\\screenshots";

			ShellExecute(HWND(nullptr), L"explore", ss.str().c_str(), L"", L"", SW_SHOWDEFAULT);
		}
	});
	fadeTimeout->eventManager->addEventListener("progress", [fade](sweet::Event * _event){
		fade->setBackgroundColour(1,1,1,1.f - _event->getFloatData("progress"));
	});
	fadeTimeout->eventManager->addEventListener("start", [fade](sweet::Event * _event){
		fade->setVisible(true);
		fade->setBackgroundColour(1,1,1,1);
	});
	childTransform->addChild(fadeTimeout);
	
	picButt->eventManager->addEventListener("click", [this, fadeTimeout](sweet::Event * _event){
		if(ready){
			MY_ResourceManager::globalAssets->getAudio("flash")->sound->play();
			uiLayer->setVisible(false);
			game->update(&sweet::step);
			game->draw(this);
			glfwSwapBuffers(sweet::currentContext);
			game->takeScreenshot();
			uiLayer->setVisible(true);
			fadeTimeout->restart();
		}
	});
	picButt->eventManager->addEventListener("mousedown", [this](sweet::Event * _event){
		clickingUI = true;
	});
	picButt->eventManager->addEventListener("mouseup", [this](sweet::Event * _event){
		clickingUI = false;
	});

	
	slideInTimeout = new Timeout(1.f, [this](sweet::Event * _event){
		controls->marginBottom.rationalSize = 0.f;
	});
	slideInTimeout->eventManager->addEventListener("progress", [this](sweet::Event * _event){
		controls->marginBottom.rationalSize = Easing::easeOutElastic(_event->getFloatData("progress"), -11/64.f, 11/64.f, 1, 22/64.f);
	});
	slideInTimeout->eventManager->addEventListener("start", [this](sweet::Event * _event){
		controls->marginBottom.rationalSize = -11/64.f;
	});
	childTransform->addChild(slideInTimeout, false);
	

	slideOutTimeout = new Timeout(0.5f, [this](sweet::Event * _event){
		controls->marginBottom.rationalSize = -11/64.f;
	});
	slideOutTimeout->eventManager->addEventListener("progress", [this](sweet::Event * _event){
		controls->marginBottom.rationalSize = Easing::easeInBack(_event->getFloatData("progress"), 0, -11/64.f, 1, 2.f);
	});
	slideOutTimeout->eventManager->addEventListener("start", [this](sweet::Event * _event){
		controls->marginBottom.rationalSize = 0.f;
	});
	childTransform->addChild(slideOutTimeout, false);


	readyTimeout = new Timeout(2.f, [this, zoomTimeout](sweet::Event * _event){
		// go to next scene
		ready = true;
		slideOutTimeout->start();
		zoomTimeout->start();
	});
	readyTimeout->eventManager->addEventListener("progress", [this](sweet::Event * _event){
		float p = _event->getFloatData("progress");
		p = glm::clamp(p - 0.5f, 0.f, 0.5f);
		
		wipe->marginLeft.rationalSize = Easing::easeOutCirc(p, 0, -1, 0.5f);
		wipe->width.rationalSize = Easing::easeOutCirc(p, 1, -1, 0.5f);
	});
	childTransform->addChild(readyTimeout, false);
	readyTimeout->start();
	readyTimeout->name = "ready timeout";


	leaveTimeout = new Timeout(1.f, [this](sweet::Event * _event){
		game->switchScene("menu", true);
		wipe->marginLeft.rationalSize = 0;
		wipe->width.rationalSize = 1;
	});
	leaveTimeout->eventManager->addEventListener("progress", [this](sweet::Event * _event){
		float p = _event->getFloatData("progress");
		
		wipe->marginLeft.rationalSize = Easing::easeOutCirc(p, 1, -1, 1.f);
		wipe->width.rationalSize = Easing::easeOutCirc(p, 2, -1, 1.f);
	});
	leaveTimeout->eventManager->addEventListener("start", [this](sweet::Event * _event){
		wipe->marginLeft.rationalSize = 1;
		wipe->width.rationalSize = 2;
		wipe->background->mesh->replaceTextures(MY_ResourceManager::globalAssets->getTexture("MENU")->texture);
	});
	childTransform->addChild(leaveTimeout, false);
	leaveTimeout->name = "ready timeout";
	
	redoButt->eventManager->addEventListener("click", [this](sweet::Event * _event){
		if(ready){
			leaveTimeout->restart();
		}
	});

	cone = new MeshEntity(MY_ResourceManager::globalAssets->getMesh("cone")->meshes.at(_cone), baseShader);
	face = new MeshEntity(MY_ResourceManager::globalAssets->getMesh("face")->meshes.at(_face), baseShader);
	topping = new MeshEntity(MY_ResourceManager::globalAssets->getMesh("topping")->meshes.at(_topping), baseShader);
	
	cone->mesh->setScaleMode(GL_NEAREST);
	face->mesh->setScaleMode(GL_NEAREST);
	topping->mesh->setScaleMode(GL_NEAREST);

	Texture * tex = MY_ResourceManager::globalAssets->getTexture("palette")->texture;
	cone->mesh->replaceTextures(tex);
	face->mesh->replaceTextures(tex);
	topping->mesh->replaceTextures(tex);

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
	for(auto & v : tileBg->background->mesh->vertices){
		v.u += 0.005f * _step->deltaTimeCorrection;
		v.v += 0.005f * _step->deltaTimeCorrection;
	}
	tileBg->background->mesh->dirty = true;
	tileBg->background->mesh->replaceTextures(MY_ResourceManager::globalAssets->getTexture("tilebg_" + std::to_string(((_step->cycles/40)%2) + 1))->texture);
	bg->background->mesh->replaceTextures(MY_ResourceManager::globalAssets->getTexture("bg_" + std::to_string(((_step->cycles/10)%3) + 1))->texture);
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
		if(keyboard->keyJustDown(GLFW_KEY_ESCAPE)){
			leaveTimeout->restart();
		}

		bool onUI = mouse->mouseY() > 11.f/64.f * sweet::getWindowHeight();

		if(!mouse->leftDown() && !mouse->leftJustReleased()){
			if(onUI){
				slideInTimeout->stop();
				slideOutTimeout->start();
			}else{
				slideOutTimeout->stop();
				slideInTimeout->start();
			}
		}
	}

	// camera movement
	zoom -= mouse->getMouseWheelDelta();
	zoom = glm::clamp(zoom, 5.f, 20.f);
	gameCamPolarCoords.y += (zoom - gameCamPolarCoords.y) * 0.05f;
	gameCamPolarCoords.y = glm::clamp(gameCamPolarCoords.y, 5.f, 20.f);

	if(mouse->leftDown() && ready){
		if(!mouse->leftJustPressed() && !clickingUI){
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
	
	targetOrbitalHeight = glm::clamp(targetOrbitalHeight, -5.f, 20.f);
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