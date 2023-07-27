local args = {...}
if #args == 0 then
  return
end

args[1] = args[1]:sub(1,1) == '/' and args[1] or getpwd() .. args[1]

if filesystem.exists(args[1]) and not filesystem.isdir(args[1]) then
  local r,f = filesystem.open(args[1], "r")
  if not r then printt("cannot open file\n") return end
  local chunk, res = ""
  while true do
    res, chunk = filesystem.read(f, 512)
    if res then printt(chunk) else break end
  end
  filesystem.close(f)
end
