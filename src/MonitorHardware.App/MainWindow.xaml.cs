using System.Windows; using MonitorHardware.App.Services; using MonitorHardware.App.ViewModels; using MonitorHardware.App.Models;
namespace MonitorHardware.App;
public partial class MainWindow : Window {
 readonly MainViewModel vm=new(new NativeHardwareCollector(),new SettingsStore());
 public MainWindow(){InitializeComponent();vm.PropertyChanged+=(_,_)=>Dispatcher.Invoke(Render);Loaded+=async(_,_)=>await vm.StartAsync();Closing+=(_,_)=>{vm.Stop();vm.Save();};}
 void Render(){var s=vm.Snapshot;Cpu.Text=$"{s.CpuPercent:0.0}%";CpuDetail.Text=$"{s.LogicalProcessors} logical processors";Memory.Text=$"{s.MemoryPercent:0.0}%";MemoryDetail.Text=$"{Bytes(s.TotalMemory-s.AvailableMemory)} / {Bytes(s.TotalMemory)} used";Gpu.Text=s.Gpu;SystemInfo.Text=$"{s.Windows} · User: {s.User}";Footer.Text=$"Uptime: {s.Uptime:d' days, 'hh' hours'} · Last update {s.CapturedAt.LocalDateTime:HH:mm:ss}";Disks.ItemsSource=s.Disks.Select(d=>$"Disk {d.Letter}: {d.UsagePercent:0}% used · {Bytes(d.FreeBytes)} free");Status.Text=s.NativeAvailable?"● LIVE":"● COLLECTOR OFFLINE";}
 static string Bytes(ulong b)=>$"{b/1024d/1024d/1024d:0.0} GB";
}
