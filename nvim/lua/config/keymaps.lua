-- Keymaps are automatically loaded on the VeryLazy event
-- Default keymaps that are always set: https://github.com/LazyVim/LazyVim/blob/main/lua/lazyvim/config/keymaps.lua
-- Add any additional keymaps her

vim.keymap.set("n", "<c-n>", ":Neotree toggle dir=./<CR>", { silent = true })
vim.keymap.set("n", "<c-u>", ":Neotree toggle dir=/home/huannc/<CR>", { silent = true })

------------------------------------------------
-- Obsidian
vim.keymap.set("n", "gf", function()
  if require("obsidian").util.cursor_on_markdown_link() then
    return "<cmd>ObsidianFollowLink<CR>"
  else
    return "gf"
  end
end, { noremap = false, expr = true })
vim.keymap.set("n", "<c-x>", ":ObsidianToggleCheckbox<CR>", { silent = true })
-- Open today note
vim.keymap.set("n", "<c-y>", ":ObsidianWorkspace personal<CR>:ObsidianToday<CR>", { silent = true })
------------------------------------------------

-- CopilotChat
vim.keymap.set({ "n", "v" }, "<c-\\>", ":CopilotChatToggle<CR>", { silent = true })
vim.keymap.set({ "n", "v" }, "<c-m>", ":CopilotChatModels<CR>", { silent = true })
vim.keymap.set({ "n", "v" }, "<c-e>", ":CopilotChatExplain<CR>", { silent = true })
vim.keymap.set({ "v" }, "<c-d>", ":CopilotChatDocs<CR>", { silent = true })

-- Switching buffers
vim.keymap.set("n", "<tab>", ":bnext<CR>", { silent = true })
vim.keymap.set("n", "<s-tab>", ":bprevious<CR>", { silent = true })
vim.keymap.set("n", "<s-q>", ":bdelete<CR>", { silent = true })
vim.keymap.set("v", "<c-_>", "gc", { silent = true })
