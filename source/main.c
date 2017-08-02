#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <3ds.h>
#include "screen_bin.h"

#define STBI_NO_STDIO
#define STBI_NO_LINEAR
#define STBI_NO_HDR
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#include "stb_image.h"

#define TOP_ROWS 30
#define TOP_COLS 50
#define BOT_ROWS 30
#define BOT_COLS 40

PrintConsole top,bottom;

int init() {
    gfxInitDefault(); //Initialize graphics with default settings
	consoleInit(GFX_TOP, &top); //Use TOP screen
	consoleSelect(&top);
	gfxSetDoubleBuffering(GFX_BOTTOM, 0);
	gfxSwapBuffers();
    return 0;
}

int deinit() {
    gfxExit();
    return 0;
}

int cursors[1] = {
	0
};

int drawImage() {
    int w,h;
    void* screenImage = stbi_load_from_memory(screen_bin, screen_bin_size, &w, &h, 0, 3);
    if (screenImage) {
        memcpy(gfxGetFramebuffer(GFX_BOTTOM, GFX_LEFT, 0, 0), screenImage, w*h*4);
    } else {
        return -1;
    }
    return 0;
}

int main() {
	init();

	printf("\x1b[2;23H\x1b[31;1ma\x1b[32;1mm\x1b[34;1mi\x1b[36mb\x1b[33;1ma\x1b[35;1mc\x1b[0m"); //Printing colored amibac text.

	drawImage();

	while (aptMainLoop()) {
		hidScanInput();
		u32 kDown = hidKeysDown();

		if (kDown & KEY_START) break;
	}

	deinit();
	return 0;
}