#pragma once
#include <stdint.h>

#ifdef _WIN32
#define MC_API __declspec(dllexport)
#else
#define MC_API
#endif

#define MC_ABI_VERSION 1u
#define MC_MAX_GPU_NAME 256
#define MC_MAX_DISKS 26

typedef enum mc_status { MC_OK=0, MC_UNAVAILABLE=1, MC_INVALID_ARGUMENT=2, MC_INTERNAL_ERROR=3 } mc_status;
typedef struct mc_metric_status { uint32_t cpu,memory,gpu,disks,system; } mc_metric_status;
typedef struct mc_cpu { double usage_percent; uint32_t logical_processors; } mc_cpu;
typedef struct mc_memory { uint64_t total_bytes, available_bytes; double usage_percent; } mc_memory;
typedef struct mc_gpu { char name[MC_MAX_GPU_NAME]; uint32_t available; } mc_gpu;
typedef struct mc_disk { char letter; uint64_t total_bytes, free_bytes; double usage_percent; } mc_disk;
typedef struct mc_system { char windows[128], user[128]; uint64_t uptime_ms; } mc_system;
typedef struct mc_snapshot {
    uint32_t struct_size, abi_version;
    mc_status status;
    mc_metric_status metrics;
    mc_cpu cpu; mc_memory memory; mc_gpu gpu;
    uint32_t disk_count; mc_disk disks[MC_MAX_DISKS]; mc_system system;
} mc_snapshot;

MC_API mc_status mc_collect_snapshot(mc_snapshot *snapshot);
MC_API const char *mc_abi_version_string(void);
