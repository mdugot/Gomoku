#include "heuristicBoard.h"

const long long HeuristicBoard::levels[6] = {L0, L1, L2, L3, L4, HAS_WON};

HeuristicBoard::HeuristicBoard()
{
//	DEBUG << "new 1\n";
	bzero(heuristic, sizeof(short int[GW][GH]));
	totalCaptured = 0;
	stone = 0;
	gomoku = NULL;
	score = 0;

}

HeuristicBoard::HeuristicBoard(HeuristicBoard *copyFrom) {
//	DEBUG << "memcopy\n";
//	DEBUG << "copy : " << heuristic << "/" << copyFrom->heuristic << "\n";
	totalCaptured = copyFrom->totalCaptured;
	stone = copyFrom->stone;
	gomoku = copyFrom->gomoku;
	score = copyFrom->score;
	memcpy((void*)heuristic, (void*)(copyFrom->heuristic), sizeof(short int[GW][GH]));
//	failure("quit");
//	DEBUG << "end copy\n";
}

HeuristicBoard::~HeuristicBoard()
{
}

void HeuristicBoard::getAdjacent(char x, char y, char vx, char vy, char &before, char &after, char &beforeWall, char &afterWall)
{
	before = 1;
	after = 1;
	beforeWall = 0;
	afterWall = 0;
	char xi = vx;
	char yi = vy;
//	DEBUG << "stone : " << stone << "\n";
//	DEBUG << "SEARCH ADJACENT : ";
	while (1) {
//		DEBUG << "+";
		if (gomoku->getStone(x+xi,y+yi) == stone)
			after += 1;
		else {
			if (gomoku->getStone(x+xi,y+yi) != FREE) {
				afterWall = 1;
			}
			break;
		}
		xi += vx;
		yi += vy;
	}
//	DEBUG << "\n";
	xi = vx;
	yi = vy;
	while (1) {
		if (gomoku->getStone(x-xi,y-yi) == stone)
			before += 1;
		else {
			if (gomoku->getStone(x-xi,y-yi) != FREE) {
				beforeWall = 1;
			}
			break;
		}
		xi += vx;
		yi += vy;
	}
}

void HeuristicBoard::clearOne(char x, char y, char vx, char vy, char shift, short int mask)
{
	char before, beforeWall;
	char after, afterWall;

	getAdjacent(x, y, vx, vy, before, after, beforeWall, afterWall);
	if (after > 4 || before > 4)
		REMOVE_THREAT(GET_THREAT(heuristic[(unsigned char)x][(unsigned char)y], mask, shift));
	else
		REMOVE_THREAT(4);
}

HeuristicBoard& HeuristicBoard::clear(unsigned char x, unsigned char y)
{
//	DEBUG << "clear\n";
	if (GET_HH(heuristic[x][y]) < 5)
		REMOVE_THREAT(GET_HH(heuristic[x][y]));
	else
		clearOne(x, y, 1, 0, HORIZONTAL_SHIFT, HORIZONTAL_MASK);

	if (GET_VH(heuristic[x][y]) < 5)
		REMOVE_THREAT(GET_VH(heuristic[x][y]));
	else
		clearOne(x, y, 0, 1, VERTICAL_SHIFT, VERTICAL_MASK);

	if (GET_DLH(heuristic[x][y]) < 5)
		REMOVE_THREAT(GET_DLH(heuristic[x][y]));
	else
		clearOne(x, y, -1, 1, DLEFT_SHIFT, DLEFT_MASK);

	if (GET_DRH(heuristic[x][y]) < 5)
		REMOVE_THREAT(GET_DRH(heuristic[x][y]));
	else
		clearOne(x, y, 1, 1, DRIGHT_SHIFT, DRIGHT_MASK);

	heuristic[x][y] = 0;
	return *this;
}

void HeuristicBoard::fiveValue(char x, char y, unsigned char &value, unsigned char heuristic) {
	(void)x;
	(void)y;
	if (value > 4 && heuristic < 4)
		value = 4;
}


