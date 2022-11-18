require("busted.runner")()
local pretty = require("pl.pretty")
local lucas = require("lucas")
local compat = require("lucas.compatibility")

describe("compatibility", function()
    it("convert string", function()
        local result = compat.convert("hello")
        assert.are.same(lucas.varchar("hello"), result)
    end)

    it("convert int", function()
        local result = compat.convert(32)
        assert.are.same(lucas.int(32), result)
    end)

    it("convert table", function()
        local result = compat.convert({
            __cql_type = 13,
            val = "foo",
        })
        assert.are.same(lucas.varchar("foo"), result)
    end)

    it("convert null", function()
        local result = compat.convert({})
        assert.are.same(lucas.null(), result)
    end)

end)