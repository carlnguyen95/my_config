local lazypath = vim.fn.stdpath("data") .. "/lazy/lazy.nvim"
if not vim.loop.fs_stat(lazypath) then
  -- bootstrap lazy.nvim
  -- stylua: ignore
  vim.fn.system({ "git", "clone", "--filter=blob:none", "https://github.com/folke/lazy.nvim.git", "--branch=stable", lazypath })
end
vim.opt.rtp:prepend(vim.env.LAZY or lazypath)

require("lazy").setup({
  spec = {
    -- add LazyVim and import its plugins
    { "LazyVim/LazyVim", import = "lazyvim.plugins" },
    -- import any extras modules here
    { import = "lazyvim.plugins.extras.lang.typescript" },
    { import = "lazyvim.plugins.extras.lang.json" },
    -- { import = "lazyvim.plugins.extras.ui.mini-animate" },
    -- import/override with your plugins
    { import = "plugins" },
  },
  defaults = {
    -- By default, only LazyVim plugins will be lazy-loaded. Your custom plugins will load during startup.
    -- If you know what you're doing, you can set this to `true` to have all your custom plugins lazy-loaded by default.
    lazy = false,
    -- It's recommended to leave version=false for now, since a lot the plugin that support versioning,
    -- have outdated releases, which may break your Neovim install.
    version = false, -- always use the latest git commit
    -- version = "*", -- try installing the latest stable version for plugins that support semver
  },
  install = { colorscheme = { "catppuccin", "cyberdream" } },
  checker = { enabled = true }, -- automatically check for plugin updates
  performance = {
    rtp = {
      -- disable some rtp plugins
      disabled_plugins = {
        "gzip",
        -- "matchit",
        -- "matchparen",
        -- "netrwPlugin",
        "tarPlugin",
        "tohtml",
        "tutor",
        "zipPlugin",
      },
    },
  },
})

require("mason-lspconfig").setup({

  ensure_installed = {

    "clangd",

    "arduino_language_server",

    "gopls",

    "rust_analyzer",

    "pyright",
  },

  handlers = {
    arduino_language_server = function()
      require("lspconfig").arduino_language_server.setup({

        cmd = {

          "arduino-language-server",

          "--clangd",

          "/home/huannc/.local/share/nvim/mason/bin/clangd",

          "--cli",

          "/bin/arduino-cli",

          "--cli-config",

          "~/.arduino15/arduino-cli.yaml",

          "--fqbn",

          "esp8266:esp8266:nodemcuv2",
        },
      })
    end,
  },
})

local nvim_lsp = require("lspconfig")

nvim_lsp.svlangserver.setup({
  on_init = function(client)
    local path = client.workspace[1].name

    if path == "/path/to/project1" then
      client.config.settings.systemverilog = {
        includeIndexing = { "**/*.{sv,svh}" },
        excludeIndexing = { "test/**/*.sv*" },
        defines = {},
        launchConfiguration = "/tools/verilator -sv -Wall --lint-only",
        formatCommand = "/tools/verible-verilog-format",
      }
    elseif path == "/path/to/project2" then
      client.config.settings.systemverilog = {
        includeIndexing = { "**/*.{sv,svh}" },
        excludeIndexing = { "sim/**/*.sv*" },
        defines = {},
        launchConfiguration = "/tools/verilator -sv -Wall --lint-only",
        formatCommand = "/tools/verible-verilog-format",
      }
    end

    client.notify("workspace/didChangeConfiguration")
    return true
  end,
})

nvim_lsp.verible.setup({
  cmd = { "verible-verilog-format", "--rules_config_search" },
})
