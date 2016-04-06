#pragma once

#include <MY_Scene_Base.h>

class MY_Scene_Menu : public MY_Scene_Base{
public:
	bool ready;
	Timeout * readyTimeout;

	MY_Scene_Menu(Game * _game);
	MeshInterface * clouds;
	virtual void update(Step * _step) override;
};