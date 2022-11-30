require("busted.runner")()
local pretty = require("pl.pretty")
local lucas = require("lucas")
local compat = require("lucas.compatibility")

local test_cases = { {
    name = "implicit string",
    input = "hello",
    expected = lucas.varchar("hello"),
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
        __cql_type = 13,
        val = "foo",
    },
    expected = lucas.varchar("foo"),
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
end)