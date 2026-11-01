# Project Plan

**Status**: Approved
**Created**: 2026-11-01
**Mode**: NEW

---

## 1. Project Overview

**Goal**: Evolve the existing Windows C hardware monitor into a native offline WPF desktop application that displays live CPU, RAM, GPU, disk, Windows, user, clock, and uptime information while preserving the existing Win32 collection logic in a testable native library. The project is designed so that every module is independently testable.

**App Type**: Static + API

**API Login**: No

**Mode**: NEW

**Deployment Plan**: No deployment plan found

---

## 2. Backend — Native Hardware Collector

| Component | Technology |
|-----------|-----------|
| **Language** | C |
| **Runtime** | Windows API |
| **Package Manager** | None |
| **Test Runner** | CTest |
| **Mocking Library** | Hand-written Win32 seams |
| **Test Command** | `ctest --test-dir build --output-on-failure` |
| **Orchestration** | docker-compose |

The collector is a DLL with a versioned C ABI. It performs one snapshot per call, writes only into caller-owned buffers, reports per-metric availability, and never owns the UI loop. CPU sampling uses successive `GetSystemTimes` snapshots; memory uses `GlobalMemoryStatusEx`; GPU and Windows identification reuse the current Win32/registry logic; fixed disks use `GetDiskFreeSpaceExA`; uptime uses `GetTickCount64`.

---

## 3. Frontend — Windows Desktop App

| Component | Technology |
|-----------|-----------|
| **Language** | C# |
| **Framework** | WPF on .NET 10 |
| **Package Manager** | dotnet (NuGet) |
| **Test Runner** | xUnit |
| **Mocking Library** | Hand-written interfaces |
| **Test Command** | `dotnet test` |

The desktop UI owns presentation, theme selection, settings, lifecycle, and the asynchronous refresh loop. It loads the native collector through a narrow interop wrapper, keeps the last valid snapshot on partial failure, and updates the WPF dispatcher once per completed collection.

---

## 4. Installer — Windows Package

| Component | Technology |
|-----------|-----------|
| **Language** | XML/MSBuild |
| **Runtime** | Windows packaging |
| **Package Manager** | dotnet (NuGet) |
| **Test Runner** | PowerShell smoke checks |
| **Mocking Library** | None |
| **Test Command** | `dotnet test` |
| **Orchestration** | docker-compose |

The release process produces two Windows artifacts: an installer that publishes the WPF executable and architecture-matched collector DLL, creates a Start menu shortcut, and supports clean uninstall; and a portable ZIP package containing the executable, native collector DLL, runtime files, and a launch script. Both formats must run without administrator privileges for basic collection.

---

## 5. Services Required

| Azure Service | Role in App | Environment Variable | Default Value (Local) | Classification |
|---------------|------------|---------------------|----------------------|----------------|
| No Azure service | Offline desktop application; configuration is stored in `%AppData%\MonitorHardware` | `MONITOR_HARDWARE_CONFIG_DIR` | `%AppData%\MonitorHardware` | Essential |

---

## 6. Prerequisites

### Run

| Tool | Service(s) | Installed | Version |
|------|-----------|-----------|---------|
| .NET SDK | desktop-ui, installer | ✅ | 10.0.302 |
| Windows SDK / Win32 headers | hardware-collector | ❓ | Unknown |
| CMake | hardware-collector | ❓ | Unknown |
| Git | * | ✅ | 2.55.0.windows.3 |

### Debug

| Tool | Service(s) | Installed | Version |
|------|-----------|-----------|---------|
| C# Dev Kit (`ms-dotnettools.csdevkit`) | desktop-ui | ❓ | Unknown |
| C/C++ (`ms-vscode.cpptools`) | hardware-collector | ❓ | Unknown |

Double-check all tools marked ❓ before scaffolding and building the native collector.

---

## 7. Design System & UI

