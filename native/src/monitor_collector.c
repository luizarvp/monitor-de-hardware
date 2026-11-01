#include "monitor_collector.h"
#include <windows.h>
#include <stdio.h>
#include <string.h>

static void safe_copy(char *dst, size_t n, const char *src) {
    if (!dst || !n) return;
    strncpy_s(dst, n, src ? src : "", _TRUNCATE);
}
static double cpu_load(void) {
    FILETIME i,k,u; static ULONGLONG pi=0,pk=0,pu=0;
    if (!GetSystemTimes(&i,&k,&u)) return -1;
    ULONGLONG ni=((ULONGLONG)i.dwHighDateTime<<32)|i.dwLowDateTime;
    ULONGLONG nk=((ULONGLONG)k.dwHighDateTime<<32)|k.dwLowDateTime;
    ULONGLONG nu=((ULONGLONG)u.dwHighDateTime<<32)|u.dwLowDateTime;
    ULONGLONG total=(nk-pk)+(nu-pu), idle=ni-pi; pi=ni;pk=nk;pu=nu;
    return total ? (100.0*(double)(total-idle)/(double)total) : 0.0;
}
MC_API mc_status mc_collect_snapshot(mc_snapshot *s) {
    if (!s || s->struct_size < sizeof(mc_snapshot) || s->abi_version != MC_ABI_VERSION) return MC_INVALID_ARGUMENT;
    memset((char*)s + sizeof(uint32_t)*2, 0, sizeof(*s)-sizeof(uint32_t)*2);
    s->status=MC_OK; s->cpu.logical_processors=GetActiveProcessorCount(ALL_PROCESSOR_GROUPS);
    double c=cpu_load(); if(c<0){s->metrics.cpu=MC_UNAVAILABLE;} else s->cpu.usage_percent=c;
    MEMORYSTATUSEX m={sizeof(m)}; if(GlobalMemoryStatusEx(&m)){s->memory.total_bytes=m.ullTotalPhys;s->memory.available_bytes=m.ullAvailPhys;s->memory.usage_percent=(double)(m.ullTotalPhys-m.ullAvailPhys)*100.0/m.ullTotalPhys;} else s->metrics.memory=MC_UNAVAILABLE;
    DISPLAY_DEVICEA d={sizeof(d)}; if(EnumDisplayDevicesA(NULL,0,&d,0)) {safe_copy(s->gpu.name,sizeof(s->gpu.name),d.DeviceString);s->gpu.available=1;} else s->metrics.gpu=MC_UNAVAILABLE;
    for(char l='A';l<='Z' && s->disk_count<MC_MAX_DISKS;l++){char root[4]={l,':','\\',0}; UINT t=GetDriveTypeA(root); if(t!=DRIVE_FIXED)continue; ULARGE_INTEGER a,b,f;if(GetDiskFreeSpaceExA(root,&a,&b,&f)){mc_disk*x=&s->disks[s->disk_count++];x->letter=l;x->total_bytes=b.QuadPart;x->free_bytes=f.QuadPart;x->usage_percent=b.QuadPart?(double)(b.QuadPart-f.QuadPart)*100.0/b.QuadPart:0;}}
    OSVERSIONINFOA v={sizeof(v)}; if(GetVersionExA(&v)) _snprintf_s(s->system.windows,sizeof(s->system.windows),_TRUNCATE,"Windows %lu.%lu",v.dwMajorVersion,v.dwMinorVersion); else s->metrics.system=MC_UNAVAILABLE;
    DWORD n=sizeof(s->system.user); if(!GetUserNameA(s->system.user,&n)) s->metrics.system=MC_UNAVAILABLE;
    s->system.uptime_ms=GetTickCount64(); return s->status;
}
MC_API const char *mc_abi_version_string(void){return "monitor-collector/1";}
