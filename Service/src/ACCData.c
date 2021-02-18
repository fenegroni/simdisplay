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

#include <stdint.h>

#include "../include/ACCData.h"

struct ACC_CarModelData ACC_CarModelDict[37] = {
	//	GT3 - 2018
	{ 12, 0000, 0000, 7750, -70.0f,  {7.9585f, 7.9585f}, 320, L"amr_v12_vantage_gt3" },
	{  3, 0000, 0000, 8650, -140.0f, {7.5980f, 7.4855f}, 360, L"audi_r8_lms" },
	{ 11, 0000, 0000, 7500, -70.0f,  {7.9585f, 7.9585f}, 320, L"bentley_continental_gt3_2016" },
	{  8, 6300, 7100, 7400, -70.0f,  {7.9585f, 7.9585f}, 320, L"bentley_continental_gt3_2018" },
	{  7, 0000, 0000, 7100, -150.0f, {7.9585f, 7.9585f}, 283, L"bmw_m6_gt3" },
	{ 14, 0000, 0000, 8750, -70.0f,  {7.9585f, 7.9585f}, 360, L"jaguar_g3" },
	{  2, 0000, 0000, 7300, -170.0f, {7.5980f, 7.4855f}, 240, L"ferrari_488_gt3" },
	{ 17, 0000, 0000, 7500, -140.0f, {7.5980f, 7.4855f}, 310, L"honda_nsx_gt3" },
	{ 13, 0000, 0000, 8650, -140.0f, {7.5980f, 7.4855f}, 360, L"lamborghini_gallardo_rex" },
	{  4, 0000, 0000, 8650, -140.0f, {7.5980f, 7.4855f}, 310, L"lamborghini_huracan_gt3" },
	{ 18, 0000, 0000, 8650, -140.0f, {7.5980f, 7.4855f}, 310, L"lamborghini_huracan_st" },
	{ 15, 0000, 0000, 7750, -140.0f, {7.9585f, 7.9585f}, 320, L"lexus_rc_f_gt3" },
	{  5, 0000, 0000, 7500, -170.0f, {7.5980f, 7.4855f}, 240, L"mclaren_650s_gt3" },
	{  1, 0000, 0000, 7900, -140.0f, {7.9585f, 7.9585f}, 320, L"mercedes_amg_gt3" },
	{ 10, 0000, 0000, 7500, -150.0f, {7.9585f, 7.9585f}, 320, L"nissan_gt_r_gt3_2017" },
	{  6, 0000, 0000, 7500, -150.0f, {7.9585f, 7.9585f}, 320, L"nissan_gt_r_gt3_2018" },
	{  0, 0000, 0000, 9250, -210.0f, {7.1497f, 6.7715f}, 400, L"porsche_991_gt3_r" },
	{  9, 0000, 0000, 8500, -50.0f,  {7.1497f, 6.7715f}, 400, L"porsche_991ii_gt3_cup" },
	//	GT3 - 2019	
	{ 20, 0000, 0000, 7250, -70.0f,  {7.9585f, 7.9585f}, 320, L"amr_v8_vantage_gt3" },
	{ 19, 0000, 0000, 8650, -140.0f, {7.5980f, 7.4855f}, 360, L"audi_r8_lms_evo" },
	{ 21, 0000, 0000, 7650, -140.0f, {7.5980f, 7.4855f}, 310, L"honda_nsx_gt3_evo" },
	{ 16, 0000, 0000, 8650, -140.0f, {7.5980f, 7.4855f}, 310, L"lamborghini_huracan_gt3_evo" },
	{ 22, 0000, 0000, 7700, -170.0f, {7.5980f, 7.4855f}, 240, L"mclaren_720s_gt3" },
	{ 23, 0000, 0000, 9250, -210.0f, {7.1497f, 6.7715f}, 400, L"porsche_991ii_gt3_r" },
	//	GT4
	{ 50, 0000, 0000, 6450, -150.0f, {10.000f, 10.000f}, 360, L"alpine_a110_gt4" },
	{ 51, 0000, 0000, 7000, -200.0f, {10.000f, 10.000f}, 320, L"amr_v8_vantage_gt4" },
	{ 52, 0000, 0000, 8650, -150.0f, {10.000f, 10.000f}, 360, L"audi_r8_gt4" },
	{ 53, 0000, 0000, 7600, -220.0f, {7.7768f, 10.000f}, 246, L"bmw_m4_gt4" },
	{ 55, 0000, 0000, 7500, -180.0f, {10.000f, 10.000f}, 360, L"chevrolet_camaro_gt4r" },
	{ 56, 0000, 0000, 7200, -180.0f, {10.000f, 10.000f}, 360, L"ginetta_g55_gt4" },
	{ 57, 0000, 0000, 6500, -200.0f, {10.000f, 10.000f}, 290, L"ktm_xbow_gt4" },
	{ 58, 0000, 0000, 7000, -150.0f, {7.7768f, 7.6142f}, 450, L"maserati_mc_gt4" },
	{ 59, 0000, 0000, 7600, -90.0f,  {10.000f, 10.000f}, 240, L"mclaren_570s_gt4" },
	{ 60, 0000, 0000, 7000, -200.0f, {10.000f, 10.000f}, 246, L"mercedes_amg_gt4" },
	{ 61, 0000, 0000, 7800, -200.0f, {10.000f, 10.000f}, 400, L"porsche_718_cayman_gt4_mr" },
	//	GT3 - 2020
	{ 24, 0000, 0000, 7600, -170.0f, {7.5980f, 7.4855f}, 240, L"ferrari_488_gt3_evo" },
	{ 25, 0000, 0000, 7600, -140.0f, {7.9585f, 7.9585f}, 320, L"mercedes_amg_gt3_evo" },
};