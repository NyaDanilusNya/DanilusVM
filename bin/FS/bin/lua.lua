printt("lua interpreter. type '!exit' to exit\n")
local res, reas, f
while true do
  printt(">")
  local prompt = read()
  if prompt == "!exit" then break end
  if prompt:sub(1,1) == '=' then prompt = "return " .. prompt:sub(2,#prompt) end
  f, reas = load(prompt)
  if not f then printt(reas) end
  local res = {pcall(f)}

  if #res > 1 then for i=2,#res do printt(res[i] .. " ") end printt("\n") end
end
