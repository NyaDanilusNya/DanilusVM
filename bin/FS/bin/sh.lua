printt("\n\nwelcome to shell\n")
local com = ""
local args = {}
local res, reas
while true do
  printt(getpwd() .. "# ")
  com = read()
  args = split(com)
  if #args > 0 then
    com = args[1]
    table.remove(args, 1)

    if com == "exit" then
      break
    else
      com = com:sub(1,1) == '/' and com or "/bin/" .. com .. ".lua"
      if filesystem.exists(com) then
        res, reas = dofile(com, table.unpack(args))
        if not res then printt(com .. ": " .. reas .. "\n") end
      else
        printt(com .. ": not found\n")
      end
    end
  end
end
