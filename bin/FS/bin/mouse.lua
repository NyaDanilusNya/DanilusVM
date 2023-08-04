local mx, my = 0, 0
local omx, omy
local clk = false
local tbl = {}
local back = {
  {0,0,0,0,0},
  {0,0,0,0,0},
  {0,0,0,0,0},
  {0,0,0,0,0},
  {0,0,0,0,0}
}
while true do
  tbl = {computer.pullevent(1)}
  if tbl[1] and tbl[1] == "mousedown" and tbl[2] == "1" then
    clk = true
  elseif tbl[1] and tbl[1] == "mouseup" and tbl[2] == "1" then
    clk = false
  elseif tbl[1] and tbl[1] == "mousemotion" then
    mx = tonumber(tbl[2])
    my = tonumber(tbl[3])
  elseif tbl[1] and tbl[1] == "keydown" and tbl[2] == "41" then
    break
  end
  if not mx then goto skip end
  if mx == omx and my == omy then goto skip end
  if omx and not clk then
    for i=1,5 do
      for b=1,5 do
        gpu.setcolor(back[i][b])
        gpu.fill(omx-2+(i-1),omy-2+(b-1),1,1)
      end
    end
  end
  for i=1,5 do
    for b=1,5 do
      back[i][b] = gpu.getpixel(mx-2+(i-1), my-2+(b-1))
    end
  end
  gpu.setcolor(0xFF99FF)
  gpu.fill(mx-2,my-2,5,5)
  gpu.update()
  omx = mx
  omy = my
  ::skip::
end
