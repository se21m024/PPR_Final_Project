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
#include <windows.h>

using namespace std::chrono;
using namespace std;

/**************************/
/********* ToDos **********/
/**************************/

// (optional? - if yes, should be done before measurement and analysis): use std::array or std::vector instead of C style array

// Calculate and print average duration
// Implement flag to automatically calculate x iterations
// Create, print and analyse behaviour and measurements

// (optional): import initial state from file
// (optional): countLivingCells could be executed in parallel to but not in focus of this exercise


/**************************/
/**** Global Constants ****/
/**************************/

const int CORES_ON_MACHINES = thread::hardware_concurrency();
const int NUM_THREADS = 4; // Set to any arbitrary number or to coresOnMachine if you wish

// Enable parallel execution
const bool USE_PARALLEL_IMPLEMENTATION = true;

// Good fit to console: 100 x 40
// Increase dimensions to e.g. 500 x 500 gain profit from parallel execution
// Attention! To big numbers lead to an exception
const int BOARD_WIDTH = 100;
const int BOARD_HEIGHT = 40;

// E.g. 5 -> change for a cell to be initially living is 1/5
const int FRACTION_OF_INITIALLY_LIVING_CELLS = 4;

// Set to true if GUI is desired
// Maximize console window for better experience
// (Not recommended when testing with large board.)
const bool PRINT_BOARD = true;


/**************************/
/******** Methods *********/
/**************************/

void copyBoard(bool source[BOARD_WIDTH][BOARD_HEIGHT], bool target[BOARD_WIDTH][BOARD_HEIGHT])
{
	for (int y = 0; y < BOARD_HEIGHT; y++)
	{
		for (int x = 0; x < BOARD_WIDTH; x++)
		{
			target[x][y] = source[x][y];
		}
	}
}

