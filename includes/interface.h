#ifndef INTERFACE_H
# define INTERFACE_H

# include "utils.h"
# include "gomoku.h"
# include "player.h"

# define WIDTH	1200 // a essayer
# define HEIGHT	1200

using namespace sf;

class Interface {

	friend class Gomoku;

	public:
		Interface();
		~Interface();

		void	start(void);
		void	drawWindow(void);
		void	drawGame(void);
		void	update(void);
		void	welcomeScreen(void);
		void	menuScreen(void);
		void	background(void);
		void	winEnd(void);
		void	timer(void);
		void	putStone(sf::Sprite, int, int);
//		sf::CircleShape	getCurrentStonePlayer(void);
		inline sf::RenderWindow &getWindow(void) {return this->_window;}
		inline sf::Event &getEvent(void) {return this->_event;}
		void	loadTexture(void);
		void	loadSprite(void);
		void	makeSprite(sf::Sprite &s, sf::Texture &t, float sy, float sx, int px, int py);

	protected:
		Gomoku 				*gomoku;
		inline void setGomoku(Gomoku *gomoku) {this->gomoku = gomoku;}
		
		sf::RenderWindow	_window;
		sf::Event			_event;
		sf::Sprite			_stonePlayerOne;
		sf::Sprite			_stonePlayerTwo;
		sf::Sprite			_backgroundSprite;
       	sf::Sprite			_boardGameSprite;
      	sf::Sprite			_gridSprite;
		sf::Texture			_stoneTexture;
		sf::Texture			_backgroundTexture;
       	sf::Texture			_boardGameTexture;
      	sf::Texture			_gridTexture;
		std::list<sf::Sprite>			_allSprite;

		void	checkEvent(Player *currentPlayer);
		int	clickX;
		int clickY;
};

#endif
