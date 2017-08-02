#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <3ds.h>
#include <3ds/ipc.h>

Result nfcmSetAmiiboSettings(NFC_AmiiboSettings data){
	Handle h = 0;
	Result ret = 0;
	ret = srvGetServiceHandle
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x0404;
	memcpy(&cmdbuf[1], &data, sizeof(NFC_AmiiboSettings))
	//if (R_FAILED(ret = svcSendSyncRequest(nfcHandle)))
}

Result nfcLoop()
{
	Result ret = 0;
	u32 pos;
	FILE *f;
	size_t tmpsize;

	NFC_TagState prevstate, curstate;
	NFC_TagInfo taginfo;
	NFC_AmiiboSettings amiibosettings;
	NFC_AmiiboConfig amiiboconfig;

	u32 amiibo_appid = 0x00000000;
	u32 appid_db[5] = {//Amiibo APPID database from 3dsbrew wiki.
					   0x10110E00,
					   0x0014F000,
					   0x00152600,
					   0x00132600,
					   0x1019C800};
	u32 appdata_initialized;

	u8 appdata[0xd8];

	char uidstr[16];
	char tmpstr[64];

	ret = nfcStartScanning(NFC_STARTSCAN_DEFAULTINPUT);
	if (R_FAILED(ret))
	{
		//CRASH
		consoleClear();
		printf("\x1b[31;1mUh-oh! A problem occurred.\x1b[0m\n");
		printf("It seems like the startScanning returned \x1b[31m0x%08x\x1b[0m...\n", (unsigned int)ret);
		printf("Feel free to ask \x1b[33;1mskiilaa\x1b[0m about the problem!\n\n");
		printf("\x1b[34;1mPress \x1b[33;1mB \x1b[34;1mto exit.");
		return ret;
	}

	ret = nfcGetTagState(&curstate);
	if (R_FAILED(ret))
	{
		//CRASH
		consoleClear();
		printf("\x1b[31;1mUh-oh! A problem occurred.\x1b[0m\n");
		printf("It seems like the getTagState returned \x1b[31m0x%08x\x1b[0m...\n", (unsigned int)ret);
		printf("Feel free to ask \x1b[33;1mskiilaa\x1b[0m about the problem!\n\n");
		printf("\x1b[34;1mPress \x1b[33;1mB \x1b[34;1mto exit.");
		nfcStopScanning();
		return ret;
	}

	prevstate = curstate;

	//Print instructions
	printf("\x1b[34;1mScan Amiibo to back it up.\n");
	printf("Hold down A and scan Amiibo to restore it.\n");
	printf("Press Y to restart scanning.\n");
	printf("Press X to re-scan Amiibo if it's still in range.\n");
	printf("Hold B to exit.\n");
	printf("Only remove Amiibo when instructed.\x1b[0m\n");

	while (1)
	{
		gspWaitForVBlank();
		hidScanInput();

		u32 kUp = hidKeysUp();

		if (hidKeysHeld() & KEY_B) //Using hidKeysHeld() because the break goes thru' 2 loops.
			break;

		if (kUp & KEY_Y) //Restart scanning
		{
			nfcStopScanning();
			ret = nfcStartScanning(NFC_STARTSCAN_DEFAULTINPUT);
			if (R_FAILED(ret))
			{
				//CRASH
				consoleClear();
				printf("\x1b[31;1mUh-oh! A problem occurred.\x1b[0m\n");
				printf("It seems like the startScanning returned \x1b[31m0x%08x\x1b[0m...\n", (unsigned int)ret);
				printf("Feel free to ask \x1b[33;1mskiilaa\x1b[0m about the problem!\n\n");
				printf("\x1b[34;1mPress \x1b[33;1mB \x1b[34;1mto exit.");
				return ret;
			}
			printf("\x1b[32;1mScanning restarted!\n");
			continue;
		}

		if (kUp & KEY_X) //Re-scan amiibo
		{
			ret = nfcResetTagScanState();
			if (R_FAILED(ret))
			{
				//CRASH
				consoleClear();
				printf("\x1b[31;1mUh-oh! A problem occurred.\x1b[0m\n");
				printf("It seems like the resetTagScanState returned \x1b[31m0x%08x\x1b[0m...\n", (unsigned int)ret);
				printf("Feel free to ask \x1b[33;1mskiilaa\x1b[0m about the problem!\n\n");
				printf("\x1b[34;1mPress \x1b[33;1mB \x1b[34;1mto exit.");
				break;
			}
		}

		ret = nfcGetTagState(&curstate);
		if (R_FAILED(ret))
		{
			//CRASH
			consoleClear();
			printf("\x1b[31;1mUh-oh! A problem occurred.\x1b[0m\n");
			printf("It seems like the getTagState returned \x1b[31m0x%08x\x1b[0m...\n", (unsigned int)ret);
			printf("Feel free to ask \x1b[33;1mskiilaa\x1b[0m about the problem!\n\n");
			printf("\x1b[34;1mPress \x1b[33;1mB \x1b[34;1mto exit.");
			break;
		}

		if (curstate != prevstate)
		{
			prevstate = curstate;

			if (curstate == NFC_TagState_InRange)
			{
				memset(&taginfo, 0, sizeof(NFC_TagInfo));
				memset(&amiibosettings, 0, sizeof(NFC_AmiiboSettings));

				ret = nfcGetTagInfo(&taginfo);
				if (R_FAILED(ret))
				{
					//CRASH
					consoleClear();
					printf("\x1b[31;1mUh-oh! A problem occurred.\x1b[0m\n");
					printf("It seems like the getTagInfo returned \x1b[31m0x%08x\x1b[0m...\n", (unsigned int)ret);
					printf("Feel free to ask \x1b[33;1mskiilaa\x1b[0m about the problem!\n\n");
					printf("\x1b[34;1mPress \x1b[33;1mB \x1b[34;1mto exit.");
					break;
				}

				memset(uidstr, 0, sizeof(uidstr));
				for (pos = 0; pos < 7; pos++)
					snprintf(&uidstr[pos * 2], 3, "%02x", taginfo.id[pos]);

				ret = nfcLoadAmiiboData();
				if (R_FAILED(ret))
				{
					//CRASH
					consoleClear();
					printf("\x1b[31;1mUh-oh! A problem occurred.\x1b[0m\n");
					printf("It seems like the loadAmiiboData returned \x1b[31m0x%08x\x1b[0m...\n", (unsigned int)ret);
					printf("Feel free to ask \x1b[33;1mskiilaa\x1b[0m about the problem!\n\n");
					printf("\x1b[34;1mPress \x1b[33;1mB \x1b[34;1mto exit.");
					break;
				}

				appdata_initialized = 1;

				bool foundAppId = false;
				for (int i = 0; i < 5 /* 5 = length of our appid database */ && !foundAppId; i++)
				{ //Loop thru' appid database and find the currently used one
					ret = nfcOpenAppData(appid_db[i]);
					if (R_FAILED(ret))
					{
						//CRASH
						printf("nfcOpenAppData() failed.\n");
						if (ret == NFC_ERR_APPDATA_UNINITIALIZED)
						{
							printf("This appdata isn't initialized.\n");
							appdata_initialized = 0;
						}
						if (appdata_initialized)
						{
							break;
						}
					}
					else
					{
						amiibo_appid = appid_db[i];
						foundAppId = true;
						break;
					}
				}

				if (!foundAppId)
				{
					//CRASH (not found appid)
					consoleClear();
					printf("\x1b[31;1mUh-oh! A problem occurred.\x1b[0m\n");
					printf("It seems like this Amiibo's AppID is missing from our database...\n");
					printf("Feel free to ask \x1b[33;1mskiilaa\x1b[0m about the problem!\n\n");
					printf("\x1b[34;1mPress \x1b[33;1mB \x1b[34;1mto exit.");
					break;
				}

				if (hidKeysHeld() & KEY_A)
				{
					struct stat st = {0};
					if (stat("/amiibo", &st) == -1) //if amiibo folder doesn't exist
					{
						printf("\x1b[31;1mNo amiibo folder found!\x1b[33;1m\n");
					}
					if (stat("/amiibo/restore", &st) == -1) //if restore folder doesn't exist
					{
						printf("\x1b[31;1mNo restore folder found!\x1b[33;1m\n");
					}
					else
					{
						memset(tmpstr, 0, sizeof(tmpstr));
						snprintf(tmpstr, sizeof(tmpstr) - 1, "/amiibo/restore/appdata_%s.bin", uidstr);

						printf("\x1b[33;1mRestoring Amiibo from file '%s'...\n", tmpstr);

						ret = 1;

						f = fopen(tmpstr, "r");
						if (f)
						{
							memset(appdata, 0, sizeof(appdata));
							tmpsize = fread(appdata, 1, sizeof(appdata), f);
							fclose(f);

							if (tmpsize != sizeof(appdata))
							{
								//CRASH
								consoleClear();
								printf("\x1b[31;1mUh-oh! A problem occurred.\x1b[0m\n");
								printf("It seems like the we weren't able to read the file...\n");
								printf("Failed to read the file, only 0x%x of 0x%x bytes were read.\n", tmpsize, sizeof(appdata));
								printf("\x1b[31mYOUR AMIIBO SHOULD NOT BE CORRUPTED!\x1b[0m\n");
								printf("Feel free to ask \x1b[33;1mskiilaa\x1b[0m about the problem!\n\n");
								printf("\x1b[34;1mPress \x1b[33;1mB \x1b[34;1mto exit.");
								break;
							}
							else
							{
								printf("Reading backup finished!\n");
								ret = 0;
							}
						}
						else
						{
							//CRASH
							consoleClear();
							printf("\x1b[31;1mUh-oh! A problem occurred.\x1b[0m\n");
							printf("It seems like the we weren't able to open the file...\n");
							printf("Feel free to ask \x1b[33;1mskiilaa\x1b[0m about the problem!\n\n");
							printf("\x1b[34;1mPress \x1b[33;1mB \x1b[34;1mto exit.");
							break;
						}

						if (ret == 0)
						{
							if (appdata_initialized)
							{
								printf("Writing to Amiibo...\n\x1b[34;1mDO NOT REMOVE FROM NFC READER!\x1b[33;1m\n");

								ret = nfcWriteAppData(appdata, sizeof(appdata), &taginfo);
								if (R_FAILED(ret))
								{
									//CRASH
									consoleClear();
									printf("\x1b[31;1mUh-oh! A problem occurred.\x1b[0m\n");
									printf("It seems like the we weren't able to write to the Amiibo...\n");
									printf("Feel free to ask \x1b[33;1mskiilaa\x1b[0m about the problem!\n\n");
									printf("\x1b[34;1mPress \x1b[33;1mB \x1b[34;1mto exit.");
									break;
								}

								ret = nfcUpdateStoredAmiiboData();
								if (R_FAILED(ret))
								{
									//CRASH
									consoleClear();
									printf("\x1b[31;1mUh-oh! A problem occurred.\x1b[0m\n");
									printf("It seems like the we weren't able to update the Amiibo data...\n");
									printf("Feel free to ask \x1b[33;1mskiilaa\x1b[0m about the problem!\n\n");
									printf("\x1b[34;1mPress \x1b[33;1mB \x1b[34;1mto exit.");
									break;
								}
							}
							else
							{
								printf("Initializing Amiibo appdata...\n");

								ret = nfcInitializeWriteAppData(amiibo_appid, appdata, sizeof(appdata));
								if (R_FAILED(ret))
								{
									//CRASH
									consoleClear();
									printf("\x1b[31;1mUh-oh! A problem occurred.\x1b[0m\n");
									printf("It seems like the we weren't able to write to the Amiibo...\n");
									printf("Feel free to ask \x1b[33;1mskiilaa\x1b[0m about the problem!\n\n");
									printf("\x1b[34;1mPress \x1b[33;1mB \x1b[34;1mto exit.");
									break;
								}
							}

							printf("\x1b[32;1mBackup successful!\x1b[0m\n");
						}
					}
				}
				else
				{
					memset(&amiibosettings, 0, sizeof(NFC_AmiiboSettings));
					memset(&amiiboconfig, 0, sizeof(NFC_AmiiboConfig));

					ret = nfcGetAmiiboSettings(&amiibosettings);
					if (R_FAILED(ret))
					{
						//CRASH
						consoleClear();
						printf("\x1b[31;1mUh-oh! A problem occurred.\x1b[0m\n");
						if (ret == NFC_ERR_AMIIBO_NOTSETUP)
						{
							printf("It seems like this Amiibo wasn't setup by the Amiibo Settings applet...\n");
						}
						else
						{
							printf("It seems like the loadAmiiboData returned \x1b[31m0x%08x\x1b[0m...\n", (unsigned int)ret);
						}
						printf("Feel free to ask \x1b[33;1mskiilaa\x1b[0m about the problem!\n\n");
						printf("\x1b[34;1mPress \x1b[33;1mB \x1b[34;1mto exit.");
						break;
					}

					ret = nfcGetAmiiboConfig(&amiiboconfig);
					if (R_FAILED(ret))
					{
						//CRASH
						consoleClear();
						printf("\x1b[31;1mUh-oh! A problem occurred.\x1b[0m\n");
						printf("It seems like the getAmiiboConfig returned \x1b[31m0x%08x\x1b[0m...\n", (unsigned int)ret);
						printf("Feel free to ask \x1b[33;1mskiilaa\x1b[0m about the problem!\n\n");
						printf("\x1b[34;1mPress \x1b[33;1mB \x1b[34;1mto exit.");
						break;
					}
					printf("\x1b[33;1mBacking up Amiibo ID \x1b[32;1m%s\x1b[33;1m...\n", uidstr);
					printf("Amiibo data successfully loaded.\n");

					printf("Last Write Date: Year %u, Month %u, Day %u\n", amiiboconfig.lastwritedate_year, amiiboconfig.lastwritedate_month, amiiboconfig.lastwritedate_day);
					printf("Amiibo was written %u times.\n", amiiboconfig.write_counter);

					printf("Opening appdata...\n");
					memset(appdata, 0, sizeof(appdata));

					if (!appdata_initialized)
					{
						printf("Uninitialized data: nothing to back up.\n");
					}
					else
					{
						printf("Reading appdata...\n");

						ret = nfcReadAppData(appdata, sizeof(appdata));
						if (R_FAILED(ret))
						{
							//CRASH
							consoleClear();
							printf("\x1b[31;1mUh-oh! A problem occurred.\x1b[0m\n");
							printf("It seems like readAppData returned \x1b[31m0x%08x\x1b[0m...\n", (unsigned int)ret);
							printf("Feel free to ask \x1b[33;1mskiilaa\x1b[0m about the problem!\n\n");
							printf("\x1b[34;1mPress \x1b[33;1mB \x1b[34;1mto exit.");
							break;
						}

						/*printf("Appdata:\n");
					for (pos = 0; pos < sizeof(appdata); pos++)
						printf("%02x", appdata[pos]);
					printf("\n");*/ //not printing appdata RN

						memset(tmpstr, 0, sizeof(tmpstr));
						snprintf(tmpstr, sizeof(tmpstr) - 1, "/amiibo/backup/appdata_%s.bin", uidstr);

						struct stat st = {0};
						if (stat("/amiibo", &st) == -1) //if amiibo folder doesn't exist
						{
							mkdir("/amiibo/", 0700);
						}
						if (stat("/amiibo/backup", &st) == -1) //if backup folder doesn't exist
						{
							mkdir("/amiibo/backup/", 0700);
						}
						printf("Backing up to file '%s'...\n", tmpstr);
						f = fopen(tmpstr, "w");
						if (f)
						{
							tmpsize = fwrite(appdata, 1, sizeof(appdata), f);
							fclose(f);

							if (tmpsize != sizeof(appdata))
							{
								//CRASH
								consoleClear();
								printf("\x1b[31;1mUh-oh! A problem occurred.\x1b[0m\n");
								printf("It seems like the we weren't able to write to the file...\n");
								printf("Failed to write to the file, only 0x%x of 0x%x bytes were written.\n", tmpsize, sizeof(appdata));
								printf("Feel free to ask \x1b[33;1mskiilaa\x1b[0m about the problem!\n\n");
								printf("\x1b[34;1mPress \x1b[33;1mB \x1b[34;1mto exit.");
								break;
							}
							else
							{
								printf("Backed up!\n");
							}
						}
						else
						{
							//CRASH
							consoleClear();
							printf("\x1b[31;1mUh-oh! A problem occurred.\x1b[0m\n");
							printf("It seems like the we weren't able to open the file...\n");
							printf("Feel free to ask \x1b[33;1mskiilaa\x1b[0m about the problem!\n\n");
							printf("\x1b[34;1mPress \x1b[33;1mB \x1b[34;1mto exit.");
							break;
						}
					}
				}

				printf("\x1b[32;1mProcessing finished!\nRemove the Amiibo.\x1b[0m\n");
				printf("\x1b[34;1mPress Y to restart scanning.\x1b[33;1m\n");
			}
			gfxSwapBuffers();
		}
	}

	nfcStopScanning();

	return ret;
}

