local function main()
  gpu.setColor(0xFFFF99FF)
  for i=1,100 do
    gpu.fill(1,600-15, i*8, 15)
    gpu.update()
  end

  computer.pushEvent("test", 69)
  print(computer.pullEvent(1000))

  print("[lua] main loop")
  while true do
    local b, p = computer.pullEvent(1000)
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
