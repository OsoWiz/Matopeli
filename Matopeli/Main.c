#pragma once

#include <stdio.h>
#include <immintrin.h>
#include <Windows.h>
#include <time.h>
//debug
#include <debugapi.h>
#ifdef NDEBUG
static const boolean debug = 0;
#else 
static const boolean debug = 1;
#define DEBUG_MODE
#endif

#include "Utility.h"

const char luokannimi[] = "Matoikkuna";
char windowTestVariable = 10;
unsigned short direction; // 16 bits representing both the location of  the head and it's direction.
// first 2 bits represent 4 directions, and last 14 the position
static unsigned char occupancy[64 * 8]; // basically 64 rows of 8 bytes, each byte representing 8 columns
// 0 means snake is not there, 1 means snake is there.

// moves the worm 1 tile in the direction it's facing
void moveWorm() {
	{
		// get the direction
		unsigned short dir = direction & 3;
		// get the position
		unsigned short pos = direction >> 2;
		// get the x and y coordinates
		unsigned short x = pos % 64;
		unsigned short y = pos / 64;
		// move the worm
		switch (dir)
		{
		case 0:
			// north
			y--;
			break;
		case 1:
			// east
			x++;
			break;
		case 2:
			// south
			y++;
			break;
		case 3:
			// west
			x--;
			break;
		}
		// set the new position
		direction = (y << 6) | x;
	}
}


LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_KEYDOWN:
        //deal with user input
        switch (wParam)
        {

        case VK_UP:
            direction &= ((3 << 14) | 0x0FFF );
            break;
        case VK_DOWN:
            direction &= ((2 << 14) | 0x0FFF);
            break;
        case VK_RIGHT:
            direction &= ((0 << 14) | 0x0FFF);
            break;
        case VK_LEFT:
            direction &= ((1 << 14) | 0x0FFF);
            break;
        default:
            break;
        }
        break;
    case WM_PAINT:
        //PAINT CASE
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        printf("paint the screen");
        
    //    for (unsigned short i = 0; i < 512; i++) {
    //        unsigned char byteiq = occupancy[i];
    //        for (unsigned char j = 0; j < 8; j++) {
    //            unsigned char bit = ((byteiq >> j) & 1); //jth bit in i:th char.
    //            unsigned char x = (i % 8) * 8 + j;
    //            unsigned char y = (i / 8);
				//if ((direction & 0x0FFF) == y * 64 + x) { // calculate if the head is in this tile
    //                RECT pepe = { x * 10, y * 10, (x + 1) * 10, (y + 1) * 10 };
    //                // All painting occurs here, between BeginPaint and EndPaint.
    //                FillRect(hdc, &pepe, (HBRUSH)(CreateSolidBrush(RGB(255, 0, 0)))); // paint if worm there?
    //            }
    //        }
    //    }
		// test window update using a rectangle
		RECT pepe = { 0, 0, windowTestVariable, 100 };
        if (windowTestVariable < 100) {
            windowTestVariable += 100;
        }
        else {
            windowTestVariable = 10;
        }
		FillRect(hdc, &pepe, (HBRUSH)(CreateSolidBrush(RGB(255, 0, 0)))); // paint if worm there?
        
        EndPaint(hwnd, &ps);
        break; //PAINT ENDS HERE
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

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, int nCmdShow)
{
    bait = 5;

    if (debug)
        OpenConsole();
    
    
    // generate random number using rdseed32
	unsigned randomNumber;
    if(!_rdseed32_step(&randomNumber))
		printf("rdseed failed");
	
		
    direction = 0;
	// set direction to south
	direction = 2 << 14;

    WNDCLASSEX wc;
    HWND hwnd;
    MSG Msg;

    //Step 1: Registering the Window Class
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

    
    //create tickrate for the game
    int tickRate = 20;
    int i = 0;
    // Step 3: The Message Loop
    while (GetMessage(&Msg, NULL, 0, 0) > 0)
    {
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
        i++;
		if (i >= tickRate) {
            printf("1 tick");
            moveWorm();
			InvalidateRect(hwnd, NULL, TRUE);
			i = 0;
            UpdateWindow(hwnd);
		}
    }
    return Msg.wParam;
}