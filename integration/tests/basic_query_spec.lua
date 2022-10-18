require "busted.runner"()
local pretty = require "pl.pretty"
local lucas = require("luacassandra")

describe("luacassandra", function()
    it("can connect", function()
        local err = lucas.connect("127.0.0.1")
        assert.is_nil(err)
    end)

    it("error when cannot connect", function ()
        local ok, err = pcall(function ()
            lucas.connect("example.com")
        end)
        assert.is_false(ok)
    end)

    it("select statement", function ()
        lucas.connect("127.0.0.1")
        local results = lucas.query("SELECT * FROM testing.data", {})
        assert.are.same(results, {
            {
                approval_status = 2,
                asset_id = "015e3714-a98b-11ec-9f51-0242ac150008",
                asset_type = 1,
                id = "9380816255dc45dfa1a57541db81df1d",
                id_type = 1,
                operator_name = "avantel"
            }
        })
    end)
end)
