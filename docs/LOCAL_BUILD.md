# Building the `.pyd` by hand (no CI)

This is the ground-truth procedure: how to build `cl384_python_wrapper.pyd`
manually on a Windows machine. The CI pipeline does exactly these steps in an
automated way. If CI ever breaks, this is how you reproduce and diagnose it, and
this is what proves the pipeline is doing the right thing.

> **Debugging CI? Start here, not in CI.** A CI round-trip costs minutes and
> gives you one hypothesis per attempt. Reproducing the same `cmake` command
> locally turns that into seconds. If it works locally but fails in CI, the
> difference is environmental (service account, env var scope, working
> directory) and section 8 tells you where to look. If it fails locally too, you
> now have a fast edit-test loop and the workflow is irrelevant to the problem.
> This single split has resolved every hard failure in this pipeline so far.

---

## 1. Requirements

Same toolchain the CI runner needs:

- Windows 10+.
- Visual Studio 2022, "Desktop development with C++" workload (MSVC v143).
- CMake >= 3.21.
- Python 3.11.7 (64-bit) with headers and `libs/python311.lib`.
- pybind11 (headers).
- The commlib dependencies, each with its `Find*.cmake` or config package:
  Opal Kelly FrontPanel, Cypress FX3 (CyAPI), toml++, ftdi_utils, **libMPSSE**,
  **ftd2xx**.

commlib requires **C++20**. The library builds statically; despite the README
mentioning Qt Creator, Qt itself is not used — plain CMake + MSVC is enough.

> libMPSSE and ftd2xx are transitive: commlib does not name them, but
> ftdi_utils' installed config calls `find_dependency(MPSSE)`, which in turn
> pulls ftd2xx. See `RUNNER_SETUP.md` 2.1.

---

## 2. Environment variables

Nine variables, all consumed directly by the CMakeLists files. See
`RUNNER_SETUP.md` section 2 for the full table and the trailing-slash
convention. For a manual build in one shell you can set them just for that
session:

```powershell
$env:PYTHON_3_11_7_PATH = "C:\Program Files\Python311\"
$env:PYBIND_11_PATH     = "C:\ElemLibraries\pybind11\"
$env:FRONT_PANEL_PATH   = "C:\ElemLibraries\FrontPanel\"   # note trailing slash
$env:CY_API_PATH        = "C:\ElemLibraries\CyAPI\"        # note trailing slash
$env:TOML_PP_PATH       = "C:\ElemLibraries\toml++\"       # note trailing slash
$env:FTDI_UTILS_PATH    = "C:\ElemLibraries\ftdi_utils\"
$env:LIBMPSSE_PATH      = "C:\ElemLibraries\libmpsse\"
$env:FTD2XX_PATH        = "C:\ElemLibraries\ftd2xx\"
# E384COMMLIB_PATH is set below, after we install commlib.
```

> On the shared build machine these are already set at **System** scope, so an
> interactive shell inherits them and you can skip this. Setting them per-session
> is for a fresh developer machine.

---

## 3. Build and install e384commlib from source

The wrapper does `find_package(e384commlib REQUIRED CONFIG)`, so commlib must be
**installed** (not merely built) to a prefix. Build it from the source at the
version you want.

> The buildable source is on the **`development`** branch or a release tag.
> `main` has no `CMakeLists.txt` — see `PIPELINE_SETUP.md` 2.1.

```powershell
cd C:\src\e384commlib          # a checkout at the desired tag/branch
cmake -B build -G "Visual Studio 17 2022" -A x64 `
      -DCMAKE_CONFIGURATION_TYPES="Release" `
      -DCMAKE_INSTALL_PREFIX="C:\src\commlib-install"
cmake --build build --config Release --target install --parallel
```

Then point the wrapper at what you just installed:

```powershell
$env:E384COMMLIB_PATH = "C:\src\commlib-install"
```

Three things about that configure line:

- **`-DCMAKE_CONFIGURATION_TYPES="Release"`, not `-DCMAKE_BUILD_TYPE`.** The
  Visual Studio generator is multi-config and ignores `CMAKE_BUILD_TYPE`
  entirely; the configuration is chosen at build time by `--config Release`.
  Restricting the configuration types also avoids
  `IMPORTED_IMPLIB not set for imported target ... "MinSizeRel"` errors, since
  the vendor imported targets only define Debug and Release.

- **Do not add `-DCMAKE_PREFIX_PATH`.** commlib's CMakeLists builds it up
  internally from the environment variables; passing the flag overrides that and
  breaks `find_package(ftdi_utils)`. If a dependency is not found, a variable is
  missing — do not reach for a `-D` flag.

