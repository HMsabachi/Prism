#!/usr/bin/env bash
# Prism - Python environment check (Linux)
# Ensures a system Python of the required major version is available for running
# build / tooling scripts. The embedded CPython under vendor/Python is a separate
# runtime dependency of the engine and is NOT touched by this script.
set -u

REQUIRED_MAJOR="${REQUIRED_MAJOR_VERSION:-3}"
REQUIRED_MINOR="${REQUIRED_MINOR_VERSION:-13}"
QUIET="${QUIET:-0}"

# --- output helpers (no color if not a tty) ---------------------------------
if [ -t 1 ]; then
    C_CYAN=$'\033[36m'; C_GREEN=$'\033[32m'; C_RED=$'\033[31m'; C_YELLOW=$'\033[33m'; C_GRAY=$'\033[90m'; C_RESET=$'\033[0m'
else
    C_CYAN=""; C_GREEN=""; C_RED=""; C_YELLOW=""; C_GRAY=""; C_RESET=""
fi
step()  { printf '%s[%s]%s %s\n' "$C_GRAY" "$(date +%H:%M:%S)" "$C_RESET" "$*"; }
good()  { printf '%s[OK]%s %s\n' "$C_GREEN" "$C_RESET" "$*"; }
bad()   { printf '%s[XX]%s %s\n' "$C_RED" "$C_RESET" "$*"; }
warn()  { printf '%s[!!]%s %s\n' "$C_YELLOW" "$C_RESET" "$*"; }

# --- locate a python of the right major.minor version ------------------------
probe_python() {
    # $1 = executable path
    local exe="$1"
    [ -x "$exe" ] || return 1
    local ver
    ver="$("$exe" -c 'import sys;print("%d.%d" % sys.version_info[:2])' 2>/dev/null)" || return 1
    # ver looks like "3.13"
    [ "$ver" = "$REQUIRED_MAJOR.$REQUIRED_MINOR" ] || return 1
    PY_EXE="$exe"
    PY_VERSION="$("$exe" -c 'import sys;print("%d.%d.%d" % sys.version_info[:3])' 2>/dev/null)"
    return 0
}

find_python() {
    step "Searching for Python $REQUIRED_MAJOR.$REQUIRED_MINOR..."
    for exe in python3."$REQUIRED_MINOR" python3 python; do
        command -v "$exe" >/dev/null 2>&1 || continue
        local path
        path="$(command -v "$exe")"
        step "Probing: $path"
        if probe_python "$path"; then
            good "Found Python $PY_VERSION at: $path"
            return 0
        fi
    done
    # /usr/bin fallback (some distros ship python3.N without a generic symlink)
    local candidate
    for candidate in /usr/bin/python3."$REQUIRED_MINOR" /usr/local/bin/python3."$REQUIRED_MINOR" /opt/python3."$REQUIRED_MINOR"/bin/python3; do
        [ -x "$candidate" ] || continue
        step "Probing: $candidate"
        if probe_python "$candidate"; then
            good "Found Python $PY_VERSION at: $candidate"
            return 0
        fi
    done
    bad "No suitable Python $REQUIRED_MAJOR.$REQUIRED_MINOR found."
    return 1
}

# --- detect package manager --------------------------------------------------
detect_pkg_mgr() {
    if command -v apt-get >/dev/null 2>&1;   then echo apt
    elif command -v dnf >/dev/null 2>&1;     then echo dnf
    elif command -v yum >/dev/null 2>&1;     then echo yum
    elif command -v pacman >/dev/null 2>&1;  then echo pacman
    elif command -v zypper >/dev/null 2>&1;  then echo zypper
    else echo none
    fi
}

install_via_pkgmgr() {
    local mgr
    mgr="$(detect_pkg_mgr)"
    case "$mgr" in
        apt)
            step "Installing python3 + pip via apt-get..."
            sudo apt-get update -y
            sudo apt-get install -y python3 python3-pip python3-venv
            ;;
        dnf|yum)
            step "Installing python3 via $mgr..."
            sudo "$mgr" install -y python3 python3-pip
            ;;
        pacman)
            step "Installing python via pacman..."
            sudo pacman -S --noconfirm python python-pip
            ;;
        zypper)
            step "Installing python3 via zypper..."
            sudo zypper install -y python3 python3-pip
            ;;
        none)
            return 1
            ;;
    esac
    return $?
}

