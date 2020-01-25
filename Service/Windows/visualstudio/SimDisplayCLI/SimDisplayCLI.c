#include <windows.h>
#include <tchar.h>

#include "..\ACCSharedMemory\ACCSharedMemory.h"

#include <stdio.h>

typedef enum {
	ERR_NONE,
	ERR_TIMER,
	ERR_MAP,
	ERR_COM,
	ERR_FILE, ERR_FILE_WRITE, ERR_FILE_READ
} Errorcode;

Errorcode mapAcpmf(struct ACCPhysics **phy, struct ACCGraphics **gra, struct ACCStatic **sta)
{
	Errorcode err = ERR_NONE;

	HANDLE phyMap;
	while (!(phyMap = OpenFileMapping(FILE_MAP_READ, FALSE, TEXT("Local\\acpmf_physics")))) {
		fprintf(stderr, "Waiting: open file mapping for ACCPhysics.\n");
		Sleep(1000);
	}
	HANDLE graMap = OpenFileMapping(FILE_MAP_READ, FALSE, TEXT("Local\\acpmf_graphics"));
	if (!graMap) {
		fprintf(stderr, "Error: open file mapping for ACCGraphics.\n");
		err = ERR_MAP;
	}
	HANDLE staMap = OpenFileMapping(FILE_MAP_READ, FALSE, TEXT("Local\\acpmf_static"));
	if (!staMap) {
		fprintf(stderr, "Error: open file mapping for ACCStatic.\n");
		err = ERR_MAP;
	}
	if (err) {
		return err;
	}

	*phy = (struct ACCPhysics *) MapViewOfFile(phyMap, FILE_MAP_READ, 0, 0, 0);
	if (!phy) {
		fprintf(stderr, "Error: mapping view ACCPhysics.\n");
		err = ERR_MAP;
	}
	*gra = (struct ACCGraphics *) MapViewOfFile(graMap, FILE_MAP_READ, 0, 0, 0);
	if (!gra) {
		fprintf(stderr, "Error: mapping view ACCGraphics.\n");
		err = ERR_MAP;
	}
	*sta = (struct ACCStatic *) MapViewOfFile(staMap, FILE_MAP_READ, 0, 0, 0);
	if (!sta) {
		err = ERR_MAP;
		fprintf(stderr, "Error: mapping view ACCStatic.\n");
	}

	return err;
}

