#pragma once

#include "Shared/Field.h"
#include "Shared/Util.h"
#include <string>

namespace Procon2018 {

class QRReader {
public:
	QRReader();
	~QRReader();
	const Field createField();


private:
	void read();
	void organize(char *qrText);
	void setEnemyAgent();
	bool isVerticalSymmetry(); // cŒü‚«‚É‘ÎÌ‚©‚Ç‚¤‚©
	bool isHorizontalSymmetry(); //‰¡cŒü‚«‚É‘ÎÌ‚©‚Ç‚¤‚©

	int qrH, qrW;
	std::vector<std::vector<Grid>> qrField;
	std::array<Point, 4> qrAgent;

};

}