void HeuristicBoard::updateThreat(char x, char y, char vx, char vy, char shift, short int mask)
{
	char before, beforeWall;
	char after, afterWall;
	unsigned char tmp;

	getAdjacent(x, y, vx, vy, before, after, beforeWall, afterWall);
	if (!beforeWall) {
		tmp = GET_THREAT(heuristic[(unsigned char)x-(before*vx)][(unsigned char)y-(before*vy)], mask, shift);
		REMOVE_THREAT(tmp);
		tmp += after;
		heuristic[(unsigned char)x-(before*vx)][(unsigned char)y-(before*vy)] = SET_THREAT(heuristic[(unsigned char)x-(before*vx)][(unsigned char)y-(before*vy)], tmp, mask, shift);
		fiveValue(x, y, tmp, GET_THREAT(heuristic[(unsigned char)x][(unsigned char)y], mask, shift));
		ADD_THREAT(tmp);
	}
	if (!afterWall) {
		tmp = GET_THREAT(heuristic[(unsigned char)x+(after*vx)][(unsigned char)y+(after*vy)], mask, shift);
		REMOVE_THREAT(tmp);
		tmp += before;
		heuristic[(unsigned char)x+(after*vx)][(unsigned char)y+(after*vy)] = SET_THREAT(heuristic[(unsigned char)x+(after*vx)][(unsigned char)y+(after*vy)], tmp, mask, shift);
		fiveValue(x, y, tmp, GET_THREAT(heuristic[(unsigned char)x][(unsigned char)y], mask, shift));
		ADD_THREAT(tmp);
	}
}

void HeuristicBoard::removeEnnemyThreat(char x, char y, char vx, char vy, char shift, short int mask)
{
	char before, beforeWall;
	char after, afterWall;

	getAdjacent(x, y, vx, vy, before, after, beforeWall, afterWall);
	heuristic[(unsigned char)x][(unsigned char)y] = SET_THREAT(heuristic[(unsigned char)x][(unsigned char)y], (after+before-2), mask, shift);
	ADD_THREAT((after+before-2) > 4 ? 4 : (after+before-2));
}

void HeuristicBoard::removeThreat(char x, char y, char vx, char vy, char shift, short int mask)
{
	char before, beforeWall;
	char after, afterWall;
	unsigned char tmp, oldLength;

	getAdjacent(x, y, vx, vy, before, after, beforeWall, afterWall);
	oldLength = before + after - 1;
	if (!beforeWall) {
		tmp = GET_THREAT(heuristic[(unsigned char)x-(before*vx)][(unsigned char)y-(before*vy)], mask, shift);
		if (tmp) {
			REMOVE_THREAT((tmp > 4 && oldLength <= 5) ? 4 : tmp);
			tmp -= after;
			heuristic[(unsigned char)x-(before*vx)][(unsigned char)y-(before*vy)] = SET_THREAT(heuristic[(unsigned char)x-(before*vx)][(unsigned char)y-(before*vy)], tmp, mask, shift);
			ADD_THREAT((tmp > 4 && before <= 5) ? 4 : tmp);
		}
	}
	if (!afterWall) {
		tmp = GET_THREAT(heuristic[(unsigned char)x+(after*vx)][(unsigned char)y+(after*vy)], mask, shift);
		if (tmp) {
			REMOVE_THREAT((tmp > 4 && oldLength <= 5) ? 4 : tmp);
			tmp -= before;
			heuristic[(unsigned char)x+(after*vx)][(unsigned char)y+(after*vy)] = SET_THREAT(heuristic[(unsigned char)x+(after*vx)][(unsigned char)y+(after*vy)], tmp, mask, shift);
			ADD_THREAT((tmp > 4 && after <= 5) ? 4 : tmp);
		}
	}
	heuristic[(unsigned char)x][(unsigned char)y] = SET_THREAT(heuristic[(unsigned char)x][(unsigned char)y], (after+before-2), mask, shift);
	ADD_THREAT((after+before-2) > 4 ? 4 : (after+before-2));
}

void HeuristicBoard::beCaptured(unsigned char x, unsigned char y)
{
	removeThreat(x, y, 0, 1, HORIZONTAL_SHIFT, HORIZONTAL_MASK);
	removeThreat(x, y, 1, 0, VERTICAL_SHIFT, VERTICAL_MASK);
	removeThreat(x, y, 1, 1, DRIGHT_SHIFT, DRIGHT_MASK);
	removeThreat(x, y, -1, 1, DLEFT_SHIFT, DLEFT_MASK);
}

