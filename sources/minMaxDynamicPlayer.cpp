#include "minMaxDynamicPlayer.h"
#include "rules.h"
#include "interface.h"

std::mutex MinMaxDynamicPlayer::mutex;

Choice::Choice(unsigned char x, unsigned char y, HeuristicBoard *myHeuristic, HeuristicBoard *ennemyHeuristic, Player *player, Gomoku *gomoku, bool ennemy) : x(x), y(y), gomoku(gomoku), myHeuristic(myHeuristic), ennemyHeuristic(ennemyHeuristic)
{
	if (!ennemy) {
		this->gomoku.setStone(player->getColor(), x, y);
		this->gomoku.updateFocus(x, y);
		this->myHeuristic.setGomoku(&this->gomoku);
		this->ennemyHeuristic.setGomoku(&this->gomoku);
		this->myHeuristic.put(x, y);
		this->ennemyHeuristic.clear(x, y);
	} else {
		this->gomoku.setStone(player->getColor(), x, y);
		this->gomoku.updateFocus(x, y);
		this->myHeuristic.setGomoku(&this->gomoku);
		this->ennemyHeuristic.setGomoku(&this->gomoku);
		this->myHeuristic.clear(x, y);
		this->ennemyHeuristic.put(x, y);
	}
}

Choice::~Choice()
{
}


MinMaxDynamicPlayer::MinMaxDynamicPlayer(std::array<unsigned char, 11> dw) : Player()
{
	minMaxDepth = 0;
	int i = 0;
	for (i = 0; dw[i] > 0; i++) {
		if (i >= 10)
			failure("max depth width for min-max is 10");
		depthWidths[i] = dw[i];
		minMaxDepth += 1;
	}
	depthWidths[i] = 0;
	DEBUG << "NEW MINMAXPLAYER\n";

}

MinMaxDynamicPlayer::~MinMaxDynamicPlayer()
{
	DEBUG << "BYE MINMAXPLAYER\n";
}

void MinMaxDynamicPlayer::heuristicMap(std::multimap<long long, std::unique_ptr<Choice>> &result, Gomoku *gomoku, Rules &rules, bool last, int depth, HeuristicBoard &myOrigin, HeuristicBoard &ennemyOrigin) {
	rules.setGomoku(gomoku);
	for (unsigned char j = 0; j < GH; j++) {
		for (unsigned char i = 0; i < GW; i++) {
			if (gomoku->isFocus(i, j) && rules.canPutStone(&myOrigin, i, j)) {


				Choice *choice = new Choice(i, j, &myOrigin, &ennemyOrigin, this, gomoku);

				choice->gomoku.checkCapture(*this, i, j, *enemy, choice->captured);
				for (auto it2 = choice->captured.begin(); it2 != choice->captured.end(); ++it2) {
					choice->gomoku.setStone(FREE, it2->first, it2->second);
				}

				for (auto it = choice->captured.begin(); it != choice->captured.end(); ++it) {
					choice->ennemyHeuristic.beCaptured(it->first, it->second);
					choice->myHeuristic.capture(it->first, it->second);
				}

				result.insert(std::pair<long long, std::unique_ptr<Choice>>(-heuristic(choice->myHeuristic, choice->ennemyHeuristic, last, depth), std::unique_ptr<Choice>(choice)));
			}
		}
	}
}

void MinMaxDynamicPlayer::heuristicMap_e(std::multimap<long long, std::unique_ptr<Choice>> &result, Gomoku *gomoku, Rules &rules, bool last, int depth, HeuristicBoard &myOrigin, HeuristicBoard &ennemyOrigin) {
	rules.setGomoku(gomoku);
	for (unsigned char j = 0; j < GH; j++) {
		for (unsigned char i = 0; i < GW; i++) {
			if (gomoku->isFocus(i, j) && rules.canPutStone(&ennemyOrigin, i, j)) {

				Choice *choice = new Choice(i, j, &myOrigin, &ennemyOrigin, enemy, gomoku, true);

				choice->gomoku.checkCapture(*enemy, i, j, *this, choice->captured);
				for (auto it2 = choice->captured.begin(); it2 != choice->captured.end(); ++it2) {
					choice->gomoku.setStone(FREE, it2->first, it2->second);
				}

				for (auto it = choice->captured.begin(); it != choice->captured.end(); ++it) {
					choice->ennemyHeuristic.capture(it->first, it->second);
					choice->myHeuristic.beCaptured(it->first, it->second);
				}

				result.insert(std::pair<long long, std::unique_ptr<Choice>>(heuristic_e(choice->myHeuristic, choice->ennemyHeuristic, last, depth), std::unique_ptr<Choice>(choice)));

			}
		}
	}
}

