using System.Text.Json; using System.IO; using MonitorHardware.App.Models;
namespace MonitorHardware.App.Services;
public sealed class SettingsStore {
 private readonly string path=Path.Combine(Environment.GetEnvironmentVariable("MONITOR_HARDWARE_CONFIG_DIR")??Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData),"MonitorHardware"),"settings.json");
 public MonitorSettings Load(){try{return JsonSerializer.Deserialize<MonitorSettings>(File.ReadAllText(path))??new();}catch{return new();}}
 public void Save(MonitorSettings s){Directory.CreateDirectory(Path.GetDirectoryName(path)!);File.WriteAllText(path,JsonSerializer.Serialize(s,new JsonSerializerOptions{WriteIndented=true}));}
}
