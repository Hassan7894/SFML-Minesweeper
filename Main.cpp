#ifdef SFML_STATIC
#pragma comment(lib, "glew.lib")
#pragma comment(lib, "freetype.lib")
#pragma comment(lib, "jpeg.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "gdi32.lib")  
#endif // SFML_STATIC

#define MAX_WINDOW_HEIGHT 690
#define MAX_WINDOW_WIDTH 1340
#define ROAD_SIZE 3.5

#include <SFML/Graphics.hpp>

#include <iostream>
#include <ctime>
#include <iomanip>
#include <windows.h>


//This fucntion takes the handle of the window, the object, and the event handle and checks whether the object was left clicked in the event or not.
//1 means it was clicked, 0 means it was not.

bool isLeftClicked(sf::RenderWindow &window, sf::RectangleShape &object, sf::Event &event)
{
	if (event.type == sf::Event::MouseButtonPressed)
	{
		if (event.mouseButton.button == sf::Mouse::Left)
		{
			const int mouseX = event.mouseButton.x;
			const int mouseY = event.mouseButton.y;
			const sf::Vector2f shapePos = object.getPosition();
			const sf::Vector2f shapeSize = object.getSize();
			if (mouseX >= shapePos.x && mouseX <= shapePos.x + shapeSize.x)
			{
				if (mouseY >= shapePos.y && mouseY <= shapePos.y + shapeSize.y)
				{
					return 1;
				}
			}
		}
	}
	return 0;
}


//This fucntion takes the handle of the window, the object, and the event handle and checks whether the object was left clicked in the event or not.
//1 means it was clicked, 0 means it was not.

bool isRightClicked(sf::RenderWindow &window, sf::RectangleShape &object, sf::Event &event)
{
	if (event.type == sf::Event::MouseButtonPressed)
	{
		if (event.mouseButton.button == sf::Mouse::Right)
		{
			const int mouseX = event.mouseButton.x;
			const int mouseY = event.mouseButton.y;
			const sf::Vector2f shapePos = object.getPosition();
			const sf::Vector2f shapeSize = object.getSize();
			if (mouseX >= shapePos.x && mouseX <= shapePos.x + shapeSize.x)
			{
				if (mouseY >= shapePos.y && mouseY <= shapePos.y + shapeSize.y)
				{
					return 1;
				}
			}
		}
	}
	return 0;
}

/*
This class is for a block in the minefield grid.
*/

class Field
{
private:
	bool mine; // 1= mine, 0 = no mine
	bool revealed; // 1 = revealed, 0 = not revealed
	bool flag; // 1 = flagged, 0 = not flagged
public:
	Field()
	{
		mine = false;
		revealed = false;
		flag = false;
	}
	void setMine(bool mine)	{ Field::mine = mine; }
	bool isMine() { return mine; }
	void setRevealed(bool revealed)	{ Field::revealed = revealed; }
	bool isRevealed() { return revealed; }
	void setFlag(bool flag) { Field::flag = flag; }
	bool isFlagged() { return flag; }
};

