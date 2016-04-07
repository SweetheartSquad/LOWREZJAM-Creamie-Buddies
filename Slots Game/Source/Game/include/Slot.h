#pragma once

#include <MeshEntity.h>

class Timeout;
class Slot : public MeshEntity{
public:
	Timeout * spinTimeout;


	int selection;
	float angle;
	float angleTarget;

	bool spinning;


	Slot(Shader * _shader, int _id);
	~Slot();

	virtual void update(Step * _step) override;
};