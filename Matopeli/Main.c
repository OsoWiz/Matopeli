#pragma once

#include <stdio.h>
#include <immintrin.h>
#include <Windows.h>
#include <sys/timeb.h>
//debug
#include <debugapi.h>
#ifdef NDEBUG
static const boolean debug = 0;
#else 
static const boolean debug = 1;
#define DEBUG_MODE
#endif

typedef unsigned short uint16_t;

#include "Utility.h"

const char luokannimi[] = "Matoikkuna";

const unsigned short dirMask = 0b1100000000000000; // points to the direction
const unsigned short prevDirMask = 0b0011000000000000; // points to the previous direction
const unsigned short posMask = 0b0000111111111111; // position
const unsigned short bitMask16 = (2 << 16) - 1;
const unsigned char north = 0;
const unsigned char east = 1;
const unsigned char south = 2;
const unsigned char west = 3;
const unsigned short rowSize = 16; // rows are 16 bytes long
const unsigned short sColSize = 4; // single column contains 4 directions. Meaning


// first 2 bits represent 4 directions, and last 14 the position
unsigned short direction = 0; // 16 bits representing both the location of the head and it's direction.
unsigned short length = 1; // length of the worm
unsigned short bait = 0; // bait location
static unsigned char occupancy[64 * 16]; // basically 64 rows of 16 bytes, each byte representing 4 columns (4 * 16 = 64 total rows)
unsigned short posSeed = 1; // seed for the position of the worm
unsigned short rounds = 1; // rounds to roll the lfsr

/**
 * @brief Returns a 16 bit random number using a linear feedback shift register
 * @param rounds to roll the lfsr
 * @param seed to use for the lfsr (don't input 0)
 * @return pseudorandom number
*/
unsigned short lfsrRandomNumber(unsigned short rounds, unsigned short seed)
{
	unsigned short lfsr = seed;
	for (unsigned short i = 0; i < rounds; i++)
	{
		unsigned short lsb = lfsr & 1u;  // least significant bit
		lfsr >>= 1u;                   // shift
		if (lsb == 1u)                  // if output is 1
			lfsr ^= 0xB400u;              // toggle
	};
	return lfsr;
}

/**
 * @brief Returns the two bits representing the direction of the worm, assuming they are the two most significant bits
 * @return value from 0 to 3 representing the direction
*/
inline unsigned short getDirbits(unsigned short dir)
{
	return (dir & dirMask) >> 14u;
}

/**
 * @brief Returns the two bits representing the previous direction of the worm
 * @return value from 0 to 3 representing the direction
*/
inline unsigned short getPrevDirbits(unsigned short dir)
{
	return (dir & prevDirMask) >> 12u;
}


/**
 * @brief Prints given direction
*/
void printDir(unsigned int dir) {
	switch (dir)
	{
		case 0:
			printf("north");
			break;
		case 1:
			printf("east");
			break;
		case 2:
			printf("south");
			break;
		case 3:
			printf("west");
			break;
	}
	printf("\n");
}

/**
 * @brief Sets the direction of the worm
 * @param newDir to set the direction to
*/
inline void setWormDir(unsigned short newDir)
{
	unsigned short nDir = getDirbits(newDir);
	unsigned short dirCase = getDirbits(newDir) ^ getDirbits(direction);
	if (dirCase != 2u) // we can change direction if and only if the new direction is not the opposite of the current direction
	{
		printf("Direction changed to: ");
		printDir(nDir);
		printDir(2 ^ nDir);
		direction = (newDir & dirMask) | (direction & posMask) | ((2 ^ getDirbits(newDir)) << 12);
	}
}

inline void raffleBait(unsigned short rounds)
{
	unsigned short rndnum = lfsrRandomNumber(rounds, posSeed);
	unsigned char x = rndnum % 64;
	unsigned char y = rndnum / 64;
	y %= 64; // make sure y does not overflow
	bait = y * 64 + x;
	posSeed = rndnum; // set the new seed
}

inline void moveCoords(unsigned short dir, unsigned char* x, unsigned char* y)
{
	unsigned char nx = *x, ny = *y;
	// move the worm
	switch (dir)
	{
		case 0:
			// north
			ny--;
			break;
		case 1:
			// east
			nx++;
			break;
		case 2:
			// south
			ny++;
			break;
		case 3:
			// west
			nx--;
			break;
	}

	nx %= 64u;
	ny %= 64u;
	*x = nx;
	*y = ny;
}