class Minesweeper
{
private:
	bool firstMove; // whether or not the first move has been made.
	int rows, cols, mines; // number of rows, cols and mines in the grid
	Field* fields; //Array of the every block in the grid. 
	sf::Clock timer;
	int elapsedTime;
	bool gameFinished; // 1 = game finished.
	bool gameWon; // 1 = game has been won
	int flags; // number of flagged fields
	sf::RectangleShape* fieldsDisplay; // The array of shapes used to display each block in the grid.
	sf::Texture grassTexture; 
	sf::Texture soilTextures[9]; // different textures of the soil. One without a number, and others with possible numbers of surrounding mines.
	sf::RectangleShape road; 
	sf::Texture roadTexture;
	sf::RectangleShape bg; // the grass is actually displayed as a huge backround behind everything.
	sf::Texture flagTexture;
	sf::Texture mineTexture;
	sf::RectangleShape* flagsDisplay; // array of flags. Separate array was require because each field has it's own flag, and flags have different coordinates from fields to achieve 3d effect.
	/*
	Randomize mines so that first revealed field has zero mines around it. The mines are randomized in clusters.
	x, y are coordinates of the field that was clicked.
	*/
	void randomizeMines(int x, int y) 
	{
		int loc = 0;
		srand(time(NULL));
		int placed = 0;
		while (placed != mines)
		{
			loc = rand() % (rows*cols);
			int clusterProb = rand() % 7;
			if (!fields[loc].isMine() && !(x == loc / cols && y == loc % cols) && (clusterProb == 0 || getSurroundingMines(loc) > 0))
			{
				fields[loc].setMine(true);
				placed++;
				if (getSurroundingMines(y + x*cols) > 0)
				{
					fields[loc].setMine(false);
					placed--;
				}
			}
		}
	}
	/*
	index = the field whose surroudning mines are to be calculated.
	Returns the number of surrounding mines
	*/
	int getSurroundingMines(int index)
	{
		int count = 0;
		const int x = index / cols;
		const int y = (index % cols);

		//top 3
		if (x > 0)
		{
			if (y > 0 && y < cols - 1)
			{
				for (int i = -1; i < 2; i++)
				{
					if (fields[y + i + (x - 1)*cols].isMine())
						count++;
				}
			}
			else if (y == 0)
			{
				for (int i = 0; i < 2; i++)
				{
					if (fields[y + i + (x - 1)*cols].isMine())
						count++;
				}
			}
			else if (y == cols - 1)
			{
				for (int i = -1; i < 1; i++)
				{
					if (fields[y + i + (x - 1)*cols].isMine())
						count++;
				}
			}
		}

		//bottom 3
		if (x < rows - 1)
		{
			if (y > 0 && y < cols - 1)
			{
				for (int i = -1; i < 2; i++)
				{
					if (fields[y + i + (x + 1)*cols].isMine())
						count++;
				}
			}
			else if (y == 0)
			{
				for (int i = 0; i < 2; i++)
				{
					if (fields[y + i + (x + 1)*cols].isMine())
						count++;
				}
			}
			else if (y == cols - 1)
			{
				for (int i = -1; i < 1; i++)
				{
					if (fields[y + i + (x + 1)*cols].isMine())
						count++;
				}
			}
		}


		//mid 2
		if (y > 0 && y < cols - 1)
		{
			for (int i = -1; i < 2; i++)
			{
				if (fields[y + i + x*cols].isMine() && (y + i + x*cols) != index)
					count++;
			}
		}
		else if (y == 0)
		{
			for (int i = 0; i < 2; i++)
			{
				if (fields[y + i + x*cols].isMine() && (y + i + x*cols) != index)
					count++;
			}
		}
		else if (y == cols - 1)
		{
			for (int i = -1; i < 1; i++)
			{
				if (fields[y + i + x*cols].isMine() && (y + i + x*cols) != index)
					count++;
			}
		}
		return count;
	}
	//Similarly returns the number of surround flags for the field whose index is passed.
	int getSurroundingFlags(int index)
	{
		int count = 0;
		const int x = index / cols;
		const int y = (index % cols);

		//top 3
		if (x > 0)
		{
			if (y > 0 && y < cols - 1)
			{
				for (int i = -1; i < 2; i++)
				{
					if (fields[y + i + (x - 1)*cols].isFlagged())
						count++;
				}
			}
			else if (y == 0)
			{
				for (int i = 0; i < 2; i++)
				{
					if (fields[y + i + (x - 1)*cols].isFlagged())
						count++;
				}
			}
			else if (y == cols - 1)
			{
				for (int i = -1; i < 1; i++)
				{
					if (fields[y + i + (x - 1)*cols].isFlagged())
						count++;
				}
			}
		}

		//bottom 3
		if (x < rows - 1)
		{
			if (y > 0 && y < cols - 1)
			{
				for (int i = -1; i < 2; i++)
				{
					if (fields[y + i + (x + 1)*cols].isFlagged())
						count++;
				}
			}
			else if (y == 0)
			{
				for (int i = 0; i < 2; i++)
				{
					if (fields[y + i + (x + 1)*cols].isFlagged())
						count++;
				}
			}
			else if (y == cols - 1)
			{
				for (int i = -1; i < 1; i++)
				{
					if (fields[y + i + (x + 1)*cols].isFlagged())
						count++;
				}
			}
		}


		//mid 2
		if (y > 0 && y < cols - 1)
		{
			for (int i = -1; i < 2; i++)
			{
				if (fields[y + i + x*cols].isFlagged() && (y + i + x*cols) != index)
					count++;
			}
		}
		else if (y == 0)
		{
			for (int i = 0; i < 2; i++)
			{
				if (fields[y + i + x*cols].isFlagged() && (y + i + x*cols) != index)
					count++;
			}
		}
		else if (y == cols - 1)
		{
			for (int i = -1; i < 1; i++)
			{
				if (fields[y + i + x*cols].isFlagged() && (y + i + x*cols) != index)
					count++;
			}
		}
		return count;
	}

