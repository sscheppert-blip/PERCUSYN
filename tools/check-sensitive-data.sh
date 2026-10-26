#!/usr/bin/env bash
set -euo pipefail

# High-confidence guard for credentials and accidental local-machine data.
# This intentionally does not try to classify ordinary prose as PII.

repo_root=$(git rev-parse --show-toplevel)
cd "$repo_root"

# Without ripgrep the scan below would silently find nothing, so refuse to "pass".
if ! command -v rg >/dev/null 2>&1; then
    echo "check-sensitive-data: ripgrep (rg) is required but not installed" >&2
    exit 2
fi

if [[ $# -eq 0 ]]; then
    range="HEAD"
elif [[ $# -eq 2 ]]; then
    range="$1..$2"
else
    echo "usage: $0 [base_sha head_sha]" >&2
    exit 2
fi

declare -a files=()
if [[ "$range" == "HEAD" ]]; then
    while IFS= read -r -d '' file; do files+=("$file"); done < <(git ls-files -z)
else
    while IFS= read -r -d '' file; do files+=("$file"); done < <(git diff --name-only -z "$range" --)
fi

if [[ ${#files[@]} -eq 0 ]]; then
    exit 0
fi

# This file necessarily contains examples of secret-shaped text in its regex.
# Its path is still protected above; skip only its contents during the scan.
# Submodules are listed as one path but are directories of someone else's repository: skip them.
scanned_files=()
for file in "${files[@]}"; do
    [[ "$file" == "tools/check-sensitive-data.sh" || -d "$file" ]] || scanned_files+=("$file")
done

if [[ ${#scanned_files[@]} -eq 0 ]]; then
    exit 0
fi

for file in "${scanned_files[@]}"; do
    case "$file" in
        .env|.env.*|*/.env|*/.env.*|*.pem|*.p12|*.pfx|*.key|*.mobileprovision|*/secrets/*|*/credentials*|*/notarize*.json)
            echo "Blocked sensitive-looking path: $file" >&2
            exit 1
            ;;
    esac
done

pattern="BEGIN (RSA |EC |OPENSSH |DSA )?PRIVATE KEY|AKIA[0-9A-Z]{16}|ASIA[0-9A-Z]{16}|gh[pousr]_[A-Za-z0-9_]{20,}|github_pat_[A-Za-z0-9_]{20,}|xox[baprs]-[A-Za-z0-9-]{10,}|-----BEGIN CERTIFICATE-----|(?i:api[_-]?key|client[_-]?secret|password|passwd|secret|access[_-]?token|auth[_-]?token)\\s*[:=]\\s*[\"']?[^[:space:]\"']{8,}|/Users/[A-Za-z0-9._-]+/|[A-Za-z]:\\\\Users\\\\[A-Za-z0-9._-]+"

# Feed the files in batches (xargs): a single rg call fails with "argument list too long" in big
# repositories, and a failed call must never count as "nothing found".
status=0
matches=$(printf '%s\0' "${scanned_files[@]}" \
    | xargs -0 rg --no-heading --line-number --hidden --text --no-messages -e "$pattern" --) || status=$?

# xargs: 0 = every batch matched, 123 = some batch exited 1 (no match), 124+ = rg could not run.
if [[ $status -ge 124 ]]; then
    echo "check-sensitive-data: scan could not run (status $status)" >&2
    exit 2
fi

if [[ -n "$matches" ]]; then
    printf '%s\n' "$matches"
    echo "Sensitive-data scan failed. Remove the value and rotate any exposed credential." >&2
    exit 1
fi

exit 0
