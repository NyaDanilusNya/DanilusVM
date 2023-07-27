local args = {...}
local path = #args > 0 and args[1] or getpwd()

local r, tab = filesystem.listdir(path:sub(1,1) == '/' and path or getpwd() .. path)
if not r then return end
for i=1,#tab do
  printt(tab[i] .. " ")
end
printt("\n")
