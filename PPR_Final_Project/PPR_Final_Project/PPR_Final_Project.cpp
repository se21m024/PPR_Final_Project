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
#include <random>
#include <cstdlib>
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
const int fractionOfInitiallyLivingCells = 5;

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

// Returns the microseconds that were necessary to calculate the new board state
long long calcNewBoardState(bool oldBoard[boardWidth][boardHeight], bool newBoard[boardWidth][boardHeight])
{
	auto startTimeStamp = high_resolution_clock::now();

	for (int y = 0; y < boardHeight; y++)
	{
		for (int x = 0; x < boardWidth; x++)
		{
			newBoard[x][y] = calcNewCellState(oldBoard, x, y);
		}
	}

	copyBoard(newBoard, oldBoard);

	auto stopTimeStamp = high_resolution_clock::now();
	return duration_cast<microseconds>(stopTimeStamp - startTimeStamp).count();
}

// Returns the microseconds that were necessary to calculate the new board state
long long evolveBoard(bool board[boardWidth][boardHeight])
{
	// Todo: check if this arry is deallocated automatically
	bool tempBoard[boardWidth][boardHeight] = {};
	
	auto calcDuration = calcNewBoardState(board, tempBoard);
	copyBoard(tempBoard, board);

	return calcDuration;
}

void printHeadline(int iteration, long long microSecondsToCalcLastIteration)
{
	// Reset cursor
	SetCursorPos(0, 0);

	cout << to_string(iteration) << ". iteration" << endl;
	cout << "The calculation of this iteration took " << to_string(microSecondsToCalcLastIteration) << " microseconds" << endl << endl;
}

void printBoard(bool board[boardWidth][boardHeight])
{
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

void printBoardAndHeadline(bool board[boardWidth][boardHeight], int iteration, long long microSecondsToCalcLastIteration)
{
	printHeadline(iteration, microSecondsToCalcLastIteration);
	printBoard(board);
}

void initBoardState(bool board[boardWidth][boardHeight])
{
	random_device dev;
	mt19937 rng(dev());
	uniform_int_distribution<mt19937::result_type> dist(0, fractionOfInitiallyLivingCells);

	for (int y = 0; y < boardHeight; y++)
	{
		for (int x = 0; x < boardWidth; x++)
		{
			// Randomly set cells alive due to given probability
			board[x][y] = (dist(rng) % fractionOfInitiallyLivingCells) == 0;
		}
	}
}

bool anyLivingCellsLeft(bool board[boardWidth][boardHeight])
{
	for (int y = 0; y < boardHeight; y++)
	{
		for (int x = 0; x < boardWidth; x++)
		{
			if (board[x][y])
			{
				return true;
			}
		}
	}

	return false;
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
	omp_set_num_threads(numThreads);

	// Todo (optional): use std::array or std::vector instead of C style array
	// Todo (optional): import initial state from file
	// ------------------------------------------------------------
	// Cells within the board:
	// True  -> Alive
	// False -> Dead
	bool board[boardWidth][boardHeight] = {};

	int iteration = 0;

	initBoardState(board);
	printBoardAndHeadline(board, iteration++, 0);

	do
	{
		cout << endl << "Press [RETURN] to continue..." << endl;
		cin.get();

		auto calcDuration = evolveBoard(board);
		clearConsole();
		printBoardAndHeadline(board, iteration++, calcDuration);
	} while (anyLivingCellsLeft(board));

	cout << endl << "No more living cells left." << endl << "Press [RETURN] to quit the programm..." << endl;
	cin.get();

	return 0;
}
