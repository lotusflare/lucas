require("busted.runner")()
local pretty = require("pl.pretty")
local lucas = require("lucas")
local compat = require("lucas.compatibility")

local test_cases = { {
    name = "implicit string",
    input = "hello",
    expected = lucas.text("hello"),
}, {
    name = "implicit int",
    input = 32,
    expected = lucas.int(32),
}, {
    name = "implicit bool",
    input = true,
    expected = lucas.boolean(true),
}, {
    name = "explicit string",
    input = {
        __cql_type = 10,
        val = "foo",
    },
    expected = lucas.text("foo"),
}, {
    name = "implicit int list",
    input = {
        __cql_type = 32,
        val = { 1, 6 },
    },
    expected = { 32, { { 9, 1 }, { 9, 6 } } },
}, {
    name = "implicit null",
    input = {},
    expected = lucas.null(),
} }

describe("compatibility", function()
    for _, tc in ipairs(test_cases) do
        it(string.format("(%s)", tc.name), function()
            assert.are.same(tc.expected, compat.convert(tc.input))
        end)
    end

    it("(implicit text map)", function()
        local result = compat.convert({ foo = "bar" })
        assert.are.equal(33, result[1])
        for key, val in ipairs(result[2]) do
            assert.are.equal(key[1], 10)
            assert.are.equal(key[2], "foo")
            assert.are.equal(val[1], 10)
            assert.are.equal(val[2], "bar")
        end
    end)
end)