	/*
	This is a recursive algorithm. It reveals the grid if a block is clicked in the grid. It continues revealing blocks in all direction 
	as long as it doesn’t reach a block that has at least one surrounding mine. If an already revealed block is clicked and the number of 
	flags surrounding it equals the number of mines surrounding it, the same recursive algorithm is used to reveal all 
	unrevealed and unflagged blocks surrounding it.
	*/
	void reveal(int index)
	{
		const int x = index / cols;
		const int y = (index % cols);
		if (!fields[index].isRevealed())
		{
			fields[index].setRevealed(true);
			if (getSurroundingMines(index) != 0)
				return;
			else
			{
				//top 3
				if (x > 0)
				{
					if (y > 0 && y < cols - 1)
					{
						for (int i = -1; i < 2; i++)
						{
							if (!fields[y + i + (x - 1)*cols].isRevealed() && !fields[y + i + (x - 1)*cols].isFlagged())
								reveal(y + i + (x - 1)*cols);
						}
					}
					else if (y == 0)
					{
						for (int i = 0; i < 2; i++)
						{
							if (!fields[y + i + (x - 1)*cols].isRevealed() && !fields[y + i + (x - 1)*cols].isFlagged())
								reveal(y + i + (x - 1)*cols);
						}
					}
					else if (y == cols - 1)
					{
						for (int i = -1; i < 1; i++)
						{
							if (!fields[y + i + (x - 1)*cols].isRevealed() && !fields[y + i + (x - 1)*cols].isFlagged())
								reveal(y + i + (x - 1)*cols);
						}
					}
				}

				//bottom 3
				if (x < rows - 1)
				{
					if (y > 0 && y < cols - 1)
					{
						for (int i = -1; i < 2; i++)
						{
							if (!fields[y + i + (x + 1)*cols].isRevealed() && !fields[y + i + (x + 1)*cols].isFlagged())
								reveal(y + i + (x + 1)*cols);
						}
					}
					else if (y == 0)
					{
						for (int i = 0; i < 2; i++)
						{
							if (!fields[y + i + (x + 1)*cols].isRevealed() && !fields[y + i + (x + 1)*cols].isFlagged())
								reveal(y + i + (x + 1)*cols);
						}
					}
					else if (y == cols - 1)
					{
						for (int i = -1; i < 1; i++)
						{
							if (!fields[y + i + (x + 1)*cols].isRevealed() && !fields[y + i + (x + 1)*cols].isFlagged())
								reveal(y + i + (x + 1)*cols);
						}
					}
				}


				//mid 2
				if (y > 0 && y < cols - 1)
				{
					for (int i = -1; i < 2; i++)
					{
						if ((y + i + x*cols) != index && !fields[y + i + x*cols].isRevealed() && !fields[y + i + x*cols].isFlagged())
							reveal(y + i + x*cols);
					}
				}
				else if (y == 0)
				{
					for (int i = 0; i < 2; i++)
					{
						if ((y + i + x*cols) != index && !fields[y + i + x*cols].isRevealed() && !fields[y + i + x*cols].isFlagged())
							reveal(y + i + x*cols);
					}
				}
				else if (y == cols - 1)
				{
					for (int i = -1; i < 1; i++)
					{
						if ((y + i + x*cols) != index && !fields[y + i + x*cols].isRevealed() && !fields[y + i + x*cols].isFlagged())
							reveal(y + i + x*cols);
					}
				}
			}
		}
		else if (getSurroundingFlags(index) == getSurroundingMines(index))     // IF FIELD IS ALREADY REVEALED
		{
			//top 3
			if (x > 0)
			{
				if (y > 0 && y < cols - 1)
				{
					for (int i = -1; i < 2; i++)
					{
						if (!fields[y + i + (x - 1)*cols].isRevealed() && !fields[y + i + (x - 1)*cols].isFlagged())
							reveal(y + i + (x - 1)*cols);
					}
				}
				else if (y == 0)
				{
					for (int i = 0; i < 2; i++)
					{
						if (!fields[y + i + (x - 1)*cols].isRevealed() && !fields[y + i + (x - 1)*cols].isFlagged())
							reveal(y + i + (x - 1)*cols);
					}
				}
				else if (y == cols - 1)
				{
					for (int i = -1; i < 1; i++)
					{
						if (!fields[y + i + (x - 1)*cols].isRevealed() && !fields[y + i + (x - 1)*cols].isFlagged())
							reveal(y + i + (x - 1)*cols);
					}
				}
			}

			//bottom 3
			if (x < rows - 1)
			{
				if (y > 0 && y < cols - 1)
				{
					for (int i = -1; i < 2; i++)
					{
						if (!fields[y + i + (x + 1)*cols].isRevealed() && !fields[y + i + (x + 1)*cols].isFlagged())
							reveal(y + i + (x + 1)*cols);
					}
				}
				else if (y == 0)
				{
					for (int i = 0; i < 2; i++)
					{
						if (!fields[y + i + (x + 1)*cols].isRevealed() && !fields[y + i + (x + 1)*cols].isFlagged())
							reveal(y + i + (x + 1)*cols);
					}
				}
				else if (y == cols - 1)
				{
					for (int i = -1; i < 1; i++)
					{
						if (!fields[y + i + (x + 1)*cols].isRevealed() && !fields[y + i + (x + 1)*cols].isFlagged())
							reveal(y + i + (x + 1)*cols);
					}
				}
			}


			//mid 2
			if (y > 0 && y < cols - 1)
			{
				for (int i = -1; i < 2; i++)
				{
					if ((y + i + x*cols) != index && !fields[y + i + x*cols].isRevealed() && !fields[y + i + x*cols].isFlagged())
						reveal(y + i + x*cols);
				}
			}
			else if (y == 0)
			{
				for (int i = 0; i < 2; i++)
				{
					if ((y + i + x*cols) != index && !fields[y + i + x*cols].isRevealed() && !fields[y + i + x*cols].isFlagged())
						reveal(y + i + x*cols);
				}
			}
			else if (y == cols - 1)
			{
				for (int i = -1; i < 1; i++)
				{
					if ((y + i + x*cols) != index && !fields[y + i + x*cols].isRevealed() && !fields[y + i + x*cols].isFlagged())
						reveal(y + i + x*cols);
				}
			}
		}
	}
	// Checks if game is won. 1 = won, 0 = not won
	bool checkWon()
	{
		for (int i = 0; i < rows*cols; i++)
		{
			if (!fields[i].isFlagged() && !fields[i].isRevealed())
				return false;
		}
		if (mines - flags != 0)
			return false;
		return true;
	}
	// Checks if game is lost. 1 = lost, 0 = not lost
	bool checkLost()
	{
		for (int i = 0; i < rows*cols; i++)
		{
			if (fields[i].isMine() && fields[i].isRevealed())
				return true;
		}
		return false;
	}
public:
	Minesweeper(int rows, int cols)
	{
		fieldsDisplay = new sf::RectangleShape[rows*cols];
		flagsDisplay = new sf::RectangleShape[rows*cols];
		grassTexture.loadFromFile("resources/grass.png");
		grassTexture.setRepeated(true);
		roadTexture.loadFromFile("resources/road.png");
		roadTexture.setRepeated(true);
		flagTexture.loadFromFile("resources/flag.png");
		mineTexture.loadFromFile("resources/mine.png");
		for (int i = 0; i < 9; i++)
		{
			soilTextures[i].loadFromFile("resources/soil" + std::to_string(i) + ".png");
		}

		firstMove = true;
		Minesweeper::rows = rows;
		Minesweeper::cols = cols;
		mines = (rows*cols) / 5;
		gameFinished = false;
		gameWon = false;
		flags = 0;
		fields = new Field[rows*cols];
	}
	// return 1 if game is won.
	bool isWon() { return gameWon; } 
	// returns zero if game is lost.
	bool isLost()
	{
		if (gameFinished && !gameWon)
			return true;
		else
			return false;
	} 
	
