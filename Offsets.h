#pragma once
#include <iostream>
#include <string>
#include <Windows.h>
#include <TlHelp32.h>
#include <vector>
#include <thread>
#include "math.h"
#include <stdlib.h> 
#include <conio.h> 
#include <fstream>
#include <ctime>
uint32_t local_player = 0x17E0A8;
uint32_t bot_list = 0x18AC04, team=0x30C, render=0xE4;
uint32_t o_x = 0x4, o_z = 0x8, o_y = 0xC, hp=0xEC, armor=0xF0, ammo=0x140, name=0x205, shoot=0x204;
uint32_t yaw = 0x34, pitch = 0x38, matrix_offset=0x17DFE4;
