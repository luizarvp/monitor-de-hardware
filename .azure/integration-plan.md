# Integration hand-off

## Backend / native collector
- Project: `native/`
- Build: `cmake -S native -B build/native -DBUILD_TESTING=ON && cmake --build build/native --config Release`
- Tests: `ctest --test-dir build/native -C Release --output-on-failure`
- ABI: `native/include/monitor_collector.h`, `mc_collect_snapshot`, version `1`
- Runtime: copy `monitor_collector.dll` beside the published WPF executable
- Health: desktop collector availability is shown by the `LIVE` / `COLLECTOR OFFLINE` status

## Frontend
- Project: `src/MonitorHardware.App`
- Build: `dotnet build src/MonitorHardware.App/MonitorHardware.App.csproj`
- Publish: `dotnet publish src/MonitorHardware.App/MonitorHardware.App.csproj -c Release -r win-x64 --self-contained true`
- API seam: `src/MonitorHardware.App/Services/IHardwareCollector.cs` and `NativeHardwareCollector`
- Mock files to delete: none (the app uses no mock datasets)
- Settings: `src/MonitorHardware.App/Services/SettingsStore.cs`

## Routes / boundaries
- `GET /` -> WPF dashboard
- `GET /settings` -> settings surface (local settings store boundary)
- `GET native://collector/snapshot` -> `mc_collect_snapshot` through `Interop/NativeCollector.cs`
- `PUT local://settings` -> `SettingsStore.Save`
- `GET /api/health` -> collector availability status in the desktop shell

## Database
- None. This is an offline desktop application; no migrations or seed data are required.

## Shared types
- Native ABI types: `native/include/monitor_collector.h`
- Managed domain types: `src/MonitorHardware.App/Models`

## Services
- Essential: native collector, WPF dashboard, local settings persistence.
- Enhancement: GPU and fixed-disk individual metrics gracefully report unavailable.
