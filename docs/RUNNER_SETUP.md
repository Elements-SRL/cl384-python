# Self-Hosted Windows Runner Setup

The `.pyd` build must run on a Windows machine that already has MSVC 2022 and the
vendor SDKs installed, because those SDKs (FrontPanel, Cypress FX3) cannot be
placed on GitHub's hosted runners. This guide sets up **one** such machine as a
GitHub Actions self-hosted runner.

> **Use one build machine, not several.** A self-hosted runner is only as
> reproducible as the machine it runs on. Multiple machines with slightly
> different SDK versions or paths will build differently. Register one machine,
> keep it as the single source of builds, and let everyone else just download
> the release. Add more runners only if you later need parallel or
> hardware-in-the-loop jobs, and use labels to target them.

---

## 1. Prerequisites on the machine

Install / confirm all of these before registering the runner. These are the same
things a human needs to build the project by hand (see `LOCAL_BUILD.md`):

- **Visual Studio 2022** with the "Desktop development with C++" workload
  (gives MSVC v143 + the "Visual Studio 17 2022" CMake generator).
- **CMake >= 3.21** (the VS installer can provide it, or install standalone).
- **Python 3.11.7** (64-bit), including headers and `libs/python311.lib`.
  **Install it for all users** — see section 2.2.
- **pybind11** (headers). Install outside any user profile — see section 2.2.
- **Opal Kelly FrontPanel SDK** (provides `okFrontPanel.dll`, `okimpl_fpoip.dll`,
  and the `cmake/FindFrontPanel.cmake` that commlib includes).
- **Cypress EZ-USB FX3 SDK** (provides `cyUSB.lib` + `cmake/FindCyAPI.cmake`).
- **toml++** (header-only + its `Findtomlpp.cmake`).
- **ftdi_utils** (built/installed; provides its CMake config package).
- **libMPSSE** (built/installed; provides `MPSSEConfig.cmake` + `MPSSE.dll`).
  Required transitively — see section 2.1.
- **ftd2xx** (FTDI D2XX driver library; provides `FTD2XX.dll`). Required
  transitively — see section 2.1.
- **EMCR / device drivers** installed (per the commlib README) if you also want
  the machine usable for manual hardware testing — not required just to build.

### 1.1 PowerShell and execution policy

The workflow runs its steps under **Windows PowerShell 5.1**
(`shell: powershell`), not PowerShell 7. Two things must be true:

- **Do not assume `pwsh` exists.** GitHub Actions defaults `run:` steps to
  `pwsh` (PowerShell 7), which is *not* installed on Windows by default. The
  workflow sets `defaults.run.shell: powershell` at job level to use the
  built-in 5.1 instead. If you remove that, every step fails with
  `pwsh: command not found`. Alternatively install PowerShell 7
  (`winget install --id Microsoft.PowerShell`) and restart the runner service.
- **The execution policy must allow local scripts.** Actions writes each `run:`
  block to a temporary `.ps1` and dot-sources it. Under the default `Restricted`
  policy this fails with `running scripts is disabled on this system`. Fix once,
  as administrator:

  ```powershell
  Set-ExecutionPolicy -Scope LocalMachine RemoteSigned
  Get-ExecutionPolicy -List      # verify
  ```

  Then restart the runner service.

---

## 2. The nine dependency environment variables

commlib and the wrapper locate every dependency through environment variables.
The build consumes them directly, so CI needs **no source changes** — it just
needs these to exist on the machine. Set every one of them:

