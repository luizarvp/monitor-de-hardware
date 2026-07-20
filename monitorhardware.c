#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define UPDATE_INTERVAL_MS 1000

typedef struct {
    double cpuLoadPercent;
    double ramUsedPercent;
    unsigned long long totalRamMB;
    unsigned long long usedRamMB;
    unsigned long long freeRamMB;
} SystemStats;

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

    FILETIME idleTime;
    FILETIME kernelTime;
    FILETIME userTime;

    if (!GetSystemTimes(&idleTime, &kernelTime, &userTime)) {
        return -1.0;
    }

    ULARGE_INTEGER idle = fileTimeToULargeInteger(&idleTime);
    ULARGE_INTEGER kernel = fileTimeToULargeInteger(&kernelTime);
    ULARGE_INTEGER user = fileTimeToULargeInteger(&userTime);

    ULONGLONG idleTicks = idle.QuadPart;
    ULONGLONG kernelTicks = kernel.QuadPart;
    ULONGLONG userTicks = user.QuadPart;

    if (!initialized) {
        initialized = 1;
        lastIdleTicks = idleTicks;
        lastKernelTicks = kernelTicks;
        lastUserTicks = userTicks;
        return 0.0;
    }

    ULONGLONG idleDelta = idleTicks - lastIdleTicks;
    ULONGLONG totalDelta = (kernelTicks - lastKernelTicks) + (userTicks - lastUserTicks);

    lastIdleTicks = idleTicks;
    lastKernelTicks = kernelTicks;
    lastUserTicks = userTicks;

    if (totalDelta == 0) {
        return 0.0;
    }

    double cpuLoad = ((double)(totalDelta - idleDelta) * 100.0) / (double)totalDelta;

    if (cpuLoad < 0.0) {
        cpuLoad = 0.0;
    }
    if (cpuLoad > 100.0) {
        cpuLoad = 100.0;
    }

    return cpuLoad;
}

static void getMemoryStats(SystemStats *stats) {
    MEMORYSTATUSEX memoryInfo;
    memoryInfo.dwLength = sizeof(memoryInfo);

    if (!GlobalMemoryStatusEx(&memoryInfo)) {
        stats->cpuLoadPercent = -1.0;
        stats->ramUsedPercent = -1.0;
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
    stats->ramUsedPercent = (totalPhys > 0) ? ((double)usedPhys * 100.0 / (double)totalPhys) : 0.0;
}

static void printBar(double percent) {
    int filled = (int)(percent / 10.0);
    int empty = 10 - filled;

    printf("[");
    for (int i = 0; i < filled; i++) {
        printf("#");
    }
    for (int i = 0; i < empty; i++) {
        printf("-");
    }
    printf("]");
}

static void printStats(const SystemStats *stats) {
    time_t now = time(NULL);
    struct tm *local = localtime(&now);

    if (stats->ramUsedPercent > 85.0) {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);
    } else {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    }

    printf("\n=== Monitor de Hardware ===\n");
    printf("Hora: %02d:%02d:%02d\n", local->tm_hour, local->tm_min, local->tm_sec);
    printf("CPU: %.2f%% ", stats->cpuLoadPercent);
    printBar(stats->cpuLoadPercent);
    printf("\n");
    printf("RAM: %.2f%% ", stats->ramUsedPercent);
    printBar(stats->ramUsedPercent);
    printf("\n");
    printf("Uso: %llu MB usados / %llu MB totais\n",
           stats->usedRamMB,
           stats->totalRamMB);
    printf("Livre: %llu MB\n", stats->freeRamMB);
    printf("===========================\n");
}

int main(void) {
    SystemStats stats;

    printf("Iniciando monitor de hardware...\n");
    printf("Pressione Ctrl+C para encerrar.\n");

    while (1) {
        system("cls");
        stats.cpuLoadPercent = getCpuLoadPercent();
        getMemoryStats(&stats);
        printStats(&stats);
        Sleep(1000);
    }

    return 0;
}
