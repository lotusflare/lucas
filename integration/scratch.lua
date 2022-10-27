local lucas = require("lucas")
local clock = os.clock

lucas.log_callback(function(message, level)
    print("callback from lua")
    print(level .. message)
end)

lucas.connect("127.0.0.1")

local results = lucas.query("SELECT * FROM testing.data", {})
