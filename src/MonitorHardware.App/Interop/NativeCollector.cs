using System.Runtime.InteropServices;
namespace MonitorHardware.App.Interop;
[StructLayout(LayoutKind.Sequential)] public struct NativeCpu { public double usage_percent; public uint logical_processors; }
[StructLayout(LayoutKind.Sequential)] public struct NativeMemory { public ulong total_bytes,available_bytes; public double usage_percent; }
[StructLayout(LayoutKind.Sequential, CharSet=CharSet.Ansi)] public struct NativeGpu {
 [MarshalAs(UnmanagedType.ByValTStr,SizeConst=256)] public string name;
 public uint available;
}
[StructLayout(LayoutKind.Sequential)] public struct NativeMetricStatus { public uint cpu,memory,gpu,disks,system; }
[StructLayout(LayoutKind.Sequential)] public struct NativeDisk { public byte letter; public ulong total_bytes,free_bytes; public double usage_percent; }
[StructLayout(LayoutKind.Sequential, CharSet=CharSet.Ansi)] public struct NativeSystem {
 [MarshalAs(UnmanagedType.ByValTStr,SizeConst=128)] public string windows;
 [MarshalAs(UnmanagedType.ByValTStr,SizeConst=128)] public string user;
 public ulong uptime_ms;
}
[StructLayout(LayoutKind.Sequential, CharSet=CharSet.Ansi)] public struct NativeSnapshot {
 public uint struct_size,abi_version; public int status; public NativeMetricStatus metrics; public NativeCpu cpu; public NativeMemory memory;
 public NativeGpu gpu;
 public uint diskCount;
 [MarshalAs(UnmanagedType.ByValArray,SizeConst=26)] public NativeDisk[] disks;
 public NativeSystem system;
}
public static partial class NativeCollector {
 [DllImport("monitor_collector",EntryPoint="mc_collect_snapshot")] private static extern int Collect(ref NativeSnapshot snapshot);
 public static bool TryCollect(out NativeSnapshot snapshot) {
   snapshot=new(){struct_size=(uint)Marshal.SizeOf<NativeSnapshot>(),abi_version=1,disks=new NativeDisk[26],gpu=new NativeGpu(),system=new NativeSystem()};
   try { return Collect(ref snapshot)==0; } catch(DllNotFoundException){return false;} catch(BadImageFormatException){return false;}
 }
}
