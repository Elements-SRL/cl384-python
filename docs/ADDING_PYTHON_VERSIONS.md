# Supporting more Python versions

Today the wrapper targets **CPython 3.11** only. A `.pyd` is tied to one Python
minor version and one architecture, so "also support 3.12" means "produce a
second `.pyd` built against 3.12". This document describes how to get there when
you need it. Until then, the single-version pipeline is simpler — don't add this
machinery pre-emptively.

---

## 1. Two things must change

1. **The wrapper's CMakeLists** must stop hardcoding 3.11.7. The current file
   hardcodes `PYTHON_3_11_7_PATH` and links `python311.lib` by name. Replace it
   with the version-agnostic form in `cl384-python/CMakeLists.suggested.txt`,
   which uses CMake's `find_package(Python ...)` so the Python on `PATH`
   determines the target.

2. **The workflow** must loop over versions (a matrix) and produce one `.pyd`
   per version, each installed/available on the runner.

---

## 2. Make each Python version available on the runner

For every version you want (3.11, 3.12, ...), install that Python (64-bit, with
dev headers/libs) on the self-hosted machine.

> **Install for all users, outside any user profile.** The runner runs as a
> Windows service under its own account, which may not be able to read a
> per-user install under `C:\Users\<name>\AppData\Local\`. This has already
> broken this pipeline once: configure succeeds (CMake only concatenates path
> strings) and the compile then fails with
> `Cannot open include file: 'Python.h'`, which looks like a wrong path rather
> than a permissions problem. Run each installer as administrator with
> "Install for all users" checked. See `RUNNER_SETUP.md` 2.2.

A predictable layout makes the matrix simple, e.g.
`C:\Program Files\Python311\`, `C:\Program Files\Python312\`.

You then need a way to put the right one first on `PATH` per matrix leg. Options:

- Install each under a known root and prepend its folder to `PATH` in a workflow
  step keyed off the matrix value; or
- Use `actions/setup-python` on the runner (works on self-hosted if the runner
  can reach the Python download hosts).

---

## 3. Turn the build job into a matrix

Sketch of the change to `build-and-release.yml` (build steps abbreviated):

```yaml
jobs:
  build:
    runs-on: [self-hosted, windows, e384-build]
    defaults:
      run:
        shell: powershell        # runner has PS 5.1, NOT pwsh - see RUNNER_SETUP.md 1.1
    strategy:
      fail-fast: false
      matrix:
        python: ['3.11', '3.12']
    steps:
      - name: Select Python ${{ matrix.python }}
        run: |
          # prepend the chosen Python to PATH so find_package(Python) picks it.
          $ver  = "${{ matrix.python }}".Replace(".","")
          $root = "C:\Program Files\Python$ver"      # adjust to your layout
          if (-not (Test-Path "$root\include\Python.h")) {
            throw "Python ${{ matrix.python }} not installed at $root"
          }
          echo "$root;$root\Scripts" | Out-File -FilePath $env:GITHUB_PATH -Append

      # ... checkout, build+install commlib (once per leg is fine) ...
      # ... configure + build wrapper: now version-agnostic via CMake ...

      - name: Name the artifact per version
        run: |
          $tag = "cp" + "${{ matrix.python }}".Replace(".","")
          echo "PY_TAG=$tag" >> $env:GITHUB_ENV   # e.g. cp311, cp312
```

Each leg produces `...-cp311.zip`, `...-cp312.zip`, etc.

> Keep `shell: powershell` (or the job-level `defaults`) on every step. The
> runner does not have PowerShell 7, so any step that falls back to the `pwsh`
> default fails with `pwsh: command not found`.
>
> Keep `run:` blocks plain ASCII — the runner writes them to a temp `.ps1` in an
> encoding that mangles em dashes and breaks the parser.

---

## 4. Attach all versions to ONE release

You want a single release ("commlib 0.40.0") carrying every version's zip, not
one release per Python version. Two clean patterns:

- **Separate build + publish jobs.** Matrix build jobs each upload their zip as a
  workflow *artifact*; a final single job (needs: build) downloads all artifacts
  and creates one release with every file attached. This is the tidiest.
- **Same release tag, additive upload.** Each leg uploads to the same release
  tag with an action that appends assets. Simpler YAML, but be careful with
  concurrency so legs don't race on release creation.

The separate build-then-publish pattern is recommended because it avoids matrix
legs racing to create the same release.

---

## 5. Build time will multiply

commlib takes ~5 minutes to compile with `--parallel`, and a naive matrix
rebuilds it once per Python version even though **commlib itself does not depend
on the Python version**. With two versions that is ~10 minutes of duplicated
work.

Worth restructuring when you add the matrix: build and install commlib **once**
in a separate job, upload the install tree as an artifact, and have each matrix
leg download it and build only the wrapper. That keeps total time roughly
"commlib once + a fast wrapper build per version".

See `PIPELINE_SETUP.md` section 7 for the single-version build-time notes.

---

## 6. Don't forget the consumer story

With multiple versions, the release notes / asset names must make it obvious
which zip matches which Python (`cp311` = CPython 3.11, `cp312` = 3.12, all
x64). A consumer downloading the wrong one gets a `DLL load failed` on import.
The `cpXYZ` tag in the filename is the standard, recognisable way to signal this.
