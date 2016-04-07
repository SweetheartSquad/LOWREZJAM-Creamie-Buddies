#pragma once

#include <MY_Game.h>

#include <MY_ResourceManager.h>

#include <MY_Scene_Menu.h>

bool MY_Game::resized = false;

MY_Game::MY_Game() :
	Game("menu", new MY_Scene_Menu(this), true) // initialize our game with a menu scene
{
	sweet::setCursorMode(GLFW_CURSOR_NORMAL);
	MY_ResourceManager::globalAssets->getAudio("bgm")->sound->play(true);
}

MY_Game::~MY_Game(){}

void MY_Game::addSplashes(){
	// add default splashes
	//Game::addSplashes();

	// add custom splashes
	//addSplash(new Scene_Splash(this, MY_ResourceManager::globalAssets->getTexture("DEFAULT")->texture, MY_ResourceManager::globalAssets->getAudio("DEFAULT")->sound));
}

void MY_Game::update(Step * _step){
	glm::uvec2 sd = sweet::getWindowDimensions();
	int s = glm::min(sd.x, sd.y);
	if(sd.x != sd.y){
		glfwSetWindowSize(sweet::currentContext, s, s);
		
	}
	if(s != lastSize){
		resized = true;
		lastSize = s;
	}
	Game::update(_step);
	resized = false;
}