#!/usr/bin/env bash
# Rebase the two open PR branches onto upstream/master, fix commit keywords
# (fixes # -> Closes # per CONTRIBUTING.md), and force-push to origin (own fork).
# SAFETY: creates backup tags before anything destructive; aborts rebase on conflict.
set -u

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$REPO_ROOT" || exit 1

git fetch upstream >/dev/null 2>&1 || { echo "fetch upstream failed"; exit 1; }

STAMP="backup-$(date +%Y%m%d-%H%M%S)"

# keyboard branch: 1 commit
KB="fix/keyboard-console-rebind"
git tag "$STAMP-kb" "$KB"
echo ">> backup tag $STAMP-kb -> $KB"
git checkout "$KB" >/dev/null 2>&1
if git rebase upstream/master >/dev/null 2>&1; then
  echo ">> rebase OK: $KB"
  # fix keyword: fixes # -> Closes # on the single commit
  git rebase --exec 'git commit --amend -m "$(git log -1 --format=%B | sed "s/fixes #/Closes #/g")"' HEAD~1 >/dev/null 2>&1 \
    && echo ">> keyword fixed: $KB"
  git push --force-with-lease origin "$KB" >/dev/null 2>&1 && echo ">> force-pushed: $KB"
else
  git rebase --abort 2>/dev/null
  echo ">> CONFLICT on $KB, aborted. Branch untouched."
fi

# editor branch: 2 commits
ED="fix/editor-crash-guards"
git tag "$STAMP-ed" "$ED"
echo ">> backup tag $STAMP-ed -> $ED"
git checkout "$ED" >/dev/null 2>&1
if git rebase upstream/master >/dev/null 2>&1; then
  echo ">> rebase OK: $ED"
  # fix keyword: fixes # -> Closes # on each commit in the branch
  git rebase --exec 'git commit --amend -m "$(git log -1 --format=%B | sed "s/fixes #/Closes #/g")"' upstream/master >/dev/null 2>&1 \
    && echo ">> keywords fixed: $ED"
  git push --force-with-lease origin "$ED" >/dev/null 2>&1 && echo ">> force-pushed: $ED"
else
  git rebase --abort 2>/dev/null
  echo ">> CONFLICT on $ED, aborted. Branch untouched."
fi

echo ">> backup tags: $STAMP-kb, $STAMP-ed (restore with: git checkout -B <branch> <tag>)"
