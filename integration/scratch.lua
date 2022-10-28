local lucas = require("lucas")
local clock = os.clock
local pretty require("pl.pretty")


lucas.log_callback(function(message, level)
    if level >= 5 then
        print(message)
    end
end)

lucas.connect("127.0.0.1")

local results = lucas.query("SELECT * FROM testing.data", {})
