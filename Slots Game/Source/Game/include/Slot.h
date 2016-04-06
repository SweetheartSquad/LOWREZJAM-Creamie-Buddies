#pragma once

#include <MeshEntity.h>

class Slot : public MeshEntity{
public:
	int selection;
	float angle;


	Slot(Shader * _shader);
	~Slot();

	virtual void update(Step * _step) override;
};