#ifndef PLAYER_H
#define PLAYER_H

#include "gomoku.h"

class Rules;

class Player
{
	friend class Gomoku;

	protected:

		unsigned int eatenStones;
		Gomoku *gomoku;
		Stone stoneColor;
		
		inline void setGomoku(Gomoku *gomoku) {this->gomoku = gomoku;}
		inline void setColor(Stone color) {this->stoneColor = color;}
		void putStone(int x, int y);
	
	public:

		Player();
		~Player();

		virtual void play(Rules &rules) = 0;
		inline Stone getColor() {return stoneColor;}
};


#endif

