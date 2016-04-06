#pragma once

#include <Slot.h>
#include <MY_ResourceManager.h>

Slot::Slot(Shader * _shader) : 
	MeshEntity(MY_ResourceManager::globalAssets->getMesh("slot")->meshes.at(0), _shader),
	angle(0),
	selection(0)
{
	mesh->setScaleMode(GL_NEAREST);
	mesh->pushTexture2D(MY_ResourceManager::globalAssets->getTexture("slot")->texture);
}

Slot::~Slot(){
}


void Slot::update(Step * _step){
	angle = selection;

	childTransform->setOrientation(glm::angleAxis(angle, glm::vec3(0, 1, 0)));

	MeshEntity::update(_step);
}