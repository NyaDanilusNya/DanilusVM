local img = {}

function img.savetofile(tbl, path)
  local r,f = filesystem.open(path, "wb")
  if not r then return false end

  local r = bit32.band(bit32.rshift(color, 16), 0xFF)
  local g = bit32.band(bit32.rshift(color, 8), 0xFF)
  local b = bit32.band(color, 0xFF)
  filesystem.write(f, string.char(r,g,b))

  filesystem.close(f)
  return true
end

function img.loadfromfile(path)
  local image = {}
  local r,f = filesystem.open(filename, "rb")
  if not r then
    return nil
  end
  local data = filesystem.read(f,3)
  local r,g,b,color
  while data do
    r, g, b = string.byte(data, 1, 3)
    color = bit32.bor(bit32.lshift(r, 16), bit32.lshift(g, 8), b)
    image[#image+1] = color
    data = filesystem.read(f,3)
  end
  filesystem.close(f)
  return image
end

function img.normalize(rawimg, w)
  local nm = {}
  local ctr = 1
  for a=1,#rawimg/w do
    nm[#nm+1] = {}
    for i=1,w do
      nm[a][i] = rawimg[ctr]
      ctr = ctr + 1
    end
  end
  return nm
end

function img.toraw(nm)
  local raw = {}
  for a=1,#nm do
    for b=1,#nm[a] do
      raw[#raw+1] = nm[a][b]
    end
  end
  return raw
end

function img.resize(nm, size)
  local newTbl = {}
  for a=1,#nm do
    for c=1,size do
      newTbl[#newTbl+1] = {}
      for b=1,#nm[a] do
        for i=1,size do
          newTbl[#newTbl][#newTbl[#newTbl]+1] = nm[a][b]
        end
      end
    end
  end
  return newTbl
end

return img
