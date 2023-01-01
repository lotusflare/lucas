local pretty = require("pl.pretty")
local tablex = require("pl.tablex")
local lucas = require("lucas")
local compat = require("lucas.compatibility")
local cassandra = require("cassandra")
local os = require("os")

describe("scalars", function()
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
        input = cassandra.text("foo"),
        expected = lucas.text("foo"),
    }, {
        name = "implicit null",
        input = {},
        expected = lucas.null(),
    } }

    for _, tc in ipairs(test_cases) do
        it(string.format("(%s)", tc.name), function()
            local converted = compat.convert(tc.input)
            assert.are.same(tc.expected, converted)
        end)
    end
end)

describe("collections", function()
    local test_cases = { {
        name = "set<text>",
        input = cassandra.set({ "Gomo_UnliData_30D_freeSIM" }),
        expected = lucas.set({ lucas.text("Gomo_UnliData_30D_freeSIM") }),
    }, {
        name = "untyped explicit list<int>",
        input = cassandra.list({ 100, 6 }),
        expected = lucas.list({ lucas.int(100), lucas.int(6) }),
    } }

    for _, tc in ipairs(test_cases) do
        it(string.format("(%s)", tc.name), function()
            local converted = compat.convert(tc.input)
            assert.are.same(tc.expected, converted)
        end)
    end
end)

describe("maps", function()
    local test_cases = { {
        name = "implicit map<text, boolean>",
        input = cassandra.map({ foo = true }),
        expected = lucas.map({ [lucas.text("foo")] = lucas.boolean(true) }),
    }, {
        name = "implicit map<text, boolean>",
        input = { foo = true },
        expected = lucas.map({ [lucas.text("foo")] = lucas.boolean(true) }),
    }, {
        name = "map<timeuuid, int>",
        input = cassandra.map({
            [cassandra.timestamp(os.time())] = cassandra.int(5),
        }),
        expected = lucas.map({ [lucas.timestamp(os.time())] = lucas.int(5) }),
    }, {
        name = "implicit map<text, text>",
        input = cassandra.map({
            birthday = "10/1/1979",
            first_name = "first",
            gender = "M",
            last_name = "last",
            usc = "123456789",
        }),
        expected = lucas.map({
            [lucas.text("birthday")] = lucas.text("10/1/1979"),
            [lucas.text("first_name")] = lucas.text("first"),
            [lucas.text("gender")] = lucas.text("M"),
            [lucas.text("last_name")] = lucas.text("last"),
            [lucas.text("usc")] = lucas.text("123456789"),
        }),
    } }

    for _, tc in ipairs(test_cases) do
        it(string.format("(%s)", tc.name), function()
            -- TODO: find a way to compare tables with tables as keys
        end)
    end
end)