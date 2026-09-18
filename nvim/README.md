# 💤 LazyVim

A starter template for [LazyVim](https://github.com/LazyVim/LazyVim).
Refer to the [documentation](https://lazyvim.github.io/installation) to get started.
Thanks to [devaslife's tutorial](https://www.youtube.com/watch?v=fFHlfbKVi30), we have a basic setup

---

# Nerd Font installation

For Neovim icons working, we need Nerd Font installed.
Visit the [Nerd Font website](https://www.nerdfonts.com/font-downloads) and download the font.
After download, unzip and put it under ~/.fonts/fonts or ~/.local/share/fonts then run:

```bash
fc-cache -fv
```

# For Codeium plugin setup

Refer to below file to setup lua/plugins/my_plugins.lua

```
~/.local/share/nvim/lazy/codeium.nvim/README.md
```

Run ':Codeium Chat' after enable_chat to run chat

There are many configurations available for Codeium plugin, but it's been limited to a few according to the README mentioned
For further configuration, please refer to the [documentation](https://github.com/Exafunction/codeium.vim). We need to further digging for the detail.

---

# For Obsidian plugin setup

[docs](https://github.com/epwalsh/obsidian.nvim) - Follow this with minor changes as following

```
./lua/.config/nvim/lua/plugins/my_plugins.lua
```
