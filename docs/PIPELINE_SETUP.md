# CI/CD Pipeline Setup — cl384 Python wrapper

This document explains how the automated `.pyd` build works and how to set it up
from scratch. It is written so that someone who has never seen this project can
reproduce the whole pipeline. Two companion documents go deeper on specific
parts:

- `RUNNER_SETUP.md` — installing and configuring the self-hosted Windows runner.
- `LOCAL_BUILD.md` — building the `.pyd` by hand, without CI (the ground truth).
- `SECURITY.md` — how the build is gated, and how to strengthen the gate.
- `ADDING_PYTHON_VERSIONS.md` — supporting more than Python 3.11.

---

## 1. What this pipeline does

When an authorised user pushes a version tag to **e384commlib**, the pipeline:

1. Verifies the tag was pushed by an approved user (actor-check).
2. Tells the **cl384_python** repo to start a build (cross-repo dispatch).
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
| **e384commlib** | The C++ library. Tagging it triggers everything. | your org |
| **cl384_python** | Merged wrapper-builder + release host. Builds and publishes the `.pyd`. | you |

> **Merge note:** `cl384_python_wrapper` (the builder) and `cl384-python` (the
> old artifact host) are merged into a single repo, here called `cl384_python`.
> The builder's `main.cpp` + `CMakeLists.txt` live at the repo root; releases
> are published on this same repo. If you have not merged yet, do that first —
> see section 7.

---

## 3. Architecture at a glance

```
   developer pushes tag  v0.40.0
            |
            v
  ┌───────────────────────┐
  │  e384commlib repo      │
  │  trigger-wrapper-build │   actor-check: is pusher authorised?
  │  (GitHub-hosted)       │   if yes -> repository_dispatch
  └──────────┬─────────────┘
             │  event: commlib-tagged  { commlib_ref: v0.40.0 }
             v
  ┌────────────────────────────────────────────┐
  │  cl384_python repo                          │
  │  build-and-release  (SELF-HOSTED Windows)   │
  │                                             │
  │  1. checkout commlib @ v0.40.0              │
  │  2. cmake build+install commlib (from src)  │
  │  3. cmake build wrapper  -> .pyd            │
  │  4. bundle .pyd + MPSSE.dll + FTD2XX.dll    │
  │  5. publish GitHub Release "commlib 0.40.0" │
  └────────────────────────────────────────────┘
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
`self-hosted, windows, e384-build`, and exposes the seven dependency paths as
**system** environment variables.

### 4.2 Create the dispatch token
The trigger workflow needs to call cl384_python's API. Create a token:

1. GitHub → Settings → Developer settings → **Fine-grained personal access
   tokens** → Generate new token.
2. Resource owner: your org. Repository access: **only `cl384_python`**.
3. Permissions: **Contents → Read and write**.
4. Copy the token value.

Store it as a secret **on the e384commlib repo** (that is where the trigger
workflow runs):

- e384commlib → Settings → Secrets and variables → Actions → New repository
  secret → name **`DISPATCH_TOKEN`**, value = the token.

> Why a PAT and not the built-in `GITHUB_TOKEN`? The built-in token cannot
> trigger workflows in a *different* repo. Cross-repo dispatch needs an explicit
> token you own.

### 4.3 Drop in the workflow files
- Copy `e384commlib/.github/workflows/trigger-wrapper-build.yml` into
  **e384commlib**.
- Copy `cl384_python/.github/workflows/build-and-release.yml` into
  **cl384_python**.

Then edit the placeholders (search for `CHANGE ME`, `YOUR_ORG`,
`your-github-username`):

| Placeholder | In file | Set to |
|---|---|---|
| `your-github-username` | trigger-wrapper-build.yml | the GitHub login allowed to release |
| `YOUR_ORG/cl384_python` | trigger-wrapper-build.yml | your org + wrapper repo |
| `YOUR_ORG/e384commlib` | build-and-release.yml (`COMMLIB_REPO`) | your org + commlib repo |
| tag pattern `v*` | trigger-wrapper-build.yml | your actual tag scheme |

### 4.4 Confirm the tag scheme
The trigger fires on tags matching `v*`. If e384commlib tags look different
(e.g. `release-0.40.0`), change the `tags:` filter to match. The build reads the
real version number from commlib's `CMakeLists.txt` regardless, so the tag text
itself only needs to *trigger*, not to carry the version.

---

## 5. Running it

### Test run first (no commlib tag needed)
On cl384_python → Actions → **Build and release .pyd** → Run workflow → enter a
commlib ref (e.g. `main`) → Run. This exercises steps 1–6 without touching
e384commlib. Confirm a release appears with a `.pyd` inside the zip.

### Real run
1. On e384commlib, the authorised user pushes a tag:
   ```
   git tag v0.40.0
   git push origin v0.40.0
   ```
2. The trigger workflow runs, passes the actor-check, dispatches.
3. cl384_python's build runs on the self-hosted runner.
4. A release named **cl384_python (commlib 0.40.0)** appears on cl384_python
   with the zipped bundle attached.

If the pusher is not the authorised user, the trigger job fails at the
actor-check step and nothing is built.

---

## 6. What a consumer downloads

The release asset is a zip containing:
- `cl384_python_wrapper.pyd` — the module (`import cl384_python_wrapper`).
- `MPSSE.dll`, `FTD2XX.dll` — FTDI runtime, must sit beside the `.pyd`.
- `BUILD_MANIFEST.txt` — records the commlib version and what is/isn't included.

To use Opal Kelly devices the consumer must additionally place
`okFrontPanel.dll` beside the `.pyd` themselves (obtained from the EMCR
installer / Opal Kelly). This is called out in the release notes automatically.

---

## 7. Merging the two old repos (if not done yet)

The wrapper builder (`cl384_python_wrapper`: `main.cpp`, `CMakeLists.txt`,
`include/`, `README.md`) becomes the content of the merged `cl384_python` repo.
Preserve history if you can:

```bash
# in a clone of the repo you want to keep as cl384_python
git remote add wrapper <url-of-cl384_python_wrapper>
git fetch wrapper
git merge wrapper/main --allow-unrelated-histories
# resolve any path clashes, commit
```

After merging, the builder files sit at the repo root and releases publish here.
There is then exactly one cross-repo link left: e384commlib → cl384_python.

---

## 8. Troubleshooting quick table

| Symptom | Likely cause | Fix |
|---|---|---|
| Trigger job fails at actor-check | pusher != authorised user | expected; only the approved user can release |
| Dispatch step 403 / 404 | `DISPATCH_TOKEN` missing/wrong scope | recreate PAT with Contents:write on cl384_python |
| Build never starts | runner offline, or label mismatch | check runner service is running; labels match |
| CMake can't find FrontPanel/CyAPI/etc. | env vars not visible to the service | make them **system** vars, restart runner service (see RUNNER_SETUP.md) |
| "No .pyd produced" | wrapper build failed upstream | read the Build wrapper step log |
| Required DLL not found | DLL not in the searched folders | adjust `DLL_SEARCH` paths in the workflow |
| find_package(e384commlib) fails | commlib install step didn't run/failed | check the commlib build+install step log |
