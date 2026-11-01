using MonitorHardware.App.Interop; using MonitorHardware.App.Models;
namespace MonitorHardware.App.Services;
public sealed class NativeHardwareCollector : IHardwareCollector {
 public HardwareSnapshot Collect() {
  if(!NativeCollector.TryCollect(out var n)) return new(0,0,0,0,0,"Collector unavailable",[],Environment.OSVersion.VersionString,Environment.UserName,TimeSpan.Zero,DateTimeOffset.Now,false);
  var disks=(n.disks??[]).Take((int)Math.Min(n.diskCount,26)).Select(d=>new DiskInfo(((char)d.letter).ToString(),d.total_bytes,d.free_bytes,d.usage_percent)).ToArray();
  return new(n.cpu.usage_percent,n.cpu.logical_processors,n.memory.total_bytes,n.memory.available_bytes,n.memory.usage_percent,n.gpu.name??"Unavailable",disks,n.windows??"",n.user??"",TimeSpan.FromMilliseconds((double)n.uptimeMs),DateTimeOffset.Now,true);
 }
}