# --- main --------------------------------------------------------------------
main() {
    printf '%s========================================%s\n' "$C_CYAN" "$C_RESET"
    printf '%s Prism - Python Environment Check (Linux)%s\n' "$C_CYAN" "$C_RESET"
    printf '%s========================================%s\n' "$C_CYAN" "$C_RESET"
    printf '%sRequired: Python %s.%s (to run build/tooling scripts)%s\n' "$C_GRAY" "$REQUIRED_MAJOR" "$REQUIRED_MINOR" "$C_RESET"
    [ "$QUIET" = "1" ] || echo

    if find_python; then
        [ "$QUIET" = "1" ] || echo
        printf '%s========================================%s\n' "$C_CYAN" "$C_RESET"
        good "Python environment is ready."
        printf '%s  Version : %s%s\n' "$C_GRAY" "$PY_VERSION" "$C_RESET"
        printf '%s  Path    : %s%s\n' "$C_GRAY" "$PY_EXE" "$C_RESET"
        printf '%s========================================%s\n' "$C_CYAN" "$C_RESET"
        exit 0
    fi

    warn "Python $REQUIRED_MAJOR.$REQUIRED_MINOR not found on PATH."
    echo
    step "Attempting install via system package manager..."
    if install_via_pkgmgr && find_python; then
        echo
        printf '%s========================================%s\n' "$C_CYAN" "$C_RESET"
        good "Python installed and ready."
        printf '%s  Version : %s%s\n' "$C_GRAY" "$PY_VERSION" "$C_RESET"
        printf '%s  Path    : %s%s\n' "$C_GRAY" "$PY_EXE" "$C_RESET"
        printf '%s========================================%s\n' "$C_CYAN" "$C_RESET"
        exit 0
    fi

    # Package manager couldn't give us the right version (common on older LTS).
    # Fall back to pyenv, which builds the exact version.
    echo
    warn "System Python is not $REQUIRED_MAJOR.$REQUIRED_MINOR. Falling back to pyenv..."
    if install_via_pyenv; then
        exit 0
    fi

    echo
    printf '%s========================================%s\n' "$C_RED" "$C_RESET"
    bad "Unable to set up Python $REQUIRED_MAJOR.$REQUIRED_MINOR automatically."
    echo
    printf '%s  Install manually:%s\n' "$C_YELLOW" "$C_RESET"
    printf '%s    https://www.python.org/downloads/%s\n' "$C_CYAN" "$C_RESET"
    printf '%s    or:  curl https://pyenv.run | bash && pyenv install %s.%s%s\n' "$C_CYAN" "$REQUIRED_MAJOR" "$REQUIRED_MINOR" "$C_RESET"
    printf '%s========================================%s\n' "$C_RED" "$C_RESET"
    exit 1
}

# Build the exact version via pyenv. This is the reliable cross-distro path
# when the system package manager only ships an older Python.
install_via_pyenv() {
    command -v pyenv >/dev/null 2>&1 || {
        step "pyenv not found, installing..."
        curl -fsSL https://pyenv.run | bash || { bad "pyenv install failed"; return 1; }
        # pyenv.run installer writes its init to ~/.bashrc; source what we can
        export PYENV_ROOT="$HOME/.pyenv"
        export PATH="$PYENV_ROOT/bin:$PATH"
    fi
    local target
    # Pick newest 3.<minor>.<patch> that pyenv knows about
    target="$(pyenv install --list 2>/dev/null | grep -E "^[[:space:]]*$REQUIRED_MAJOR\.$REQUIRED_MINOR\.[0-9]+$" | tail -1 | xargs)"
    [ -n "$target" ] || { bad "pyenv has no Python $REQUIRED_MAJOR.$REQUIRED_MINOR candidate"; return 1; }

    step "Building Python $target via pyenv (this takes a few minutes)..."
    pyenv install -s "$target" || { bad "pyenv install $target failed"; return 1; }
    pyenv global "$target" || true

    export PYENV_ROOT="$HOME/.pyenv"
    export PATH="$PYENV_ROOT/shims:$PYENV_ROOT/bin:$PATH"
    if find_python; then
        echo
        printf '%s========================================%s\n' "$C_CYAN" "$C_RESET"
        good "Python installed and ready (via pyenv)."
        printf '%s  Version : %s%s\n' "$C_GRAY" "$PY_VERSION" "$C_RESET"
        printf '%s  Path    : %s%s\n' "$C_GRAY" "$PY_EXE" "$C_RESET"
        printf '%s  Add to shell rc:  eval "$(pyenv init -)"%s\n' "$C_GRAY" "$C_RESET"
        printf '%s========================================%s\n' "$C_CYAN" "$C_RESET"
        return 0
    fi
    return 1
}

main "$@"
