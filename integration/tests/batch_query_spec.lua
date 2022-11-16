require("busted.runner")()
local lucas = require("lucas")

describe("batch", function()
    it("insert", function()
        local err =
            lucas.connect(os.getenv("CASSANDRA_HOST"), {
                port = os.getenv("CASSANDRA_PORT"),
            })

        lucas.batch(
            "INSERT INTO testing.data (operator_name, id, id_type, asset_id, asset_type, approval_status) VALUES (?, ?, ?, ?, ?, ?)",
            {
                {
                    lucas.varchar("avantel"),
                    lucas.varchar("9380816255dc45dfa1a57541db81df1d"),
                    lucas.int(1),
                    lucas.timeuuid("015e3714-a98b-11ec-9f51-0242ac150008"),
                    lucas.int(2),
                    lucas.int(1),
                },
                {
                    lucas.varchar("tmo"),
                    lucas.varchar("9380816255dc45dfa1a57541db81df1d"),
                    lucas.int(1),
                    lucas.timeuuid("015e3714-a98b-11ec-9f51-0242ac150008"),
                    lucas.int(2),
                    lucas.int(1),
                },
            }
        )

        local results =
            lucas.query(
                "SELECT * FROM testing.data WHERE operator_name = 'tmo' ALLOW FILTERING",
                {}
            )

        assert.array.has(
            { {
                approval_status = 1,
                asset_id = "015e3714-a98b-11ec-9f51-0242ac150008",
                asset_type = 2,
                created_at = 35184372088832,
                id = "9380816255dc45dfa1a57541db81df1d",
                id_type = 1,
                operator_name = "tmo",
            } },
            results
        )
    end)
end)