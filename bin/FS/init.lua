local function main()
  print("[lua]HI")

  gpu.setColor(0xFFFF00FF)
  gpu.fill(100,100,200,200)
  gpu.update()

  while true do
    local b, p = computer.pollEvent()
    print("[lua] " .. b .. ", " .. p)
    if (p == 41) then print("[lua] dead") break end
  end
end

local res, reas = pcall(main)
if not res then
  collectgarbage()
  print(reas)
end
