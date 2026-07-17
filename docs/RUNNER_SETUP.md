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
- **CMake ≥ 3.21** (the VS installer can provide it, or install standalone).
- **Python 3.11.7** (64-bit), including headers and `libs/python311.lib`.
- **pybind11** (headers).
- **Opal Kelly FrontPanel SDK** (provides `okFrontPanel.dll`, `okimpl_fpoip.dll`,
  and the `cmake/FindFrontPanel.cmake` that commlib includes).
- **Cypress EZ-USB FX3 SDK** (provides `cyUSB.lib` + `cmake/FindCyAPI.cmake`).
- **toml++** (header-only + its `Findtomlpp.cmake`).
- **ftdi_utils** (built/installed; provides its CMake config package and the
  FTDI runtime DLLs `MPSSE.dll`, `FTD2XX.dll`).
- **EMCR / device drivers** installed (per the commlib README) if you also want
  the machine usable for manual hardware testing — not required just to build.

---

## 2. The seven dependency environment variables

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

> Note the trailing-slash convention: commlib does
> `include(${FRONT_PANEL_PATH_CMAKE}cmake/FindFrontPanel.cmake)` — the variable
> must end with a separator so the path resolves. Match how you set it locally.
>
> During CI, `E384COMMLIB_PATH` is overridden by the workflow to point at the
> commlib it just built from source. You still set a value here so local/manual
> builds work.

### ⚠️ Critical: make them SYSTEM variables

The runner will run as a **Windows service** under its own account. Services do
**not** see your per-user environment variables. If you set these paths only in
your own user profile, the build will fail in CI with "cannot find FrontPanel /
CyAPI / python311.lib" **even though building by hand on the same machine
works**. This is the single most common self-hosted gotcha.

Set them at the **System** level:

1. Start → "Edit the system environment variables" → Environment Variables.
2. Add each one under **System variables** (the lower box), not User variables.
3. Click OK.
4. **Restart the runner service afterwards** (section 4) so it picks them up.

To verify what the service actually sees, you can temporarily add a step to the
workflow: `run: Get-ChildItem Env: | Sort-Object Name` and read the log.

---

## 3. Register the runner

1. In GitHub: **cl384_python → Settings → Actions → Runners → New self-hosted
   runner → Windows**.
2. Follow the shown commands on the build machine. They download the runner
   agent and run `config.cmd` with a registration token. Example shape:
   ```powershell
   mkdir C:\actions-runner ; cd C:\actions-runner
   # (download command shown by GitHub)
   .\config.cmd --url https://github.com/YOUR_ORG/cl384_python --token <TOKEN>
   ```
3. When `config.cmd` asks for **labels**, add: `windows,e384-build`
   (the runner also gets `self-hosted` automatically). The workflow targets
   `[self-hosted, windows, e384-build]`, so these must match.

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

After any change to the system environment variables in section 2, **stop and
start the service** so it re-reads the environment.

---

## 5. Security note for self-hosted runners

If cl384_python is **public**, do not let the build workflow run on pull requests
from forks — a fork's PR could run arbitrary code on your machine. The provided
workflow only triggers on `repository_dispatch` and manual `workflow_dispatch`,
neither of which a fork can invoke, so you are covered as written. If you later
add `pull_request` triggers, restrict them or keep the runner on a private repo.
See `SECURITY.md`.

---

## 6. Quick validation

1. Runner shows **Idle** (green) under Settings → Actions → Runners.
2. Trigger a manual build (`workflow_dispatch`, commlib ref = `main`).
3. Watch the job land on your runner and go green.
4. Confirm a release with the zipped bundle appears.

If step 3 fails on a "cannot find <dependency>" error, revisit section 2 — it is
almost always the user-vs-system environment variable issue.
