local function main()
  --[[
  print("rmdir: /duda: " .. tostring(filesystem.rmdir("/duda")))
  print("exists: /duda: " .. tostring(filesystem.exists("/duda")))
  print("rmfile: /aboba: " .. tostring(filesystem.rmfile("/aboba")))
  print("isdir: /aboba: " .. tostring(filesystem.isdir("/aboba")))
  ]]
  --[[
  gpu.setcolor(0xFFFF99FF)
  for i=1,100 do
    gpu.fill(1,600-15, i*8, 15)
    gpu.update()
  end

  computer.pushevent("test", 69)
  print(computer.pullevent(1000))
  ]]

  --[[
  local r, fd
  filesystem.mkdir("/aboba")
  r,fd = filesystem.open("/aboba/test", "w")
  print("w: ", r)
  filesystem.write(fd, "super test of bububa\nagakakskagesh")
  filesystem.close(fd)

  r,fd = filesystem.open("/aboba/test", "a")
  print("a: ", r)
  filesystem.write(fd, "\nheker\n")
  filesystem.close(fd)

  print("rf: ", filesystem.rmfile("/aboba"))
  print("rd: ", filesystem.rmdir("/aboba"))

  local buffer = ""
  res, fd = filesystem.open("/aboba/test", "r")
  if res then
    while true do
      local res, chunk = filesystem.read(fd, 64)
      if res then buffer = buffer .. chunk else break end
    end
    print("[lua] text: '" .. buffer .. "'")
    filesystem.close(fd)
  end
  ]]

  --[[
  local res, list = filesystem.listdir("/aboba")
  print(res)
  if res then
    for i=1,#list do
      print(list[i])
    end
  end
  local res, resp = filesystem.size("/init.lua")
  if res then
    print("[Lua] size of /init.lua = " .. resp)
  end
  ]]
  --[[
  local res1 = filesystem.exists("/init.lua")
  local res2 = filesystem.exists("init.lua")
  local res3 = filesystem.exists("/../main")
  print("[lua] 1: ", res1, ", 2: ", res2, ", 3: ", res3)
  ]]

  --[[
  draw(3, "aboba linux\n\n[danilus@aboba]$ ")
  draw(3, "sudo pacman -s aboba\n")
  draw(3, "aboba linux\n\n[danilus@aboba]$ ")
  ]]

  --[[
  local r, f = filesystem.open("/aboba", "w")
  for i,k in pairs(_G) do
    if i:sub(1,1) ~= "_" and type(k) == "table" then
      for a,b in pairs(k) do filesystem.write(f,i .. "." .. a .. "\n") end
    end
  end
  filesystem.close(f)
  ]]

  --[[
  print("[lua] main loop")
  while true do
    local b, p = computer.pullevent(1000)
    if b then
      print("[lua] " .. (b or "(nil)") .. ", " .. (p or "(nil)"))
      if p == "41" then
        print("[lua] dead")
        break
      end
      if b == "keydown" and keysym[tonumber(p)] ~= nil then
        draw(3, keysym[tonumber(p)])
      end
    end
  end
  ]]
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
