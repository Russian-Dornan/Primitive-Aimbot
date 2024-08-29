#include "Offsets.h"
using namespace std;
#ifdef UNICODE
wstring exeFileName = L"ac_client.exe";
wstring moduleName = L"ac_client.exe";
#else // UNICODE
string exeFileName = "ac_client.exe";
string moduleName = "ac_client.exe";
#endif

uint32_t findProcess()
{
    HANDLE processSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (processSnap == NULL)
    {
        return 0;
    }
    PROCESSENTRY32 pe{ sizeof(pe) };
    if (Process32First(processSnap, &pe) == FALSE)
    {
        CloseHandle(processSnap);
        return 0;
    }
    do
    {
        if (pe.szExeFile == exeFileName)
        {
            cout << pe.th32ProcessID << endl;
            CloseHandle(processSnap);
            return pe.th32ProcessID;
        }
    } while (Process32Next(processSnap, &pe));
    CloseHandle(processSnap);
    return 0;
}

uint32_t findModuleBaseAddress(uint32_t pid,
#ifdef UNICODE
    wstring moduleName
#else // UNICODE
    string moduleName
#endif
)
{
    HANDLE moduleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);
    if (moduleSnap == NULL)
    {
        return 0;
    }
    MODULEENTRY32 me{ sizeof(me) };
    if (Module32First(moduleSnap, &me) == FALSE)
    {
        CloseHandle(moduleSnap);
        return 0;
    }
    do
    {
        if (me.szModule == moduleName)
        {
            cout << reinterpret_cast<uint32_t>(me.modBaseAddr) << endl;
            CloseHandle(moduleSnap);
            return reinterpret_cast<uint32_t>(me.modBaseAddr);
        }
    } while (Module32Next(moduleSnap, &me));

    CloseHandle(moduleSnap);
    return 0;
}

uint32_t findAddress(HANDLE processHandle, uint32_t moduleBaseAddress, const vector<uint32_t>& offsets)
{
    uint32_t prevAddress = moduleBaseAddress;
    for (int i = 0; i < offsets.size(); i++)
    {
        auto offset = offsets[i];
        prevAddress += offset;
        if (offsets.size() - i == 1)
        {
            break;
        }
        uint32_t readBuffer = 0;
        SIZE_T bytesReadCount = 0;
        if (ReadProcessMemory(processHandle, reinterpret_cast<void*>(prevAddress), &readBuffer, sizeof(readBuffer), NULL) == FALSE)
        {
            return 0;
        }
        prevAddress = readBuffer;
    }
    return prevAddress;
}




