#pragma once

#include <Slot.h>
#include <MY_ResourceManager.h>
#include <Timeout.h>

Slot::Slot(Shader * _shader) : 
	MeshEntity(MY_ResourceManager::globalAssets->getMesh("slot")->meshes.at(0), _shader),
	angle(0),
	angleTaget(0),
	selection(0),
	spinning(false)
{
	mesh->setScaleMode(GL_NEAREST);
	mesh->pushTexture2D(MY_ResourceManager::globalAssets->getTexture("slot")->texture);

	spinTimeout = new Timeout(2.f, [this](sweet::Event * _event){
		spinning = false;
	});
	spinTimeout->eventManager->addEventListener("start", [this](sweet::Event * _event){
		spinning = true;
	});
	/*spinTimeout->eventManager->addEventListener("progress", [this](sweet::Event * _event){

	});*/
	childTransform->addChild(spinTimeout, false);
}

Slot::~Slot(){
}


void Slot::update(Step * _step){
	if(spinning){
		angleTaget += 45.f;
		if(angleTaget > 360.f){
			angleTaget -= 360.f;
			angle -= 360.f;
		}
	}else{
		angleTaget = selection / 12.f * 360.f;
	}

	angle += (angleTaget - angle)*0.1f;

	childTransform->setOrientation(glm::angleAxis(angle, glm::vec3(0, 1, 0)));

	MeshEntity::update(_step);
}