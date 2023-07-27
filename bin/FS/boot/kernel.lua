local r,kb = dofile("/lib/kbutils.lua")
local leters = kb.leters
local keysym = kb.keysym
local keysymup = kb.keysymup
local winw,winh = gpu.getresolution()
local vw,vh = winw/12,winh/12

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
local csize = 2

local pwd = "/"

function drawchar(text,size, fg, bg, posx, posy)
  while line > vh do
    gpu.copy(1,1,winw,winh,0,-(6*csize))
    gpu.setcolor(0xFF000000)
    gpu.fill(1,winh-(6*csize),winw,(6*csize))
    gpu.update()
    line = line - 1
  end
  size = size or csize
  posx = posx or row
  posy = posy or line
  fg = fg or 0xFFFFFFFF
  bg = bg or 0xFF000000
  local ch = text:sub(1,1):lower()
  if leters[ch] == nil then return end
  for x=1,5 do
    for y=1,5 do
      if leters[ch][y][x] == true then
        gpu.setcolor(fg)
      else
        gpu.setcolor(bg)
      end
      gpu.fill((row-1)*size*6+x*size,(line-1)*size*6+y*size,size,size)
    end
  end
  gpu.update()
end

function setcursor(x,y)
  x = x or row
  y = y or line

  row = x
  line = y
end
function getcursor() --> number. number
  return row, line
end
function offsetcursor(x,y)
  x = x or 0
  y = y or 0

  row = row + x
  line = line + y
end
function getpwd() --> string
  return pwd
end
function setpwd(newpwd)
  pwd = newpwd
end
function getsize() --> number
  return csize
end
function setsize(newsize)
  csize = tonumber(newsize) or 2
  gpu.setcolor(0xFF000000)
  gpu.fill(1,1,winw,winh)
  gpu.update()
  setcursor(1,1)
  vw,vh = winw/(6*csize), winh/(6*csize)
  print(vw,vh)
end

function printt(text)
  local ch = ''
  for i=1,#text do
    ch = text:sub(i,i)
    if ch == '\n' then
      line = line + 1
      row = 1
    elseif ch == '\t' then
      drawchar(' ')
      drawchar(' ')
      drawchar(' ')
    elseif ch == '\r' then
      row = 1
    else
      drawchar(ch)
      row = row + 1
    end
    if row > vw then
      line = line + 1
      row = 1
    end
  end
end

function read() --> string
  local tbl = {}
  local ch = ''
  local buf = ""
  local shift = false
  drawchar('|', csize,nil,nil,row)
  while true do
    tbl = {computer.pullevent(1000)}
    if tbl[1] and tbl[1] == "keydown" and keysym[tonumber(tbl[2])] then
      ch = shift and keysymup[tonumber(tbl[2])] or keysym[tonumber(tbl[2])]
      if ch == '\n' then
        drawchar(' ', csize,nil,nil,row)
        printt("\n")
        return buf
      elseif ch == '\b' and #buf > 0 then
        buf = buf:sub(1,#buf-1)
        drawchar(' ', csize,nil,nil,row)
        row = row - 1
        drawchar(' ')
        drawchar('|', csize,nil,nil,row)
      elseif leters[ch:lower()] then
        buf = buf .. ch
        drawchar(ch)
        row = row + 1
        drawchar('|', csize,nil,nil,row)
      end
    elseif tbl[1] and tbl[2] == "225" then
      if tbl[1] == "keydown" then
        shift = true
      elseif tbl[1] == "keyup" then
        shift = false
      end
    end
  end
end

function split (inputstr, sep)
  sep = sep or '%s'
  local t={}
  for str in string.gmatch(inputstr, "([^"..sep.."]+)") do
    t[#t+1] = str
  end
  return t
end

gpu.setcolor(0xFFFFFFFF)
printt("pain! os\nhttps://www.much-pain.org/\n\n")

dofile("/bin/sh.lua")
