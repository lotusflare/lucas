local lucas = require("lucas")
local clock = os.clock
local pretty = require("pl.pretty")

lucas.logger(function(message, level)
    if level >= 5 then
        print(message)
    end
end)

lucas.connect("cassandra")

local results = lucas.query("SELECT * FROM testing.data", {})