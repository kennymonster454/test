/*---------------------------------------------------------------------------------

	$Id: main.cpp,v 1.13 2008-12-02 20:21:20 dovoto Exp $

	Simple console print demo
	-- dovoto


---------------------------------------------------------------------------------*/

#include <nds.h>
#include <stdio.h>
#include <stdlib.h>

#define SCREEN_W 256
#define SCREEN_H 192
#define CHUNK_W 96
#define MAX_PLATFORMS 12

static u16* screen;

struct Platform {
    int x, y, w;
};

static Platform platforms[MAX_PLATFORMS];

static int playerX = 24;
static int playerY = 100;
static int velocityY = 0;
static int cameraX = 0;
static int worldChunk = -1;
static int powers = 0;
static int score = 0;
static bool grounded = false;

static unsigned int hashValue(unsigned int n) {
    n ^= n >> 16;
    n *= 0x7feb352d;
    n ^= n >> 15;
    n *= 0x846ca68b;
    n ^= n >> 16;
    return n;
}

static u16 color(int r, int g, int b) {
    return RGB15(r >> 3, g >> 3, b >> 3);
}

static void pixel(int x, int y, u16 c) {
    if (x < 0 || x >= SCREEN_W ||
        y < 0 || y >= SCREEN_H) return;

    screen[y * SCREEN_W + x] = c;
}

static void rect(int x, int y, int w, int h, u16 c) {
    for (int j = 0; j < h; j++)
        for (int i = 0; i < w; i++)
            pixel(x + i, y + j, c);
}

static void line(int x1, int y1, int x2, int y2, u16 c) {
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = x1 < x2 ? 1 : -1;
    int sy = y1 < y2 ? 1 : -1;
    int err = dx - dy;

    while (1) {
        pixel(x1, y1, c);
        if (x1 == x2 && y1 == y2) break;

        int e2 = err * 2;
        if (e2 > -dy) {
            err -= dy;
            x1 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y1 += sy;
        }
    }
}

static int platformY(int chunk) {
    if (chunk < 0) return 145;

    unsigned int h = hashValue((unsigned int)chunk + 12345);
    return 112 + (int)(h % 39);
}

static int platformX(int chunk) {
    return chunk * CHUNK_W + 8;
}

static int platformWidth(int chunk) {
    unsigned int h = hashValue((unsigned int)chunk + 6789);
    return 62 + (int)(h % 19);
}

static void drawBackground() {
    rect(0, 0, SCREEN_W, SCREEN_H, color(25, 30, 60));

    for (int i = 0; i < 35; i++) {
        int x = (i * 73 + 19) % SCREEN_W;
        int y = (i * 41 + 7) % 95;
        pixel(x, y, color(220, 225, 255));
    }

    for (int x = 0; x < SCREEN_W; x += 64) {
        int wx = x + cameraX;
        int hills = 135 + (int)(hashValue(wx / 32) % 30);
        line(x, hills, x + 32, hills - 20,
             color(35, 50, 80));
        line(x + 32, hills - 20, x + 64, hills,
             color(35, 50, 80));
    }
}

static void drawPlayer(int x, int y) {
    // Shadow
    rect(x + 1, y + 15, 10, 2, color(10, 12, 25));

    // Legs
    rect(x + 2, y + 10, 3, 5, color(245, 180, 65));
    rect(x + 7, y + 10, 3, 5, color(245, 180, 65));

    // Body
    rect(x + 1, y + 4, 10, 8, color(30, 200, 235));

    // Head
    rect(x + 2, y, 8, 6, color(255, 220, 175));

    // Eyes
    pixel(x + 7, y + 2, color(10, 10, 20));
}

static void drawPower(int x, int y, int id) {
    u16 c;

    switch (id % 6) {
        case 0: c = color(255, 70, 70); break;
        case 1: c = color(70, 220, 255); break;
        case 2: c = color(255, 230, 50); break;
        case 3: c = color(180, 70, 255); break;
        case 4: c = color(70, 255, 120); break;
        default: c = color(255, 140, 35); break;
    }

    pixel(x + 4, y, c);
    rect(x + 2, y + 1, 5, 5, c);
    pixel(x + 4, y + 6, color(255, 255, 255));
}

