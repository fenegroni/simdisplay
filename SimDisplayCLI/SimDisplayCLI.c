#include <windows.h>
#include <tchar.h>

#include "..\ACCSharedMemory\ACCSharedMemory.h"

#include <stdio.h>


struct ACCPhysics *phy;
struct ACCGraphics *gra;
struct ACCStatic *sta;

int main(int argc, char *argv[]) {
	int err = 0;
	int dump = 0;

	if (argc > 1 && !strcmp(argv[1], "dump")) {
		dump = 1;
	}

	HANDLE phyMap;
	while (!(phyMap = OpenFileMapping(FILE_MAP_READ, FALSE, TEXT("Local\\acpmf_physics")))) {
		fprintf(stderr, "Waiting: open file mapping for ACCPhysics.\n");
		Sleep(1000);
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
		HANDLE dumpFile = CreateFile(TEXT("accdump.bin"), GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (INVALID_HANDLE_VALUE == dumpFile) {
			fprintf(stderr, "Error: create accdump.bin\n");
			return 20;
		}
		HANDLE dumpTimer = CreateWaitableTimer(NULL, FALSE, NULL);
		if (NULL == dumpTimer) {
			fprintf(stderr, "Error: create timer.\n");
			return 30;
		}
		LARGE_INTEGER dueTime;
		dueTime.QuadPart = -200000LL; // 20ms == 50Hz
		if (!SetWaitableTimer(dumpTimer, &dueTime, 20, NULL, NULL, FALSE)) {
			printf("Error: SetWaitableTimer: %d\n", GetLastError());
			return 40;
		}
		DWORD bytesWritten;
		while (WaitForSingleObject(dumpTimer, INFINITE) == WAIT_OBJECT_0) {
			WriteFile(dumpFile, phy, sizeof(*phy), &bytesWritten, NULL);
			WriteFile(dumpFile, gra, sizeof(*gra), &bytesWritten, NULL);
			WriteFile(dumpFile, sta, sizeof(*sta), &bytesWritten, NULL);
		}
		fprintf(stderr, "Error: WaitForSingleObject: %d\n", GetLastError());
	} else {
		HANDLE comPort = CreateFile(L"\\.\\COM3", GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
		if (comPort == INVALID_HANDLE_VALUE) {
			fprintf(stderr, "Error: open serial port \\.\\COM3\n");
			fprintf(stderr, "Exiting with code 10\n");
			return 10;
		}
	}

	return 0;
}