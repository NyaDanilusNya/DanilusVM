local function main()
  function dofile(path, ...)
    if filesystem.exists(path) and not filesystem.isdir(path) then
      local r,f = filesystem.open(path, "r")
      if not r then return false, "cannot open file" end
      local buf, chunk, res = "", ""
      while true do
        res, chunk = filesystem.read(f, 512)
        if res then buf = buf .. chunk else break end
      end
      filesystem.close(f)
      local func, reas = load(buf, "=" ..path)
      if not func then return false, reas end
      return pcall(func,...)
    end
  end
  function log(text)
    local r,f = filesystem.open("/boot/log", "a")
    if r then filesystem.write(f, tostring(text) .. "\n") filesystem.close(f) end
  end
  local res, reas = dofile("/boot/kernel.lua")
  print(res, reas)
  if not res then log(reas) end

end

local res, reas = pcall(main)
if not res then
  collectgarbage()
  print(reas)
end
