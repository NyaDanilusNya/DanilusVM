local args = {...}
if #args == 0 then
  return
end

local newpwd = args[1]:sub(1,1) == '/' and args[1] or getpwd() .. args[1]
if filesystem.isdir(newpwd) then
  newpwd = newpwd:sub(#newpwd,#newpwd) == "/" and newpwd or newpwd .. "/"
  setpwd(newpwd)
end
