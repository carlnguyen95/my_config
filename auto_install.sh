#!/usr/bin/env bash
set -Eeuo pipefail

# ============================================================
# Dev / AI workstation bootstrap
#
# Installs:
#   - zsh
#   - Neovim + LazyVim profile from $GIT_PATH
#   - Ollama
#   - Ollama models: gemma4, deepseek-coder-v2
#   - Latest stable Python via pyenv
#   - Latest pip for that Python
#
# Usage:
#   export GIT_PATH="https://github.com/your-user/.nvim.git"
#   chmod +x install_dev_env.sh
#   ./install_dev_env.sh
#
# Optional:
#   PYTHON_VERSION=latest ./install_dev_env.sh
#   NVIM_CONFIG_DIR="$HOME/.config/nvim" ./install_dev_env.sh
# ============================================================

GIT_PATH="${GIT_PATH:-}"
NVIM_CONFIG_DIR="${NVIM_CONFIG_DIR:-$HOME/.config/nvim}"
PYTHON_VERSION="${PYTHON_VERSION:-latest}"

OLLAMA_MODELS=(
  "gemma4"
  "qwen3:8b"
  "qwen3-vl:8b"
)

log() {
  printf '\n\033[1;34m==> %s\033[0m\n' "$*"
}

warn() {
  printf '\n\033[1;33mWARNING: %s\033[0m\n' "$*" >&2
}

die() {
  printf '\n\033[1;31mERROR: %s\033[0m\n' "$*" >&2
  exit 1
}

cleanup_tmp() {
  [[ -n "${TMP_DIR:-}" && -d "$TMP_DIR" ]] && rm -rf "$TMP_DIR"
}
trap cleanup_tmp EXIT

require_command() {
  command -v "$1" >/dev/null 2>&1 || die "Required command not found: $1"
}

detect_os() {
  if [[ ! -f /etc/os-release ]]; then
    die "Unsupported OS: /etc/os-release not found."
  fi

  # shellcheck disable=SC1091
  source /etc/os-release

  case "${ID:-}" in
  ubuntu | debian)
    PKG_MANAGER="apt"
    ;;
  *)
    die "This script currently supports Ubuntu/Debian. Detected: ${ID:-unknown}"
    ;;
  esac

  log "Detected ${PRETTY_NAME:-$ID}"
}

install_system_packages() {
  log "Installing base packages"

  sudo apt-get update
  sudo apt-get install -y \
    zsh \
    git \
    curl \
    wget \
    unzip \
    build-essential \
    ca-certificates \
    software-properties-common \
    ripgrep \
    fd-find \
    fzf \
    luarocks \
    xclip \
    python3-dev \
    libssl-dev \
    zlib1g-dev \
    libbz2-dev \
    libreadline-dev \
    libsqlite3-dev \
    libffi-dev \
    liblzma-dev \
    tk-dev \
    uuid-dev \
    llvm \
    libncursesw5-dev
}

install_neovim() {
  log "Installing latest stable Neovim"

  # Neovim's official GitHub release is used instead of the often-old
  # distro package.
  local arch nvim_url tmp_archive install_dir
  arch="$(uname -m)"

  case "$arch" in
  x86_64)
    nvim_url="https://github.com/neovim/neovim/releases/latest/download/nvim-linux-x86_64.tar.gz"
    ;;
  aarch64 | arm64)
    nvim_url="https://github.com/neovim/neovim/releases/latest/download/nvim-linux-arm64.tar.gz"
    ;;
  *)
    die "Unsupported CPU architecture for Neovim: $arch"
    ;;
  esac

  TMP_DIR="./Programs"
  tmp_archive="$TMP_DIR/nvim.tar.gz"
  install_dir="$TMP_DIR/nvim-linux-x86_64"

  curl -fL "$nvim_url" -o "$tmp_archive"
  tar -xzf "$tmp_archive" -C "$TMP_DIR"

  sudo rm -rf /opt/nvim
  sudo mv "$install_dir" /opt/nvim

  if ! grep -qs '/opt/nvim/bin' "$HOME/.profile" 2>/dev/null; then
    printf '\nexport PATH="/opt/nvim/bin:$PATH"\n' >>"$HOME/.profile"
  fi

  export PATH="/opt/nvim/bin:$PATH"

  log "Neovim version: $(nvim --version | head -n 1)"
}

install_ollama() {
  log "Installing Ollama"

  if command -v ollama >/dev/null 2>&1; then
    log "Ollama already installed: $(ollama --version || true)"
  else
    curl -fsSL https://ollama.com/install.sh | sh
  fi

  require_command ollama

  # Start Ollama if systemd is available.
  if command -v systemctl >/dev/null 2>&1; then
    sudo systemctl enable --now ollama || true
  fi

  # Give the service a moment to become available.
  for _ in {1..15}; do
    if ollama list >/dev/null 2>&1; then
      break
    fi
    sleep 1
  done
}

pull_ollama_models() {
  log "Pulling Ollama models"

  for model in "${OLLAMA_MODELS[@]}"; do
    log "Pulling $model"
    ollama pull "$model"
  done

  log "Installed Ollama models"
  ollama list
}

