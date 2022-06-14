#include <iostream>
#include <random>
#include <chrono>
#include <Windows.h>
//------------------------------------------------------------------
using namespace std;
//------------------------------------------------------------------
const HANDLE 	hStdOut				= GetStdHandle(STD_OUTPUT_HANDLE);
const short		cursorPosX_3x3[3]	= { 2, 6, 10 };
const short		cursorPosY_3x3[3]	= { 3, 5 ,7 };
uint16_t		computerScore		= 0;
uint16_t		humanScore			= 0;
uint8_t			tmpPosX				= 1;
uint8_t			tmpPosY				= 1;
//------------------------------------------------------------------
enum TCell : char {
	CROSS	= 'X',
	ZERO	= '0',
	EMPTY	= ' '
};

enum TProgress {
	IN_PROGRESS,
	WON_HUMAN,
	WON_AI,
	DRAW
};

struct TCoord {
	size_t	y = 0;
	size_t 	x = 0;
};

struct TGame {
	TCell** 		ppField		= nullptr; // our playing field
	const size_t 	SIZE		= 3;
	TProgress 		progress	= IN_PROGRESS;
	TCell 			human		= EMPTY,	ai	= EMPTY; //compiler warning fix
	size_t 			turn		= 0U;  // move number - even human, odd AI
};
//------------------------------------------------------------------
void __fastcall setCursorPos(short x, short y)
{
	COORD pos = { x, y };
	SetConsoleCursorPosition(hStdOut, pos);
}
//------------------------------------------------------------------
int32_t getRandomNum(int32_t min, int32_t max)
{
	const static auto seed = chrono::system_clock::now().time_since_epoch().count();
	static mt19937_64 generator(seed);
	uniform_int_distribution<int32_t> dis(min, max);
	return dis(generator);
}
//------------------------------------------------------------------
void __fastcall createGameField(TGame& g)
{
	// dynamically create our field
	g.ppField = new TCell * [g.SIZE];
	for (size_t i = 0; i < g.SIZE; i++)
	{
		g.ppField[i] = new TCell[g.SIZE];
	}
}
//------------------------------------------------------------------
void __fastcall deleteGameField(TGame& g)
{
	for (size_t i = 0; i < g.SIZE; i++)
	{
		delete[] g.ppField[i];
	}
	delete[] g.ppField;
}
//------------------------------------------------------------------
void __fastcall prepareNewGame(TGame& g)
{
	for (size_t y = 0; y < g.SIZE; y++)
	{
		for (size_t x = 0; x < g.SIZE; x++)
		{
			g.ppField[y][x] = EMPTY;
		}
	}

	if (getRandomNum(0, 1000) > 500)
	{
		g.human = CROSS; // human goes first
		g.ai = ZERO;
		g.turn = 0;
	}
	else
	{
		g.human = ZERO;
		g.ai = CROSS;  // computer goes first
		g.turn = 1;
	}
	tmpPosX = 1; tmpPosY = 1;
}
//------------------------------------------------------------------
void __fastcall drawGame(const TGame& g)
{
	size_t y, x;
	SetConsoleTextAttribute(hStdOut, FOREGROUND_INTENSITY);

	system("cls"); // Clear screen
	cout << static_cast<char>(17) << " - Left   ";
	cout << static_cast<char>(16) << " - Right   ";
	cout << static_cast<char>(30) << " - Up   ";
	cout << static_cast<char>(31) << " - Down     ";
	cout << "ESCAPE: exit   ";
	cout << "SPACE: place";
	cout << endl << endl << endl;

	SetConsoleTextAttribute(hStdOut, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
	for (y = 0; y < g.SIZE; y++)
	{
		//cout << "   ";
		cout << "  ";
		for (x = 0; x < g.SIZE - 1; x++)
		{
			cout << static_cast<char>(g.ppField[y][x]) << " | ";
		}
		cout << static_cast<char>(g.ppField[y][x]) << endl;

		if (y < g.SIZE - 1)
		{
			//cout << " ";
			for (size_t i = 0; i < g.SIZE; i++)
			{
				cout << " ---";
			}
			SetConsoleTextAttribute(hStdOut, FOREGROUND_INTENSITY);
			if (!y)
				cout << "           HUM " << humanScore;
			else
				cout << "            AI " << computerScore;
			SetConsoleTextAttribute(hStdOut, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
			cout << endl;
		}
	}
	cout << endl << endl;
	SetConsoleTextAttribute(hStdOut, FOREGROUND_INTENSITY);
	//cout << "Compu score: " << computerScore << endl 
	//	 << "Human score: " << humanScore << endl << endl;
	cout << "Human symbol (" << static_cast<char>(g.human) << ")   ";
}
//------------------------------------------------------------------
// did someone win
TProgress __fastcall getWon(const TGame & g)
{
	// iterate over the rows of the matrix
	for (size_t y = 0; y < g.SIZE; y++)
	{
		if (g.ppField[y][0] == g.ppField[y][1] && g.ppField[y][0] == g.ppField[y][2])
		{
			if (g.ppField[y][0] == g.human)
				return WON_HUMAN;

			if (g.ppField[y][0] == g.ai)
				return WON_AI;
		}
	}
	// iterate over the columns of the matrix
	for (size_t x = 0; x < g.SIZE; x++)
	{
		if (g.ppField[0][x] == g.ppField[1][x] && g.ppField[0][x] == g.ppField[2][x])
		{
			if (g.ppField[0][x] == g.human)
				return WON_HUMAN;

			if (g.ppField[0][x] == g.ai)
				return WON_AI;
		}
	}
	// checking the diagonals
	if (g.ppField[0][0] == g.ppField[1][1] && g.ppField[0][0] == g.ppField[2][2])
	{
		if (g.ppField[1][1] == g.human)
			return WON_HUMAN;

		if (g.ppField[1][1] == g.ai)
			return WON_AI;
	}

	if (g.ppField[0][2] == g.ppField[1][1] && g.ppField[2][0] == g.ppField[1][1])
	{
		if (g.ppField[1][1] == g.human)
			return WON_HUMAN;

		if (g.ppField[1][1] == g.ai)
			return WON_AI;
	}
	// checking for a draw
	bool draw = true;
	for (size_t y = 0; y < g.SIZE; y++)
	{
		for (size_t x = 0; x < g.SIZE; x++)
		{
			if (g.ppField[y][x] == EMPTY)
			{
				draw = false;
				break;
			}
		}
		if (!draw)
			break;
	}
	if (draw)
		return DRAW;

	return IN_PROGRESS;
}
//------------------------------------------------------------------
inline void setPosAndSleep() { // for getHumanCoord()
	setCursorPos(cursorPosX_3x3[tmpPosX], cursorPosY_3x3[tmpPosY]);
	Sleep(200);
}
// get the coordinates where the human goes
TCoord __fastcall getHumanCoord(TGame & g)  //we will also process the exit from the application here
{
	TCoord c;
	setCursorPos(cursorPosX_3x3[tmpPosX], cursorPosY_3x3[tmpPosY]);
	FlushConsoleInputBuffer(hStdOut);

	do {
		if (GetAsyncKeyState(VK_ESCAPE)) { // Exit Game
			deleteGameField(g);
			setCursorPos(1, 12);
			exit(0);
		}

		if (((GetAsyncKeyState(VK_UP)) || (GetAsyncKeyState(0x57))) && (tmpPosY > 0)) {				//0x26 Up		//0x57 W
			tmpPosY--;
			setPosAndSleep();
		}
		else if (((GetAsyncKeyState(VK_LEFT)) || (GetAsyncKeyState(0x41))) && (tmpPosX > 0)) {		//0x25 Left		//0x41 A
			tmpPosX--;
			setPosAndSleep();
		}
		else if (((GetAsyncKeyState(VK_DOWN)) || (GetAsyncKeyState(0x53))) && (tmpPosY < 2)) {		//0x28 Down		//0x53 S
			tmpPosY++;
			setPosAndSleep();
		}
		else if (((GetAsyncKeyState(VK_RIGHT)) || (GetAsyncKeyState(0x44))) && (tmpPosX < 2)) {		//0x27 Right	//0x44 D
			tmpPosX++;
			setPosAndSleep();
		}

		if ((GetAsyncKeyState(VK_SPACE)) && (g.ppField[tmpPosY][tmpPosX] == EMPTY)) {	//32 Space
			c.x = tmpPosX;
			c.y = tmpPosY;
			return c;
		}  // space pressed and cell free
	} while (true);
}
//------------------------------------------------------------------
// get the coordinates where the PC goes
TCoord __fastcall getAICoord(TGame & g)
{
	// 1. Pre win situation
	for (size_t y = 0; y < g.SIZE; y++)
	{
		for (size_t x = 0; x < g.SIZE; x++)
		{
			if (g.ppField[y][x] == EMPTY)
			{
				g.ppField[y][x] = g.ai;
				if (getWon(g) == WON_AI)
				{
					g.ppField[y][x] = EMPTY;
					return { y, x };
				}
				g.ppField[y][x] = EMPTY;
			}
		}
	}
	// 2. Pre fail situation //Makes it difficult to person victory
	for (size_t y = 0; y < g.SIZE; y++)
	{
		for (size_t x = 0; x < g.SIZE; x++)
		{
			if (g.ppField[y][x] == EMPTY)
			{
				g.ppField[y][x] = g.human;
				if (getWon(g) == WON_HUMAN)
				{
					g.ppField[y][x] = EMPTY;
					return { y, x };
				}
				g.ppField[y][x] = EMPTY;
			}
		}
	}

StartRndAndPriority:  // GoTo this position if the last time free cells were skipped due to randomness
	// 3. priority + ramdon
	int rnd = getRandomNum(0, 1000);
	if ((g.ppField[1][1] == EMPTY) && (rnd > 333))  // 2 of 3 chance
	{
		return { 1, 1 };
	}

	TCoord buf[4];
	size_t num = 0;

	rnd = getRandomNum(0, 1000);
	if (rnd > 333)
	{
		// checking free corners
		if (g.ppField[0][0] == EMPTY)
		{
			buf[num] = { 0, 0 };
			num++;
		}
		if (g.ppField[2][2] == EMPTY)
		{
			buf[num] = { 2, 2 };
			num++;
		}
		if (g.ppField[2][0] == EMPTY)
		{
			buf[num] = { 2, 0 };
			num++;
		}
		if (g.ppField[0][2] == EMPTY)
		{
			buf[num] = { 0, 2 };
			num++;
		}
		if (num > 0)
		{
			const size_t index = getRandomNum(0, 1000) % num; // choose a random corner
			return buf[index];
		}
	}
	// checking free NO corners
	if (g.ppField[1][0] == EMPTY)
	{
		buf[num] = { 1, 0 };
		num++;
	}
	if (g.ppField[1][2] == EMPTY)
	{
		buf[num] = { 1, 2 };
		num++;
	}
	if (g.ppField[0][1] == EMPTY)
	{
		buf[num] = { 0, 1 };
		num++;
	}
	if (g.ppField[2][1] == EMPTY)
	{
		buf[num] = { 2, 1 };
		num++;
	}
	if (num > 0)
	{
		const size_t index = getRandomNum(0, 1000) % num; // choose a random corner
		return buf[index];
	}
	goto StartRndAndPriority;
	//return { 1, 1 }; //Fix compiler warning// not importent
}
//------------------------------------------------------------------
void __fastcall congrats(const TGame & g)
{
	//SetConsoleTextAttribute(hStdOut, FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
	if (g.progress == WON_HUMAN)
	{
		SetConsoleTextAttribute(hStdOut, FOREGROUND_GREEN);
		cout << " HUMAN WIN !!!" << endl;
		humanScore++;
	}
	else if (g.progress == WON_AI)
	{
		SetConsoleTextAttribute(hStdOut, FOREGROUND_RED);
		cout << " COMPUTER WIN" << endl;
		computerScore++;
	}
	else
	{
		SetConsoleTextAttribute(hStdOut, FOREGROUND_RED | FOREGROUND_GREEN);
		cout << "     DRAW" << endl;
	}
	SetConsoleTextAttribute(hStdOut, FOREGROUND_INTENSITY);
}
//------------------------------------------------------------------
void showLogo() {
	for (uint8_t i = 0; i < 8; i++)
	{
		system("cls");
		for (uint8_t t = 0; t < i; t++)
		{
			cout << endl;
		}
		cout
			<< "      _____   _            _____                    _____               " << endl
			<< "     |_   _| (_)   ___    |_   _|   __ _    ___    |_   _|   ___     ___ " << endl
			<< "       | |   | |  / __|     | |    / _' |  / __|     | |    / _ \\   / _ \\" << endl
			<< "       | |   | | | (__      | |   | (_| | | (__      | |   | (_) | |  __/" << endl
			<< "       |_|   |_|  \\___|     |_|    \\__,_|  \\___|     |_|    \\___/   \\___|" << endl;

		Sleep(10);
	}
	Sleep(3000);
}
//==================================================================
int main() {
	TGame g;

	showLogo();

	createGameField(g);

	while (true)
	{
		prepareNewGame(g);
		drawGame(g);

		do {
			if (g.turn % 2 == 0)
			{
				//human's move
				TCoord c = getHumanCoord(g); // or exit
				g.ppField[c.y][c.x] = g.human;
			}
			else
			{
				//computers move
				TCoord c = getAICoord(g);
				g.ppField[c.y][c.x] = g.ai;
			}

			drawGame(g);
			g.turn++;
			g.progress = getWon(g);

		} while (g.progress == IN_PROGRESS);

		congrats(g);

		setCursorPos(cursorPosX_3x3[tmpPosX], cursorPosY_3x3[tmpPosY]);
		Sleep(1500);
	}

	deleteGameField(g);

	return 0;
}
//==================================================================