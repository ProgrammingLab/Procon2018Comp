#pragma once
#include"Shared/Util.h"
#include"Panel.h"


namespace Procon2018 {
class Panel {
public:
	Panel();
	Panel(Point pos, int _width, int _heght, s3d::String _text);
	~Panel();

	void draw();
	bool isClicked();

private:

	int cx, cy;
	int w, h;
	
	s3d::TextBox text;

	


};

}
