/*---------------------------------------------------------------------------------

	$Id: main.cpp,v 1.13 2008-12-02 20:21:20 dovoto Exp $

	Simple console print demo
	-- dovoto


---------------------------------------------------------------------------------*/
#include <nds.h>
#include <stdio.h>
#include <fat.h>

int playerX = 20;
int playerY = 10;
int worldX = 0;
int powerCount = 0;

int main() {
    consoleDemoInit();

    bool saveReady = fatInitDefault();

    iprintf("POWERBOUND: INFINITE\n");
    iprintf("----------------------\n");
    iprintf("Infinite World\n");
    iprintf("Move: D-pad\n");
    iprintf("Collect powers: A\n\n");

    while (1) {
        scanKeys();
        int keys = keysDown();

        if (keys & KEY_LEFT)  playerX--;
        if (keys & KEY_RIGHT) playerX++;
        if (keys & KEY_UP)    playerY--;
        if (keys & KEY_DOWN)  playerY++;

        if (playerX < 0) {
            playerX = 30;
            worldX--;
        }

        if (playerX > 30) {
            playerX = 0;
            worldX++;
        }

        if (keys & KEY_A) {
            powerCount++;
        }

        iprintf("\x1b[8;0HWorld: %d   ", worldX);
        iprintf("\x1b[10;0HPlayer: %d, %d   ", playerX, playerY);
        iprintf("\x1b[12;0HPowers: %d   ", powerCount);
        iprintf("\x1b[15;0HSave support: %s   ",
                saveReady ? "Ready" : "Unavailable");

        swiWaitForVBlank();
    }

    return 0;
}
