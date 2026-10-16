/*---------------------------------------------------------------------------------

	$Id: main.cpp,v 1.13 2008-12-02 20:21:20 dovoto Exp $

	Simple console print demo
	-- dovoto


---------------------------------------------------------------------------------*/

#include <nds.h>
#include <stdlib.h>

#define W 256
#define H 192
#define CHUNK 80

static u16* screen;

static int playerX = 30;
static int playerY = 130;
static int velocityY = 0;
static int cameraX = 0;
static int powers = 0;
static bool grounded = false;

static u16 rgb(int r, int g, int b) {
    return RGB15(r, g, b);
}

static void pixel(int x, int y, u16 c) {
    if (x >= 0 && x < W && y >= 0 && y < H)
        screen[y * W + x] = c;
}

static void box(int x, int y, int w, int h, u16 c) {
    for (int j = 0; j < h; j++)
        for (int i = 0; i < w; i++)
            pixel(x + i, y + j, c);
}

static unsigned int randomWorld(unsigned int n) {
    n ^= n >> 16;
    n *= 0x7feb352d;
    n ^= n >> 15;
    n *= 0x846ca68b;
    n ^= n >> 16;
    return n;
}

static int platformY(int chunk) {
    if (chunk <= 0) return 150;
    return 125 + randomWorld(chunk + 123u) % 20;
}

static int platformWidth(int chunk) {
    return 52 + randomWorld(chunk + 456u) % 20;
}

static int platformX(int chunk) {
    return chunk * CHUNK + 8;
}

static void drawCrystal(int x, int y, int type) {
    u16 c1 = rgb(0, 31, 31);
    u16 c2 = rgb(31, 31, 8);

    if (type % 4 == 0) c1 = rgb(31, 5, 8);
    if (type % 4 == 1) c1 = rgb(4, 20, 31);
    if (type % 4 == 2) c1 = rgb(31, 25, 0);
    if (type % 4 == 3) c1 = rgb(18, 4, 31);

    box(x + 2, y + 2, 7, 8, c1);
    pixel(x + 5, y, c2);
    pixel(x + 3, y + 1, c2);
    pixel(x + 7, y + 1, c2);
    box(x + 4, y + 3, 3, 4, c2);
    pixel(x + 5, y + 10, c1);
}

static void drawPlayer(int x, int y) {
    // Shadow
    box(x - 1, y + 15, 14, 2, rgb(4, 7, 14));

    // Boots
    box(x + 1, y + 12, 5, 4, rgb(31, 12, 2));
    box(x + 7, y + 12, 5, 4, rgb(31, 12, 2));

    // Bright blue outfit
    box(x + 1, y + 5, 11, 8, rgb(2, 12, 31));
    box(x + 3, y + 6, 7, 5, rgb(0, 25, 31));

    // Skin and hair
    box(x + 2, y, 9, 7, rgb(31, 23, 15));
    box(x + 1, y, 11, 3, rgb(8, 4, 2));

    // Eyes
    pixel(x + 8, y + 3, rgb(0, 0, 0));
}

static void drawScene(void) {
    // Blue sky
    box(0, 0, W, H, rgb(8, 17, 31));

    // Stars
    for (int i = 0; i < 45; i++) {
        int x = (i * 67 + 19) % W;
        int y = (i * 31 + 7) % 95;
        pixel(x, y, rgb(31, 31, 25));
    }

    // Distant mountains
    for (int x = 0; x < W; x += 32) {
        int y = 105 + randomWorld((x + cameraX) / 16) % 25;
        box(x, y, 32, H - y, rgb(5, 8, 22));
        box(x + 8, y - 5, 16, 5, rgb(7, 11, 27));
    }

    // Infinite platforms
    int first = cameraX / CHUNK - 1;

    for (int c = first; c < first + 6; c++) {
        if (c < 0) continue;

        int wx = platformX(c);
        int sx = wx - cameraX;
        int py = platformY(c);
        int pw = platformWidth(c);

        // Platform underside
        box(sx, py, pw, 10, rgb(15, 8, 25));

        // Grass top
        box(sx, py, pw, 4, rgb(4, 31, 10));
        box(sx, py, pw, 2, rgb(12, 31, 14));

        // Glowing power crystal
        drawCrystal(sx + pw / 2 - 5, py - 15, c);
    }

    // Player
    drawPlayer(playerX - cameraX, playerY);

    // Colorful HUD strip
    box(0, 0, W, 5, rgb(0, 20, 25));
    box(0, 0, 45, 3, rgb(0, 31, 20));
    box(45, 0, 45, 3, rgb(0, 20, 31));
    box(90, 0, 45, 3, rgb(25, 10, 31));
    box(135, 0, 45, 3, rgb(31, 20, 0));
    box(180, 0, 76, 3, rgb(31, 5, 12));

    // Power meter
    for (int i = 0; i < powers % 20; i++)
        box(5 + i * 7, 10, 5, 4, rgb(31, 25, 0));
}

int main(void) {
    videoSetMode(MODE_5_2D);
    vramSetBankA(VRAM_A_MAIN_BG);

    int bg = bgInit(
        2, BgType_Bmp16, BgSize_B16_256x256, 0, 0
    );

    screen = (u16*)bgGetGfxPtr(bg);

    while (1) {
        swiWaitForVBlank();
        scanKeys();

        int held = keysHeld();
        int pressed = keysDown();

        // Movement
        if (held & KEY_LEFT) playerX -= 2;
        if (held & KEY_RIGHT) playerX += 2;

        // Jump
        if ((pressed & KEY_A) && grounded) {
            velocityY = -8;
            grounded = false;
        }

        // Gravity
        velocityY++;
        if (velocityY > 7) velocityY = 7;
        playerY += velocityY;
        grounded = false;

        // Platform collision
        int current = playerX / CHUNK;

        for (int c = current - 1; c <= current + 1; c++) {
            if (c < 0) continue;

            int px = platformX(c);
            int py = platformY(c);
            int pw = platformWidth(c);

            if (playerX + 12 > px &&
                playerX < px + pw &&
                playerY + 16 >= py &&
                playerY + 16 <= py + 9 &&
                velocityY >= 0) {
                playerY = py - 16;
                velocityY = 0;
                grounded = true;
            }
        }

        // Fall back to the starting area
        if (playerY > 190) {
            playerY = 100;
            velocityY = 0;
            cameraX = playerX;
        }

        // Camera follows the player
        if (playerX - cameraX > 170)
            cameraX = playerX - 170;

        if (playerX - cameraX < 40)
            cameraX = playerX - 40;

        drawScene();
    }

    return 0;
}

