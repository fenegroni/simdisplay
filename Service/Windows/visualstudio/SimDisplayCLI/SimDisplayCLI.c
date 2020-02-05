/*
simdisplay - A simracing dashboard created using Arduino to show shared memory telemetry from Assetto Corsa Competizione.

Copyright (C) 2020  Filippo Erik Negroni

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <windows.h>
#include <tchar.h>

#include <stdio.h>
#include <math.h>
#include <stdint.h>

#include "..\include\ACCSharedMemory.h"
#include "..\include\SimDisplayProtocol.h"

enum mapAcpmf_action {
	MAPACPMF_CREATE,
	MAPACPMF_OPEN_EXISTING
};

const wchar_t acpmf_physics[] = L"Local\\acpmf_physics";
const wchar_t acpmf_graphics[] = L"Local\\acpmf_graphics";
const wchar_t acpmf_static[] = L"Local\\acpmf_static";

int mapAcpmf(enum mapAcpmf_action action, struct ACCPhysics **phy, struct ACCGraphics **gra, struct ACCStatic **sta)
{
	int err = 0;

	HANDLE phyMap, graMap, staMap;

	if (MAPACPMF_OPEN_EXISTING == action) {
		while (!(phyMap = OpenFileMapping(FILE_MAP_READ, FALSE, acpmf_physics))) {
			fprintf(stderr, "Waiting: open file mapping for ACCPhysics.\n");
			Sleep(1000);
			// TODO: don't try forever: exit after 5 minutes?
		}
		graMap = OpenFileMapping(FILE_MAP_READ, FALSE, acpmf_graphics); 
		staMap = OpenFileMapping(FILE_MAP_READ, FALSE, acpmf_static);
	} else {
		phyMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(struct ACCPhysics), acpmf_physics);
		graMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(struct ACCGraphics), acpmf_graphics);
		staMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(struct ACCStatic), acpmf_static);
	}
	if (!phyMap) {
		fprintf(stderr, "Error: %d: file mapping ACCPhysics.\n", action);
		err = 1;
	}
	if (!graMap) {
		fprintf(stderr, "Error: %d: file mapping ACCGraphics.\n", action);
		err = 1;
	}
	if (!staMap) {
		fprintf(stderr, "Error: %d: file mapping ACCStatic.\n", action);
		err = 1;
	}
	if (err) {
		return err;
	}

	*phy = (struct ACCPhysics *) MapViewOfFile(phyMap, (action == MAPACPMF_OPEN_EXISTING) ? FILE_MAP_READ : FILE_MAP_WRITE, 0, 0, 0);
	if (!phy) {
		fprintf(stderr, "Error: mapping view ACCPhysics.\n");
		err = 1;
	}
	*gra = (struct ACCGraphics *) MapViewOfFile(graMap, (action == MAPACPMF_OPEN_EXISTING) ? FILE_MAP_READ : FILE_MAP_WRITE, 0, 0, 0);
	if (!gra) {
		fprintf(stderr, "Error: mapping view ACCGraphics.\n");
		err = 1;
	}
	*sta = (struct ACCStatic *) MapViewOfFile(staMap, (action == MAPACPMF_OPEN_EXISTING) ? FILE_MAP_READ : FILE_MAP_WRITE, 0, 0, 0);
	if (!sta) {
		err = 1;
		fprintf(stderr, "Error: mapping view ACCStatic.\n");
	}

	return err;
}


/* TODO: pass name of comport or list comports and scan for Arduino. */
int doSend(void)
{
	struct ACCPhysics *phy;
	struct ACCGraphics *gra;
	struct ACCStatic *sta;

	if (mapAcpmf(MAPACPMF_OPEN_EXISTING, &phy, &gra, &sta)) {
		return 1;
	}

	HANDLE comPort = CreateFile(L"\\.\\COM3", GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (comPort == INVALID_HANDLE_VALUE) {
		fprintf(stderr, "Error: open serial port \\.\\COM3\n");
		return 1;
	}

	DCB comPortDCB;

	if (!GetCommState(comPort, &comPortDCB)) { //retreives  the current settings
		fprintf(stderr, "Error: get COM state.");
		return 1;
	}

	comPortDCB.BaudRate = CBR_9600;      //BaudRate = 9600
	comPortDCB.ByteSize = 8;             //ByteSize = 8
	comPortDCB.StopBits = ONESTOPBIT;    //StopBits = 1
	comPortDCB.Parity = NOPARITY;      //Parity = None

	if (!SetCommState(comPort, &comPortDCB)) {
		fprintf(stderr, "Error: set COM state.");
		return 1;
	}

	struct SimDisplayPacket packet;

	HANDLE sendTimer = CreateWaitableTimer(NULL, FALSE, NULL);
	if (NULL == sendTimer) {
		fprintf(stderr, "Error: create timer.\n");
		return 1;
	}
	LARGE_INTEGER dueTime;
	dueTime.QuadPart = -400000LL; // 40ms == 25Hz
	if (!SetWaitableTimer(sendTimer, &dueTime, 40, NULL, NULL, FALSE)) {
		printf("Error: SetWaitableTimer: %d\n", GetLastError());
		return 1;
	}
	DWORD bytesWritten;
	int prevStatus = ACC_OFF;
	while (WaitForSingleObject(sendTimer, INFINITE) == WAIT_OBJECT_0) {
		if (ACC_LIVE != gra->status && prevStatus == gra->status) continue;
		packet.status = prevStatus = gra->status;
		packet.rpm = phy->rpms;
		packet.maxRpm = sta->maxRpm;
		packet.pitLimiterOn = phy->pitLimiterOn;
		packet.gear = phy->gear;
		packet.tc = gra->TC;
		packet.tcc = gra->TCCut;
		packet.tcInAction = (uint8_t)(phy->tc);
		packet.abs = gra->ABS;
		packet.absInAction = (uint8_t)(phy->abs);
		packet.bb = 50; // TODO: offset by car model table lookup.
		packet.fuelEstimatedLaps = (uint8_t)lroundf(gra->fuelEstimatedLaps);
		packet.engineMap = gra->EngineMap + 1;
		packet.airTemp = (uint8_t)lroundf(phy->airTemp);
		packet.roadTemp = (uint8_t)lroundf(phy->roadTemp);

		WriteFile(comPort, &packet, sizeof(packet), &bytesWritten, NULL);
		// TODO: validate bytes written
		// TODO: will the serial port be open?
		// TODO: what if I disconnect and reconnect the arduino?
	}
	fprintf(stderr, "Error: WaitForSingleObject: %d\n", GetLastError());
	return 1;
}

int doDump(void)
{
	struct ACCPhysics *phy;
	struct ACCGraphics *gra;
	struct ACCStatic *sta;

	if (mapAcpmf(MAPACPMF_OPEN_EXISTING, &phy, &gra, &sta)) {
		return 1;
	}

	fprintf(stderr, "Read memory at 50Hz and dump contents to accdump.bin\n");
	HANDLE dumpFile = CreateFile(TEXT("accdump.bin"), GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == dumpFile) {
		fprintf(stderr, "Error: create accdump.bin\n");
		return 1;
	}
	HANDLE dumpTimer = CreateWaitableTimer(NULL, FALSE, NULL);
	if (NULL == dumpTimer) {
		fprintf(stderr, "Error: create timer.\n");
		return 1;
	}
	LARGE_INTEGER dueTime;
	dueTime.QuadPart = -200000LL; // 20ms == 50Hz
	if (!SetWaitableTimer(dumpTimer, &dueTime, 20, NULL, NULL, FALSE)) {
		printf("Error: SetWaitableTimer: %d\n", GetLastError());
		return 1;
	}
	DWORD bytesWritten;
	while (WaitForSingleObject(dumpTimer, INFINITE) == WAIT_OBJECT_0) {
		WriteFile(dumpFile, phy, sizeof(*phy), &bytesWritten, NULL);
		WriteFile(dumpFile, gra, sizeof(*gra), &bytesWritten, NULL);
		WriteFile(dumpFile, sta, sizeof(*sta), &bytesWritten, NULL);
	}
	fprintf(stderr, "Error: WaitForSingleObject: %d\n", GetLastError());
	return 1;
}

int doCsv(void)
{
	fprintf(stderr, "Read accdump.bin contents and write into accdump.csv\n");
	HANDLE csvFile = CreateFile(TEXT("accdump.csv"), GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == csvFile) {
		fprintf(stderr, "Error: create accdump.csv: %d\n", GetLastError());
		return 1;
	}
	HANDLE binFile = CreateFile(TEXT("accdump.bin"), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == binFile) {
		fprintf(stderr, "Error: open accdump.bin: %d\n", GetLastError());
		return 1;
	}
	int maxCsvRecord = 8192;
	char *csvRecord = malloc(maxCsvRecord);
	if (!csvRecord) ExitProcess(1);
	DWORD writtenBytes;
	if (!WriteFile(csvFile, csvRecord,
			snprintf(csvRecord, maxCsvRecord, "status,rpm,maxrpm,pitlimiteron,gear,tc,tccut,tcaction,abs,absaction,bb,fuellaps,map,aurt,roadt\n"),
			&writtenBytes, NULL)) {
		fprintf(stderr, "Error: write CSV header: %d\n", GetLastError());
		return 1;
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
				snprintf(csvRecord, maxCsvRecord, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%f,%d,%d,%d\n",
					gra->status, phy->rpms, sta->maxRpm, phy->pitLimiterOn, phy->gear, gra->TC, gra->TCCut, (uint8_t)(phy->tc),
					gra->ABS, (uint8_t)(phy->abs), 50, gra->fuelEstimatedLaps, gra->EngineMap,
					(uint8_t)lroundf(phy->airTemp), (uint8_t)lroundf(phy->roadTemp)),
				&writtenBytes, NULL)) {
			fprintf(stderr, "Error: write CSV record: %d\n", GetLastError());
			return 1;
		}
	}
	return 0;
}

