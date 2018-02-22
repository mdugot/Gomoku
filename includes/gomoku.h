#ifndef GOMOKU_H
#define GOMOKU_H

#include "utils.h"
#include "colors.h"
class Interface;
class Player;
class Rules;

#define GW 19
#define GH 19

typedef enum Stone {
	BLACK = 1, WHITE = 2, FREE = 0, OUT_LIMIT = -1
} Stone;

class Gomoku
{
	private:
		
		Player	&whitePlayer;
		Player	&blackPlayer;
		Player	*currentPlayer;
		Rules	&rules;
		Interface &interface;
		Stone	board[GW][GH];
		int		nbEatenStone;

		bool	checkLine(Stone color, int x, int y);
		bool	leftDiagonal(Stone color, int x, int y);
		bool	rightDiagonal(Stone color, int x, int y);
		bool	verticalLine(Stone color, int x, int y);
		bool	horizontalLine(Stone color, int x, int y);
		
		bool	checkLeft(Player &current, int x, int y, Player &enemy);
		bool	checkRight(Player &current, int x, int y, Player &enemy);
		bool	checkUp(Player &current, int x, int y, Player &enemy);
		bool	checkDown(Player &current, int x, int y, Player &enemy);
		bool	checkUpLeft(Player &current, int x, int y, Player &enemy);
		bool	checkDownLeft(Player &current, int x, int y, Player &enemy);
		bool	checkUpRight(Player &current, int x, int y, Player &enemy);
		bool	checkDownRight(Player &current, int x, int y, Player &enemy);
		bool	checkBetween(Stone colorEnemy, int x1, int y1, int x2, int y2);
		void	capture(Player &current, sf::Sprite *spriteEnemy, int x1, int y1, int x2, int y2);
	
	public:

		Gomoku(Player& p1, Player& p2, Rules& rules, Interface& interface);
		~Gomoku();

		inline Player &getWhitePlayer() {return whitePlayer;}
		inline Player &getBlackPlayer() {return blackPlayer;}
		inline Player *getCurrentPlayer() {return currentPlayer;}
		inline Rules &getRules() {return rules;}
		inline Interface &getInterface() {return interface;}
		inline int getNbEatenStone() {return nbEatenStone;}
		Stone getStone(int x, int y);
		inline void setStone(Stone stone, int x, int y) {board[x][y] = stone;}

		bool fiveStoneLine(Stone color, int &x, int &y);
		bool checkCapture(Player &current, int x, int y, Player &enemy);

		void printBoard();
		void start();
};


#endif
