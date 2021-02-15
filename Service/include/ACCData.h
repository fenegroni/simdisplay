/*
simdisplay - A simracing dashboard created using Arduino to show shared memory telemetry from Assetto Corsa Competizione.

Copyright (C) 2021  Filippo Erik Negroni

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

//wchar_t from stdint.h

struct ACC_CarModelData {
	int carId;
	int optRpm;
	int shiftRpm;
	int maxRpm;
	float bbOffset;
	float brakePressureCo[2];
	int maxSteeringAngle;
	wchar_t* carModel;
};

extern struct ACC_CarModelData ACC_CarModelDict[37];