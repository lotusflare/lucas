require("busted.runner")()
local pretty = require("pl.pretty")
local lucas = require("lucas")

describe("query", function()
    it("basic select", function()
        lucas.connect("cassandra")
        local results = lucas.query("SELECT * FROM testing.data", {})
        assert.array.has(
            { {
                approval_status = 2,
                asset_id = "015e3714-a98b-11ec-9f51-0242ac150008",
                asset_type = 1,
                id = "9380816255dc45dfa1a57541db81df1d",
                id_type = 1,
                operator_name = "avantel",
            } },
            results
        )
    end)

    it("select with positional parameters", function()
        lucas.connect(os.getenv("CASSANDRA_HOST"))
        local results =
            lucas.query(
                "SELECT * FROM testing.data WHERE asset_type = ? ALLOW FILTERING",
                { lucas.int(1) }
            )
        assert.are.same(
            { {
                approval_status = 2,
                asset_id = "015e3714-a98b-11ec-9f51-0242ac150008",
                asset_type = 1,
                id = "9380816255dc45dfa1a57541db81df1d",
                id_type = 1,
                operator_name = "avantel",
            } },
            results
        )
    end)

    it("select with named parameters", function()
        lucas.connect(os.getenv("CASSANDRA_HOST"))
        local results =
            lucas.query(
                "SELECT * FROM testing.data WHERE asset_type = :asset_id ALLOW FILTERING",
                { asset_id = lucas.int(1) }
            )
        assert.are.same(
            { {
                approval_status = 2,
                asset_id = "015e3714-a98b-11ec-9f51-0242ac150008",
                asset_type = 1,
                id = "9380816255dc45dfa1a57541db81df1d",
                id_type = 1,
                operator_name = "avantel",
            } },
            results
        )
    end)

    it("select map", function()
    lucas.connect(os.getenv("CASSANDRA_HOST"))
        local results =
            lucas.query(
                "SELECT misc FROM testing.data WHERE asset_type = :asset_id ALLOW FILTERING",
                { asset_id = lucas.int(1) }
            )
        assert.are.same(
            { {
                misc = {
                    car = 'Honda',
                    fruit = 'apple'
                }
            } },
            results
        )
    end)

    it("select list", function()
        local results =
            lucas.query(
                "SELECT list FROM testing.data WHERE asset_type = :asset_id ALLOW FILTERING",
                { asset_id = lucas.int(1) }
            )
        assert.are.same(
            { {
                list = {
                    1,2,3
                }
            } },
            results
        )
    end)
end)
