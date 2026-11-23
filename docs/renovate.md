# Renovate setup (manual workflow with a repository GitHub App)

The [renovate workflow](../.github/workflows/renovate.yml) authenticates with a private GitHub App, installed
on this repository only. It needs no personal token. The `GITHUB_TOKEN` is not enough: PRs created with it do not
trigger the pipeline (so nothing would be automerged), and it can not update workflow files.

## 1. Create the GitHub App
1. Open <https://github.com/settings/apps/new> (Settings → Developer settings → GitHub Apps → New GitHub App).
2. Fill in:
   - **GitHub App name:** something globally unique, e.g. `snesctl-renovate-milkpirate`
   - **Homepage URL:** `https://github.com/milkpirate/snesctl`
   - **Identifying and authorizing users:** leave everything empty/unchecked
   - **Post installation:** leave empty
   - **Webhook:** uncheck **Active**
3. **Permissions → Repository permissions:**

   | Permission      | Access         |
   |-----------------|----------------|
   | Checks          | Read-only      |
   | Commit statuses | Read and write |
   | Contents        | Read and write |
   | Issues          | Read and write |
   | Metadata        | Read-only (preselected, mandatory) |
   | Pull requests   | Read and write |
   | Workflows       | Read and write |

   Leave all other repository, organization and account permissions at **No access**.
4. **Where can this GitHub App be installed?** → **Only on this account**
5. Click **Create GitHub App**.

## 2. Note the Client ID and create a private key
1. On the app's **General** page, copy the **Client ID** (starts with `Iv`).
2. Scroll down to **Private keys** → **Generate a private key**. A `.pem` file is downloaded.

## 3. Install the app on this repository
1. On the app's page, click **Install App** (left sidebar) → **Install** next to your account.
2. Choose **Only select repositories** → `milkpirate/snesctl` → **Install**.

## 4. Store Client ID and private key in the repository
In <https://github.com/milkpirate/snesctl/settings/secrets/actions> (Settings → Secrets and variables → Actions):
1. **Variables** tab → **New repository variable**
   - Name: `RENOVATE_APP_CLIENT_ID`
   - Value: the Client ID
2. **Secrets** tab → **New repository secret**
   - Name: `RENOVATE_APP_PRIVATE_KEY`
   - Value: the whole content of the `.pem` file, incl. the `-----BEGIN ...-----` and `-----END ...-----` lines

Or with the `gh` CLI:
```bash
gh variable set RENOVATE_APP_CLIENT_ID --repo milkpirate/snesctl --body "<client id>"
gh secret set RENOVATE_APP_PRIVATE_KEY --repo milkpirate/snesctl < path/to/key.pem
```

Afterwards delete the downloaded `.pem` file (a new key can be generated at any time).

## 5. Allow auto-merge (recommended)
Settings → General → Pull Requests → check **Allow auto-merge**.

Renovate then marks its PRs for auto-merge and GitHub merges them as soon as the pipeline passed. Otherwise
Renovate merges them itself, which only happens on the next Renovate run.

## 6. Run it
Actions → **renovate** → **Run workflow**:
- **ignore-schedule** (default on): update and merge right away instead of only in the nightly window of
  [`renovate.json`](../renovate.json)
- **dry-run**: only log what would be done
- **log-level**: `debug` for troubleshooting

The first run creates the **Dependency Dashboard** issue, listing all detected dependencies and pending updates.
Commits and PRs show up as `<app name>[bot]`.
