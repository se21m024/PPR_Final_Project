/*
	Parallel Programming

	Final Project:
	Game of Life

	Student 1: Thomas Bründl
	Student 2: Thomas Stummer
*/

#include <stdlib.h>
#include <iostream>
#include <omp.h>
#include <stdio.h>
#include <chrono>
#include <vector>
#include <string>
#include <thread>

//#include <WinUser.h>

#include <cstdlib>
//#include <unistd.h>
#include <fstream>
#include <sstream>

#include <windows.h>
#include <wincon.h>

using namespace std::chrono;
using namespace std;

/**** Constants ****/

const int coresOnMachine = thread::hardware_concurrency();
const int numThreads = 2; // or set to coresOnMachine if you wish

const int boardWidth = 100;
const int boardHeight = 40;

// e.g. 5 -> change for a cell to be initially living is 1/5
const int fractionOfInitiallyLivingCells = 3;

/**** Defines ****/

//#define SET_COLOR_RED SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED)
//#define SET_COLOR_BLUE SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE)

/**** Methods ****/

void copyBoard(bool source[boardWidth][boardHeight], bool target[boardWidth][boardHeight])
{
	for (int y = 0; y < boardHeight; y++)
	{
		for (int x = 0; x < boardWidth; x++)
		{
			target[x][y] = source[x][y];
		}
	}
}

bool calcNewCellState(bool board[boardWidth][boardHeight], int cellX, int cellY)
{
	int numLivingNeighbours = 0;

	int leftLimit = cellX - 1 < 0 ? 0 : cellX - 1;
	int rightLimit = cellX + 1 > boardWidth ? boardWidth : cellX + 1;
	int topLimit = cellY - 1 < 0 ? 0 : cellY - 1;
	int bottomLimit = cellY + 1 > boardHeight ? boardHeight : cellY + 1;

	for (int y = topLimit; y < bottomLimit; y++)
	{
		for (int x = leftLimit; x < rightLimit; x++)
		{
			if (board[x][y])
			{
				numLivingNeighbours++;
			}
		}
	}

	bool currentState = board[cellX][cellY];

	// Currently alive
	if (currentState)
	{
		return numLivingNeighbours == 2 || numLivingNeighbours == 3;
	}
	
	// Currently dead
	return numLivingNeighbours == 3;
}

void calcNewBoardState(bool oldBoard[boardWidth][boardHeight], bool newBoard[boardWidth][boardHeight])
{
	for (int y = 0; y < boardHeight; y++)
	{
		for (int x = 0; x < boardWidth; x++)
		{
			newBoard[x][y] = calcNewCellState(oldBoard, x, y);
		}
	}

	copyBoard(newBoard, oldBoard);
}

void evolveBoard(bool board[boardWidth][boardHeight])
{
	// Todo: check if this produces a memory leak
	bool tempBoard[boardWidth][boardHeight] = {};
	
	calcNewBoardState(board, tempBoard);
	copyBoard(tempBoard, board);
}

void printBoard(bool board[boardWidth][boardHeight], int iteration)
{
	// Reset cursor
	SetCursorPos(0, 0);

	cout << to_string(iteration) << ". iteration" << endl << endl;

	for (int y = 0; y < boardHeight; y++)
	{
		for (int x = 0; x < boardWidth; x++)
		{
			if (board[x][y])
			{
				cout << "O";
			}
			else
			{
				cout << "-";
			}
		}

		cout << endl;
	}
}

void initBoardState(bool board[boardWidth][boardHeight])
{
	for (int y = 0; y < boardHeight; y++)
	{
		for (int x = 0; x < boardWidth; x++)
		{
			// Randomly set cells alive due to given percentage
			int r = rand() % fractionOfInitiallyLivingCells;
			board[x][y] = r == 0;
		}
	}
}

void clearConsole()
{
	COORD topLeft = { 0, 0 };
	HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO screen;
	DWORD written;

	GetConsoleScreenBufferInfo(console, &screen);
	FillConsoleOutputCharacterA(
		console, ' ', screen.dwSize.X * screen.dwSize.Y, topLeft, &written
	);
	FillConsoleOutputAttribute(
		console, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE,
		screen.dwSize.X * screen.dwSize.Y, topLeft, &written
	);
	SetConsoleCursorPosition(console, topLeft);
}

int main()
{
	// Todo: use std::array or std::vector instead of C style array
	// Todo (optinal): import initial state from file
	// ------------------------------------------------------------
	// Docu:
	// True  -> Alive
	// False -> Dead
	bool board[boardWidth][boardHeight] = {};

	int iteration = 0;

	initBoardState(board);
	printBoard(board, iteration++);

	while (true)
	{
		cout << endl << "Press any button to continue..." << endl;
		cin.get();

		evolveBoard(board);
		clearConsole();
		printBoard(board, iteration++);
	}

	cout << "Press any button to quit programm..." << endl;
	cin.get();

	return 0;
}