// moves the worm 1 tile in the direction it's facing
void moveWorm()
{
	// get the direction
	unsigned short dir = getDirbits(direction);
	unsigned short prevDir = getPrevDirbits(direction);
	// get the position
	unsigned short pos = direction & posMask;
	// decode the position
	unsigned char y = 0, x = 0, by = 0, bx = 0;
	int ret = getLocation(pos, &x, &y);
	int bret = getLocation(bait, &bx, &by);

	moveCoords(dir, &x, &y);

	if (!(x - bx) && !(y - by))
	{
		// if the worm is on the bait, move the bait to a new location
		raffleBait(1);
		// make the worm longer by 1
		length++;
	}
	printf("Previous snake part is:\n");
	printDir(prevDir);
	unsigned short writePos = rowSize * y + x / sColSize;
	unsigned char dirBits = (x % 4u) * 2u; // offset of the direction inside a single byte
	unsigned char current = occupancy[writePos];
	unsigned char mask = ~(3u << dirBits);
	occupancy[writePos] = (prevDir << dirBits) + (current & mask);  // write the previous direction to the occupancy
	// set the new position
	direction = (dir << 14) | (prevDir << 12) | (y * rowSize + x); // previous direction does not change when
}
/**
 * @brief Decodes the position from the 16 bit integer
 * @param pos to decode
 * @param x to set
 * @param y to set
 * @return int 0 if successful
*/
inline int getLocation(unsigned short pos, unsigned char* x, unsigned char* y)
{
	if (x == NULL || y == NULL)
		return 1;

	unsigned short posi = pos & posMask;
	*x = posi % 64u;
	*y = posi / 64u;
	if (63u < *y) // if y is greater than 63, return 1. pos is invalid.
		return 1;

	return 0;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	short newDir = direction;
	switch (msg)
	{
		case WM_KEYDOWN:
			printf("Keypress detected!");
			// Deal with user input
			switch (wParam)
			{
				case VK_UP:
					newDir = (0 << 14);
					break;
				case VK_DOWN:
					newDir = (2 << 14);
					break;
				case VK_RIGHT:
					newDir = (1 << 14);
					break;
				case VK_LEFT:
					newDir = (3 << 14);
					break;
				default:
					break;
			}
			setWormDir(newDir);
			break;
		case WM_PAINT:
			// PAINT CASE
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);
			unsigned char x = 0, y = 0;

			int ret = getLocation(direction, &x, &y);
			unsigned char prevDirection = getPrevDirbits(direction);
			for (unsigned short i = 0; i < length; i++)
			{
				RECT pepe = { x * 10, y * 10, (x + 1) * 10, (y + 1) * 10 };
				// All painting occurs here, between BeginPaint and EndPaint.
				FillRect(hdc, &pepe, (HBRUSH)(CreateSolidBrush(RGB(255, 0, 0)))); // paint if worm there?
				moveCoords(prevDirection, &x, &y); // move coords to the previous direction
				// get new previous direction
				unsigned short writePos = rowSize * y + x / sColSize;
				unsigned char dirBits = (x % 4) * 2; // offset of the direction inside a single byte
				prevDirection = (occupancy[writePos] >> dirBits) & 3u;
			}
			// draw bait
			unsigned char baitX = bait % 64;
			unsigned char baitY = bait / 64;
			RECT baitRect = { baitX * 10, baitY * 10, (baitX + 1) * 10, (baitY + 1) * 10 };
			FillRect(hdc, &baitRect, (HBRUSH)(CreateSolidBrush(RGB(0, 255, 0))));

			EndPaint(hwnd, &ps);
			break; // PAINT ENDS HERE
		case WM_CLOSE:
			DestroyWindow(hwnd);
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
	bait = 5;

	if (debug)
		OpenConsole();


	// generate random number using rdseed32
	unsigned randomNumber;
	if (!_rdseed32_step(&randomNumber))
		printf("rdseed failed");

	posSeed = (unsigned short)randomNumber & posMask;
	rounds = (unsigned short)(randomNumber >> 16u) & bitMask16;
	// roll the lfsr
	raffleBait(rounds);

	printf("Bait rolled: %d x, %d y\n", bait % 64, bait / 64);

	direction = 0;
	// set direction to south
	direction = 2 << 14;

	WNDCLASSEX wc;
	HWND hwnd;
	MSG Msg;

	// Step 1: Registering the Window Class
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = 0;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = luokannimi;
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	if (!RegisterClassEx(&wc))
	{
		MessageBox(NULL, "Window Registration Failed!", "Error!",
			MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	// Step 2: Creating the Window
	hwnd = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		luokannimi,
		"Matopeli",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 640, 640,
		NULL, NULL, hInstance, NULL);

	if (hwnd == NULL)
	{
		MessageBox(NULL, "Window Creation Failed!", "Error!",
			MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);


	// Create tickrate for the game
	int timetresholdms = 200; // 0.2 seconds = 200ms
	struct timeb start, end;
	ftime(&start);
	ftime(&end);
	GetMessage(&Msg, NULL, 0, 0);

	// Step 3: The Message Loop
	while (Msg.message != WM_QUIT)
	{
		// use peekmessage instead of getmessage
		if (PeekMessage(&Msg, 0, 0, 0, PM_REMOVE) > 0)
		{
			TranslateMessage(&Msg);
			DispatchMessage(&Msg);
		}

		// get time diff in milliseconds
		int diff = 1000 * (end.time - start.time) + (end.millitm - start.millitm);

		// check if the time has passed
		if (timetresholdms < diff)
		{
			// printf("Time passed: %d\n", diff);
			//update the game
			moveWorm();
			InvalidateRect(hwnd, NULL, TRUE);
			//UpdateWindow(hwnd);
			//// print direction after update
			//unsigned int x = direction & posMask;
			//unsigned int y = x / 64;
			//unsigned int dir = getDirbits(direction);

			//printf("Worm position: x: %d, y: %d\n", x % 64, y);
			//printf("Worm direction: ");
			//printDir(dir);

			start = end;
		}
		ftime(&end);
	}
	return Msg.wParam;


}