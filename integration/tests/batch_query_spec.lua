require"busted.runner"()
local lucas = require("lucas")

describe("batch", function()
    it("insert", function()
        local err = lucas.connect("127.0.0.1")

        lucas.batch(
            "INSERT INTO testing.data (operator_name, id, id_type, asset_id, asset_type, approval_status) VALUES (?, ?)",
            { {
                approval_status = { lucas.int(), 2 },
                asset_id = { lucas.varchar(), "015e3714-a98b-11ec-9f51-0242ac150008" },
                asset_type = { lucas.int(), 1 },
                id = { lucas.varchar(), "9380816255dc45dfa1a57541db81df1d" },
                id_type = { lucas.int(), 1 },
                operator_name = { lucas.varchar(), "avantel" },
            }, {
                approval_status = { lucas.int(), 2 },
                asset_id = { lucas.varchar(), "015e3714-a98b-11ec-9f51-0242ac150008" },
                asset_type = { lucas.int(), 1 },
                id = { lucas.varchar(), "9380816255dc45dfa1a57541db81df1d" },
                id_type = { lucas.int(), 1 },
                operator_name = { lucas.varchar(), "tmo" },
            } }
        )

        local results =
            lucas.query("SELECT * FROM testing.data WHERE operator_name = 'tmo' ALLOW FILTERING")

        assert.are.same(
            { {
                approval_status = 2,
                asset_id = "015e3714-a98b-11ec-9f51-0242ac150008",
                asset_type = 1,
                id = "9380816255dc45dfa1a57541db81df1d",
                id_type = 1,
                operator_name = "tmo",
            } },
            results
        )
    end)
end)