#pragma once

#include <Slot.h>
#include <MY_ResourceManager.h>
#include <Timeout.h>
#include <NumberUtils.h>

Slot::Slot(Shader * _shader, int _id) : 
	MeshEntity(new TriMesh(true), _shader),
	angle(0),
	angleTarget(0),
	selection(0),
	spinning(false)
{
	mesh->setScaleMode(GL_NEAREST);
	mesh->uvEdgeMode = GL_CLAMP_TO_BORDER;
	mesh->pushTexture2D(MY_ResourceManager::globalAssets->getTexture("slots_" + std::to_string(_id))->texture);
	mesh->insertVertices(*MY_ResourceManager::globalAssets->getMesh("slot")->meshes.at(0));

	spinTimeout = new Timeout(2.f, [this, _id](sweet::Event * _event){
		spinning = false;
		MY_ResourceManager::globalAssets->getAudio("slot"+std::to_string(_id))->sound->play();
	});
	spinTimeout->eventManager->addEventListener("start", [this](sweet::Event * _event){
		selection = sweet::NumberUtils::randomInt(0, 11);
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
		angleTarget -= 20.f;
	}else{
		angleTarget = -(selection) / 12.f * 360.f;
	}
	float d = angleTarget - angle;
	if(glm::abs(d) > FLT_EPSILON){
		if(d > 90.f){
			d -= 360.f;
		}

		angle += d*0.1f;

		childTransform->setOrientation(glm::angleAxis(angle, glm::vec3(0, 1, 0)));
	}
	MeshEntity::update(_step);
}