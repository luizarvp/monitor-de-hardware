#include "monitor_collector.h"
#include <assert.h>
#include <string.h>
int main(void) {
    mc_snapshot s; memset(&s,0,sizeof(s)); s.struct_size=sizeof(s); s.abi_version=MC_ABI_VERSION;
    assert(strcmp(mc_abi_version_string(),"monitor-collector/1")==0);
    return 0;
}
