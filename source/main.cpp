/*---------------------------------------------------------------------------------

	$Id: main.cpp,v 1.13 2008-12-02 20:21:20 dovoto Exp $

	Simple console print demo
	-- dovoto


---------------------------------------------------------------------------------*/

#include <nds.h>
#include <stdio.h>
#include <stdlib.h>

int playerX = 10;
int playerY = 10;
int worldX = 0;
int powerCount = 0;
int powerX = 20;
int powerY = 10;
int frameCount = 0;

void drawGame() {
    consoleClear();

    iprintf("POWERBOUND: INFINITE\n");
    iprintf("====================\n\n");

    iprintf("World: %d\n", worldX);
    iprintf("Powers collected: %d\n\n", powerCount);

    iprintf("Controls: D-pad to move\n");
    iprintf("Press A to collect a power\n\n");

    iprintf("Player: (%d, %d)\n", playerX, playerY);
    iprintf("Power:  (%d, %d)\n\n", powerX, powerY);

    iprintf("      +----------------+\n");
    iprintf("      |                |\n");
    iprintf("      |       P        |\n");
    iprintf("      |                |\n");
    iprintf("      +----------------+\n\n");

    iprintf("P = Player, power at shown coordinates\n");
}

int main(void) {
    consoleDemoInit();

    while (1) {
        swiWaitForVBlank();
        scanKeys();

        int keys = keysDown();

        if (keys & KEY_LEFT) {
            playerX--;
            if (playerX < 0) {
                playerX = 31;
                worldX--;
            }
        }

        if (keys & KEY_RIGHT) {
            playerX++;
            if (playerX > 31) {
                playerX = 0;
                worldX++;
            }
        }

        if (keys & KEY_UP && playerY > 0) {
            playerY--;
        }

        if (keys & KEY_DOWN && playerY < 20) {
            playerY++;
        }

        if (keys & KEY_A) {
            if (playerX == powerX && playerY == powerY) {
                powerCount++;
                powerX = rand() % 32;
                powerY = rand() % 21;
            }
        }

        frameCount++;

        if (frameCount >= 60) {
            frameCount = 0;
            srand(worldX + powerCount + powerX + powerY);
        }

        drawGame();
    }

    return 0;
}

