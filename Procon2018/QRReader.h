#pragma once

#include "Shared/Field.h"
#include "Shared/Util.h"
#include <string>

namespace Procon2018 {

class QRReader {
public:
	QRReader(PlayerId pId);
	~QRReader();
	Field createField();


private:
	void read();
	void organize(char *qrText);
	void setEnemyAgent();
	bool isVerticalSymmetry(); // 縦向きに対称かどうか
	bool isHorizontalSymmetry(); //横縦向きに対称かどうか

	int qrH, qrW;
	std::vector<std::vector<Grid>> qrField;
	std::array<Point, 4> qrAgent;
	PlayerId pId, pId_;
	AgentId a0, a1, a2, a3;

};

}

