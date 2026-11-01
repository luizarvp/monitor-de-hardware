namespace MonitorHardware.App.Models;
public sealed record DiskInfo(string Letter, ulong TotalBytes, ulong FreeBytes, double UsagePercent);
public sealed record HardwareSnapshot(double CpuPercent, uint LogicalProcessors, ulong TotalMemory, ulong AvailableMemory, double MemoryPercent, string Gpu, IReadOnlyList<DiskInfo> Disks, string Windows, string User, TimeSpan Uptime, DateTimeOffset CapturedAt, bool NativeAvailable);
