#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <3ds.h>
#include <dirent.h>
#include <math.h>
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

int currentMenu = 0;
char currentMenuText[0x1001];
char tempText[0x1001];

PrintConsole topS;

int init() {
	gfxInitDefault(); //Initialize graphics with default settings.
	consoleInit(GFX_TOP, &topS); //Initialize screen.
	consoleSelect(&topS); //Select top screen.
	gfxSetDoubleBuffering(GFX_TOP, 0);
	gfxSetDoubleBuffering(GFX_BOTTOM, 0);
	gfxSwapBuffers();
	return 0;
}

int deinit() {
	gfxExit();
	return 0;
}

char menuTexts[2][0x1000] = {
	{"\x1b[14;22HBackup\x1b[15;22HRestore\x1b[16;21HDuplicate\0"},
	{"\x1b[13;%dH%s\x1b[14;%dH%s\x1b[15;%dH%s\x1b[16;%dH%s\x1b[17;%dH%s\0"}
};

int menuParent[2] = { //Used to get parents of menus. Aka. parent of backup menu(2nd) = main menu(0)
	0,
	0
};

int cursors[2] = { //Storing cursors here. Having an array for the sake of "restoring" cursor positions on menu change. Menu ID 0 would use Cursor ID 0, etc.
	0,
	0
};

int cursorsMax[2] = { //Maximum cursor values. Aka. cursors[0]'s value must not go above cursorsMax[0]. Also used to get how many items are there in a menu.
	2,
	4
};

int cursorsX[2] = { //Storing where to start printing the cursors.
	(TOP_ROWS/2)-1,
	(TOP_ROWS/2)-2
};

char currentFileListing[5][0x1001] = {
	{"\0"},
	{"\0"},
	{"\0"},
	{"\0"},
	{"\0"}
};

//Call after setting currentMenu to different value
void updateMenuText() {
	consoleClear(); //MAKE SURE TOP CONSOLE IS SELECTED!!!
	memset(currentMenuText, 0, 0x1001); //Clearing currentMenuText.
	strncpy(currentMenuText, menuTexts[currentMenu], 0x1000); //Copy 0x1000 bytes, making sure there is a null-terminator at the end.
}

int drawImage() {
	int w,h;
	void* screenImage = stbi_load_from_memory(screen_bin, screen_bin_size, &w, &h, 0, 3); //Load bottom screen .png image.
	if (screenImage) {
		memcpy(gfxGetFramebuffer(GFX_BOTTOM, GFX_LEFT, 0, 0), screenImage, w*h*4);
	} else {
		return -1;
	}
	return 0;
}

int main() {
	init();

	updateMenuText();

	if (drawImage() == -1) { //If drawing failed
		printf("Drawing failed.");
	}

	while (aptMainLoop()) {
		//Pre-printing text
		//printf("\x1b[2;22H\x1b[31;1ma\x1b[32;1mm\x1b[34;1mi\x1b[36mb\x1b[33;1ma\x1b[35;1mc\x1b[0m"); //Printing colored amibac text. //Unused because of amibac logo on bottom.
		printf(currentMenuText); //Printing this menu's text. Affected by updateMenuText()
		printf("\x1b[30;16HPress Start to exit."); //Exit instruction at bottom.

		hidScanInput();
		u32 kUp = hidKeysUp(); //Detect key releases

		if (kUp & KEY_START) break; //Terminate on start button

		if (kUp & KEY_B) { //On Key B
			currentMenu = menuParent[currentMenu]; //Switch to the parent of the current menu
			updateMenuText();
		}

		if (kUp & KEY_A) { //On Key A
			if (currentMenu == 0) { //If main menu
				int currentCursor = cursors[currentMenu]; //Update currentcursor variable
				if (currentCursor == 0) { //If user selected Backup menu item
					currentMenu = 1; //Set currentmenu to backup menu
					updateMenuText();
					DIR* pDir;
					struct dirent *ent;
					int i = 0;
					if ((pDir = opendir("sdmc:/amibac/saves/")) != NULL){
						while ((ent = readdir(pDir)) != NULL) {
							if (i == 5) {
								break;
							}
							strncpy(currentFileListing[i], ent->d_name, strlen(ent->d_name));
							printf("%d\n", strlen(ent->d_name));
							currentFileListing[i][strlen(ent->d_name)+1] = '\0';
							i++;
						}
						closedir(pDir);
						//snprintf(currentMenuText, 0x1001, currentMenuText, (int)floor(strlen(currentFileListing[0]) / 2), currentFileListing[0], (int)floor(strlen(currentFileListing[1]) / 2), currentFileListing[1], (int)floor(strlen(currentFileListing[2]) / 2), currentFileListing[2], (int)floor(strlen(currentFileListing[3]) / 2), currentFileListing[3], (int)floor(strlen(currentFileListing[4]) / 2), currentFileListing[4]); //doesn't work?
						printf("%d\n", (int)floor(strlen(currentFileListing[0]) / 2));
						printf("%s\n", currentFileListing[0]);
						printf("%s\n", currentFileListing[1]);
						printf("%s\n", currentFileListing[2]);
						printf("%s\n", currentFileListing[3]);
						printf("%s\n", currentFileListing[4]);
					} else {
						printf("\nFailed to get files\n");
						printf("\nPress B to go back to the main menu.\n");
					}
				}
			}
		}

		if (kUp & KEY_DOWN) { //On D-Pad Down
			if (cursors[currentMenu] == cursorsMax[currentMenu]){
				cursors[currentMenu] = 0; //Reset cursor when passing max value
			} else {
				cursors[currentMenu]++; //Increment cursor counter
			}
			//printf("%d/%d\n", cursors[0], cursorsMax[0]); //Debugging this method.
		}

		if (kUp & KEY_UP) { //On D-Pad Up
			if (cursors[currentMenu] == 0){
				cursors[currentMenu] = cursorsMax[currentMenu]; //Reset cursor when passing max value
			} else {
				cursors[currentMenu]--; //Decrement cursor counter
			}
			//printf("%d/%d\n", cursors[0], cursorsMax[0]); //Debugging this method.
		}

		//Old, unused CursorHandler (used when only 1 menu was present.)
		/*if (cursors[0] == 0) {
			printf("\x1b[%d;%dH>", cursorsX[currentMenu], 17);
		} else {
			printf("\x1b[%d;%dH.", cursorsX[currentMenu], 17);
		}
		if (cursors[0] == 1) {
			printf("\x1b[%d;%dH>", cursorsX[0]+1, 17);
		} else {
			printf("\x1b[%d;%dH.", cursorsX[currentMenu]+1, 17);
		}
		if (cursors[0] == 2) {
			printf("\x1b[%d;%dH>", cursorsX[0]+2, 17);
		} else {
			printf("\x1b[%d;%dH.", cursorsX[0]+2, 17);
		}*/

		//CursorHandler
		for (int i = 0; i <= cursorsMax[currentMenu]; ++i) { //For each menu items
			if (cursors[currentMenu] == i){ //If current menu selection is the current line in i
				printf("\x1b[%d;%dH>", cursorsX[currentMenu]+i, 2); //Display ">" indicating it's selected
			} else {
				printf("\x1b[%d;%dH.", cursorsX[currentMenu]+i, 2); //Display "." indicating it's not selected
			}
		}

		gfxFlushBuffers();
		gfxSwapBuffers();
		gspWaitForVBlank();
	}


	deinit();
	return 0;
}