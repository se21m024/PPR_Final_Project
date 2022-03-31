/*
	Parallel Programming

	Final Project:
	Game of Life

	Student 1: Thomas Br�ndl
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
/**** Global Constants ****/
/**************************/

const int CORES_ON_MACHINES = thread::hardware_concurrency();
const int NUM_THREADS = CORES_ON_MACHINES; // Set to any arbitrary number or to coresOnMachine if you wish

// Enable parallel execution
const bool USE_PARALLEL_IMPLEMENTATION = true;

const int MAX_ITERATIONS = 1000;

// Good fit to console: 100 x 40
// Increase dimensions to e.g. 500 x 500 gain profit from parallel execution
// Attention! To big numbers lead to an exception
const int BOARD_WIDTH = 100;
const int BOARD_HEIGHT = 40;

// E.g. 5 -> change for a cell to be initially living is 1/5
const int FRACTION_OF_INITIALLY_LIVING_CELLS = 4;

// Set to true if GUI is desired in Demo Mode
// Maximize console window for better experience
// (Not recommended when testing with large board.)
const bool USE_GUI = true;

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
	auto startTimeStamp = high_resolution_clock::now();

#pragma omp parallel for
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
	bool tempBoard[BOARD_WIDTH][BOARD_HEIGHT] = {};

	auto calcDuration = calcNewBoardState(board, tempBoard);

	copyBoard(tempBoard, board);

	return calcDuration;
}

void printHeadline(int iteration, int numLivingCells, long long microSecondsToCalcLastIteration)
{
	// Reset cursor
	SetCursorPos(0, 0);

	cout << to_string(iteration) << ". iteration" << endl;
	cout << "Living cells: " << to_string(numLivingCells) << endl;
	cout << "The calculation of this iteration took " << to_string(microSecondsToCalcLastIteration) << " microseconds" << endl
		 << endl;
}

void printBoard(bool board[BOARD_WIDTH][BOARD_HEIGHT])
{
	if (!USE_GUI)
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
	COORD topLeft = {0, 0};
	HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO screen;
	DWORD written;

	GetConsoleScreenBufferInfo(console, &screen);
	FillConsoleOutputCharacterA(
		console, ' ', screen.dwSize.X * screen.dwSize.Y, topLeft, &written);
	FillConsoleOutputAttribute(
		console, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE,
		screen.dwSize.X * screen.dwSize.Y, topLeft, &written);
	SetConsoleCursorPosition(console, topLeft);
}

void BenchmarkMode()
{
	// Cells within the board:
	// True  -> Alive
	// False -> Dead
	bool board[BOARD_WIDTH][BOARD_HEIGHT] = {};
	bool initBoard[BOARD_WIDTH][BOARD_HEIGHT] = {};

	initBoardState(initBoard);
	cout << "[";
	for (int threads = 1; threads < NUM_THREADS; threads++)
	{
		omp_set_num_threads(threads);
		// cout << endl << "Number of threads: " << to_string(threads) << endl;
		cout << "[";

		copyBoard(initBoard, board);
		int iteration = 0;

		long long calcDurations[MAX_ITERATIONS] = {};

		do
		{
			auto calcDuration = evolveBoard(board);
			calcDurations[iteration++] = calcDuration;
		} while (iteration < MAX_ITERATIONS);

		for (int i = 0; i < MAX_ITERATIONS; i++)
		{
			if (i > 0)
			{
				cout << ", ";
			}

			// Iteration, CalcDuration
			cout << to_string(calcDurations[i]);
		}

		cout << "]," << endl;
	}
	cout << "]";
}

void DemoMode()
{
	omp_set_num_threads(NUM_THREADS);

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
		cout << endl
			 << "Press [RETURN] to calculate next iteration..." << endl;
		cin.get();

		auto calcDuration = evolveBoard(board);
		numLivingCells = countLivingCells(board);
		clearConsole();
		printBoardAndHeadline(board, iteration++, numLivingCells, calcDuration);
	} while (numLivingCells > 0);

	cout << endl
		 << "----------------------------->" << endl;
	cout << "No more living cells left." << endl
		 << "Press [RETURN] to quit the programm..." << endl;
	cin.get();
}

int main()
{
	if (USE_GUI)
	{
		DemoMode();
	}
	else
	{
		BenchmarkMode();
	}

	return 0;
}
