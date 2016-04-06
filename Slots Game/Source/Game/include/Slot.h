#pragma once

#include <MeshEntity.h>

class Timeout;
class Slot : public MeshEntity{
public:
	Timeout * spinTimeout;


	int selection;
	float angle;
	float angleTaget;

	bool spinning;


	Slot(Shader * _shader);
	~Slot();

	virtual void update(Step * _step) override;
};