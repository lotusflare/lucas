require("busted.runner")()
local pretty = require("pl.pretty")
local lucas = require("lucas")
local compat = require("lucas.compatibility")

local function convert_type(value)
    local t = type(value)
    if (t == "string") then
        return lucas.varchar(value)
    elseif (t == "boolean") then
        return lucas.boolean(value)
    elseif (t == "number") then
        return lucas.int(value)
    elseif (t == "table") then
        if (value.__cql_type == nil) then
            return lucas.null()
        end
        return { value.__cql_type, value.val }
    end
end

describe("compatibility", function()
    it("convert string", function()
        local result = compat.convert("hello")
        assert.are.same(convert_type("hello"), result)
    end)

    it("convert int", function()
        local result = compat.convert(32)
        assert.are.same(lucas.int(32), result)
    end)

    it("convert bool", function()
        local result = compat.convert(true)
        assert.are.same(convert_type(true), result)
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