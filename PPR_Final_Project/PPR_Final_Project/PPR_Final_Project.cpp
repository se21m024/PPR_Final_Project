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

int coresOnMachine = thread::hardware_concurrency();
int numThreads = 2; // or set to coresOnMachine if you wish

const int boardWidth = 60;
const int boardHeight = 20;

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

void calcNewBoardState(bool oldBoard[boardWidth][boardHeight], bool newBoard[boardWidth][boardHeight])
{
	for (int y = 0; y < boardHeight; y++)
	{
		for (int x = 0; x < boardWidth; x++)
		{
			// for demo: just invert everything
			newBoard[x][y] = !oldBoard[x][y];
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

void printBoard(bool board[boardWidth][boardHeight])
{
	// Reset cursor
	SetCursorPos(0, 0);

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
			// for demo: just randomly set every x-th dot alive
			int r = rand() % 10;
			board[x][y] = r == 0;
		}
	}
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

	initBoardState(board);
	printBoard(board);

	cout << endl << "Press any button to continue..." << endl;
	cin.get();

	evolveBoard(board);
	printBoard(board);

	cout << "Press any button to quit programm..." << endl;
	cin.get();

	return 0;
}