| Variable | Points at | Consumed by |
|---|---|---|
| `PYTHON_3_11_7_PATH` | Python 3.11.7 root (has `include/`, `libs/`) | wrapper |
| `PYBIND_11_PATH` | pybind11 root (has `include/`) | wrapper |
| `E384COMMLIB_PATH` | commlib install prefix | wrapper (overridden by CI) |
| `FRONT_PANEL_PATH` | FrontPanel SDK root (trailing slash, ends before `cmake/`) | commlib |
| `CY_API_PATH` | FX3 / CyAPI SDK root (trailing slash) | commlib |
| `TOML_PP_PATH` | toml++ root (trailing slash) | commlib |
| `FTDI_UTILS_PATH` | ftdi_utils install prefix | commlib |
| `LIBMPSSE_PATH` | libMPSSE install prefix (has `cmake/MPSSEConfig.cmake`) | ftdi_utils (transitive) |
| `FTD2XX_PATH` | ftd2xx install prefix | MPSSE (transitive) |

Known-good values on the current build machine:

```
FRONT_PANEL_PATH    C:\ElemLibraries\FrontPanel\
CY_API_PATH         C:\ElemLibraries\CyAPI\
TOML_PP_PATH        C:\ElemLibraries\toml++\
FTDI_UTILS_PATH     C:\ElemLibraries\ftdi_utils\
LIBMPSSE_PATH       C:\ElemLibraries\libmpsse\
FTD2XX_PATH         C:\ElemLibraries\ftd2xx\
E384COMMLIB_PATH    C:\ElemLibraries\e384CommLib\
PYBIND_11_PATH      C:\ElemLibraries\pybind11\
PYTHON_3_11_7_PATH  C:\Program Files\Python311\
```

> Note the trailing-slash convention: commlib does
> `include(${FRONT_PANEL_PATH_CMAKE}cmake/FindFrontPanel.cmake)` — the variable
> must end with a separator so the path resolves. Match how you set it locally.
>
> During CI, `E384COMMLIB_PATH` is overridden by the workflow. You still set a
> value here so local/manual builds work. See the warning in section 2.3.

### 2.1 The two transitive dependencies

`LIBMPSSE_PATH` and `FTD2XX_PATH` are easy to miss because nothing in commlib's
`CMakeLists.txt` mentions them. The chain is:

```
commlib  --find_package(ftdi_utils)-->  ftdi_utilsConfig.cmake
         --find_dependency(MPSSE)---->  MPSSEConfig.cmake
         --find_dependency(FTD2XX)--->  ftd2xx
```

commlib's `find_package(ftdi_utils REQUIRED CONFIG)` succeeds, and *then*
ftdi_utils' own installed config file calls `find_dependency(MPSSE)`. If
`LIBMPSSE_PATH` is unset you get:

```
CMake Error at .../CMakeFindDependencyMacro.cmake:93 (find_package):
  Could not find a package configuration file provided by "MPSSE"
Call Stack (most recent call first):
  C:/ElemLibraries/ftdi_utils/cmake/ftdi_utilsConfig.cmake:31 (find_dependency)
  CMakeLists.txt:188 (find_package)
```

The call stack is the giveaway — the error surfaces *inside* a dependency's
config file, not in commlib's own CMakeLists.

To find every environment variable the whole dependency tree reads:

```powershell
Get-ChildItem C:\ElemLibraries -Recurse -Include *.cmake,*.cmake.in,CMakeLists.txt |
  Select-String -Pattern '\$ENV\{(\w+)\}' |
  ForEach-Object { $_.Matches.Groups[1].Value } | Sort-Object -Unique
```

Run this after adding or upgrading any dependency.

### 2.2 Critical: SYSTEM scope, and never under a user profile

Two separate rules, both of which have broken this build before.

**Rule 1 — set them as System variables, not User variables.** The runner runs
as a **Windows service** under its own account. Services do **not** see your
per-user environment. If you set these paths only in your own user profile, the
build fails in CI with "cannot find FrontPanel / CyAPI / python311.lib" **even
though building by hand on the same machine works**.

1. Start -> "Edit the system environment variables" -> Environment Variables.
2. Add each one under **System variables** (the lower box), not User variables.
3. Click OK.
4. **Restart the runner service afterwards** (section 4).

Or from an elevated PowerShell:

```powershell
[Environment]::SetEnvironmentVariable("LIBMPSSE_PATH","C:\ElemLibraries\libmpsse\","Machine")
```

