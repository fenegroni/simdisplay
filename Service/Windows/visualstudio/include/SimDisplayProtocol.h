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

/* Requires uint8_t and uint16_t, found in stdint.h */

#pragma pack (push)
#pragma pack (1)

struct SimDisplayPacket {
	uint8_t status;
	uint16_t rpm;
	uint16_t maxRpm;
	uint8_t pitLimiterOn;
	uint8_t gear;
	uint8_t tc;
	uint8_t tcc;
	uint8_t tcInAction;
	uint8_t abs;
	uint8_t absInAction;
	uint8_t bb;
	uint8_t fuelEstimatedLaps;
	uint8_t engineMap;
	uint8_t airTemp;
	uint8_t roadTemp;
};

#pragma pack (pop)

#define SDP_STATUS_OFF 0
#define SDP_STATUS_REPLAY 1
#define SDP_STATUS_LIVE 2
#define SDP_STATUS_PAUSE 3
