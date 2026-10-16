/*---------------------------------------------------------------------------------

	$Id: main.cpp,v 1.13 2008-12-02 20:21:20 dovoto Exp $

	Simple console print demo
	-- dovoto


---------------------------------------------------------------------------------*/


#include <nds.h>
#include <stdio.h>

int playerX = 5;
int playerY = 16;
int velocityY = 0;
int cameraX = 0;
int powers = 0;
bool grounded = false;

int platformHeight(int x) {
    if (x < 0) return 17;
    unsigned int n = (unsigned int)x;
    n = n * 1103515245u + 12345u;
    return 14 + (n % 4);
}

int main(void) {
    consoleDemoInit();

    while (1) {
        swiWaitForVBlank();
        scanKeys();

        int held = keysHeld();
        int pressed = keysDown();

        if (held & KEY_LEFT) playerX--;
        if (held & KEY_RIGHT) playerX++;

        if (playerX < 0) playerX = 0;

        if ((pressed & KEY_A) && grounded) {
            velocityY = -4;
            grounded = false;
        }

        velocityY++;
        if (velocityY > 3) velocityY = 3;
        playerY += velocityY;

        grounded = false;

        int h = platformHeight(playerX);

        if (playerY >= h - 1 && velocityY >= 0) {
            playerY = h - 1;
            velocityY = 0;
            grounded = true;
        }

        if (playerX - cameraX > 25)
            cameraX = playerX - 25;

        if (playerY > 22) {
            playerY = 10;
            velocityY = 0;
        }

        int crystalX = ((playerX / 8) * 8) + 4;
        if ((pressed & KEY_B) && playerX >= crystalX - 1 &&
            playerX <= crystalX + 1) {
            powers++;
        }

        consoleClear();

        iprintf("\x1b[36;1mPOWERBOUND: INFINITE\x1b[0m\n");
        iprintf("\x1b[35m====================\x1b[0m\n");
        iprintf("\x1b[33mWorld: %d  Powers: %d\x1b[0m\n\n",
                cameraX, powers);

        for (int row = 0; row < 17; row++) {
            for (int col = 0; col < 32; col++) {
                int wx = cameraX + col;
                int ph = platformHeight(wx);

                if (row == playerY && col == playerX - cameraX) {
                    iprintf("\x1b[31;1m@\x1b[0m");
                } else if (row == ph) {
                    iprintf("\x1b[32;1m#\x1b[0m");
                } else if (row > ph) {
                    iprintf("\x1b[33m#\x1b[0m");
                } else if (row == ph - 1 && wx % 8 == 4) {
                    iprintf("\x1b[35;1m*\x1b[0m");
                } else {
                    iprintf(" ");
                }
            }
            iprintf("\n");
        }

        iprintf("\n\x1b[36mLEFT/RIGHT: Move\x1b[0m\n");
        iprintf("\x1b[32mA: Jump   B: Power\x1b[0m\n");
        iprintf("\x1b[33mExplore the endless world!\x1b[0m\n");
    }

    return 0;
}
