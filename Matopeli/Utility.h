#pragma once

void OpenConsole() {
	AllocConsole();
	FILE* filu = freopen("CONOUT$", "w+", stdout);
}

static unsigned short bait;



/*Snake struct works quite oddly. 
It has two members: head, and the tiles it occupies, the so called "occupancy"
The head features two things: direction, and position.
Direction is indicated by the first two bits. 11 = north, 10 = south, 00 = east and 00 = west.
The position of the head is indicated by the rest of the variable as a number. The x and y can deduced as follows:
y = number / 64
x = number % 64

Occupancy is a column-major list of bits that correspond to each of the 4096 tiles possible in the game.
Hence, the coordinate is of form y, x where both x and y range from 0 to 63.
Now one can easily determine whether a part of a snake exists on a specific tile like this:
(occupancy[y]>>x)& 1
*/


#ifdef DEBUG_MODE
void SnakeLength() {

}
#endif // DEBUG_MODE