	/*
	This fucntion draws the entire GUI at the coordinates passed in the parameter gridPos.
	font = the font to be used for text drawing.
	window = handle of the window where it is supposed to be drawn
	fieldSize = fieldSize to be used, this determines the zoom level of the display	
	*/
	void drawGrid(sf::RenderWindow &window, const int fieldSize, const sf::Vector2f gridPos, sf::Font &font)
	{
		bg.setSize(sf::Vector2f(window.getSize().x, window.getSize().y));
		bg.setPosition(0, 0);
		bg.setTexture(&grassTexture);
		bg.setTextureRect(sf::IntRect(0, 0, bg.getSize().x, bg.getSize().y));
		window.draw(bg);

		

		for (int i = 0; i < rows*cols; i++)
		{
			const int x = i / cols;
			const int y = i % cols;
			fieldsDisplay[i].setSize(sf::Vector2f(fieldSize, fieldSize));
			fieldsDisplay[i].setPosition(sf::Vector2f(gridPos.x + y * fieldSize, gridPos.y + x * fieldSize));
			fieldsDisplay[i].setOutlineThickness(0);
			fieldsDisplay[i].setOutlineColor(sf::Color::Green);
			fieldsDisplay[i].setFillColor(sf::Color::Transparent);

			flagsDisplay[i].setSize(sf::Vector2f(fieldSize, fieldSize));
			flagsDisplay[i].setPosition(sf::Vector2f(fieldsDisplay[i].getPosition().x + fieldSize / 2, fieldsDisplay[i].getPosition().y - fieldSize / 2));
			flagsDisplay[i].setTexture(&flagTexture);
			flagsDisplay[i].setFillColor(sf::Color::Transparent);
		}

		for (int i = 0; i < rows*cols; i++)
		{
			if (fields[i].isRevealed())
			{
				if (fields[i].isMine())
				{
					fieldsDisplay[i].setFillColor(sf::Color::White);
					fieldsDisplay[i].setTexture(&mineTexture);
				}
				else
				{
					fieldsDisplay[i].setTexture(&soilTextures[getSurroundingMines(i)]);
					fieldsDisplay[i].setFillColor(sf::Color::White);
				}
			}
			else if (fields[i].isFlagged())
			{
				flagsDisplay[i].setFillColor(sf::Color::White);
			}
			else if (gameFinished && !gameWon)
			{
				if (fields[i].isMine() && !fields[i].isFlagged())
				{
					fieldsDisplay[i].setFillColor(sf::Color::White);
					fieldsDisplay[i].setTexture(&mineTexture);
				}
				if (fields[i].isFlagged() && !fields[i].isMine())
					fieldsDisplay[i].setFillColor(sf::Color::Red);
			}

			window.draw(fieldsDisplay[i]);
			window.draw(flagsDisplay[i]);
		}

		road.setSize(sf::Vector2f(2*ROAD_SIZE*fieldSize, 1.4*ROAD_SIZE*fieldSize));
		for (int i = 0; i < window.getSize().x / (2*ROAD_SIZE*fieldSize); i++)
		{
			road.setPosition(gridPos.x + (2*i*fieldSize*ROAD_SIZE), rows*fieldSize + gridPos.y - (0.4*ROAD_SIZE*fieldSize));
			road.setTexture(&roadTexture);
			window.draw(road);
		}


		if (!firstMove)
		{
			sf::Text timeDisplay("", font, 30);
			if (!gameFinished)
			{
				elapsedTime = (int)timer.getElapsedTime().asSeconds();
				timeDisplay.setString(std::string("Mines remaining: " + std::to_string(mines-flags) +"\n Time: " + std::to_string(elapsedTime) + " seconds"));
				timeDisplay.setOrigin(120, 20);
			}
			else
			{
				if (gameWon)
				{
					timeDisplay.setString(std::string("You Won! Time: " + std::to_string(elapsedTime) + " seconds"));
					timeDisplay.setOrigin(180, 20);
				}
				else
				{
					timeDisplay.setString(std::string("You Lost!"));
					timeDisplay.setOrigin(50, 20);
				}
			}
			timeDisplay.setPosition(window.getSize().x / 2, road.getPosition().y + road.getSize().y/1.5 );
			timeDisplay.setColor(sf::Color::Black);
			/*sf::RectangleShape dot(sf::Vector2f(5, 5));
			dot.setPosition(timeDisplay.getPosition());
			dot.setFillColor(sf::Color::White);
			window.draw(dot);*/
			window.draw(timeDisplay);
		}
	}