bool calcNewCellState(bool board[BOARD_WIDTH][BOARD_HEIGHT], int cellX, int cellY)
{
	int numLivingNeighbours = 0;

	int leftLimit = cellX - 1 < 0 ? 0 : cellX - 1;
	int rightLimit = cellX + 1 > BOARD_WIDTH ? BOARD_WIDTH : cellX + 1;
	int topLimit = cellY - 1 < 0 ? 0 : cellY - 1;
	int bottomLimit = cellY + 1 > BOARD_HEIGHT ? BOARD_HEIGHT : cellY + 1;

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
long long calcNewBoardStateSerial(bool oldBoard[BOARD_WIDTH][BOARD_HEIGHT], bool newBoard[BOARD_WIDTH][BOARD_HEIGHT])
{
	auto startTimeStamp = high_resolution_clock::now();

	for (int y = 0; y < BOARD_HEIGHT; y++)
	{
		for (int x = 0; x < BOARD_WIDTH; x++)
		{
			newBoard[x][y] = calcNewCellState(oldBoard, x, y);
		}
	}

	auto stopTimeStamp = high_resolution_clock::now();
	return duration_cast<microseconds>(stopTimeStamp - startTimeStamp).count();
}

// Returns the microseconds that were necessary to calculate the new board state
long long calcNewBoardStateParallel(bool oldBoard[BOARD_WIDTH][BOARD_HEIGHT], bool newBoard[BOARD_WIDTH][BOARD_HEIGHT])
{
	omp_set_num_threads(NUM_THREADS);

	auto startTimeStamp = high_resolution_clock::now();

#pragma omp parallel
	{
		// Uncomment to test if the programm is really executed in parallel
		//cout << to_string(omp_get_thread_num());

		for (int y = omp_get_thread_num(); y < BOARD_HEIGHT; y += omp_get_num_threads())
		{
			for (int x = 0; x < BOARD_WIDTH; x++)
			{
				newBoard[x][y] = calcNewCellState(oldBoard, x, y);
			}
		}
	}

	// Uncomment to test if the programm is really executed in parallel
	//cin.get();

	auto stopTimeStamp = high_resolution_clock::now();
	return duration_cast<microseconds>(stopTimeStamp - startTimeStamp).count();
}

// Returns the microseconds that were necessary to calculate the new board state
long long calcNewBoardState(bool oldBoard[BOARD_WIDTH][BOARD_HEIGHT], bool newBoard[BOARD_WIDTH][BOARD_HEIGHT])
{
	if (USE_PARALLEL_IMPLEMENTATION)
	{
		return calcNewBoardStateParallel(oldBoard, newBoard);
	}

	return calcNewBoardStateSerial(oldBoard, newBoard);
}

// Returns the microseconds that were necessary to calculate the new board state
long long evolveBoard(bool board[BOARD_WIDTH][BOARD_HEIGHT])
{
	// Todo: check if this arry is deallocated automatically
	bool tempBoard[BOARD_WIDTH][BOARD_HEIGHT] = {};
	
	auto calcDuration = calcNewBoardState(board, tempBoard);

	// Todo: tbd: should this also be done in parallel?
	copyBoard(tempBoard, board);

	return calcDuration;
}

void printHeadline(int iteration, int numLivingCells, long long microSecondsToCalcLastIteration)
{
	// Reset cursor
	SetCursorPos(0, 0);

	cout << to_string(iteration) << ". iteration" << endl;
	cout << "Living cells: " << to_string(numLivingCells) << endl;
	cout << "The calculation of this iteration took " << to_string(microSecondsToCalcLastIteration) << " microseconds" << endl << endl;
}

void printBoard(bool board[BOARD_WIDTH][BOARD_HEIGHT])
{
	if (!PRINT_BOARD)
	{
		cout << endl;
		return;
	}

	for (int y = 0; y < BOARD_HEIGHT; y++)
	{
		for (int x = 0; x < BOARD_WIDTH; x++)
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

void printBoardAndHeadline(bool board[BOARD_WIDTH][BOARD_HEIGHT], int iteration, int numLivingCells, long long microSecondsToCalcLastIteration)
{
	printHeadline(iteration, numLivingCells, microSecondsToCalcLastIteration);
	printBoard(board);
}

void initBoardState(bool board[BOARD_WIDTH][BOARD_HEIGHT])
{
	random_device dev;
	mt19937 rng(dev());
	uniform_int_distribution<mt19937::result_type> dist(0, FRACTION_OF_INITIALLY_LIVING_CELLS);

	for (int y = 0; y < BOARD_HEIGHT; y++)
	{
		for (int x = 0; x < BOARD_WIDTH; x++)
		{
			// Randomly set cells alive due to given probability
			board[x][y] = (dist(rng) % FRACTION_OF_INITIALLY_LIVING_CELLS) == 0;
		}
	}
}

int countLivingCells(bool board[BOARD_WIDTH][BOARD_HEIGHT])
{
	int livingCells = 0;

	for (int y = 0; y < BOARD_HEIGHT; y++)
	{
		for (int x = 0; x < BOARD_WIDTH; x++)
		{
			if (board[x][y])
			{
				livingCells++;
			}
		}
	}

	return livingCells;
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
	// Cells within the board:
	// True  -> Alive
	// False -> Dead
	bool board[BOARD_WIDTH][BOARD_HEIGHT] = {};

	int iteration = 0;

	initBoardState(board);
	int numLivingCells = countLivingCells(board);
	printBoardAndHeadline(board, iteration++, numLivingCells, 0);

	do
	{
		cout << endl << "Press [RETURN] to calculate next iteration..." << endl;
		cin.get();

		auto calcDuration = evolveBoard(board);
		numLivingCells = countLivingCells(board);
		clearConsole();
		printBoardAndHeadline(board, iteration++, numLivingCells, calcDuration);
	} while (numLivingCells > 0);

	cout << endl << "----------------------------->" << endl;
	cout << "No more living cells left." << endl << "Press [RETURN] to quit the programm..." << endl;
	cin.get();

	return 0;
}
