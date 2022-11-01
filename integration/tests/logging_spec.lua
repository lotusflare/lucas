require("busted.runner")()
local lucas = require("lucas")
local spy = require("luassert.spy")
local pretty = require("pl.pretty")

assert:register("matcher", "contains", function(state, args)
    return function(value)
        if string.find(value, args[1], 0, true) then
            return true
        else
            return false
        end
    end
end)

describe("logging", function()
    it("callback works", function()
        local s = spy.new(function() end)
        lucas.logger(function(message, level)
            s(message, level)
        end)
        local err = lucas.connect(os.getenv("CASSANDRA_HOST"))
        assert.spy(s).was.called_with(
            match.contains("Connected to host"),
            match.is_number()
        )
    end)
end)

describe("metrics", function()
    it("collects stats", function()
        lucas.connect(os.getenv("CASSANDRA_HOST"))
        local metrics = lucas.metrics()
    end)
end)