	/*
	This function is the main public function that is used to basically manipulate the minesweeper grid.
	This functin plays out a move in the game. The x and y coordinates determine the block in the grid.
	If 1 is passed in flag, it flags the block at x,y. Otherwise it reveals the block at x,y.
	*/
	void playMove(const int x, const int y, bool flag)
	{
		if (firstMove)
			timer.restart();
		if (!flag) //If field is to be revealed.
		{
			if (firstMove)
			{
				randomizeMines(x, y);
				firstMove = false;
			}
			if (!fields[y + x*cols].isFlagged())
			{
				reveal(y + x*cols);
				if (checkLost())
					gameFinished = true;
				else if (checkWon())
				{
					gameWon = true;
					gameFinished = true;
				}
			}
		}
		else if (!fields[y + x*cols].isRevealed()) //If field is to be flagged
		{
			if (fields[y + x*cols].isFlagged())
			{
				fields[y + x*cols].setFlag(false);
				flags--;
			}
			else
			{
				fields[y + x*cols].setFlag(true);
				flags++;
			}
			if (checkWon())
			{
				gameWon = true;
				gameFinished = true;
			}
		}
	}

	~Minesweeper()
	{
		delete[rows*cols] fields;
		delete[rows*cols] fieldsDisplay;
		delete[rows*cols] flagsDisplay;
	}
};