int AimBot(HANDLE gameHandle, float x, float y, float z, float n_x, float n_y,
    float n_z, uint32_t yaw_pointer, uint32_t pitch_pointer, uint32_t shoot_pointer)
{
    if (x * y * z * n_x * n_y * n_z == 0) return 0;
    float del_x = (x - n_x), del_y = (y - n_y), del_z = (z - n_z), pitch, yaw;
    
    float xz = sqrt(del_x * del_x + del_z * del_z);
    yaw = atan(del_x / del_z) / 0.0174533;
    if (del_x < 0 and del_z < 0) yaw = 180 - yaw;
    else if (del_x > 0 and del_z < 0) yaw = 180 - yaw;
    else if (del_x > 0 and del_z > 0) yaw = 360 - yaw;
    else if (del_x < 0 and del_z>0) yaw = abs(yaw);
   
    pitch = atan(sqrt(del_x*del_x+del_z*del_z)/del_y) / 0.0174533;
    if (del_y > 0) pitch = (pitch - 90);
    else pitch = abs(abs(pitch)-90);
    SIZE_T numberOfBytesWritten;
    
    WriteProcessMemory(gameHandle, reinterpret_cast<void*>(yaw_pointer), &yaw, sizeof(yaw), &numberOfBytesWritten);
    WriteProcessMemory(gameHandle, reinterpret_cast<void*>(pitch_pointer), &pitch, sizeof(pitch), &numberOfBytesWritten);
    
    return 0;
}
int main()
{
    
    uint32_t pid = findProcess();
    if (pid == 0)
    {
        return 1;
    }
    HANDLE gameHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (gameHandle == NULL)
    {
        return 2;
    }
    uint32_t baseAddress = findModuleBaseAddress(pid, moduleName);
    if (baseAddress == 0)
    {
        return 3;
    }
    
    uint32_t player_x= findAddress(gameHandle, baseAddress, {local_player,o_x});
    uint32_t player_z = findAddress(gameHandle, baseAddress, { local_player,o_z });
    uint32_t player_y = findAddress(gameHandle, baseAddress, { local_player,o_y });
    uint32_t player_yaw = findAddress(gameHandle, baseAddress, { local_player,yaw });
    uint32_t player_pitch = findAddress(gameHandle, baseAddress, { local_player,pitch });
    uint32_t player_team = findAddress(gameHandle, baseAddress, { local_player,team });
    uint32_t shoot_pointer= findAddress(gameHandle, baseAddress, { local_player,shoot });
    vector<int> rendered = { 0,0,0,0,
    0,0,0,0,
    0,0,0,0,
    0,0,0,0, 0,0,0,0,
    0,0,0,0,
    0,0,0,0,
    0,0,0,0 };
    vector<vector<uint32_t>> pos_pointers = { {0,0,0,0},{0,0,0,0},{0,0,0,0}
    ,{0,0,0,0},{0,0,0,0} ,{0,0,0,0},{0,0,0,0} ,
    {0,0,0,0},{0,0,0,0} ,{0,0,0,0},{0,0,0,0} , 
    {0,0,0,0},{0,0,0,0} ,{0,0,0,0},{0,0,0,0} ,
    {0,0,0,0},{0,0,0,0} ,{0,0,0,0},{0,0,0,0} ,
    {0,0,0,0},{0,0,0,0} ,{0,0,0,0},{0,0,0,0} ,
    {0,0,0,0},{0,0,0,0} ,{0,0,0,0},{0,0,0,0} ,
    {0,0,0,0},{0,0,0,0} ,{0,0,0,0},{0,0,0,0} ,
    {0,0,0,0},{0,0,0,0} ,{0,0,0,0},{0,0,0,0} , },
        matrix_pointers = { {0,0,0,0}, {0,0,0,0},{0,0,0,0},{0,0,0,0} };
    for (uint32_t i = 0; i < 32; i++) {

        uint32_t bot_x = findAddress(gameHandle, baseAddress, { bot_list, 0x4*(i+1),o_x });
        uint32_t bot_z = findAddress(gameHandle, baseAddress, { bot_list, 0x4 * (i + 1),o_z });
        uint32_t bot_y = findAddress(gameHandle, baseAddress, { bot_list, 0x4 * (i + 1),o_y });
        uint32_t bot_health = findAddress(gameHandle, baseAddress, { bot_list, 0x4 * (i + 1),hp });
        uint32_t bot_team = findAddress(gameHandle, baseAddress, { bot_list, 0x4 * (i + 1),team });
        uint32_t bot_render = findAddress(gameHandle, baseAddress, { bot_list, 0x4 * (i + 1),render });
        pos_pointers[i] = { bot_x, bot_z, bot_y, bot_health , bot_team, bot_render};
       
    }
    int in1 = 0, in2 = 0;
    
    
    while (true) {
        float p_x, p_z, p_y, p_yaw, p_pitch;
        int p_team;

        SIZE_T num_bytes = 0;
        ReadProcessMemory(gameHandle, reinterpret_cast<void*>(player_x), &p_x, sizeof(p_x), &num_bytes);
        ReadProcessMemory(gameHandle, reinterpret_cast<void*>(player_z), &p_z, sizeof(p_z), &num_bytes);
        ReadProcessMemory(gameHandle, reinterpret_cast<void*>(player_y), &p_y, sizeof(p_y), &num_bytes);
        ReadProcessMemory(gameHandle, reinterpret_cast<void*>(player_yaw), &p_yaw, sizeof(p_yaw), &num_bytes);
        ReadProcessMemory(gameHandle, reinterpret_cast<void*>(player_pitch), &p_pitch, sizeof(p_pitch), &num_bytes);
        ReadProcessMemory(gameHandle, reinterpret_cast<void*>(player_team), &p_team, sizeof(p_team), &num_bytes);
        
        float b_x, b_z, b_y, b_yaw, b_pitch, real_distance, n_x = 0, n_z = 0, n_y = 0;
        float nearest = 1000000.0;
        int b_team;
        int flag = 1;
        int b_render;
        for (int i = 0; i < 31; i++) {


            ReadProcessMemory(gameHandle, reinterpret_cast<void*>(pos_pointers[i][0]), &b_x, sizeof(b_x), &num_bytes);
            ReadProcessMemory(gameHandle, reinterpret_cast<void*>(pos_pointers[i][1]), &b_z, sizeof(b_z), &num_bytes);
            ReadProcessMemory(gameHandle, reinterpret_cast<void*>(pos_pointers[i][2]), &b_y, sizeof(b_y), &num_bytes);
            ReadProcessMemory(gameHandle, reinterpret_cast<void*>(pos_pointers[i][4]), &b_team, sizeof(b_team), &num_bytes);
            ReadProcessMemory(gameHandle, reinterpret_cast<void*>(pos_pointers[i][5]), &b_render, sizeof(b_render), &num_bytes);
            if (b_x == 0 or b_y == 0 or b_z == 0) break;

            
            real_distance = sqrt((b_x - p_x) * (b_x - p_x) + (b_z - p_z) * (b_z - p_z)); // Аимбот метится в самых близких врагов

            if (real_distance < nearest and b_team != p_team and rendered[i]!=b_render) { //Ближе всех? Враг? Прорисовывается? 
                float b_hp;
                ReadProcessMemory(gameHandle, reinterpret_cast<void*>(pos_pointers[i][3]), &b_hp, sizeof(b_hp), &num_bytes);
                if (b_hp > 0) {
                    nearest = real_distance;
                    n_x = b_x;
                    n_y = b_y;
                    n_z = b_z;
                    flag = 0;
                }

            }
           
            rendered[i] = b_render; // Чтобы метился только на прорисованных врагов

        }
        SIZE_T numberOfBytesWritten;
        int n = 0;
        if ((GetKeyState(VK_RBUTTON) & 0x8000) && false) {
            AimBot(gameHandle, p_x, p_y, p_z, n_x, n_y, n_z, player_yaw, player_pitch, shoot_pointer);
          
        }
        
        if (GetKeyState(VK_MBUTTON) & 0x8000) return 0; // Выход
       
        this_thread::sleep_for(10ms);
    }
    return 0;
}