int doReplay(void)
{
	struct ACCPhysics *phy;
	struct ACCGraphics *gra;
	struct ACCStatic *sta;

	if (mapAcpmf(MAPACPMF_CREATE, &phy, &gra, &sta)) {
		return 1;
	}

	HANDLE binFile = CreateFile(TEXT("accdump.bin"), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == binFile) {
		fprintf(stderr, "Error: open accdump.bin: %d\n", GetLastError());
		return 1;
	}

	HANDLE dumpTimer = CreateWaitableTimer(NULL, FALSE, NULL);
	if (NULL == dumpTimer) {
		fprintf(stderr, "Error: create timer.\n");
		return 1;
	}
	LARGE_INTEGER dueTime;
	dueTime.QuadPart = -200000LL; // 20ms == 50Hz
	if (!SetWaitableTimer(dumpTimer, &dueTime, 20, NULL, NULL, FALSE)) {
		printf("Error: SetWaitableTimer: %d\n", GetLastError());
		return 1;
	}
	while (WaitForSingleObject(dumpTimer, INFINITE) == WAIT_OBJECT_0) {
		DWORD bytesRead;
		if (!ReadFile(binFile, phy, sizeof(*phy), &bytesRead, NULL) || bytesRead < sizeof(*phy)) {
			return 1;
		}
		if (!ReadFile(binFile, gra, sizeof(*gra), &bytesRead, NULL) || bytesRead < sizeof(*gra)) {
			return 1;
		}
		if (!ReadFile(binFile, sta, sizeof(*sta), &bytesRead, NULL) || bytesRead < sizeof(*sta)) {
			return 1;
		}
	}
	fprintf(stderr, "Error: WaitForSingleObject: %d\n", GetLastError());
	return 1;
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