/*
Main function that calls the Minesweeper draw function. Calculations of windows size and zoom level are made in this fucntion and then passed.
font = the font to be used for text drawing.
window = handle of the window where it is supposed to be drawn
rows and cols = number of rows and columns in the grid
returns the game state so that the game can return to the main menu after it is over
*/
int playGame(sf::RenderWindow &window, const int rows, const int cols, sf::Font &font)
{
	Minesweeper game(rows, cols);
	int fieldSize = MAX_WINDOW_HEIGHT / (rows + ROAD_SIZE);
	if (fieldSize*cols > MAX_WINDOW_WIDTH)
		fieldSize = MAX_WINDOW_WIDTH / cols;
	window.setPosition(sf::Vector2i(0, 0));
	window.setSize(sf::Vector2u(cols*fieldSize, fieldSize*(rows + ROAD_SIZE)));
	window.setView(sf::View(sf::FloatRect(0, 0, cols*fieldSize, fieldSize*(rows + ROAD_SIZE))));

	// To be used for displaying image of instructions for the instructions screen
	while (window.isOpen())
	{
		window.clear();
		game.drawGrid(window, fieldSize, sf::Vector2f(0, 0), font);
		window.display();
		sf::Event event;
		if (!game.isWon() && !game.isLost())
		{
			if (window.pollEvent(event))
			{
				if (event.type == sf::Event::Closed)
					window.close();
				if (event.type == sf::Event::MouseButtonPressed)
				{
					const float fieldX = (event.mouseButton.y - 1) / fieldSize;
					const float fieldY = (event.mouseButton.x - 1) / fieldSize;
					if (fieldX >= 0 && fieldX < rows && fieldY >= 0 && fieldY < cols)
					{
						if (event.mouseButton.button == sf::Mouse::Left)
							game.playMove((int)fieldX, (int)fieldY, 0);
						else if (event.mouseButton.button == sf::Mouse::Right)
							game.playMove((int)fieldX, (int)fieldY, 1);
					}
				}
			}
		}
		else
		{
			if (window.pollEvent(event))
			{
				if (event.type == sf::Event::Closed)
					window.close();
				if (event.type == sf::Event::MouseButtonPressed)
				{
					window.setSize(sf::Vector2u(500, 600));
					window.setView(window.getDefaultView());
					window.setPosition(sf::Vector2i(50, 50));
					return 1;
				}
			}
		}
	}
}


