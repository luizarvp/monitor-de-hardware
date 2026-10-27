#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define UPDATE_INTERVAL_MS 1000
#define COLOR_DEFAULT (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)
#define COLOR_CYAN (FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY)
#define COLOR_MAGENTA (FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY)
#define COLOR_GREEN (FOREGROUND_GREEN | FOREGROUND_INTENSITY)
#define COLOR_YELLOW (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY)
#define COLOR_RED (FOREGROUND_RED | FOREGROUND_INTENSITY)

typedef struct {
    double cpuLoadPercent;
    double ramUsedPercent;
    unsigned long long totalRamMB;
    unsigned long long usedRamMB;
    unsigned long long freeRamMB;
} SystemStats;

static void setColor(WORD color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

static ULARGE_INTEGER fileTimeToULargeInteger(const FILETIME *fileTime) {
    ULARGE_INTEGER result;
    result.LowPart = fileTime->dwLowDateTime;
    result.HighPart = fileTime->dwHighDateTime;
    return result;
}

static double getCpuLoadPercent(void) {
    static ULONGLONG lastIdleTicks = 0;
    static ULONGLONG lastKernelTicks = 0;
    static ULONGLONG lastUserTicks = 0;
    static int initialized = 0;
    FILETIME idleTime, kernelTime, userTime;

    if (!GetSystemTimes(&idleTime, &kernelTime, &userTime)) {
        return 0.0;
    }

    ULONGLONG idleTicks = fileTimeToULargeInteger(&idleTime).QuadPart;
    ULONGLONG kernelTicks = fileTimeToULargeInteger(&kernelTime).QuadPart;
    ULONGLONG userTicks = fileTimeToULargeInteger(&userTime).QuadPart;

    if (!initialized) {
        initialized = 1;
        lastIdleTicks = idleTicks;
        lastKernelTicks = kernelTicks;
        lastUserTicks = userTicks;
        return 0.0;
    }

    ULONGLONG idleDelta = idleTicks - lastIdleTicks;
    ULONGLONG totalDelta = (kernelTicks - lastKernelTicks) +
                           (userTicks - lastUserTicks);

    lastIdleTicks = idleTicks;
    lastKernelTicks = kernelTicks;
    lastUserTicks = userTicks;

    if (totalDelta == 0) {
        return 0.0;
    }

    double load = (double)(totalDelta - idleDelta) * 100.0 /
                  (double)totalDelta;
    if (load < 0.0) load = 0.0;
    if (load > 100.0) load = 100.0;
    return load;
}

static void getMemoryStats(SystemStats *stats) {
    MEMORYSTATUSEX memoryInfo;
    memoryInfo.dwLength = sizeof(memoryInfo);

    if (!GlobalMemoryStatusEx(&memoryInfo)) {
        stats->ramUsedPercent = 0.0;
        stats->totalRamMB = 0;
        stats->usedRamMB = 0;
        stats->freeRamMB = 0;
        return;
    }

    ULONGLONG totalPhys = memoryInfo.ullTotalPhys;
    ULONGLONG freePhys = memoryInfo.ullAvailPhys;
    ULONGLONG usedPhys = totalPhys - freePhys;

    stats->totalRamMB = totalPhys / (1024ULL * 1024ULL);
    stats->usedRamMB = usedPhys / (1024ULL * 1024ULL);
    stats->freeRamMB = freePhys / (1024ULL * 1024ULL);
    stats->ramUsedPercent = totalPhys > 0
        ? (double)usedPhys * 100.0 / (double)totalPhys
        : 0.0;
}

static void printBar(double percent, int width) {
    int filled = (int)(percent * width / 100.0);
    if (filled < 0) filled = 0;
    if (filled > width) filled = width;

    printf("[");
    for (int i = 0; i < width; i++) {
        printf(i < filled ? "#" : "-");
    }
    printf("] %5.1f%%", percent);
}

static void printValueColor(double percent) {
    if (percent >= 85.0) {
        setColor(COLOR_RED);
    } else if (percent >= 60.0) {
        setColor(COLOR_YELLOW);
    } else {
        setColor(COLOR_GREEN);
    }
}

static void getGpuName(char *name, DWORD nameSize) {
    DISPLAY_DEVICEA device;
    ZeroMemory(&device, sizeof(device));
    device.cb = sizeof(device);

    if (EnumDisplayDevicesA(NULL, 0, &device, 0) && device.DeviceString[0] != '\0') {
        snprintf(name, nameSize, "%s", device.DeviceString);
    } else {
        snprintf(name, nameSize, "Nao detectada");
    }
}

static void getWindowsName(char *name, DWORD nameSize) {
    HKEY key;
    DWORD type = 0;
    DWORD buildSize = 32;
    char buildText[32] = {0};
    DWORD buildNumber = 0;

    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
            "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion",
            0, KEY_READ, &key) == ERROR_SUCCESS) {
        if (RegQueryValueExA(key, "CurrentBuildNumber", NULL, &type,
                (LPBYTE)buildText, &buildSize) == ERROR_SUCCESS &&
                type == REG_SZ) {
            buildNumber = (DWORD)strtoul(buildText, NULL, 10);
        }
        RegCloseKey(key);
    }

    if (buildNumber >= 22000) {
        snprintf(name, nameSize, "Windows 11");
    } else if (buildNumber > 0) {
        snprintf(name, nameSize, "Windows 10");
    } else {
        snprintf(name, nameSize, "Windows");
    }
}

