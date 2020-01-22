#include <windows.h>
#include <tchar.h>

#include "..\ACCSharedMemory\ACCSharedMemory.h"

#include <stdio.h>

mapAcpmf(struct ACCPhysics **phy, struct ACCGraphics **gra, struct ACCStatic **sta) {
	int err = 0;

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
		return err;
	}

	*phy = (struct ACCPhysics *) MapViewOfFile(phyMap, FILE_MAP_READ, 0, 0, 0);
	if (!phy) {
		fprintf(stderr, "Error: mapping view ACCPhysics.\n");
		err = 2;
	}
	*gra = (struct ACCGraphics *) MapViewOfFile(graMap, FILE_MAP_READ, 0, 0, 0);
	if (!gra) {
		fprintf(stderr, "Error: mapping view ACCGraphics.\n");
		err = 2;
	}
	*sta = (struct ACCStatic *) MapViewOfFile(staMap, FILE_MAP_READ, 0, 0, 0);
	if (!sta) {
		err = 2;
		fprintf(stderr, "Error: mapping view ACCStatic.\n");
	}

	return err;
}

doSend(void) {
	struct ACCPhysics *phy;
	struct ACCGraphics *gra;
	struct ACCStatic *sta;

	if (mapAcpmf(&phy, &gra, &sta)) {
		return 1;
	}

	HANDLE comPort = CreateFile(L"\\.\\COM3", GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (comPort == INVALID_HANDLE_VALUE) {
		fprintf(stderr, "Error: open serial port \\.\\COM3\n");
		return 2;
	}

	return 0;
}

doDump(void) {
	struct ACCPhysics *phy;
	struct ACCGraphics *gra;
	struct ACCStatic *sta;

	if (mapAcpmf(&phy, &gra, &sta)) {
		return 1;
	}

	fprintf(stderr, "Read memory at 50Hz and dump contents to accdump.bin\n");
	HANDLE dumpFile = CreateFile(TEXT("accdump.bin"), GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == dumpFile) {
		fprintf(stderr, "Error: create accdump.bin\n");
		return 2;
	}
	HANDLE dumpTimer = CreateWaitableTimer(NULL, FALSE, NULL);
	if (NULL == dumpTimer) {
		fprintf(stderr, "Error: create timer.\n");
		return 3;
	}
	LARGE_INTEGER dueTime;
	dueTime.QuadPart = -200000LL; // 20ms == 50Hz
	if (!SetWaitableTimer(dumpTimer, &dueTime, 20, NULL, NULL, FALSE)) {
		printf("Error: SetWaitableTimer: %d\n", GetLastError());
		return 4;
	}
	DWORD bytesWritten;
	while (WaitForSingleObject(dumpTimer, INFINITE) == WAIT_OBJECT_0) {
		WriteFile(dumpFile, phy, sizeof(*phy), &bytesWritten, NULL);
		WriteFile(dumpFile, gra, sizeof(*gra), &bytesWritten, NULL);
		WriteFile(dumpFile, sta, sizeof(*sta), &bytesWritten, NULL);
	}
	fprintf(stderr, "Error: WaitForSingleObject: %d\n", GetLastError());
	return 5;
}

doCsv(void) {
	fprintf(stderr, "Read accdump.bin contents and write into accdump.csv\n");
	HANDLE csvFile = CreateFile(TEXT("accdump.csv"), GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == csvFile) {
		fprintf(stderr, "Error: create accdump.csv: %d\n", GetLastError());
		return 1;
	}
	HANDLE binFile = CreateFile(TEXT("accdump.bin"), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == csvFile) {
		fprintf(stderr, "Error: open accdump.bin: %d\n", GetLastError());
		return 2;
	}
	// read from the dumpbin file
	// loop through, populate strcutures
	// each iteration, dump as a CSV
	// select a few parameters
	// they can be arguments in the future.
	return 0;
}

main(int argc, char *argv[]) {
	enum { SEND, DUMP, CSV } action = SEND;
	
	if (argc > 1) {
		if (!strcmp(argv[1], "dump")) {
			action = DUMP;
		}
		else if (!strcmp(argv[1], "csv")) {
			action = CSV;
		}
	}

	if (action == SEND) {
		return doSend();
	} else if (action == DUMP) {
		return doDump();
	} else if (action == CSV) {
		return doCsv();
	}

	return 0;
}