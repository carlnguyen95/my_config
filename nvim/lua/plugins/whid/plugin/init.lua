if vim.fn.exists("g:loaded_whid") then
  return
end

vim.cmd("let s:save_cpo = &cpo")
vim.cmd("set cpo&vim")

local whid = require("whid")
whid.whid()

vim.cmd("let &cpo = s:save_cpo")
vim.cmd("unlet s:save_cpo")

vim.g["loaded_whid"] = 1