/* 
Code for the screen where user selects the number of columns
font = the font to be used for text drawing.
window = handle of the window where it is supposed to be drawn
cols = number of columns in the grid
t1,t2,t3,bg = textures for graphics
returns the new game state when selection is made
*/
int selectCols(sf::RenderWindow &window, int &cols, sf::Font &font, sf::Texture &t1, sf::Texture &t2, sf::Texture &t3, sf::Texture &bg)
{
	// Background image
	sf::RectangleShape background(sf::Vector2f(500, 600));
	background.setTexture(&bg);

	sf::Text heading("Select the number of columns:", font, 30);
	heading.setPosition(sf::Vector2f(50, 50));
	heading.setColor(sf::Color::White);

	// To be used for displaying image of instructions for the instructions screen
	sf::RectangleShape button1(sf::Vector2f(200, 50));
	button1.setPosition(sf::Vector2f(window.getSize().x / 2 - (200 / 2), 200));
	// To be used for displaying image of instructions for the instructions screen
	sf::RectangleShape button2(sf::Vector2f(200, 50));
	button2.setPosition(sf::Vector2f(window.getSize().x / 2 - (200 / 2), 330));
	// To be used for displaying image of instructions for the instructions screen
	sf::RectangleShape button3(sf::Vector2f(200, 50));
	button3.setPosition(sf::Vector2f(window.getSize().x / 2 - (200 / 2), 460));

	button1.setTexture(&t1);
	button2.setTexture(&t2);
	button3.setTexture(&t3);

	window.clear();
	window.draw(background);
	window.draw(button1);
	window.draw(button2);
	window.draw(button3);
	window.draw(heading);
	window.display();
	sf::Event event;
	while (window.pollEvent(event))
	{
		if (event.type == sf::Event::Closed)
			window.close();
		if (isLeftClicked(window, button1, event))
		{
			cols = 15;
			return 5;
		}
		else if (isLeftClicked(window, button2, event))
		{
			cols = 20;
			return 5;
		}
		else if (isLeftClicked(window, button3, event))
		{
			cols = 30;
			return 5;
		}
	}
	return 4;
}
/*
Code for the screen where user selects the number of rows
font = the font to be used for text drawing.
window = handle of the window where it is supposed to be drawn
row = number of rows in the grid
t1,t2,t3,bg = textures for graphics
returns the new game state when selection is made
*/
int selectRows(sf::RenderWindow &window, int &rows, sf::Font &font, sf::Texture &t1, sf::Texture &t2, sf::Texture &t3, sf::Texture &bg)
{
	// Background image
	sf::RectangleShape background(sf::Vector2f(500, 600));
	background.setTexture(&bg);

	sf::Text heading("Select the number of rows:", font, 30);
	heading.setPosition(sf::Vector2f(50, 50));
	heading.setColor(sf::Color::White);

	// To be used for displaying image of instructions for the instructions screen
	sf::RectangleShape button1(sf::Vector2f(200, 50));
	button1.setPosition(sf::Vector2f(window.getSize().x / 2 - (200 / 2), 200));
	// To be used for displaying image of instructions for the instructions screen
	sf::RectangleShape button2(sf::Vector2f(200, 50));
	button2.setPosition(sf::Vector2f(window.getSize().x / 2 - (200 / 2), 330));
	// To be used for displaying image of instructions for the instructions screen
	sf::RectangleShape button3(sf::Vector2f(200, 50));
	button3.setPosition(sf::Vector2f(window.getSize().x / 2 - (200 / 2), 460));

	button1.setTexture(&t1);
	button2.setTexture(&t2);
	button3.setTexture(&t3);


	window.clear();
	window.draw(background);
	window.draw(button1);
	window.draw(button2);
	window.draw(button3);
	window.draw(heading);
	window.display();
	sf::Event event;
	while (window.pollEvent(event))
	{
		if (event.type == sf::Event::Closed)
			window.close();
		if (isLeftClicked(window, button1, event))
		{
			rows = 7;
			return 4;
		}
		else if (isLeftClicked(window, button2, event))
		{
			rows = 10;
			return 4;
		}
		else if (isLeftClicked(window, button3, event))
		{
			rows = 15;
			return 4;
		}
	}
	return 3;
}
/*
Code for help screen
font = the font to be used for text drawing.
window = handle of the window where it is supposed to be drawn
instruct, cont, bg = textures for graphics
*/
int helpScreen(sf::RenderWindow &window, sf::Font &font, sf::Texture instruct, sf::Texture cont, sf::Texture bg)
{
	// Background image
	sf::RectangleShape background(sf::Vector2f(500, 600));
	background.setTexture(&bg);

	// To be used for displaying image of instructions for the instructions screen
	sf::RectangleShape instructions(sf::Vector2f(350, 450));
	instructions.setPosition(sf::Vector2f(window.getSize().x / 2 - (350 / 2), 30));
	instructions.setTexture(&instruct);
	// Continue button for use on the help screen.
	sf::RectangleShape continueButton(sf::Vector2f(200, 50));
	continueButton.setPosition(sf::Vector2f(window.getSize().x / 2 - (200 / 2), 515));
	continueButton.setTexture(&cont);


	window.clear();
	window.draw(background);
	window.draw(continueButton);
	window.draw(instructions);
	window.display();
	sf::Event event;
	while (window.pollEvent(event))
	{
		if (event.type == sf::Event::Closed)
			window.close();
		if (isLeftClicked(window, continueButton, event))
			return 3;
	}
	return 2;
}
/*
Code for the first screen
window = handle of the window where it is supposed to be drawn
play, bg = textures for graphics
*/
int mainMenu(sf::RenderWindow &window, sf::Texture &play, sf::Texture &bg)
{
	// Background image
	sf::RectangleShape background(sf::Vector2f(500, 600));
	background.setTexture(&bg);

	// Play button for use on the main menu screen.
	sf::RectangleShape playButton(sf::Vector2f(200, 50));
	playButton.setPosition(sf::Vector2f(window.getSize().x / 2 - (200 / 2), 500));
	playButton.setTexture(&play);

	window.clear();
	window.draw(background);
	window.draw(playButton);
	window.display();
	sf::Event event;
	while (window.pollEvent(event))
	{
		if (event.type == sf::Event::Closed)
			window.close();
		if (isLeftClicked(window, playButton, event))
			return 2;
	}
	return 1;
}

