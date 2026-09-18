local colorscheme = require("lazyvim.plugins.colorscheme")
return {
  {
    "CopilotC-Nvim/CopilotChat.nvim",
    dependencies = {
      { "nvim-lua/plenary.nvim", branch = "master" },
    },
    build = "make tiktoken",
    config = function()
      require("CopilotChat.config").providers.ollama = {
        prepare_input = require("CopilotChat.config.providers").copilot.prepare_input,
        prepare_output = require("CopilotChat.config.providers").copilot.prepare_output,

        get_models = function(headers)
          local response, err = require("CopilotChat.utils").curl_get("http://localhost:11434/v1/models", {
            headers = headers,
            json_response = true,
          })

          if err then
            error(err)
          end

          return vim.tbl_map(function(model)
            return {
              id = model.id,
              name = model.id,
            }
          end, response.body.data)
        end,

        get_url = function()
          return "http://localhost:11434/v1/chat/completions"
        end,
      }
    end,
    opts = {
      -- See Configuration section for options
    },
  },
  {
    "L3MON4D3/LuaSnip",
    dependencies = {
      "rafamadriz/friendly-snippets",
    },

    config = function()
      require("luasnip.loaders.from_vscode").load({ paths = { "~/.config/nvim/snippets/" } })
      require("luasnip.loaders.from_lua").load({ paths = { "~/.config/nvim/lua/config/snip/" } })
    end,
  },
  {
    "epwalsh/obsidian.nvim",
    version = "3.7.10", -- recommended, use latest release instead of latest commit
    dependencies = {
      -- Required.
      "nvim-lua/plenary.nvim",
    },
    config = function()
      require("obsidian").setup({
        workspaces = {
          {
            name = "personal",
            path = "~/Documents/HomeVault/",
          },
          {
            name = "work",
            path = "~/Documents/HomeVault/work/",
          },
        },
        daily_notes = {
          folder = "Daily_notes",
          date_format = "%m-%d-%Y",
        },
        new_notes_location = os.execute("pwd"),
        templates = {
          subdir = "Template", -- Directory for templates
          date_format = "%Y-%m-%d", -- Format for {{date}}
          time_format = "%H:%M", -- Format for {{time}}
          substitutions = {
            -- Custom dynamic variables if needed
          },
        },
      })
    end,
  },
  {
    "s1n7ax/nvim-window-picker",
    name = "window-picker",
    event = "VeryLazy",
    version = "2.*",
    config = function()
      require("window-picker").setup()
    end,
  },
  {
    "mason-org/mason-lspconfig.nvim",
    opts = {},
    dependencies = {
      { "mason-org/mason.nvim", opts = {} },
      "neovim/nvim-lspconfig",
    },
  },
  {
    "scottmckendry/cyberdream.nvim",
    lazy = false,
    priority = 1000,
    config = function()
      require("cyberdream").setup({
        -- Set light or dark variant
        variant = "default", -- use "light" for the light variant. Also accepts "auto" to set dark or light colors based on the current value of `vim.o.background`

        -- Enable transparent background
        transparent = true,

        -- Reduce the overall saturation of colours for a more muted look
        saturation = 1, -- accepts a value between 0 and 1. 0 will be fully desaturated (greyscale) and 1 will be the full color (default)

        -- Enable italics comments
        italic_comments = false,

        -- Replace all fillchars with ' ' for the ultimate clean look
        hide_fillchars = false,

        -- Apply a modern borderless look to pickers like Telescope, Snacks Picker & Fzf-Lua
        borderless_pickers = false,

        -- Set terminal colors used in `:terminal`
        terminal_colors = true,

        -- Improve start up time by caching highlights. Generate cache with :CyberdreamBuildCache and clear with :CyberdreamClearCache
        cache = false,

        -- Override highlight groups with your own colour values
        highlights = {
          -- Highlight groups to override, adding new groups is also possible
          -- See `:h highlight-groups` for a list of highlight groups or run `:hi` to see all groups and their current values

          -- Example:
          Comment = { fg = "#696969", bg = "NONE", italic = true },

          -- More examples can be found in `lua/cyberdream/extensions/*.lua`
        },

        -- Override a highlight group entirely using the built-in colour palette
        overrides = function(colors) -- NOTE: This function nullifies the `highlights` option
          -- Example:
          return {
            Comment = { fg = colors.green, bg = "NONE", italic = true },
            ["@property"] = { fg = colors.magenta, bold = true },
          }
        end,

        -- Override colors
        colors = {
          -- For a list of colors see `lua/cyberdream/colours.lua`

          -- Override colors for both light and dark variants
          bg = "#000000",
          green = "#00ff00",

          -- If you want to override colors for light or dark variants only, use the following format:
          dark = {
            magenta = "#ff00ff",
            fg = "#eeeeee",
          },
          light = {
            red = "#ff5c57",
            cyan = "#5ef1ff",
          },
        },

        -- Disable or enable colorscheme extensions
        extensions = {
          telescope = true,
          notify = true,
          mini = true,
        },

        -- Alternatively, you can use 'default' to set all extensions at once
        -- cache = true, -- Use cache for fastest loads
        -- extensions = {
        --     default = false, -- Disable all by default
        --     base = true, -- Enable all built-in hl groups (you probably want this)
        --
        --     -- Now enable only what you want to use
        --     telescope = true,
        --     cmp = true,
        --     gitsigns = true,
        -- },
      })
    end,
  },
  {
    "vague-theme/vague.nvim",
    lazy = false,
    priority = 1000,
    config = function()
      require("vague").setup({
        transparent = false, -- If true, background is not set
        bold = true, -- Disable bold globally
        italic = true, -- Disable italic globally
        on_highlights = function(hl, colors) end,
        colors = {
          bg = "#141415",
          inactiveBg = "#1c1c24",
          fg = "#cdcdcd",
          floatBorder = "#878787",
          line = "#252530",
          comment = "#606079",
          builtin = "#b4d4cf",
          func = "#c48282",
          string = "#e8b589",
          number = "#e0a363",
          property = "#c3c3d5",
          constant = "#aeaed1",
          parameter = "#bb9dbd",
          visual = "#333738",
          error = "#d8647e",
          warning = "#f3be7c",
          hint = "#7e98e8",
          operator = "#90a0b5",
          keyword = "#6e94b2",
          type = "#9bb4bc",
          search = "#405065",
          plus = "#7fa563",
          delta = "#f3be7c",
        },
      })
    end,
  },
  {
    "olimorris/onedarkpro.nvim",
    priority = 1000, -- Ensure it loads first
  },
  {
    "diegoulloao/neofusion.nvim",
    priority = 1000,
    config = function()
      require("neofusion").setup({
        palette_overrides = {
          bright_green = "#ec30ac",
          -- rest,
        },
        transparent_mode = true,
        overrides = {
          ["@comment.lua"] = { fg = "#696969", bg = "NONE", italic = true },
        },
      })
    end,
  },
  {
    "LazyVim/LazyVim",
    opts = {
      colorscheme = "cyberdream",
    },
  },
  {
    "yuukiflow/Arduino-Nvim",
    dependencies = {
      "nvim-telescope/telescope.nvim",
      "neovim/nvim-lspconfig",
    },
    config = function()
      -- Load Arduino plugin for .ino files
      vim.api.nvim_create_autocmd("FileType", {
        pattern = "arduino",
        callback = function()
          require("Arduino-Nvim")
        end,
      })
    end,
  },
  {
    "neovim/nvim-lspconfig",
    dependencies = {
      -- Automatically install LSPs to stdpath for neovim
      "williamboman/mason.nvim",
      "williamboman/mason-lspconfig.nvim",
    },
    config = function()
      -- 1. Initialize Mason
      require("mason").setup()

      -- 2. Ensure your preferred Python server is installed
      require("mason-lspconfig").setup({
        ensure_installed = { "basedpyright", "ruff" }, -- Or 'pylsp'
      })

      -- 3. Configure the servers via lspconfig
      local lspconfig = require("lspconfig")

      -- Configure Pyright / Basedpyright
      lspconfig.basedpyright.setup({
        settings = {
          basedpyright = {
            analysis = {
              autoSearchPaths = true,
              diagnosticMode = "openFilesOnly",
              useLibraryCodeForTypes = true,
            },
          },
        },
      })

      -- Configure Ruff for fast linting/formatting
      lspconfig.ruff.setup({})

      -- Keymaps to trigger when an LSP connects to a buffer
      vim.api.nvim_create_autocmd("LspAttach", {
        group = vim.api.nvim_create_augroup("UserLspConfig", {}),
        callback = function(ev)
          local opts = { buffer = ev.buf }
          vim.keymap.set("n", "gd", vim.lsp.buf.definition, opts)
          vim.keymap.set("n", "K", vim.lsp.buf.hover, opts)
          vim.keymap.set("n", "<leader>rn", vim.lsp.buf.rename, opts)
          vim.keymap.set({ "n", "v" }, "<leader>ca", vim.lsp.buf.code_action, opts)
        end,
      })
    end,
  },
}