install_pyenv() {
  log "Installing/updating pyenv"

  if [[ ! -d "$HOME/.pyenv" ]]; then
    curl https://pyenv.run | bash
  fi

  # Configure pyenv for both bash and zsh.
  local shell_init
  for shell_init in "$HOME/.bashrc" "$HOME/.zshrc"; do
    touch "$shell_init"

    if ! grep -qs 'PYENV_ROOT' "$shell_init"; then
      cat >>"$shell_init" <<'EOF'

# pyenv
export PYENV_ROOT="$HOME/.pyenv"
[[ -d "$PYENV_ROOT/bin" ]] && export PATH="$PYENV_ROOT/bin:$PATH"
eval "$(pyenv init -)"
EOF
    fi
  done

  export PYENV_ROOT="$HOME/.pyenv"
  export PATH="$PYENV_ROOT/bin:$PATH"

  # Initialize pyenv in this shell.
  eval "$(pyenv init -)"
}

install_latest_python() {
  log "Installing latest stable Python"

  # Refresh pyenv's definition database.
  cd "$HOME/.pyenv" || die "Cannot access ~/.pyenv"
  git fetch --quiet origin master || git fetch --quiet origin main || true

  if [[ "$PYTHON_VERSION" == "latest" ]]; then
    PYTHON_VERSION="$(pyenv install --list |
      sed 's/^[[:space:]]*//' |
      grep -E '^3\.[0-9]+\.[0-9]+$' |
      tail -n 1)"
  fi

  [[ -n "$PYTHON_VERSION" ]] || die "Could not determine Python version."

  log "Python selected: $PYTHON_VERSION"

  if ! pyenv versions --bare | grep -qx "$PYTHON_VERSION"; then
    pyenv install "$PYTHON_VERSION"
  fi

  pyenv global "$PYTHON_VERSION"

  python --version
  python -m pip install --upgrade pip setuptools wheel

  log "Python: $(python --version)"
  log "pip: $(python -m pip --version)"
}

configure_zsh() {
  log "Configuring zsh"

  # Do not force a shell change in non-interactive automation.
  # Change the login shell only if the user explicitly opts in.
  if [[ "${SET_ZSH_DEFAULT:-0}" == "1" ]]; then
    chsh -s "$(command -v zsh)"
    log "zsh set as default login shell"
  else
    log "zsh installed. Set SET_ZSH_DEFAULT=1 to make it the default shell."
  fi
}

install_lazyvim_profile() {
  [[ -n "$GIT_PATH" ]] || die \
    'GIT_PATH is not set. Example: export GIT_PATH="https://github.com/user/.nvim.git"'

  log "Installing Neovim/LazyVim profile from GIT_PATH"

  require_command git

  mkdir -p "$(dirname "$NVIM_CONFIG_DIR")"

  if [[ -e "$NVIM_CONFIG_DIR" ]]; then
    local backup
    backup="${NVIM_CONFIG_DIR}.backup.$(date +%Y%m%d_%H%M%S)"
    warn "$NVIM_CONFIG_DIR already exists."
    mv "$NVIM_CONFIG_DIR" "$backup"
    log "Existing Neovim config moved to: $backup"
  fi

  git clone "$GIT_PATH" "$NVIM_CONFIG_DIR"

  log "Neovim profile cloned to: $NVIM_CONFIG_DIR"

  # LazyVim normally bootstraps itself on first nvim launch.
  # Running headless sync here lets plugins install during setup.
  if [[ -f "$NVIM_CONFIG_DIR/lazy-lock.json" ]]; then
    log "LazyVim lockfile detected; syncing plugins"
    nvim --headless "+Lazy! sync" +qa ||
      warn "Lazy.nvim sync returned an error. Run 'nvim' manually to inspect it."
  else
    log "No lazy-lock.json detected; launching Neovim will bootstrap the profile."
  fi
}

print_summary() {
  log "Installation complete"

  echo
  echo "Installed:"
  echo "  zsh       : $(zsh --version | head -n 1)"
  echo "  neovim    : $(nvim --version | head -n 1)"
  echo "  python    : $(python --version 2>&1)"
  echo "  pip       : $(python -m pip --version)"
  echo "  ollama    : $(ollama --version 2>/dev/null || true)"
  echo
  echo "Neovim config:"
  echo "  $NVIM_CONFIG_DIR"
  echo
  echo "Ollama models:"
  ollama list || true
  echo
  echo "If you changed your shell configuration, start a new terminal or run:"
  echo "  source ~/.profile"
  echo "  source ~/.zshrc"
}

create_proj_structure() {
  log "Creating project structure"

  mkdir -p "./proj"
  mv my_config/Python_agents ./proj/
}

main() {
  detect_os
  install_system_packages
  install_neovim
  install_ollama
  pull_ollama_models
  install_pyenv
  install_latest_python
  configure_zsh
  install_lazyvim_profile
  create_proj_structure
  print_summary
}

main "$@"
