#!/usr/bin/env bash
# Apply local fork-only patches to submodules that upstream has not accepted.
#
# Why this exists:
#   external/SDL_ttf is pinned to SuperTux/SDL_ttf@861c047, which still uses the
#   SDL2 API. The fork's SDL3 port needs the SDL3-converted SDL_ttf (RWops ->
#   IOStream, SDL_bool -> bool, SDL_version -> SDL_GetVersion, opaque-surface
#   reads). That port lives ONLY as a local working-tree diff and was never
#   pushed anywhere (bumblei3 has no SDL_ttf fork, and upstream PRs are blocked).
#   To keep the fork CI reproducible on a fresh checkout, the diff is vendored
#   here and applied after `git submodule update`.
#
#   external/tinygettext needs NO patch: the parent pointer (f1ad3af
#   "Allow setting SDL version") already ships SDL3 support, selected via
#   -DTINYGETTEXT_SDL_VERSION=3 in the top-level CMakeLists.txt.
#
# Idempotent: skips any patch that is already applied (git apply --check fails
# because the change is present). Safe to re-run.
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
PATCH_DIR="${REPO_ROOT}/patches/submodules"

apply_patch() {
  local name="$1"
  local submod_path="$2"
  local patch="${PATCH_DIR}/${name}"
  if [[ ! -f "${patch}" ]]; then
    echo "apply_submodule_patches: skip ${name} (patch file missing)"
    return 0
  fi
  if git -C "${submod_path}" apply --check "${patch}" 2>/dev/null; then
    echo "apply_submodule_patches: applying ${name} -> ${submod_path}"
    git -C "${submod_path}" apply "${patch}"
  else
    echo "apply_submodule_patches: ${name} already applied or not applicable -> skip"
  fi
}

apply_patch sdl_ttf_sdl3.patch external/SDL_ttf