static void printDiskStats(void) {
    DWORD drives = GetLogicalDrives();
    char root[4] = "A:\\";

    for (int letter = 0; letter < 26; letter++) {
        if ((drives & (1u << letter)) == 0) continue;

        root[0] = (char)('A' + letter);
        UINT driveType = GetDriveTypeA(root);
        if (driveType != DRIVE_FIXED) continue;

        ULARGE_INTEGER freeBytes, totalBytes, availableBytes;
        if (!GetDiskFreeSpaceExA(root, &availableBytes, &totalBytes, &freeBytes)) {
            continue;
        }

        double used = totalBytes.QuadPart > 0
            ? (double)(totalBytes.QuadPart - freeBytes.QuadPart) *
              100.0 / (double)totalBytes.QuadPart
            : 0.0;
        unsigned long long totalGB = totalBytes.QuadPart / (1024ULL * 1024ULL * 1024ULL);
        unsigned long long freeGB = freeBytes.QuadPart / (1024ULL * 1024ULL * 1024ULL);

        printf("  %c: ", root[0]);
        printValueColor(used);
        printBar(used, 12);
        setColor(COLOR_DEFAULT);
        printf("  (%llu GB livres / %llu GB)\n", freeGB, totalGB);
    }
}

static void printHardwareArt(void) {
    setColor(COLOR_MAGENTA);
    printf("              .-------------------------.              \n");
    printf("             /  .---------------------.  \\             \n");
    printf("            /  /                       \\  \\            \n");
    printf("           |  |     HARDWARE MONITOR    |  |           \n");
    printf("           |  |       .---------.       |  |           \n");
    printf("           |  |      /  .---.    \\      |  |           \n");
    printf("           |  |     |  | o o |    |     |  |           \n");
    printf("           |  |     |  |  ^  |    |     |  |           \n");
    printf("           |  |      \\  '---'   /      |  |           \n");
    printf("           |  |       '---------'       |  |           \n");
    printf("           |  |_________________________|  |           \n");
    printf("           |_______________________________|           \n");
    printf("                 /_____________________\\               \n");
    printf("                    |             |                    \n");
    printf("                 ___|_____________|___                 \n");
    printf("                /_______________________\\                \n");
    setColor(COLOR_DEFAULT);
}

static void printPanel(const SystemStats *stats, const char *gpu,
                       const char *windowsName, const char *userName,
                       ULONGLONG uptimeMs) {
    time_t now = time(NULL);
    struct tm *local = localtime(&now);
    ULONGLONG totalMinutes = uptimeMs / 60000ULL;
    unsigned long long days = totalMinutes / (24ULL * 60ULL);
    unsigned long long hours = (totalMinutes / 60ULL) % 24ULL;
    unsigned long long minutes = totalMinutes % 60ULL;

    system("cls");
    setColor(COLOR_CYAN);
    printf("+------------------------------------------------------------------+\n");
    printf("|                    MONITOR DE HARDWARE                          |\n");
    printf("+------------------------------------------------------------------+\n");
    printHardwareArt();
    printf("+------------------------------------------------------------------+\n");
    setColor(COLOR_MAGENTA);
    printf("| HARDWARE                                                         |\n");
    setColor(COLOR_DEFAULT);
    printf("| CPU   ");
    printValueColor(stats->cpuLoadPercent);
    printBar(stats->cpuLoadPercent, 20);
    setColor(COLOR_DEFAULT);
    printf("\n| RAM   ");
    printValueColor(stats->ramUsedPercent);
    printBar(stats->ramUsedPercent, 20);
    setColor(COLOR_DEFAULT);
    printf("  (%llu MB usados / %llu MB)\n", stats->usedRamMB, stats->totalRamMB);
    printf("| GPU   %s\n", gpu);
    printf("| RAM livre: %llu MB\n", stats->freeRamMB);
    printf("+------------------------------------------------------------------+\n");
    setColor(COLOR_MAGENTA);
    printf("| DISCOS                                                         |\n");
    setColor(COLOR_DEFAULT);
    printDiskStats();
    printf("+------------------------------------------------------------------+\n");
    setColor(COLOR_MAGENTA);
    printf("| SESSAO                                                          |\n");
    setColor(COLOR_DEFAULT);
    printf("| Usuario: %-20s  Hora: %02d:%02d:%02d\n",
           userName, local->tm_hour, local->tm_min, local->tm_sec);
    printf("| Sistema: %s\n", windowsName);
    printf("| Uptime:  %llu dias, %02llu horas, %02llu minutos\n",
           days, hours, minutes);
    printf("+------------------------------------------------------------------+\n");
    setColor(COLOR_CYAN);
    printf("| Atualizacao automatica a cada %d segundo(s) | Ctrl+C encerra    |\n",
           UPDATE_INTERVAL_MS / 1000);
    printf("+------------------------------------------------------------------+\n");
    setColor(COLOR_DEFAULT);
}

int main(void) {
    SystemStats stats;
    char gpu[128];
    char windowsName[128];
    char userName[128];
    DWORD userNameSize = sizeof(userName);

    getGpuName(gpu, sizeof(gpu));
    getWindowsName(windowsName, sizeof(windowsName));
    if (!GetUserNameA(userName, &userNameSize)) {
        snprintf(userName, sizeof(userName), "desconhecido");
    }

    printf("Iniciando monitor de hardware...\n");
    Sleep(1000);

    while (1) {
        stats.cpuLoadPercent = getCpuLoadPercent();
        getMemoryStats(&stats);
        printPanel(&stats, gpu, windowsName, userName, GetTickCount64());
        Sleep(UPDATE_INTERVAL_MS);
    }

    return 0;
}
