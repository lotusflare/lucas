local pretty = require"pl.pretty"
local tablex = require"pl.tablex"
local lucas = require"lucas"
local compat = require"lucas.compatibility"

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
    name = "implicit list<int>",
    input = {
        __cql_type = 32,
        val = { 100, 6 },
    },
    expected = { 32, { lucas.int(100), lucas.int(6) } },
}, {
    name = "implicit null",
    input = {},
    expected = lucas.null(),
}, {
    name = "implicit map<text, text>",
    input = { foo = "bar" },
    expected = { 33, { { lucas.text("foo"), lucas.text("bar") } } },
}, {
    name = "explicit map<text, boolean>",
    input = {
        __cql_type = 32,
        val = { foo = true },
    },
    expected = { 33, { { lucas.text("foo"), lucas.boolean(true) } } },
}, {
    name = "implicit map<text, boolean>",
    input = { foo = true },
    expected = { 33, { { lucas.text("foo"), lucas.boolean(true) } } },
}, {
    name = "set<text>",
    input = {
        __cql_type = 34,
        val = { "Gomo_UnliData_30D_freeSIM" },
    },
    expected = lucas.set(
        lucas.text("Gomo_UnliData_30D_freeSIM"),
        lucas.text("sdfsd")
    ),
} }

describe("compatibility", function()
    for _, tc in ipairs(test_cases) do
        it(string.format("(%s)", tc.name), function()
            local converted = compat.convert(tc.input)
            -- pretty.dump(converted)
            -- pretty.dump(tc.expected)
            assert.are.same(tc.expected, converted)
        end)
    end
end)