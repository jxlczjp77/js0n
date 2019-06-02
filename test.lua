local js = require "js0n"

local json = '{"foo":"bar","barbar":[1,2,3],"obj":{"a":"b"}}'
print(string.format("parsing '%s'", json))

local parser = js.new(json)
print(string.format("returned %s", json:sub(parser("barbar"))))

local array = '["foo","bar",[1,2,3],{"a":"b"},42,[],[4,5,6,{"a":"b"}]]'
print(string.format("parsing '%s'", array))
parser:reset(array)

for i = 1, 100 do
    local s, e = parser(0)
    if not s then break end
    print(string.format("array[%d] %s", i, array:sub(s, e)))
end

local array1 = '[],{}'
print(string.format("parsing '%s'", array1))
parser:reset(array1)
print(string.format("array1[0] %s", parser(0)))
