local pretty = require("pl.pretty")
local lucas = require("lucas")

describe("query", function()
    it("basic select", function()
        lucas.connect({
            contact_points = os.getenv("CASSANDRA_HOST"),
            port = os.getenv("CASSANDRA_PORT"),
        })
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
        lucas.connect({
            contact_points = os.getenv("CASSANDRA_HOST"),
            port = os.getenv("CASSANDRA_PORT"),
        })
        local results =
            lucas.query(
                "SELECT * FROM testing.data WHERE asset_type = ? ALLOW FILTERING",
                { lucas.int(1) }
            )
        assert.array.has(
            { {
                approval_status = 2,
                asset_id = "015e3714-a98b-11ec-9f51-0242ac150008",
                asset_type = 1,
                id = "9380816255dc45dfa1a57541db81df1d",
                id_type = 1,
                operator_name = "avantel",
                list = { 1, 2, 3 },
                misc = {
                    car = "Honda",
                    fruit = "apple",
                },
            } },
            results
        )
    end)

    it("select with named parameters", function()
        lucas.connect({
            contact_points = os.getenv("CASSANDRA_HOST"),
            port = os.getenv("CASSANDRA_PORT"),
        })
        local results =
            lucas.query(
                "SELECT * FROM testing.data WHERE asset_type = :asset_id ALLOW FILTERING",
                { asset_id = lucas.int(1) }
            )
        assert.array.has(
            { {
                approval_status = 2,
                asset_id = "015e3714-a98b-11ec-9f51-0242ac150008",
                asset_type = 1,
                id = "9380816255dc45dfa1a57541db81df1d",
                id_type = 1,
                operator_name = "avantel",
                list = { 1, 2, 3 },
                misc = {
                    car = "Honda",
                    fruit = "apple",
                },
            } },
            results
        )
    end)
end)