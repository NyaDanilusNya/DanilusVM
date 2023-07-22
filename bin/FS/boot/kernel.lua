local r,kb = dofile("/lib/kbutils.lua")
local leters = kb.leters

local function fixfont(tb)
  for c in pairs(tb) do
    for a=1,5 do
      for b=1,5 do
        tb[c][a][b] = tb[c][a][b] == 1 and true or false
      end
    end
  end
end

fixfont(leters)

local line = 1
local row = 1

local function drawchar(text,size)
  local ch = text:sub(1,1)
  for x=1,5 do
    for y=1,5 do
      if leters[ch][y][x] == true then
        gpu.fill((row-1)*size*6+x*size,(line-1)*size*6+y*size,size,size)
      end
    end
  end
  gpu.update()
  row = row + 1
end

function printt(text)
  local ch = ' '
  for i=1,#text do
    ch = text:sub(i,i)
    if ch == '\n' then
      line = line + 1
      row = 1
    elseif ch == '\t' then
      drawchar(' ',2)
      drawchar(' ',2)
      drawchar(' ',2)
    else
      drawchar(ch,2)
    end
  end
end

gpu.setcolor(0xFFFFFFFF)
printt("pain! os\nhttps://www.much-pain.org/\n")

while true do computer.pullevent(1000) end
