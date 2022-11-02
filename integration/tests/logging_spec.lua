require("busted.runner")()
local lucas = require("lucas")
local spy = require("luassert.spy")
local pretty = require("pl.pretty")

assert:register("matcher", "contains", function(state, args)
    local needle = args[1]
    return function(haystack)
        if string.find(haystack, needle, 0, true) then
            return true
        else
            return false
        end
    end
end)

describe("logging", function()
    it("callback hook works", function()
        local s = spy.new(function() end)
        lucas.logger(function(message, level)
            s(message, level)
        end)
        lucas.connect(os.getenv("CASSANDRA_HOST"))
        assert.spy(s).was.called_with(
            match.contains("Connected to host"),
            match.is_number()
        )
    end)
end)

describe("metrics", function()
    it("collects general metrics", function()
        lucas.connect(os.getenv("CASSANDRA_HOST"))
        local metrics = lucas.metrics()
        assert.array.has({}, metrics)
    end)

    it("collects speculative execution metrics", function()
        lucas.connect(os.getenv("CASSANDRA_HOST"))
        local metrics = lucas.speculative_execution_metrics()
        assert.array.has({}, metrics)
    end)
end)