**Component Library**: WPF native controls with MahApps.Metro styling
**Style Direction**: A compact, data-dense terminal-inspired dashboard with near-black surfaces, thin neon borders, magenta and cyan status accents, and clear color-coded utilization thresholds. The interface should feel technical without sacrificing readable typography or accessible contrast.
**Typography**: Cascadia Mono for metrics and headings, Segoe UI for explanatory labels

### Color Palette

| Token | Hex | Usage |
|-------|-----|-------|
| `primary` | `#D946EF` | Active navigation, key metric emphasis, and magenta section labels |
| `accent` | `#22D3EE` | Refresh status, focus states, and cyan dashboard highlights |
| `surface` | `#0B1020` | Main window and dashboard background |
| `text` | `#E5E7EB` | Metric values and primary labels |
| `muted` | `#94A3B8` | Timestamps, device details, and secondary captions |
| `border` | `#334155` | Metric cards, separators, and settings controls |

### Pages

| Page | Route | Purpose | Layout |
|------|-------|---------|--------|
| Hardware Dashboard | `/` | Show the current host snapshot and collection health at a glance | `header, nav, main, grid, card-list, footer` |
| Monitor Settings | `/settings` | Configure refresh interval and the visual theme | `header, nav, main, form, actions, footer` |

### Sample Content

Hardware Dashboard — metric snapshot:
| Metric | Current value | Detail | Status |
|--------|---------------|--------|--------|
| CPU | 37.4% | 8 logical processors | Normal |
| RAM | 12,288 MB / 32,768 MB | 37.5% used, 20,480 MB free | Normal |
| GPU | NVIDIA GeForce RTX 3060 | Adapter detected | Available |
| Disk C: | 61% used | 390 GB free / 1 TB | Warning |

Monitor Settings — field: Refresh interval: 1 second · Theme: Neon Magenta · Start minimized: Off

---

## 8. Project Structure

```text
/
├── .azure/
│   ├── project-plan.md
│   └── .preview-temp/
├── native/
│   ├── include/monitor_collector.h
│   ├── src/monitor_collector.c
│   ├── src/metrics_cpu.c
│   ├── src/metrics_memory.c
│   ├── src/metrics_gpu.c
│   ├── src/metrics_disks.c
│   ├── src/metrics_system.c
│   └── tests/
├── src/
│   └── MonitorHardware.App/
│       ├── App.xaml
│       ├── Views/
│       ├── ViewModels/
│       ├── Models/
│       ├── Services/
│       ├── Interop/
│       └── MonitorHardware.App.csproj
├── tests/
│   ├── MonitorHardware.App.Tests/
│   └── native/
└── installer/
    └── MonitorHardware.Package.wapproj
```

---

## 9. Route Definitions

| # | Method | Path | Description | Request Body | Response Body | Status Codes |
|---|--------|------|-------------|-------------|--------------|-------------|
| 1 | GET | `/` | Render the live hardware dashboard | — | Dashboard view with current snapshot | 200 |
| 2 | GET | `/settings` | Render local monitor settings | — | Settings view | 200 |
| 3 | GET | `native://collector/snapshot` | Collect one host snapshot through the interop boundary | `{ structSize, contractVersion }` | `{ cpu, memory, gpu, disks, system, statuses }` | 200, 206, 400, 500 |
| 4 | PUT | `local://settings` | Persist refresh interval and theme locally | `{ refreshIntervalMs, theme }` | `{ refreshIntervalMs, theme }` | 200, 422 |
| 5 | GET | `/api/health` | Verify the app and native collector are loaded | — | `{ status, services }` | 200, 503 |

---

## 10. Next Steps

1. Run **azure-project-scaffold** to execute this plan
2. Run **azure-project-integrate** to wire the frontend to live data, smoke-test the backend, and create the migrations
3. Configure release publishing for both the Windows installer and the portable executable ZIP
4. Run **azure-debug-plan** → **azure-debug-generate** for Docker emulators and VS Code debugging
5. Run the **azure-deploy** agent when ready; it uses **azure-app-onboard** for architecture, cost estimation, IaC generation, provisioning, and health verification