bool MinMaxDynamicPlayer::win(int &rx, int &ry, Rules &rules) {
	for (unsigned char j = 0; j < GH; j++) {
		for (unsigned char i = 0; i < GW; i++) {
			if (rules.canPutStone(this->getMyHeuristic(), i, j)) {
				if (myHeuristic.getBestLevel(i, j) >= 4) {
					rx = i;
					ry = j;
					return true;
				}
			}
		}
	}
	return false;
}

void MinMaxDynamicPlayer::startMinMax(int &rx, int &ry, Rules &rules)
{
	long long option;
	long long best = MIN_LONG;
	long long maxBestOption = MIN_LONG;

	if (rules.getTurnCounter() < STARTING_ROUND && minMaxDepth > STARTING_DEPTH)
		usedDepth = STARTING_DEPTH;
	else
		usedDepth = minMaxDepth;

	complexity = 0;
	std::multimap<long long, std::unique_ptr<Choice>> choices;
	heuristicMap(choices, gomoku, rules, ennemyHeuristic.fiveLine, usedDepth, myHeuristic, ennemyHeuristic);
	rx = choices.begin()->second->x;
	ry = choices.begin()->second->y;
	std::thread threads[NUM_THREADS];

	for (int i = 0; i < NUM_THREADS; i++) {
		threads[i] = std::thread(&MinMaxDynamicPlayer::startThread, this, std::ref(rx), std::ref(ry), std::ref(option), std::ref(best), std::ref(maxBestOption), std::ref(choices), i);
	}
	for (auto& th : threads) th.join();

	DEBUG << "complexity : " << complexity << "\n";
}

void MinMaxDynamicPlayer::startThread(int &rx, int &ry, long long &option, long long &best, long long &maxBestOption, std::multimap<long long, std::unique_ptr<Choice>> &choices, int threadIndex) {

	if ((size_t)threadIndex >= choices.size())
		return ;

	long long tmp;
	unsigned char i = threadIndex;
	Rules *copyRules = gomoku->getRules().copy();

	auto it = std::next(choices.begin(), threadIndex);
	while (1) {


		if (i > depthWidths[0]) {
			break;
		}

		complexity++;

		tmp = min(&it->second->gomoku, usedDepth - 1, MAX_LONG, maxBestOption, *copyRules, it->second->myHeuristic, it->second->ennemyHeuristic);

		mutex.lock();
		DEBUG << YELLOW << "try : " << (int)it->second->x << "/" << (int)it->second->y << DEFAULT_COLOR << " - " << (int)i << "\n";
		if (tmp >= BIG) {
			DEBUG << GREEN << "I CAN WIN : " <<  tmp << DEFAULT_COLOR << "\n";
		} else if (tmp <= -BIG) {
			DEBUG << RED << "I WILL LOOSE : " << tmp << DEFAULT_COLOR << "\n";
		} else {
			DEBUG << DARK_YELLOW << "score : " << tmp << DEFAULT_COLOR << "\n";
		}

		option = tmp;
		if (option > best) {
			best = option;
			rx = it->second->x;
			ry = it->second->y;
		}
		if (option > maxBestOption) {
			maxBestOption = option;
		}
		mutex.unlock();

		i += NUM_THREADS;
		if ((size_t)i >= choices.size()) {
			break ;
		} else {
			it = std::next(it, NUM_THREADS);
		}
	}
	delete copyRules;
}

long long MinMaxDynamicPlayer::min(Gomoku *origin, int depth, long long minBestOption, long long maxBestOption, Rules &rules, HeuristicBoard myH, HeuristicBoard ennemyH)
{
	if (depth <= 0 || ennemyH.fiveLine || myH.totalCaptured >= NB_CAPTURE_TO_WIN) {
		return heuristic(myH, ennemyH, ennemyH.fiveLine, depth+1);
	}
	long long option;
	long long best = MAX_LONG;
	std::multimap<long long, std::unique_ptr<Choice>> choices;
	heuristicMap_e(choices, origin, rules, myH.fiveLine, depth, myH, ennemyH);
	unsigned char i = 0;
	for (auto it = choices.begin(); it != choices.end(); ++it) {
		if (i > depthWidths[usedDepth - depth]) {
			break;
		}


		complexity++;
		option = max(&it->second->gomoku, depth-1, minBestOption, maxBestOption, rules, it->second->myHeuristic, it->second->ennemyHeuristic);
		if (option < best) {
			best = option;
			if (option < minBestOption) {
				minBestOption = option;
			}
			if (maxBestOption >= minBestOption - 1) {
				return best;
			}
		}
		i++;
	}
	return best;
}

