#pragma once

#include <MY_Scene_Menu.h>
#include <MY_Scene_Main.h>
#include <shader/ComponentShaderText.h>
#include <sweet/UI.h>

MY_Scene_Menu::MY_Scene_Menu(Game * _game) :
	MY_Scene_Base(_game),
	ready(false)
{
	{
	VerticalLinearLayout * vl = new VerticalLinearLayout(uiLayer->world);
	uiLayer->addChild(vl);
	vl->setRationalHeight(1.f, uiLayer);
	vl->setRationalWidth(1.f, uiLayer);
	vl->horizontalAlignment = kCENTER;
	vl->verticalAlignment = kMIDDLE;
	NodeUI * menu = new NodeUI(uiLayer->world);
	vl->addChild(menu);
	menu->setRationalHeight(1.f, vl);
	menu->setSquareWidth(1.f);
	menu->background->mesh->pushTexture2D(MY_ResourceManager::globalAssets->getTexture("MENU")->texture);
	menu->background->mesh->setScaleMode(GL_NEAREST);
	}
	{
	VerticalLinearLayout * vl = new VerticalLinearLayout(uiLayer->world);
	uiLayer->addChild(vl);
	vl->setRationalHeight(1.f, uiLayer);
	vl->setRationalWidth(1.f, uiLayer);
	vl->horizontalAlignment = kCENTER;
	vl->verticalAlignment = kMIDDLE;
	NodeUI * menu = new NodeUI(uiLayer->world);
	vl->addChild(menu);
	menu->setRationalHeight(1.f, vl);
	menu->setSquareWidth(1.f);
	menu->background->mesh->pushTexture2D(MY_ResourceManager::globalAssets->getTexture("CLICK")->texture);
	menu->background->mesh->setScaleMode(GL_NEAREST);
	}

	readyTimeout = new Timeout(0.5f, [this](sweet::Event * _event){
		ready = true;
	});
	childTransform->addChild(readyTimeout, false);
	readyTimeout->start();
	readyTimeout->name = "ready timeout";
}

void MY_Scene_Menu::update(Step * _step){
	if(ready){
		if(mouse->leftJustPressed() || keyboard->keyJustDown(GLFW_KEY_SPACE)){
			if(game->scenes.count("main") == 0){
				game->scenes["main"] = new MY_Scene_Main(game);
			}
			game->switchScene("main", false);
		}
	}

	MY_Scene_Base::update(_step);
}