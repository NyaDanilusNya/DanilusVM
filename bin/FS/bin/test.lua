printt("starting test script...\n")
printt("test return cursor")
local winw,winh = gpu.getresolution()
local vw,vh = winw/12,winh/12
for i=1,5 do
  printt('.')
  computer.pullevent(500)
end
printt("\n")
for i=1,10 do
  printt("\raboba " .. i .. ": [" .. string.rep('#', i-1) .. string.rep(' ', 9-(i-1)) .. "]")
end
--printt("\nwell done.\nwaiting.....\n")
printt("\nresolution is: " .. winw .. "x" .. winh .. "\n")
printt("test new line")
for i=1,5 do
  printt('.')
  computer.pullevent(500)
end
printt("\n")
for i=1,69 do
  printt("new line: " .. i .. "\n")
end

printt("test colors")
for i=1,5 do
  printt('.')
  computer.pullevent(500)
end
printt("\n")

drawchar('#', nil, 0x000000)
offsetcursor(1)
drawchar('#', nil, 0xFFFFFF)
offsetcursor(1)
drawchar('#', nil, 0xFF0000)
offsetcursor(1)
drawchar('#', nil, 0x00FF00)
offsetcursor(1)
drawchar('#', nil, 0x0000FF)
offsetcursor(1)
drawchar('#', nil, 0xFFFF00)
offsetcursor(1)
drawchar('#', nil, 0x00FFFF)
offsetcursor(1)
drawchar('#', nil, 0xFF00FF)
offsetcursor(1)
drawchar('#', nil, 0xFF99FF)
offsetcursor(1)
drawchar('#', nil, 0xFFFF99)
offsetcursor(1)
drawchar('#', nil, 0x99FFFF)
offsetcursor(1)
drawchar('#', nil, 0x99FF99)
offsetcursor(1)
drawchar('#', nil, 0x9999FF)
offsetcursor(1)
drawchar('#', nil, 0xFF9999)
offsetcursor(1)
printt("\ndone!\n")
setcursor(vw/2-1)
drawchar('|', nil, 0xFFFFFF)
offsetcursor(1)
drawchar(' ', nil, nil, 0x00FFFF)
offsetcursor(1)
drawchar(' ', nil, nil, 0x00FFFF)
offsetcursor(1)
drawchar(' ', nil, nil, 0x00FFFF)
offsetcursor(1)
printt("\n")
setcursor(vw/2-1)
drawchar('|', nil, 0xFFFFFF)
offsetcursor(1)
drawchar(' ', nil, nil, 0xFFFF00)
offsetcursor(1)
drawchar(' ', nil, nil, 0xFFFF00)
offsetcursor(1)
drawchar(' ', nil, nil, 0xFFFF00)
offsetcursor(1)
printt("\n")
setcursor(vw/2-1)
drawchar('|', nil, 0xFFFFFF)
offsetcursor(1)
printt("\n")
setcursor(vw/2-1)
drawchar('|', nil, 0xFFFFFF)
offsetcursor(1)

printt("\n\n\nwaiting (press any key to exit)\n")
local tbl = {}
local stop = true
while stop do
  for i=1,25 do
    printt("\r[" .. string.rep('-', i-1) .. "###" .. string.rep('-', 25-(i-1)) .. "]")
    tbl = {computer.pullevent(1)}
    if tbl[1] == "keydown" then stop = false break end
  end
  if not stop then break end
  for i=1,25 do
    printt("\r[" .. string.rep('-', 25-(i-1)) .. "###" .. string.rep('-', (i-1)) .. "]")
    tbl = {computer.pullevent(1)}
    if tbl[1] == "keydown" then stop = false break end
  end
end
printt("\n")
