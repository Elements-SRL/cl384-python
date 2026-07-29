# CI/CD Pipeline Setup — cl384 Python wrapper

This document explains how the automated `.pyd` build works and how to set it up
from scratch. It is written so that someone who has never seen this project can
reproduce the whole pipeline. Companion documents go deeper on specific parts:

- `RUNNER_SETUP.md` — installing and configuring the self-hosted Windows runner.
- `LOCAL_BUILD.md` — building the `.pyd` by hand, without CI (the ground truth).
- `SECURITY.md` — how the build is gated, and how to strengthen the gate.
- `ADDING_PYTHON_VERSIONS.md` — supporting more than Python 3.11.

---

## 1. What this pipeline does

When an authorised user pushes a version tag to **e384commlib**, the pipeline:

1. Verifies the tag was pushed by an approved user (actor-check).
2. Tells the **cl384-python** repo to start a build (cross-repo dispatch).
3. On a self-hosted Windows machine: checks out e384commlib **at that tag** and
   builds it **from source**, then installs it.
4. Builds the pybind11 wrapper against that freshly-built commlib, producing
   `cl384_python_wrapper.pyd`.
5. Bundles the `.pyd` with its runtime DLLs (`MPSSE.dll`, `FTD2XX.dll`).
6. Publishes a GitHub Release whose name and notes state exactly which
   e384commlib version produced it.

`okFrontPanel.dll` is deliberately **not** shipped (licensing) and `main.o` is
deliberately **not** shipped (it is a build artifact, not a runtime dependency).

---

## 2. The repositories

| Repo | Role | Owner |
|---|---|---|
| **e384commlib** | The C++ library. Tagging it triggers everything. | Elements-SRL |
| **cl384-python** | Merged wrapper-builder + release host. Builds and publishes the `.pyd`. | Elements-SRL |

> **Merge note:** `cl384_python_wrapper` (the builder) and `cl384-python` (the
> old artifact host) are merged into a single repo, here called `cl384-python`.
> The builder's `main.cpp` + `CMakeLists.txt` live at the repo root; releases
> are published on this same repo. If you have not merged yet, do that first —
> see section 8.

### 2.1 Which commlib branch has the build

**`main` does not contain a `CMakeLists.txt`.** The buildable source lives on
the **`development`** branch, and on release tags. Building against `main`
fails at the first configure step with:

```
CMake Error: The source directory ".../commlib" does not appear to contain CMakeLists.txt.
```

The workflow's `workflow_dispatch` default is therefore `development`, not
`main`. Update this section if the CMakeLists is ever merged to `main`.

---

## 3. Architecture at a glance

```
   developer pushes tag  v0.40.0
            |
            v
  +-----------------------+
  |  e384commlib repo      |
  |  trigger-wrapper-build |   actor-check: is pusher authorised?
  |  (GitHub-hosted)       |   if yes -> repository_dispatch
  +----------+-------------+
             |  event: commlib-tagged  { commlib_ref: v0.40.0 }
             v
  +--------------------------------------------+
  |  cl384-python repo                          |
  |  build-and-release  (SELF-HOSTED Windows)   |
  |                                             |
  |  1. checkout commlib @ v0.40.0  (needs PAT) |
  |  2. cmake build+install commlib (from src)  |
  |  3. cmake build wrapper  -> .pyd            |
  |  4. bundle .pyd + MPSSE.dll + FTD2XX.dll    |
  |  5. publish GitHub Release "commlib 0.40.0" |
  +--------------------------------------------+
```

The trigger job runs on a free GitHub-hosted Linux runner (it only checks and
forwards). The heavy build runs on your self-hosted Windows machine because it
needs MSVC 2022 and the vendor SDKs.

---

## 4. One-time setup checklist

Do these in order. Each links to detail where needed.

