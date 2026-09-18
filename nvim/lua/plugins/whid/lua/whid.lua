local api = vim.api
local buf, win
local position = 0

local function center(str)
  local width = api.nvim_win_get_width(0)
  local shift = math.floor(width / 2) - math.floor(string.len(str) / 2)
  return string.rep(" ", shift) .. str
end

local function open_window()
  buf = api.nvim_create_buf(false, true)
end
