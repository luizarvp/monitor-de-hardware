using Xunit; using MonitorHardware.App.Models; using MonitorHardware.App.Services;
public class SettingsStoreTests {
 [Fact] public void DefaultsAreSafe(){var s=new SettingsStore().Load();Assert.True(s.RefreshIntervalMs>=250);Assert.NotNull(s.Theme);}
}