Errorcode doSend(void)
{
	Errorcode err;
	
	struct ACCPhysics *phy;
	struct ACCGraphics *gra;
	struct ACCStatic *sta;

	if (err = mapAcpmf(&phy, &gra, &sta)) {
		return err;
	}

	HANDLE comPort = CreateFile(L"\\.\\COM3", GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (comPort == INVALID_HANDLE_VALUE) {
		fprintf(stderr, "Error: open serial port \\.\\COM3\n");
		return ERR_COM;
	}

	return 0;
}

Errorcode doDump(void)
{
	Errorcode err;
	
	struct ACCPhysics *phy;
	struct ACCGraphics *gra;
	struct ACCStatic *sta;

	if (err = mapAcpmf(&phy, &gra, &sta)) {
		return err;
	}

	fprintf(stderr, "Read memory at 50Hz and dump contents to accdump.bin\n");
	HANDLE dumpFile = CreateFile(TEXT("accdump.bin"), GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == dumpFile) {
		fprintf(stderr, "Error: create accdump.bin\n");
		return ERR_FILE;
	}
	HANDLE dumpTimer = CreateWaitableTimer(NULL, FALSE, NULL);
	if (NULL == dumpTimer) {
		fprintf(stderr, "Error: create timer.\n");
		return ERR_TIMER;
	}
	LARGE_INTEGER dueTime;
	dueTime.QuadPart = -200000LL; // 20ms == 50Hz
	if (!SetWaitableTimer(dumpTimer, &dueTime, 20, NULL, NULL, FALSE)) {
		printf("Error: SetWaitableTimer: %d\n", GetLastError());
		return ERR_TIMER;
	}
	DWORD bytesWritten;
	while (WaitForSingleObject(dumpTimer, INFINITE) == WAIT_OBJECT_0) {
		WriteFile(dumpFile, phy, sizeof(*phy), &bytesWritten, NULL);
		WriteFile(dumpFile, gra, sizeof(*gra), &bytesWritten, NULL);
		WriteFile(dumpFile, sta, sizeof(*sta), &bytesWritten, NULL);
	}
	fprintf(stderr, "Error: WaitForSingleObject: %d\n", GetLastError());
	return ERR_TIMER;
}

Errorcode doCsv(void)
{
	fprintf(stderr, "Read accdump.bin contents and write into accdump.csv\n");
	HANDLE csvFile = CreateFile(TEXT("accdump.csv"), GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == csvFile) {
		fprintf(stderr, "Error: create accdump.csv: %d\n", GetLastError());
		return ERR_FILE;
	}
	HANDLE binFile = CreateFile(TEXT("accdump.bin"), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == binFile) {
		fprintf(stderr, "Error: open accdump.bin: %d\n", GetLastError());
		return ERR_FILE;
	}
	int maxCsvRecord = 8192;
	char *csvRecord = malloc(maxCsvRecord);
	if (!csvRecord) ExitProcess(1);
	DWORD writtenBytes;
	if (!WriteFile(csvFile, csvRecord,
			snprintf(csvRecord, maxCsvRecord, "Status,Damage 0,Damage 1,Damage 2,Damage 3,Damage 4,PHY Pid,TC Active,ABS Active,TC,TC Cut,ABS,Lights Stage,Flashing Lights\n"),
			&writtenBytes, NULL)) {
		fprintf(stderr, "Error: write CSV header: %d\n", GetLastError());
		return ERR_FILE_WRITE;
	}
	int binBufferSize = sizeof(struct ACCPhysics) + sizeof(struct ACCGraphics) + sizeof(struct ACCStatic);
	char *binBuffer = malloc(binBufferSize);
	if (!binBuffer) ExitProcess(1);
	struct ACCPhysics *phy = (struct ACCPhysics *)binBuffer;
	struct ACCGraphics *gra = (struct ACCGraphics *)(binBuffer + sizeof(struct ACCPhysics));
	struct ACCStatic *sta = (struct ACCStatic *)(binBuffer + sizeof(struct ACCPhysics) + sizeof(struct ACCGraphics));
	DWORD readBytes;
	while (ReadFile(binFile, binBuffer, binBufferSize, &readBytes, NULL) && readBytes == binBufferSize) {
		if (!WriteFile(csvFile, csvRecord,
				snprintf(csvRecord, maxCsvRecord, "%d,%f,%f,%f,%f,%f,%f,%f,%d,%d,%d\n",
					gra->status, phy->carDamage[0], phy->carDamage[1], phy->carDamage[2], phy->carDamage[3], phy->carDamage[4], phy->tc, phy->abs, gra->TC, gra->TCCut, gra->ABS),
				&writtenBytes, NULL)) {
			fprintf(stderr, "Error: write CSV record: %d\n", GetLastError());
			return ERR_FILE_WRITE;
		}
	}
	return 0;
}

Errorcode doReplay(void)
{
	fprintf(stderr, "Read accdump.bin contents and dump into shared memory at 50Hz\n");
	HANDLE phyMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(struct ACCPhysics), TEXT("Local\\acpmf_physics"));
	if (!phyMap) {
		fprintf(stderr, "Error create file mapping for ACCPhysics.\n");
	}
	HANDLE graMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(struct ACCGraphics), TEXT("Local\\acpmf_graphics"));
	if (!graMap) {
		fprintf(stderr, "Error create file mapping for ACCGraphics.\n");
	}
	HANDLE staMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(struct ACCStatic), TEXT("Local\\acpmf_static"));
	if (!staMap) {
		fprintf(stderr, "Error create file mapping for ACCStatic.\n");
	}
	if (!staMap || !graMap || !phyMap) {
		return ERR_MAP;
	}

	struct ACCPhysics *phy = (struct ACCPhysics *) MapViewOfFile(phyMap, FILE_MAP_WRITE, 0, 0, 0);
	if (!phy) {
		fprintf(stderr, "Error mapping view ACCPhysics.\n");
	}
	struct ACCGraphics *gra = (struct ACCGraphics *) MapViewOfFile(graMap, FILE_MAP_WRITE, 0, 0, 0);
	if (!gra) {
		fprintf(stderr, "Error mapping view ACCGraphics.\n");
	}
	struct ACCStatic *sta = (struct ACCStatic *) MapViewOfFile(staMap, FILE_MAP_WRITE, 0, 0, 0);
	if (!sta) {
		fprintf(stderr, "Error mapping view ACCStatic.\n");
	}

	if (!phy || !gra || !sta) {
		return ERR_MAP;
	}

	HANDLE binFile = CreateFile(TEXT("accdump.bin"), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == binFile) {
		fprintf(stderr, "Error: open accdump.bin: %d\n", GetLastError());
		return ERR_FILE;
	}

	HANDLE dumpTimer = CreateWaitableTimer(NULL, FALSE, NULL);
	if (NULL == dumpTimer) {
		fprintf(stderr, "Error: create timer.\n");
		return ERR_TIMER;
	}
	LARGE_INTEGER dueTime;
	dueTime.QuadPart = -200000LL; // 20ms == 50Hz
	if (!SetWaitableTimer(dumpTimer, &dueTime, 20, NULL, NULL, FALSE)) {
		printf("Error: SetWaitableTimer: %d\n", GetLastError());
		return ERR_TIMER;
	}
	while (WaitForSingleObject(dumpTimer, INFINITE) == WAIT_OBJECT_0) {
		DWORD bytesRead;
		if (!ReadFile(binFile, phy, sizeof(*phy), &bytesRead, NULL) || bytesRead < sizeof(*phy)) {
			return ERR_FILE_READ;
		}
		if (!ReadFile(binFile, gra, sizeof(*gra), &bytesRead, NULL) || bytesRead < sizeof(*gra)) {
			return ERR_FILE_READ;
		}
		if (!ReadFile(binFile, sta, sizeof(*sta), &bytesRead, NULL) || bytesRead < sizeof(*sta)) {
			return ERR_FILE_READ;
		}
	}
	fprintf(stderr, "Error: WaitForSingleObject: %d\n", GetLastError());
	return ERR_TIMER;
}

int main(int argc, char *argv[])
{
	enum { SEND, DUMP, CSV, REPLAY } action = SEND;
	
	if (argc > 1) {
		if (!strcmp(argv[1], "dump")) {
			action = DUMP;
		} else if (!strcmp(argv[1], "csv")) {
			action = CSV;
		} else if (!strcmp(argv[1], "replay")) {
			action = REPLAY;
		}
	}

	switch (action) {
	case SEND:
		return doSend();
	case DUMP:
		return doDump();
	case CSV:
		return doCsv();
	case REPLAY:
		return doReplay();
	}

	return 0;
}