- **Pass an explicit `-DCMAKE_INSTALL_PREFIX`.** commlib's CMakeLists forces the
  install prefix to `$ENV{E384COMMLIB_PATH}` when none is given, which would
  install over your shared library location. Note that `E384COMMLIB_PATH` must
  be **unset or empty** for this to be reliable — in CI the workflow blanks it.
  See `RUNNER_SETUP.md` 2.3.

Verify the install produced the config package before moving on:

```powershell
Test-Path "C:\src\commlib-install\cmake\e384commlibConfig.cmake"
```

If this is `False`, the install went somewhere else (almost certainly
`$env:E384COMMLIB_PATH`) and the wrapper will fail with "Could not find a
package configuration file provided by e384commlib".

---

## 4. Build the wrapper -> `.pyd`

```powershell
cd C:\src\cl384-python         # the merged wrapper repo
cmake -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_CONFIGURATION_TYPES="Release"
cmake --build build --config Release
```

The output is `cl384_python_wrapper.pyd` somewhere under `build\` (typically
`build\Release\`). The module name comes from the `PYBIND11_MODULE(
cl384_python_wrapper, m)` macro in `main.cpp`, so in Python you
`import cl384_python_wrapper`.

> The wrapper's CMakeLists reads `PYTHON_3_11_7_PATH` and `PYBIND_11_PATH`
> without a `find_package`, so a wrong or empty value **passes configure** and
> only fails at compile time with `Cannot open include file: 'Python.h'` or
> `'pybind11/pybind11.h'`. A clean configure does not mean those paths are good.

---

## 5. Assemble a usable bundle

The `.pyd` alone is not runnable — it needs its companion DLLs beside it (or on
`PATH`). Put these in one folder together:

- `cl384_python_wrapper.pyd`
- `MPSSE.dll`  (from libMPSSE / the FTDI runtime)
- `FTD2XX.dll` (from ftd2xx / the FTDI runtime)
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

## 7. Reproducing a CI failure locally

Use a **fresh build directory name** so you are not inheriting a stale
`CMakeCache.txt`, and run the same command the failing step ran:

```powershell
cd C:\src\e384commlib
cmake -B build-test -G "Visual Studio 17 2022" -A x64 `
      -DCMAKE_CONFIGURATION_TYPES="Release" `
      -DCMAKE_INSTALL_PREFIX="$PWD/commlib-install-test"
```

For the wrapper, set `E384COMMLIB_PATH` the way the workflow does, then
configure:

```powershell
cmake --build build-test --config Release --target install
cd C:\src\cl384-python
$env:E384COMMLIB_PATH = "C:\src\e384commlib\commlib-install-test"
cmake -B build-test -G "Visual Studio 17 2022" -A x64 -DCMAKE_CONFIGURATION_TYPES="Release"
```

Also check you are on the same branch CI checks out
(`git branch --show-current`) and that `Test-Path CMakeLists.txt` is `True`.

> **Watch out for conda.** If your shell shows `(base)`, a conda environment is
> modifying `PATH` and may supply a Python or headers the runner does not have.
> A local build that only works inside conda is not a valid reproduction.

---

## 8. Mapping to CI

| Manual step here | CI step in `build-and-release.yml` |
|---|---|
| checkout commlib at a tag | "Checkout e384commlib source" (needs `COMMLIB_PAT`) |
| — | "Verify commlib checkout" (fails fast if the ref has no CMakeLists) |
| build+install commlib (§3) | "Configure commlib" + "Build + install commlib" |
| verify config package exists | done inline at the end of "Build + install commlib" |
| set `E384COMMLIB_PATH` | done inline via `env:` on the wrapper steps |
| build wrapper (§4) | "Configure wrapper" + "Build wrapper" |
| assemble bundle (§5) | "Assemble bundle" |
| (manual: you test) | consumer tests after download |

If CI produces something your manual build doesn't (or vice versa), compare
step-by-step against this table. In practice the difference has always been one
of these, in rough order of frequency:

1. **An environment variable the service cannot see.** Set at User scope instead
   of System, or the runner service was not restarted after the change. The
   workflow prints Process-scope vs Machine-scope values for exactly this.
2. **A dependency path under a user profile.** Your interactive shell can read
   `C:\Users\you\...`; the service account may not. Configure still passes;
   compile fails on a missing header.
3. **A stale `CMakeCache.txt`** in the persisted CI workspace holding values from
   a previous run.
4. **conda** supplying something locally that the runner lacks.
5. **A different dependency version** installed on the runner.