long long MinMaxDynamicPlayer::max(Gomoku *origin, int depth, long long minBestOption, long long maxBestOption, Rules &rules, HeuristicBoard myH, HeuristicBoard ennemyH)
{
	if (depth <= 0 || myH.fiveLine || ennemyH.totalCaptured >= NB_CAPTURE_TO_WIN) {
		return heuristic_e(myH, ennemyH, myH.fiveLine, depth+1);
	}
	long long option;
	long long best = MIN_LONG;
	std::multimap<long long, std::unique_ptr<Choice>> choices;
	heuristicMap(choices, origin, rules, ennemyH.fiveLine, depth, myH, ennemyH);
	unsigned char i = 0;
	for (auto it = choices.begin(); it != choices.end(); ++it) {
		if (i > depthWidths[usedDepth - depth]) {
			break;
		}


		complexity++;
		option = min(&it->second->gomoku, depth-1, minBestOption, maxBestOption, rules, it->second->myHeuristic, it->second->ennemyHeuristic);
		if (option > best) {
			best = option;
			if (option > maxBestOption) {
				maxBestOption = option;
			}
			if (maxBestOption >= minBestOption - 1) {
				return best;
			}
		}
		i++;
	}
	return best;
}


void MinMaxDynamicPlayer::play(Rules &rules, Interface &i) {
	(void)i;
	int x = -1;
	int y = -1;
	startMinMax(x, y, rules);
	putStone(x, y);
	setCoordPlayed(x, y);
}

void MinMaxDynamicPlayer::playToHelp(Rules &rules, Interface &i) {
	(void)i;
	int x = -1;
	int y = -1;
	startMinMax(x, y, rules);
	i.putStoneToHelp(x, y);
}

long long MinMaxDynamicPlayer::heuristic(HeuristicBoard &myH, HeuristicBoard &ennemyH, bool last, int depth) {
	if (last && !canAvoidDefeat(myH, ennemyH)) {
		return (-BIG * (depth));
	}
	if (ennemyH.fiveLine || ennemyH.totalCaptured >= NB_CAPTURE_TO_WIN) {
		return -BIG * (depth);
	}
	if (myH.fiveLine || myH.totalCaptured >= NB_CAPTURE_TO_WIN) {
		return BIG * (depth);
	}
	return (myH.score + CAPTURE_BONUS(myH.totalCaptured) - (ennemyH.score + CAPTURE_BONUS(ennemyH.totalCaptured)) );
}

bool MinMaxDynamicPlayer::canAvoidDefeat(HeuristicBoard &myH, HeuristicBoard &ennemyH) {
	if (!ennemyH.fiveLine || myH.totalCaptured >= NB_CAPTURE_TO_WIN)
		return true;
	return false;
}

long long MinMaxDynamicPlayer::heuristic_e(HeuristicBoard &myH, HeuristicBoard &ennemyH, bool last, int depth) {
	if (last && !canAvoidDefeat_e(myH, ennemyH)) {
		return (BIG * (depth));
	}
	if (myH.fiveLine || myH.totalCaptured >= NB_CAPTURE_TO_WIN) {
		return BIG * (depth);
	}
	if (ennemyH.fiveLine || ennemyH.totalCaptured >= NB_CAPTURE_TO_WIN) {
		return -BIG * (depth);
	}
	return (myH.score + CAPTURE_BONUS(myH.totalCaptured) - (ennemyH.score + CAPTURE_BONUS(ennemyH.totalCaptured)) );
}

bool MinMaxDynamicPlayer::canAvoidDefeat_e(HeuristicBoard &myH, HeuristicBoard &ennemyH) {
	if (!myH.fiveLine || ennemyH.totalCaptured >= NB_CAPTURE_TO_WIN)
		return true;
	return false;
}
