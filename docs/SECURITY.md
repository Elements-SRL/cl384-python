# Build Authorisation & Security

This pipeline can publish releases automatically, so it matters *who* is allowed
to set it off. This document explains the gate that is implemented (actor-check),
its honest limits, and the stronger gate you can add later (required reviewers).

---

## 1. What is implemented: the actor-check

In `e384commlib/.github/workflows/trigger-wrapper-build.yml`, the first job step
compares the user who pushed the tag against an allow-listed username:

```yaml
- name: Verify authorised actor
  env:
    AUTHORISED_ACTOR: your-github-username
  run: |
    if [ "$GITHUB_ACTOR" != "$AUTHORISED_ACTOR" ]; then
      echo "::error::User '$GITHUB_ACTOR' is not authorised..."
      exit 1
    fi
```

If anyone other than the authorised user pushes a tag, the job fails here and no
dispatch is sent, so no build and no release happen.

To change or extend the allow-list, edit `AUTHORISED_ACTOR`. For more than one
user, switch to a membership test, e.g.:

```bash
case " alice bob carol " in
  *" $GITHUB_ACTOR "*) echo "authorised" ;;
  *) echo "::error::not authorised"; exit 1 ;;
esac
```

---

## 2. Honest limits of the actor-check

`github.actor` tells you which account performed the push. That is an
**authentication** signal, not a tamper-proof **authorisation** control.

What it is good for:
- Stops the wrong teammate from accidentally cutting a release.
- Zero friction — fully automatic, no clicks.

What it does **not** do:
- It does not defend against a **compromised account**. If someone gets the
  authorised user's credentials/session, `github.actor` will read as that user.
- It is not a cryptographic guarantee of intent.

For most internal library workflows the actor-check is the right amount of
control. If your release carries higher stakes, add the reviewer gate below.

---

## 3. Stronger gate (sketch, not implemented): required reviewers

GitHub **Environments** let you require a named human to approve a job before it
runs. This is real authorisation: the build **pauses** and waits for a click from
someone on the reviewer list, regardless of who triggered it.

This is described here so you can turn it on later; it is intentionally **not**
wired into the provided workflow (you asked to implement only the actor-check).

### How you would add it

1. In **cl384_python** → Settings → **Environments** → New environment, name it
   e.g. `release`.
2. In that environment, enable **Required reviewers** and add the approver(s).
   Optionally add a wait timer or restrict to specific branches/tags.
3. In `build-and-release.yml`, attach the release-producing job to that
   environment:

   ```yaml
   jobs:
     build:
       runs-on: [self-hosted, windows, e384-build]
       environment: release        # <-- add this line
       steps:
         ...
   ```

Now, when the build is dispatched, GitHub holds the job in a "Waiting" state and
notifies the reviewers. Nothing is built or released until one of them approves
in the GitHub UI. You can keep the actor-check *as well* — the two compose:
actor-check filters who can *trigger*, the environment reviewer controls who can
*approve*.

### Trade-off
It adds one manual click per release. That is the point — it converts an
automatic pipeline into a gated one. Use it when an unattended release would be
unacceptable; skip it when speed matters more than a second pair of eyes.

---

## 4. Self-hosted runner exposure

Independent of the trigger gate, the self-hosted runner is itself a security
surface: whoever can make it run a workflow can run code on that machine.

- The provided build workflow triggers only on `repository_dispatch` and
  `workflow_dispatch`. Neither can be invoked by a fork's pull request, so a
  public fork cannot execute code on your runner **as written**.
- If you later add a `pull_request` trigger to any workflow that runs on the
  self-hosted runner, restrict it (e.g. only for same-repo branches, or require
  approval for first-time contributors), or move that workflow to a
  hosted runner. Never run untrusted fork PRs on a self-hosted machine that
  holds your vendor SDKs and tokens.
- Keep the `DISPATCH_TOKEN` fine-grained and scoped to only `cl384_python` with
  only `Contents: write`. Rotate it if it may have leaked.