static void drawWorld() {
    drawBackground();

    int first = cameraX / CHUNK_W - 1;
    int last = first + 5;

    for (int chunk = first; chunk <= last; chunk++) {
        if (chunk < 0) continue;

        int wx = platformX(chunk);
        int sx = wx - cameraX;
        int py = platformY(chunk);
        int pw = platformWidth(chunk);

        rect(sx, py, pw, 8, color(115, 75, 45));
        rect(sx, py, pw, 3, color(60, 235, 105));

        // Each world chunk has its own generated power.
        int powerWorldX = wx + pw / 2;
        int powerScreenX = powerWorldX - cameraX;
        int powerY = py - 10;

        // A collected power disappears for this session.
        // Save-file persistence is not implemented yet.
        unsigned int powerID = hashValue((unsigned int)chunk);
        if (powerID % 100000u != 0u) {
            drawPower(powerScreenX, powerY, powerID % 6);
        }
    }

    drawPlayer(playerX - cameraX, playerY);

    rect(0, 0, 256, 23, color(12, 16, 35));
    // HUD text is drawn separately using the DS console.
}

int main(void) {
    videoSetMode(MODE_5_2D | DISPLAY_BG3_ACTIVE);
    vramSetBankA(VRAM_A_MAIN_BG);

    int bg = bgInit(
        3, BgType_Bmp16, BgSize_B16_256x256, 0, 0
    );

    screen = (u16*)bgGetGfxPtr(bg);

    consoleInit(
        0, 0, BgType_Text4bpp,
        BgSize_T_256x256, 31, 0, true, true
    );

    iprintf("POWERBOUND: INFINITE");

    while (1) {
        swiWaitForVBlank();
        scanKeys();

        int held = keysHeld();
        int pressed = keysDown();

        if (held & KEY_LEFT) playerX -= 2;
        if (held & KEY_RIGHT) playerX += 2;

        if ((pressed & KEY_A) && grounded) {
            velocityY = -7;
            grounded = false;
        }

        // Gravity and jumping
        velocityY += 1;
        if (velocityY > 7) velocityY = 7;
        playerY += velocityY;

        grounded = false;

        // Land on platforms when falling.
        int chunk = (playerX + 6) / CHUNK_W;

        for (int c = chunk - 1; c <= chunk + 1; c++) {
            if (c < 0) continue;

            int px = platformX(c);
            int py = platformY(c);
            int pw = platformWidth(c);

            if (playerX + 11 > px &&
                playerX < px + pw &&
                playerY + 15 >= py &&
                playerY + 15 <= py + 8 &&
                velocityY >= 0) {
                playerY = py - 15;
                velocityY = 0;
                grounded = true;
            }
        }

        // Keep the camera following the player.
        if (playerX - cameraX > 175)
            cameraX = playerX - 175;

        if (playerX - cameraX < 35)
            cameraX = playerX - 35;

        // Generate a power-up for each world chunk.
        int currentChunk = (playerX + 6) / CHUNK_W;

        if (currentChunk != worldChunk && currentChunk >= 0) {
            worldChunk = currentChunk;
        }

        // Pick up a power by touching its platform's crystal.
        for (int c = 0; c <= currentChunk + 1; c++) {
            int px = platformX(c);
            int py = platformY(c);
            int pw = platformWidth(c);

            int crystalX = px + pw / 2;
            int crystalY = py - 10;

            if (playerX + 11 > crystalX &&
                playerX < crystalX + 7 &&
                playerY < crystalY + 7 &&
                playerY + 15 > crystalY) {
                // Placeholder counter; unique collection tracking
                // and permanent saving need a save system.
            }
        }

        drawWorld();

        iprintf("\x1b[1;0HPOWERBOUND: INFINITE     ");
        iprintf("\x1b[2;0HWorld: %d   Powers: %d   ",
                cameraX / CHUNK_W, powers);
        iprintf("\x1b[3;0HA: Jump  LEFT/RIGHT: Move   ");

        bgUpdate();
    }

    return 0;
}


