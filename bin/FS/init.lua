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
  local res1 = filesystem.exists("/init.lua")
  local res2 = filesystem.exists("init.lua")
  local res3 = filesystem.exists("/../main")
  print("[lua] 1: ", res1, ", 2: ", res2, ", 3: ", res3)

  local leters = {
    a={
      {0,0,1,0,0},
      {0,1,0,1,0},
      {0,1,1,1,0},
      {0,1,0,1,0},
      {0,1,0,1,0},
    },
    b={
      {0,1,1,0,0},
      {0,1,0,1,0},
      {0,1,1,0,0},
      {0,1,0,1,0},
      {0,1,1,0,0},
    },
    o={
      {0,0,1,0,0},
      {0,1,0,1,0},
      {0,1,0,1,0},
      {0,1,0,1,0},
      {0,0,1,0,0},
    }
  }

  str = "aboba"
  fontsize = 15

  gpu.setcolor(0xFF0000FF)
  for i=1,#str do
    for x=1,5 do
      for y=1,5 do
        if leters[str:sub(i,i)][y][x] == 1 then
          gpu.fill(((i-1)*fontsize*5) + 2 + x*fontsize,y*fontsize,fontsize,fontsize)
          gpu.update()
        end
      end
    end
  end

  print("[lua] main loop")
  while true do
    local b, p = computer.pullevent(1000)
    if b then
      print("[lua] " .. (b or "(nil)") .. ", " .. (p or "(nil)"))
      if p == "41" then
        print("[lua] dead")
        break
      end
    else
      print("[lua] elsa")
    end
  end

end

local res, reas = pcall(main)
if not res then
  collectgarbage()
  print(reas)
end