int main()
{
	FreeConsole();
	/* Main game window */
	sf::RenderWindow window(sf::VideoMode(500, 600), "Minesweeper");
	window.setPosition(sf::Vector2i(50, 50));
	sf::Image icon;
	icon.loadFromFile("resources/icon.png");
	window.setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());
	window.setFramerateLimit(60);
	/* Font for drawing text */
	sf::Font font;
	if (!font.loadFromFile("resources/font.ttf"))
		return EXIT_FAILURE;

	//Textures for the menu screens

	sf::Texture bg;
	sf::Texture play;
	sf::Texture instruct;
	sf::Texture cont;
	sf::Texture seven;
	sf::Texture ten;
	sf::Texture fifteen;
	sf::Texture twenty;

	bg.loadFromFile("resources/menu bg.png");
	play.loadFromFile("resources/play.png");
	instruct.loadFromFile("resources/instructions.png");
	cont.loadFromFile("resources/continue.png");
	seven.loadFromFile("resources/7.png");
	ten.loadFromFile("resources/10.png");
	fifteen.loadFromFile("resources/15.png");
	twenty.loadFromFile("resources/20.png");
	

	/* gameState 1 = Main menu
	gameState 2 = Instructions
	gameState 3 = Select rows
	gameState 4 = Select cols
	gameState 5 = Play game
	*/
	int gameState = 1;

	/* To store number of rows and cols that will be selected by the player. */
	int rows = 0, cols = 0;

	while (window.isOpen())
	{
		switch (gameState)
		{
		case 1:
			gameState = mainMenu(window,play,bg);
			break;
		case 2:
			gameState = helpScreen(window,font,instruct,cont,bg);
			break;
		case 3:
			gameState = selectRows(window, rows, font,seven,ten,fifteen,bg);
			break;
		case 4:
			gameState = selectCols(window, cols, font,ten,fifteen,twenty,bg);
			break;
		case 5:
			gameState = playGame(window, rows, cols, font);
			break;
		}
	}

	return 0;
}
