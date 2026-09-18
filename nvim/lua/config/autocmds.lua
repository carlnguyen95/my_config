-- Autocmds are automatically loaded on the VeryLazy event
-- Default autocmds that are always set: https://github.com/LazyVim/LazyVim/blob/main/lua/lazyvim/config/autocmds.lua
-- Add any additional autocmds here
vim.api.nvim_create_autocmd({ "BufEnter" }, {
  pattern = { "*.sh", "*.pl" },
  callback = function()
    local lines = vim.api.nvim_buf_line_count(0)
    if lines ~= 1 or vim.api.nvim_buf_get_lines(0, 0, 1, true)[1] ~= "" then
      return
    end
    local author = vim.fn.system("whoami"):gsub("\n", "")
    local date = vim.fn.system("date"):gsub("\n", "")
    local filename = vim.fn.expand("%:t")
    local header = vim.fn.readfile("/home/huannc/scripts/.template/header.txt")
    local content = vim.api.nvim_buf_get_lines(0, 0, -1, true)
    local extension = vim.fn.fnamemodify(filename, ":e")
    if extension == "pl" then
      header[1] = header[1]:gsub("bash", "perl")
    end
    header[3] = header[3]:gsub("FILENAME", filename)
    header[4] = header[4]:gsub("AUTHOR", author)
    header[5] = header[5]:gsub("DATE", date)
    header[6] = header[6]:gsub("DATE", date)
    vim.api.nvim_buf_set_lines(0, 0, 0, true, header)
    vim.api.nvim_buf_set_lines(0, #header, -1, true, content)
  end,
})

vim.api.nvim_create_autocmd({ "BufRead", "BufNewFile", "BufEnter" }, {
  pattern = "*.ino",
  callback = function()
    vim.bo.filetype = "cpp"
  end,
})

vim.api.nvim_create_autocmd({ "BufEnter", "BufWinEnter" }, {
  pattern = "*.h",
  callback = function()
    vim.bo.filetype = "c"
  end,
})

vim.api.nvim_create_autocmd({ "BufEnter", "BufWinEnter" }, {
  pattern = "*.hpp",
  callback = function()
    vim.bo.filetype = "cpp"
  end,
})

vim.api.nvim_create_autocmd({ "BufRead", "BufNewFile", "BufEnter" }, {
  pattern = "*.v",
  callback = function()
    vim.bo.filetype = "verilog"
  end,
})
