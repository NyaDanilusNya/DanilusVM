local tab = {}
while true do
  tab = {computer.pullevent(1000)}
  if tab[1] then
    printt(tab[1] .. ": " .. tab[2] .. (tab[3] and " " .. tab[3] or "") .. "\n")
    if tab[1] == "keydown" and tab[2] == "41" then break end
  end
end
