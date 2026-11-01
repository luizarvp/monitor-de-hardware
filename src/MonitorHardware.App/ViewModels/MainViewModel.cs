using System.ComponentModel; using System.Runtime.CompilerServices; using MonitorHardware.App.Models; using MonitorHardware.App.Services;
namespace MonitorHardware.App.ViewModels;
public sealed class MainViewModel : INotifyPropertyChanged {
 readonly IHardwareCollector collector; readonly SettingsStore store; CancellationTokenSource? cts;
 public HardwareSnapshot Snapshot {get;private set;}=new(0,0,0,0,0,"Loading",[], "", "",TimeSpan.Zero,DateTimeOffset.Now,false);
 public int RefreshIntervalMs {get;set;}=1000; public event PropertyChangedEventHandler? PropertyChanged;
 public MainViewModel(IHardwareCollector c,SettingsStore s){collector=c;store=s;RefreshIntervalMs=s.Load().RefreshIntervalMs;}
 public async Task StartAsync(){cts=new();while(!cts.IsCancellationRequested){var next=await Task.Run(collector.Collect);Snapshot=next;PropertyChanged?.Invoke(this,new(nameof(Snapshot)));await Task.Delay(RefreshIntervalMs,cts.Token).ContinueWith(_=>{});}}
 public void Stop()=>cts?.Cancel(); public void Save()=>store.Save(new(RefreshIntervalMs));
 void OnPropertyChanged([CallerMemberName]string? n=null)=>PropertyChanged?.Invoke(this,new(n));
}
