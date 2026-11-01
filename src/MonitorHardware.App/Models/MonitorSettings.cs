namespace MonitorHardware.App.Models;
public sealed record MonitorSettings(int RefreshIntervalMs=1000, string Theme="Neon Magenta", bool StartMinimized=false);