void HeuristicBoard::capture(unsigned char x, unsigned char y)
{
	removeEnnemyThreat(x, y, 0, 1, HORIZONTAL_SHIFT, HORIZONTAL_MASK);
	removeEnnemyThreat(x, y, 1, 0, VERTICAL_SHIFT, VERTICAL_MASK);
	removeEnnemyThreat(x, y, 1, 1, DRIGHT_SHIFT, DRIGHT_MASK);
	removeEnnemyThreat(x, y, -1, 1, DLEFT_SHIFT, DLEFT_MASK);
	totalCaptured += 1;
}

HeuristicBoard& HeuristicBoard::put(unsigned char x, unsigned char y, bool prediction)
{
//	unsigned char before, beforeWall;
//	unsigned char after, afterWall;
//	unsigned char tmp;


	(void)prediction;
	updateThreat(x, y, 0, 1, HORIZONTAL_SHIFT, HORIZONTAL_MASK);
	updateThreat(x, y, 1, 0, VERTICAL_SHIFT, VERTICAL_MASK);
	updateThreat(x, y, 1, 1, DRIGHT_SHIFT, DRIGHT_MASK);
	updateThreat(x, y, -1, 1, DLEFT_SHIFT, DLEFT_MASK);
////	DEBUG << "put\n";
//	//HORIZONTAL THREATS UPDATE
//	getAdjacent(x, y, 0, 1, before, after, beforeWall, afterWall);
//	if (!beforeWall) {
////		WATCH(x, "horizontal");
////		WATCH(y-before, "horizontal");
////		DEBUG << "new horizontal threat\n";
//		tmp = GET_HH(heuristic[x][y-before]);
//		REMOVE_THREAT(tmp);
//		tmp += after;
//		fiveValue(x, y, tmp, GET_HH(heuristic[x][y]));
////		WATCH(x, "horizontal");
////		WATCH(y-before, "horizontal");
//		heuristic[x][y-before] = SET_HH(heuristic[x][y-before], tmp);
////		if (!prediction) DEBUG << "ADD left " << tmp << "\n";
//		ADD_THREAT(tmp);
//	}
//	if (!afterWall) {
////		DEBUG << "new horizontal threat\n";
////		WATCH(x, "horizontal");
////		WATCH(y+after, "horizontal");
//		tmp = GET_HH(heuristic[x][y+after]);
//		REMOVE_THREAT(tmp);
//		tmp += before;
//		fiveValue(x, y, tmp, GET_HH(heuristic[x][y]));
////		WATCH(x, "horizontal");
////		WATCH(y+after, "horizontal");
//		heuristic[x][y+after] = SET_HH(heuristic[x][y+after], tmp);
////		if (!prediction) DEBUG << "ADD right " << tmp << "\n";
//		ADD_THREAT(tmp);
//	}
//
//	//VERTICAL THREATS UPDATE
//	getAdjacent(x, y, 1, 0, before, after, beforeWall, afterWall);
//	if (!beforeWall) {
////		WATCH(x-before, "vertical");
////		WATCH(y, "vertical");
//		tmp = GET_VH(heuristic[x-before][y]);
//		REMOVE_THREAT(tmp);
//		tmp += after;
//		fiveValue(x, y, tmp, GET_VH(heuristic[x][y]));
////		WATCH(x-before, "vertical");
////		WATCH(y, "vertical");
//		heuristic[x-before][y] = SET_VH(heuristic[x-before][y], tmp);
////		if (!prediction) DEBUG << "ADD up " << tmp << "\n";
//		ADD_THREAT(tmp);
//	}
//	if (!afterWall) {
////		WATCH(x+after, "vertical");
////		WATCH(y, "vertical");
//		tmp = GET_VH(heuristic[x+after][y]);
//		REMOVE_THREAT(tmp);
//		tmp += before;
//		fiveValue(x, y, tmp, GET_VH(heuristic[x][y]));
////		WATCH(x+after, "vertical");
////		WATCH(y, "vertical");
//		heuristic[x+after][y] = SET_VH(heuristic[x+after][y], tmp);
////		if (!prediction) DEBUG << "ADD down " << tmp << "\n";
//		ADD_THREAT(tmp);
//	}
//
//	//DIAGONAL RIGTH THREATS UPDATE
//	getAdjacent(x, y, 1, 1, before, after, beforeWall, afterWall);
//	if (!beforeWall) {
////		WATCH(x-before, "right");
////		WATCH(y-before, "right");
//		tmp = GET_DRH(heuristic[x-before][y-before]);
//		REMOVE_THREAT(tmp);
//		tmp += after;
//		fiveValue(x, y, tmp, GET_DRH(heuristic[x][y]));
////		WATCH(x-before, "right");
////		WATCH(y-before, "right");
//		heuristic[x-before][y-before] = SET_DRH(heuristic[x-before][y-before], tmp);
////		if (!prediction) DEBUG << "ADD up-left " << tmp << "\n";
//		ADD_THREAT(tmp);
//	}
//	if (!afterWall) {
////		WATCH(x+after, "right");
////		WATCH(y+after, "right");
//		tmp = GET_DRH(heuristic[x+after][y+after]);
//		REMOVE_THREAT(tmp);
//		tmp += before;
//		fiveValue(x, y, tmp, GET_DRH(heuristic[x][y]));
////		WATCH(x+after, "right");
////		WATCH(y+after, "right");
//		heuristic[x+after][y+after] = SET_DRH(heuristic[x+after][y+after], tmp);
////		if (!prediction) DEBUG << "ADD down-right " << tmp << "\n";
//		ADD_THREAT(tmp);
//	}
//
//	//DIAGONAL LEFT THREATS UPDATE
//	getAdjacent(x, y, -1, 1, before, after, beforeWall, afterWall);
//	if (!beforeWall) {
////		WATCH(x+before, "left");
////		WATCH(y-before, "left");
//		tmp = GET_DLH(heuristic[x+before][y-before]);
//		REMOVE_THREAT(tmp);
//		tmp += after;
//		fiveValue(x, y, tmp, GET_DLH(heuristic[x][y]));
////		WATCH(x+before, "left");
////		WATCH(y-before, "left");
//		heuristic[x+before][y-before] = SET_DLH(heuristic[x+before][y-before], tmp);
//		ADD_THREAT(tmp);
//	}
//	if (!afterWall) {
////		WATCH(x-after, "left");
////		WATCH(y+after, "left");
//		tmp = GET_DLH(heuristic[x-after][y+after]);
//		REMOVE_THREAT(tmp);
//		tmp += before;
//		fiveValue(x, y, tmp, GET_DLH(heuristic[x][y]));
////		WATCH(x-after, "left");
////		WATCH(y+after, "left");
//		heuristic[x-after][y+after] = SET_DLH(heuristic[x-after][y+after], tmp);
////		if (!prediction) DEBUG << "ADD down-left " << tmp << "\n";
//		ADD_THREAT(tmp);
//	}

	return clear(x, y);
}

