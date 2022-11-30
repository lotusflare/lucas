require("busted.runner")()
local pretty = require("pl.pretty")
local lucas = require("lucas")

describe("collections", function()
    it("select map", function()
        lucas.connect({
            contact_points = os.getenv("CASSANDRA_HOST"),
            port = os.getenv("CASSANDRA_PORT"),
        })
        local results = lucas.query("SELECT * FROM testing.collections", {})
        assert.are.same(
            { {
                id = 2,
                int_set = { 50 },
                tinyint_list = { 10, 20 },
                uuid_to_text_map = {
                    ["0f854625-9bfd-4407-8c28-c4cdb67106f8"] = "foo",
                    ["e00e3562-9b62-40dc-ac40-0b6834ffc5e5"] = "bar",
                },
            } },
            results
        )
    end)

    it("insert implicitly typed collection", function()
        lucas.connect({
            contact_points = os.getenv("CASSANDRA_HOST"),
            port = os.getenv("CASSANDRA_PORT"),
        })
        lucas.query(
            "INSERT INTO testing.collections (id, int_set) VALUES (?, ?)",
            { lucas.bigint(10), lucas.set({ 1, 2, 3, 4, 5 }) }
        )
    end)
end)