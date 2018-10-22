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
	bool isVerticalSymmetry(); // �c�����ɑΏ̂��ǂ���
	bool isHorizontalSymmetry(); //���c�����ɑΏ̂��ǂ���

	int qrH, qrW;
	std::vector<std::vector<Grid>> qrField;
	std::array<Point, 4> qrAgent;

};

}