int HeuristicBoard::getBestLevel(unsigned char x, unsigned char y) {
	int r = 0;
	int tmp = 0;
	tmp = GET_HH(heuristic[x][y]);
	if (tmp > r)
		r = tmp;
	tmp = GET_VH(heuristic[x][y]);
	if (tmp > r)
		r = tmp;
	tmp = GET_DRH(heuristic[x][y]);
	if (tmp > r)
		r = tmp;
	tmp = GET_DLH(heuristic[x][y]);
	if (tmp > r)
		r = tmp;
	return r;
}

void HeuristicBoard::print(int lastX, int lastY) {
	DEBUG << "\n";
	int level;
	for (int j = 0; j < GH; j++) {
		for (int i = 0; i < GW; i++) {
			level = getBestLevel(i, j);
			if (gomoku->getStone(i, j) != FREE) {
				gomoku->printStone(i, j, lastX, lastY);
			} else if (level == 1) {
				DEBUG << BLUE << level << DEFAULT_COLOR;
			} else if (level == 2) {
				DEBUG << PURPLE << level << DEFAULT_COLOR;
			} else if (level == 3) {
				DEBUG << YELLOW << level << DEFAULT_COLOR;
			} else if (level == 4) {
				DEBUG << RED << level << DEFAULT_COLOR;
			} else if (level == 0) {
				DEBUG << "." << DEFAULT_COLOR;
			} else {
				DEBUG << BLACK_COLOR << HLL_RED << level << DEFAULT_COLOR;
			}
				
		}
		DEBUG << "\n";
	}
}