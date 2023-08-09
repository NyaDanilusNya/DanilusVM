local r,img = dofile("/lib/image.lua")

if not r then printt(img .. "\n") end

local min = {
  0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF,
  0xFF99FF, 0xFF99FF, 0xFF99FF, 0xFF99FF, 0xFF99FF,
  0xFF0000, 0xFF0000, 0xFF0000, 0xFF0000, 0xFF0000,
  0x0000FF, 0x0000FF, 0x0000FF, 0x0000FF, 0x0000FF,
  0x555555, 0x555555, 0x555555, 0x555555, 0x555555
}

local nm = img.normalize(min,5)
nm = img.resize(nm, 25)

res, ptr = gpu.storetexture(img.toraw(nm))
if not res then printt(ptr .. "\n") return end
local sw,sh = #nm, #nm[1]
nm = nil
collectgarbage()
gpu.drawtexture(ptr, 200, 200, sw, sh)
gpu.update()

local boba = img.normalize(min, 5)
for w=1,#boba do
  for h=1,#boba[w] do
    gpu.setcolor(boba[w][h])
    gpu.fill(h*25, 175+w*25, 25, 25)
    gpu.update()
  end
end

gpu.freetexture(ptr)

local oldx,oldy = getcursor()
setcursor(1,1)
drawchar('%', nil, nil, nil, 1, 1)
setcursor(oldx,oldy)

res,ptr = gpu.copytexture(1,1,100,100)
gpu.drawtexture(ptr,600,100,100,100)
gpu.freetexture(ptr)