### 4.1 Set up the self-hosted runner
Follow `RUNNER_SETUP.md` completely. At the end you will have a Windows machine
that: runs the GitHub Actions runner as a service, carries the label set
`self-hosted, windows, e384-build`, and exposes the **nine** dependency paths as
**system** environment variables pointing at locations outside any user profile.

### 4.2 Create the dispatch token
The trigger workflow needs to call cl384-python's API. Create a token:

1. GitHub -> Settings -> Developer settings -> **Fine-grained personal access
   tokens** -> Generate new token.
2. Resource owner: your org. Repository access: **only `cl384-python`**.
3. Permissions: **Contents -> Read and write**.
4. Copy the token value.

Store it as a secret **on the e384commlib repo** (that is where the trigger
workflow runs):

- e384commlib -> Settings -> Secrets and variables -> Actions -> New repository
  secret -> name **`DISPATCH_TOKEN`**, value = the token.

> Why a PAT and not the built-in `GITHUB_TOKEN`? The built-in token cannot
> trigger workflows in a *different* repo. Cross-repo dispatch needs an explicit
> token you own.

### 4.3 Create the commlib read token

The build checks out **e384commlib** from within **cl384-python**'s workflow.
`GITHUB_TOKEN` **cannot do this**, even though both repos are in the same org —
it is scoped to the repository running the workflow only. Being in the same org
grants it nothing. If `e384commlib` is private, checkout fails (a 404, which is
Git's way of saying "no access") and you get a confusing downstream error about
a missing `CMakeLists.txt` because the `commlib` directory ends up empty.

1. GitHub -> Settings -> Developer settings -> **Fine-grained personal access
   tokens** -> Generate new token.
2. **Resource owner: `Elements-SRL`** (the org owning e384commlib — not your
   personal account, or it cannot see the repo).
3. Repository access: **Only select repositories** -> `e384commlib`.
4. Permissions -> Repository permissions -> **Contents: Read-only**. That is the
   only permission you actively choose; Metadata auto-enables as read-only.
   Everything else: No access.
5. Store on **cl384-python** -> Settings -> Secrets and variables -> Actions ->
   New repository secret -> name **`COMMLIB_PAT`**.

> **Org approval:** fine-grained PATs against org repos may require an org owner
> to approve them (Org -> Settings -> Personal access tokens -> Pending
> requests). Until approved, the token exists but cannot read the repo, and the
> failure looks identical to not having a token at all.
>
> **Expiry:** when this token expires the checkout starts failing again with the
> same symptom. Note the expiry date somewhere visible.
>
> A deploy key on e384commlib is a valid alternative that sidesteps org PAT
> policy; use `ssh-key:` instead of `token:` in the checkout step.

### 4.4 Drop in the workflow files
- Copy `e384commlib/.github/workflows/trigger-wrapper-build.yml` into
  **e384commlib**.
- Copy `cl384-python/.github/workflows/build-and-release.yml` into
  **cl384-python**.

Then edit the placeholders (search for `CHANGE ME`, `YOUR_ORG`,
`your-github-username`):

| Placeholder | In file | Set to |
|---|---|---|
| `your-github-username` | trigger-wrapper-build.yml | the GitHub login allowed to release |
| `YOUR_ORG/cl384-python` | trigger-wrapper-build.yml | your org + wrapper repo |
| `YOUR_ORG/e384commlib` | build-and-release.yml (`COMMLIB_REPO`) | your org + commlib repo |
| tag pattern `v*` | trigger-wrapper-build.yml | your actual tag scheme |

### 4.5 Confirm the tag scheme
The trigger fires on tags matching `v*`. If e384commlib tags look different
(e.g. `release-0.40.0`), change the `tags:` filter to match. The build reads the
real version number from commlib's `CMakeLists.txt` regardless, so the tag text
itself only needs to *trigger*, not to carry the version.

---

## 5. Workflow implementation notes

Non-obvious things in `build-and-release.yml`. Each exists because its absence
broke a real build.

**Job-level `defaults.run.shell: powershell`.** The runner has Windows
PowerShell 5.1, not PowerShell 7. Without this, Actions defaults to `pwsh` and
every step fails with `pwsh: command not found`. See `RUNNER_SETUP.md` 1.1.

**No non-ASCII characters inside `run:` blocks.** The runner writes each block to
a temp `.ps1` in an encoding where an em dash becomes `â€”`, which breaks the
PowerShell parser mid-string with `Unexpected token` / `The string is missing the
terminator`. Comments and the release `body:` are safe (not executed by
PowerShell); anything inside `run:` must be plain ASCII. To check before
committing:

```powershell
Select-String -Path .github\workflows\build-and-release.yml -Pattern '[^\x00-\x7F]' |
  Select-Object LineNumber, Line
```

**`-DCMAKE_CONFIGURATION_TYPES="Release"`.** The Visual Studio generator is
multi-config: it emits Debug, Release, MinSizeRel and RelWithDebInfo at generate
time regardless of `CMAKE_BUILD_TYPE`. The vendor imported targets (FrontPanel,
CyAPI, ftd2xx) define only Debug and Release, so the other two fail with
`IMPORTED_IMPLIB not set for imported target ... configuration "MinSizeRel"`.
Restricting the configuration types avoids generating them.

Note also that **`-DCMAKE_BUILD_TYPE` does nothing** with the VS generator; the
configuration is chosen at build time via `--build --config Release`.

**Do NOT pass `-DCMAKE_PREFIX_PATH`.** commlib's `CMakeLists.txt` assembles
`CMAKE_PREFIX_PATH` itself from the environment variables. Passing the flag on
the command line creates a cache entry that overrides this and breaks
`find_package(ftdi_utils)` with "Could not find a package configuration file
provided by ftdi_utils" — even when every path is correct and the config file
demonstrably exists. If a dependency is not found, the fix is a missing
environment variable, not a `-D` flag.

**`E384COMMLIB_PATH: ''` on the Configure commlib step.** See `RUNNER_SETUP.md`
2.3 — otherwise the install silently lands in the machine-wide location and the
wrapper cannot find it.

**Deleting the `build` directory before configuring.** The self-hosted workspace
persists between runs; a stale `CMakeCache.txt` pins old values.

**`--parallel` on the commlib build.** commlib is ~90 translation units and
builds serially without it. Roughly 5 minutes on the current machine with
`--parallel`; see section 7 if that becomes a problem.

**Install verification.** The Build + install step checks that
`commlib-install\cmake\e384commlibConfig.cmake` exists and fails loudly if not,
so a misdirected install is caught immediately rather than surfacing two steps
later as a `find_package` error in the wrapper.

---

## 6. Running it

### Test run first (no commlib tag needed)
On cl384-python -> Actions -> **Build and release .pyd** -> Run workflow -> enter
a commlib ref (use **`development`**, or a release tag — **not `main`**, which
has no `CMakeLists.txt`, see 2.1) -> Run. This exercises the whole pipeline
without touching e384commlib. Confirm a release appears with a `.pyd` inside the
zip.

### Real run
1. On e384commlib, the authorised user pushes a tag:
   ```
   git tag v0.40.0
   git push origin v0.40.0
   ```
2. The trigger workflow runs, passes the actor-check, dispatches.
3. cl384-python's build runs on the self-hosted runner.
4. A release named **cl384_python (commlib 0.40.0)** appears on cl384-python
   with the zipped bundle attached.

If the pusher is not the authorised user, the trigger job fails at the
actor-check step and nothing is built.

---

## 7. Build time

A full run takes roughly 5 minutes, dominated by compiling commlib's ~90 device
implementation translation units. Options if that becomes a problem:

- **Confirm `--parallel` is saturating the machine.** Pass an explicit count
  matching the core count and watch CPU utilisation during a run:
  ```powershell
  (Get-CimInstance Win32_ComputerSystem).NumberOfLogicalProcessors
  ```
- **Skip the rebuild when the ref has not changed.** The workflow currently
  wipes `build` every run, forcing a from-scratch compile even for an identical
  ref. Stashing the ref in the build tree and comparing lets MSBuild rebuild
  incrementally. A reasonable split: always wipe for `repository_dispatch`
  (real tagged releases, where reproducibility matters) and allow reuse for
  `workflow_dispatch` (testing, where iteration speed matters).
- **Cache the install tree keyed on the commlib ref.** A given tag always
  produces the same output. More machinery, but turns minutes into seconds.
  Only worth it if the pipeline runs frequently.

---

## 8. Merging the two old repos (if not done yet)

The wrapper builder (`cl384_python_wrapper`: `main.cpp`, `CMakeLists.txt`,
`include/`, `README.md`) becomes the content of the merged `cl384-python` repo.
Preserve history if you can:

```bash
# in a clone of the repo you want to keep as cl384-python
git remote add wrapper <url-of-cl384_python_wrapper>
git fetch wrapper
git merge wrapper/main --allow-unrelated-histories
# resolve any path clashes, commit
```

After merging, the builder files sit at the repo root and releases publish here.
There is then exactly one cross-repo link left: e384commlib -> cl384-python.

---

## 9. Troubleshooting quick table

| Symptom | Likely cause | Fix |
|---|---|---|
| Trigger job fails at actor-check | pusher != authorised user | expected; only the approved user can release |
| Dispatch step 403 / 404 | `DISPATCH_TOKEN` missing/wrong scope | recreate PAT with Contents:write on cl384-python |
| Build never starts | runner offline, or label mismatch | check runner service is running; labels match (`RUNNER_SETUP.md` 3) |
| `pwsh: command not found` | PowerShell 7 not installed | job-level `defaults.run.shell: powershell` (`RUNNER_SETUP.md` 1.1) |
| `running scripts is disabled on this system` | execution policy `Restricted` | `Set-ExecutionPolicy -Scope LocalMachine RemoteSigned` |
| `Unexpected token` / `string is missing the terminator` | non-ASCII character in a `run:` block | replace em dashes etc. with ASCII (section 5) |
| `commlib` directory empty / no `CMakeLists.txt` | checkout used `GITHUB_TOKEN`, or PAT unapproved/expired | use `COMMLIB_PAT` (4.3) |
| `source directory ... does not contain CMakeLists.txt` | building against `main` | use `development` or a tag (2.1) |
| Cannot find `MPSSE`, stack ends in `ftdi_utilsConfig.cmake` | `LIBMPSSE_PATH` unset on runner | `RUNNER_SETUP.md` 2.1 |
| `IMPORTED_IMPLIB not set ... "MinSizeRel"` | VS generator emitting all four configs | `-DCMAKE_CONFIGURATION_TYPES="Release"` (section 5) |
| Cannot find `ftdi_utils` despite correct paths | `-DCMAKE_PREFIX_PATH` was passed | remove it (section 5) |
| CMake can't find FrontPanel/CyAPI/etc. | env vars not visible to the service | make them **system** vars, restart runner service (`RUNNER_SETUP.md` 2.2) |
| `Cannot open include file: 'Python.h'` after configure passed | Python installed under a user profile | reinstall all-users (`RUNNER_SETUP.md` 2.2) |
| find_package(e384commlib) fails in wrapper | install redirected by `E384COMMLIB_PATH` | `E384COMMLIB_PATH: ''` on Configure commlib (`RUNNER_SETUP.md` 2.3) |
| "No .pyd produced" | wrapper build failed upstream | read the Build wrapper step log |
| Required DLL not found | DLL not in the searched folders | adjust `DLL_SEARCH` paths in the workflow |
| Failure persists after fixing the workflow | stale `CMakeCache.txt` in persisted workspace | delete the `build` directory (`RUNNER_SETUP.md` 5) |