**Rule 2 — the paths themselves must not live under `C:\Users\...`.** Even with
the variable correctly set at System scope, the runner's service account may not
be able to *read* a directory inside another user's profile.
`C:\Users\<name>\AppData\Local\` is especially restricted. This produces a
particularly misleading failure: CMake **configure succeeds** (it only
concatenates path strings), and the build then dies at compile time with

```
error C1083: Cannot open include file: 'Python.h': No such file or directory
```

which reads like a wrong path rather than a permissions problem.

Both `PYBIND_11_PATH` and `PYTHON_3_11_7_PATH` originally pointed into a
developer's profile and had to be moved:

```powershell
# pybind11: copy out of the user profile
Copy-Item "C:\Users\<name>\development\pybind11" "C:\ElemLibraries\pybind11" -Recurse
[Environment]::SetEnvironmentVariable("PYBIND_11_PATH","C:\ElemLibraries\pybind11\","Machine")

# Python: do NOT copy - reinstall for all users (registry entries and paths are
# baked into a per-user install). Run the 3.11.7 installer as administrator with
# "Install for all users" checked, which lands in C:\Program Files\Python311\
[Environment]::SetEnvironmentVariable("PYTHON_3_11_7_PATH","C:\Program Files\Python311\","Machine")
```

Verify afterwards:

```powershell
Test-Path "C:\Program Files\Python311\include\Python.h"
Test-Path "C:\Program Files\Python311\libs\python311.lib"
Test-Path "C:\ElemLibraries\pybind11\include\pybind11\pybind11.h"
```

To audit for any remaining profile-dependent paths:

```powershell
Get-ChildItem Env: | Where-Object { $_.Value -like "*C:\Users\*" } | Select-Object Name, Value
```

### 2.3 `E384COMMLIB_PATH` hijacks the install prefix

commlib's `CMakeLists.txt` contains:

```cmake
cmake_path(SET E384COMMLIB_PATH_CMAKE "$ENV{E384COMMLIB_PATH}")
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX ${E384COMMLIB_PATH_CMAKE} CACHE PATH "Install path" FORCE)
endif()
```

On the runner, where `E384COMMLIB_PATH` is set machine-wide, this can redirect
the CI install into `C:\ElemLibraries\e384CommLib\` instead of the per-run
prefix. The symptom appears two steps later as

```
CMake Error at CMakeLists.txt:44 (find_package):
  Could not find a package configuration file provided by "e384commlib"
```

because the wrapper looks in `commlib-install` and nothing was written there.
The workflow therefore sets `E384COMMLIB_PATH: ''` on the **Configure commlib**
step, and verifies after install that
`commlib-install\cmake\e384commlibConfig.cmake` exists.

Side effect worth knowing: before this was fixed, CI runs were overwriting the
machine-wide commlib install that local builds depend on.

### 2.4 Verifying what the service actually sees

Machine-scope value and process-scope value can differ if the runner service was
not restarted. The workflow's Configure steps print both:

```
=== dependency prefixes seen by runner process ===
FTDI_UTILS_PATH    ok  'C:\ElemLibraries\ftdi_utils\'
LIBMPSSE_PATH      MISSING  ''
```

`MISSING` with a correct Machine value means: restart the runner service.

---

## 3. Register the runner

1. In GitHub: **cl384-python -> Settings -> Actions -> Runners -> New self-hosted
   runner -> Windows**.
2. Follow the shown commands on the build machine. They download the runner
   agent and run `config.cmd` with a registration token. Example shape:
   ```powershell
   mkdir C:\actions-runner ; cd C:\actions-runner
   # (download command shown by GitHub)
   .\config.cmd --url https://github.com/YOUR_ORG/cl384-python --token <TOKEN>
   ```
3. When `config.cmd` asks for **labels**, add: `windows,e384-build`
   (the runner also gets `self-hosted` automatically). The workflow targets
   `[self-hosted, windows, e384-build]`, so these must match.

> A green "Idle" runner means only that the agent is online — **not** that your
> job landed on it. If `runs-on:` says `windows-latest`, the job goes to a
> GitHub-hosted VM instead, and none of the vendor SDKs are there.

---

## 4. Run it as a service (recommended)

Running as a service means the runner starts on boot and works even when nobody
is logged in.

```powershell
cd C:\actions-runner
.\svc.cmd install
.\svc.cmd start
```

Useful later:
```powershell
.\svc.cmd status
.\svc.cmd stop
.\svc.cmd start     # run this after changing system env vars
```

Or:

```powershell
Get-Service actions.runner.* | Restart-Service
```

**After any change to the system environment variables in section 2, restart the
service.** Rebooting the machine is *not* a reliable substitute — a service
account's inherited environment does not always refresh on reboot.

To check which account the service runs as (relevant to the profile-path rule in
2.2):

```powershell
Get-Service actions.runner.* |
  Select-Object Name, @{n='Account';e={(Get-WmiObject Win32_Service -Filter "Name='$($_.Name)'").StartName}}
```

---

## 5. Workspace persistence

Unlike hosted runners, the self-hosted workspace **persists between runs** at
`C:\actions-runner\_work\cl384-python\cl384-python\`. A stale `CMakeCache.txt`
from a previous configure pins old cache values and produces confusing
`find_package` failures that survive workflow edits. The workflow's Configure
steps delete the `build` directory before configuring for this reason.

When debugging by hand, always use a fresh build directory name
(`cmake -B build-test ...`) so you are not inheriting a stale cache.

---

## 6. Security note for self-hosted runners

If cl384-python is **public**, do not let the build workflow run on pull requests
from forks — a fork's PR could run arbitrary code on your machine. The provided
workflow only triggers on `repository_dispatch` and manual `workflow_dispatch`,
neither of which a fork can invoke, so you are covered as written. If you later
add `pull_request` triggers, restrict them or keep the runner on a private repo.
See `SECURITY.md`.

---

## 7. Quick validation

1. Runner shows **Idle** (green) under Settings -> Actions -> Runners.
2. Trigger a manual build (`workflow_dispatch`, commlib ref = `development`).
3. Watch the job land on your runner and go green.
4. Confirm a release with the zipped bundle appears.

If step 3 fails on a "cannot find <dependency>" error, revisit section 2 — it is
almost always the user-vs-system environment variable issue, the missing
transitive vars (2.1), or a path under a user profile (2.2).

---

## 8. Runner-side troubleshooting

| Symptom | Cause | Fix |
|---|---|---|
| `pwsh: command not found` | PowerShell 7 not installed; Actions defaults to it | job-level `defaults.run.shell: powershell`, or install PS7 (1.1) |
| `running scripts is disabled on this system` | Execution policy is `Restricted` | `Set-ExecutionPolicy -Scope LocalMachine RemoteSigned` (1.1) |
| `Could not find ... "MPSSE"`, call stack ends in `ftdi_utilsConfig.cmake` | `LIBMPSSE_PATH` unset | set it System-scope, restart service (2.1) |
| `Cannot open include file: 'Python.h'` but configure passed | Python under a user profile the service cannot read | reinstall all-users (2.2) |
| `Cannot open include file: 'pybind11/pybind11.h'` | same, for pybind11 | move out of the profile (2.2) |
| Env var correct at Machine scope, `MISSING` in the job log | service not restarted | `Get-Service actions.runner.* \| Restart-Service` (4) |
| `find_package(e384commlib)` fails in the wrapper | install went to `E384COMMLIB_PATH` instead of the CI prefix | ensure `E384COMMLIB_PATH: ''` on Configure commlib (2.3) |
| `find_package` fails despite correct paths | stale `CMakeCache.txt` in a persisted workspace | delete the `build` directory (5) |
| Job never lands on the runner | `runs-on:` label mismatch | must be `[self-hosted, windows, e384-build]` (3) |
