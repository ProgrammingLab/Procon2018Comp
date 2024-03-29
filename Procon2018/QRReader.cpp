#include "stdafx.h"
#include "QRReader.h"

namespace Procon2018 {

	QRReader::QRReader(PlayerId pId) : pId(pId) {
		pId_ = (PlayerId)(1 - (int)pId);
		a0 = (AgentId)((int)pId*2 + 0);
		a1 = (AgentId)((int)pId*2 + 1);
		a2 = (AgentId)((int)pId_*2 + 0);
		a3 = (AgentId)((int)pId_*2 + 1);
	}


	QRReader::~QRReader() {
	}


	Field QRReader::createField() {
		read();
		//organize("aaaa");
		Field ret(40, qrH, qrW, qrField, qrAgent);
		ret.setColor(qrAgent[0], PlayerId::A);
		ret.setColor(qrAgent[1], PlayerId::A);
		ret.setColor(qrAgent[2], PlayerId::B);
		ret.setColor(qrAgent[3], PlayerId::B);
		return ret;
	}

	void QRReader::read() {
		FILE	*fp;
		FILE    *out;

		char	*cmdline = "zbarcam.exe";
		if ((fp = _popen(cmdline, "r")) == NULL) {
			perror("can not exec commad");
			exit(EXIT_FAILURE);
		}
		out = fopen("out.txt", "w");

		char qrText[10000];

		while (!feof(fp)) {
			fgets(qrText, sizeof(qrText), fp);
			if (fprintf(out, "%s", qrText) != 0) {
				organize(qrText);
				break;
			}
		}

		_pclose(fp);

		//exit(EXIT_SUCCESS);
	}

	void QRReader::organize(char *qrText) {

		std::vector<std::string> organizedQrText;
		std::string tmp;


		for (int i = 0; i < strlen(qrText); ++i) {
			
			if (qrText[i] == '-' && (qrText[i+1]<'0' || qrText[i+1]>'9'))continue;
			if ((qrText[i]<'0' || qrText[i]>'9') && qrText[i] != '-') {
				if (tmp.size() != 0) {
					organizedQrText.push_back(tmp);
					tmp.clear();
				}
				continue;
			}
			tmp += qrText[i];
		}

		qrH = stoi(organizedQrText[0]);
		organizedQrText.erase(organizedQrText.begin());
		qrW = stoi(organizedQrText[0]);
		organizedQrText.erase(organizedQrText.begin());

		for (int y = 0; y < qrH; ++y) {
			std::vector<Grid> grid;
			for (int x = 0; x < qrW; ++x) {
				int score = stoi(organizedQrText[0]);
				Grid tmp;
				tmp.score = score;
				tmp.color = std::nullopt;
				grid.push_back(tmp);
				organizedQrText.erase(organizedQrText.begin());
			}
			qrField.push_back(grid);
		}

		qrAgent[(int)a0] = Point(stoi(organizedQrText[1]) - 1, stoi(organizedQrText[0]) - 1);
		qrAgent[(int)a1] = Point(stoi(organizedQrText[3]) - 1, stoi(organizedQrText[2]) - 1);
		organizedQrText.erase(organizedQrText.begin(), organizedQrText.end());
		
		
		/*Field tmpField = Field::RandomState();
		qrH = tmpField.h();
		qrW = tmpField.w();
		for (int y = 0; y < tmpField.h(); ++y) {
			std::vector<Grid> grid;
			for (int x = 0; x < tmpField.w(); ++x) {
				int score = tmpField.grid(Point(x, y)).score;
				Grid tmp;
				tmp.score = score;
				tmp.color = std::nullopt;
				grid.push_back(tmp);
			}
			qrField.push_back(grid);
		}


		qrAgent[0] = tmpField.agentPos(AgentId(0));
		qrAgent[1] = tmpField.agentPos(AgentId(1));*/
		
		setEnemyAgent();
		

	}

	void QRReader::setEnemyAgent() {
		if (isVerticalSymmetry() && !(qrAgent[(int)a0] == Point(qrAgent[(int)a1].x, qrH - 1 - qrAgent[(int)a1].y))) {
			qrAgent[(int)a2] = Point(qrAgent[(int)a0].x, qrH - 1 - qrAgent[(int)a0].y);
			qrAgent[(int)a3] = Point(qrAgent[(int)a1].x, qrH - 1 - qrAgent[(int)a1].y);
		}
		else {
			qrAgent[(int)a2] = Point(qrW - 1 - qrAgent[(int)a0].x, qrAgent[(int)a0].y);
			qrAgent[(int)a3] = Point(qrW - 1 - qrAgent[(int)a1].x, qrAgent[(int)a1].y);
		}
	}

	bool QRReader::isVerticalSymmetry() {
		for (int y = 0; y < qrH / 2; ++y) {
			for (int x = 0; x < qrW; ++x) {
				if (qrField[y][x].score != qrField[qrH - 1 - y][x].score)return false;
			}
		}
		return true;
	}


}