int main()
{
	Result ret = 0;

	gfxInitDefault();
	consoleInit(GFX_TOP, NULL);

	printf("Welcome to \x1b[31;1ma\x1b[32;1mm\x1b[34;1mi\x1b[36mb\x1b[33;1ma\x1b[35;1mc\x1b[0m!\n");

	ret = nfcInit(NFC_OpType_NFCTag);
	if (R_FAILED(ret))
	{
		//CRASH
		consoleClear();
		printf("\x1b[31;1mUh-oh! A problem occurred.\x1b[0m\n");
		printf("It seems like the INIT returned \x1b[31m0x%08x\x1b[0m...\n", (unsigned int)ret);
		printf("Feel free to ask \x1b[33;1mskiilaa\x1b[0m about the problem!\n\n");
		printf("\x1b[34;1Press \x1b[33;1mB \x1b[34;1mto exit.");
	}
	else
	{
		ret = nfcLoop();
		nfcExit();
	}

	// Main loop
	while (aptMainLoop())
	{
		gspWaitForVBlank();
		hidScanInput();

		if (hidKeysHeld() & KEY_B)
		{ //Using hidKeysHeld because button B is coming from nfcLoop()
			consoleClear();
			printf("\x1b[0mGoodbye!\nRemember: a backup a day keeps corruption away!");
			svcSleepThread(3000000000); //Sleep for 1 seconds so message is readable
			break;						// break in order to return to hbmenu
		}
	}

	gfxExit();
	return 0;
}