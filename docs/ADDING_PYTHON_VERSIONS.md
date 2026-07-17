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
   with the version-agnostic form in `cl384_python/CMakeLists.suggested.txt`,
   which uses CMake's `find_package(Python ...)` so the Python on `PATH`
   determines the target.

2. **The workflow** must loop over versions (a matrix) and produce one `.pyd`
   per version, each installed/available on the runner.

---

## 2. Make each Python version available on the runner

For every version you want (3.11, 3.12, ...), install that Python (64-bit, with
dev headers/libs) on the self-hosted machine. You then need a way to put the
right one first on `PATH` per matrix leg. Options:

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
    strategy:
      fail-fast: false
      matrix:
        python: ['3.11', '3.12']
    steps:
      - name: Select Python ${{ matrix.python }}
        shell: pwsh
        run: |
          # prepend the chosen Python to PATH so find_package(Python) picks it.
          $root = "C:\python\${{ matrix.python }}"   # adjust to your layout
          echo "$root;$root\Scripts" | Out-File -FilePath $env:GITHUB_PATH -Append

      # ... checkout, build+install commlib (once per leg is fine) ...
      # ... configure + build wrapper: now version-agnostic via CMake ...

      - name: Name the artifact per version
        shell: pwsh
        run: |
          $tag = "cp" + "${{ matrix.python }}".Replace(".","")
          echo "PY_TAG=$tag" >> $env:GITHUB_ENV   # e.g. cp311, cp312
```

Each leg produces `...-cp311.zip`, `...-cp312.zip`, etc.

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

## 5. Don't forget the consumer story

With multiple versions, the release notes / asset names must make it obvious
which zip matches which Python (`cp311` = CPython 3.11, `cp312` = 3.12, all
x64). A consumer downloading the wrong one gets a `DLL load failed` on import.
The `cpXYZ` tag in the filename is the standard, recognisable way to signal this.
