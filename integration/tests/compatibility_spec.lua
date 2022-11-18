require("busted.runner")()
local pretty = require("pl.pretty")
local lucas = require("lucas")
local compat = require("lucas.compatibility")

describe("compatibility", function()
    it("convert string", function()
        local result = compat.convert("hello")
        assert.array.is({ 13, "hello" }, result)
    end)
end)