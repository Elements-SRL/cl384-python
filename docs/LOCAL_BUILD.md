# Building the `.pyd` by hand (no CI)

This is the ground-truth procedure: how to build `cl384_python_wrapper.pyd`
manually on a Windows machine. The CI pipeline does exactly these steps in an
automated way. If CI ever breaks, this is how you reproduce and diagnose it, and
this is what proves the pipeline is doing the right thing.

---

## 1. Requirements

Same toolchain the CI runner needs:

- Windows 10+.
- Visual Studio 2022, "Desktop development with C++" workload (MSVC v143).
- CMake ≥ 3.21.
- Python 3.11.7 (64-bit) with headers and `libs/python311.lib`.
- pybind11 (headers).
- The four commlib dependencies, each with its `Find*.cmake` and its binaries:
  Opal Kelly FrontPanel, Cypress FX3 (CyAPI), toml++, ftdi_utils.

commlib requires **C++20**. The library builds statically; despite the README
mentioning Qt Creator, Qt itself is not used — plain CMake + MSVC is enough.

---

## 2. Environment variables

Set these (see `RUNNER_SETUP.md` section 2 for the full table and the
trailing-slash convention). For a manual build in one shell you can set them
just for that session, e.g. in PowerShell:

```powershell
$env:PYTHON_3_11_7_PATH = "C:\path\to\Python311"
$env:PYBIND_11_PATH     = "C:\path\to\pybind11"
$env:FRONT_PANEL_PATH   = "C:\path\to\FrontPanel\"   # note trailing slash
$env:CY_API_PATH        = "C:\path\to\FX3SDK\"       # note trailing slash
$env:TOML_PP_PATH       = "C:\path\to\tomlplusplus\" # note trailing slash
$env:FTDI_UTILS_PATH    = "C:\path\to\ftdi_utils-install"
# E384COMMLIB_PATH is set below, after we install commlib.
```

---

## 3. Build and install e384commlib from source

The wrapper does `find_package(e384commlib REQUIRED CONFIG)`, so commlib must be
**installed** (not merely built) to a prefix. Build it from the source at the
version you want:

```powershell
cd C:\src\e384commlib          # a checkout at the desired tag
cmake -B build -G "Visual Studio 17 2022" -A x64 `
      -DCMAKE_BUILD_TYPE=Release `
      -DCMAKE_INSTALL_PREFIX="C:\src\commlib-install"
cmake --build build --config Release --target install
```

Then point the wrapper at what you just installed:

```powershell
$env:E384COMMLIB_PATH = "C:\src\commlib-install"
```

> commlib's `CMakeLists.txt` forces the install prefix to `E384COMMLIB_PATH` if
> you don't pass one, so passing an explicit `-DCMAKE_INSTALL_PREFIX` as above
> keeps things predictable.

---

## 4. Build the wrapper → `.pyd`

```powershell
cd C:\src\cl384_python         # the merged wrapper repo
cmake -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

The output is `cl384_python_wrapper.pyd` somewhere under `build\` (typically
`build\Release\`). The module name comes from the `PYBIND11_MODULE(
cl384_python_wrapper, m)` macro in `main.cpp`, so in Python you
`import cl384_python_wrapper`.

---

## 5. Assemble a usable bundle

The `.pyd` alone is not runnable — it needs its companion DLLs beside it (or on
`PATH`). Put these in one folder together:

- `cl384_python_wrapper.pyd`
- `MPSSE.dll`  (from the ftdi_utils / FTDI runtime)
- `FTD2XX.dll` (from the ftdi_utils / FTDI runtime)
- `okFrontPanel.dll` (from the FrontPanel SDK) — **required at runtime for Opal
  Kelly devices**, but not redistributed by CI. For a personal local build you
  can copy it in; for a public release, leave it out and document it.

Do **not** ship `main.o` — it is a compiler intermediate, not a runtime file.

---

## 6. Smoke test

```powershell
cd <the bundle folder>
python -c "import cl384_python_wrapper as m; print(m.MessageDispatcher)"
```

If it imports without a `DLL load failed` error, the companion DLLs are in the
right place. `DLL load failed while importing cl384_python_wrapper` almost always
means a companion DLL (FTDI, or FrontPanel if you touch OK devices) is missing
from the folder / PATH, or you are running a different-bitness Python (must be
64-bit) or a different Python minor version than the one the `.pyd` was built
against (must be 3.11).

---

## 7. Mapping to CI

| Manual step here | CI step in `build-and-release.yml` |
|---|---|
| checkout commlib at a tag | "Checkout e384commlib source" |
| build+install commlib (§3) | "Configure commlib" + "Build + install commlib" |
| set `E384COMMLIB_PATH` | done inline via `env:` on the wrapper steps |
| build wrapper (§4) | "Configure wrapper" + "Build wrapper" |
| assemble bundle (§5) | "Assemble bundle" |
| (manual: you test) | consumer tests after download |

If CI produces something your manual build doesn't (or vice versa), compare
step-by-step against this table — the difference is almost always an environment
variable the service can't see (system vs user) or a different dependency
version installed on the runner.
