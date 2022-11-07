require("busted.runner")()
local pretty = require("pl.pretty")
local lucas = require("lucas")

describe("query", function()
    it("collections select", function()
        lucas.connect(os.getenv("CASSANDRA_HOST"))
        local results = lucas.query("SELECT * FROM testing.collections", {})
        assert.are.same(
            { {
                id = 2,
                tinyint_list = { 99, 102 },
                uuid_to_text_map = { ["0f854625-9bfd-4407-8c28-c4cdb67106f8"] = "foo" }
            } },
            results
        )
    end)
end)
