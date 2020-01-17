#include <windows.h>
#include <tchar.h>

#include "..\ACCSharedMemory\ACCSharedMemory.h"

#include <stdio.h>

struct ACCPhysics *phy;
struct ACCGraphics *gra;
struct ACCStatic *sta;
HANDLE dumpH;

int main(int argc, char *argv[]) {
	int err = 0;
	int dump = 0;

	if (argc > 1 && !strcmp(argv[1], "dump")) {
		dump = 1;
	}

	HANDLE phyMap = OpenFileMapping(FILE_MAP_READ, FALSE, TEXT("Local\\acpmf_physics"));
	if (!phyMap) {
		fprintf(stderr, "Error: open file mapping for ACCPhysics.\n");
		err = 1;
	}
	HANDLE graMap = OpenFileMapping(FILE_MAP_READ, FALSE, TEXT("Local\\acpmf_graphics"));
	if (!graMap) {
		fprintf(stderr, "Error: open file mapping for ACCGraphics.\n");
		err = 1;
	}
	HANDLE staMap = OpenFileMapping(FILE_MAP_READ, FALSE, TEXT("Local\\acpmf_static"));
	if (!staMap) {
		fprintf(stderr, "Error: open file mapping for ACCStatic.\n");
		err = 1;
	}
	if (err) {
		fprintf(stderr, "Exiting with code %d\n", err);
		return err;
	}

	phy = (struct ACCPhysics*) MapViewOfFile(phyMap, FILE_MAP_READ, 0, 0, 0);
	if (!phy) {
		fprintf(stderr, "Error: mapping view ACCPhysics.\n");
		err = 3;
	}
	gra = (struct ACCGraphics*) MapViewOfFile(graMap, FILE_MAP_READ, 0, 0, 0);
	if (!gra) {
		fprintf(stderr, "Error: mapping view ACCGraphics.\n");
		err = 3;
	}
	sta = (struct ACCStatic*) MapViewOfFile(staMap, FILE_MAP_READ, 0, 0, 0);
	if (!sta) {
		err = 3;
		fprintf(stderr, "Error: mapping view ACCStatic.\n");
	}
	if (err) {
		fprintf(stderr, "Exiting with code %d\n", err);
		return err;
	}

	if (dump) {
		fprintf(stderr, "Read files at 50Hz and dump contents to accdump.bin\n");
		// Create a local file and save the handle.
		dumpH = CreateFile(TEXT("accdump.bin"), GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (INVALID_HANDLE_VALUE == dumpH) {
			fprintf(stderr, "Could not create accdump.bin\n");
			return 20;
		}
		// create a multimedia timer that calls us 1/50 sec = every 20msec
	} else {
		HANDLE comPort = CreateFile(L"\\.\\COM3", GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
		if (comPort == INVALID_HANDLE_VALUE) {
			fprintf(stderr, "Error: open serial port \\.\\COM3\n");
			fprintf(stderr, "Exiting with code 10\n");
			return 10;
		}
	}
	// we must wait until something happens after we set a multimedia timer?
	// Wait on a semaphore?
	// A Mutex?
	getch();
	return 0;
}