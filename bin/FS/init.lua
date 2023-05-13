local function main()
  while true do end

  print("[lua]HI")

  print("[lua] Set col")
  gpu.setColor(0xFFFF00FF)

  print("[lua] Fill")
  gpu.fill(1,1,800,20)

  print("[lua] Set col")
  gpu.setColor(0xFF000000)

  print("[lua] Fill")
  gpu.fill(1,600-25,25,25)

  print("[lua] Set col")
  gpu.setColor(0xFFFF0000)

  print("[lua] Copy")
  gpu.copy(1,1,20,20,60,60)

  print("[lua] Update")
  gpu.update()

  print("[lua] Dangerous funcs")
  gpu.setColor(0xFFFF99FF)
  gpu.fill(200,200, 10, 10)
  for i=1,100 do
    gpu.fill(10,100+i, 10*i, 1)
    gpu.update()
  end
  gpu.copy(1,1,100,200,200,250)
  gpu.update()


  print("[lua] main loop")
  while true do
    local b, p = computer.pollEvent()
    print("[lua] " .. b .. ", " .. p)
    if p == 41 then
      print("[lua] dead")
      break
    end
  end
end

local res, reas = pcall(main)
if not res then
  collectgarbage()
  print(reas)
end
