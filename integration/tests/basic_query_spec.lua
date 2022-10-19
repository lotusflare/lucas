require "busted.runner"()
local pretty = require "pl.pretty"
local lucas = require("luacassandra")

describe("luacassandra", function()
    it("can connect", function()
        local err = lucas.connect("127.0.0.1")
        assert.is_nil(err)
    end)

    it("error when cannot connect", function ()
        assert.has.errors(function ()
            lucas.connect("example.com")
        end)
    end)

    it("select statement", function ()
        lucas.connect("127.0.0.1")
        local results = lucas.query("SELECT * FROM testing.data", {})
        assert.are.same({
            {
                approval_status = 2,
                asset_id = "015e3714-a98b-11ec-9f51-0242ac150008",
                asset_type = 1,
                id = "9380816255dc45dfa1a57541db81df1d",
                id_type = 1,
                operator_name = "avantel"
            }
        }, results)
    end)

    it("binding positional parameters", function ()
        lucas.connect("127.0.0.1")
        local results = lucas.query("SELECT * FROM testing.data WHERE asset_type = ? ALLOW FILTERING", {
            {9, 1}
        })
        assert.are.same({
            {
                approval_status = 2,
                asset_id = "015e3714-a98b-11ec-9f51-0242ac150008",
                asset_type = 1,
                id = "9380816255dc45dfa1a57541db81df1d",
                id_type = 1,
                operator_name = "avantel"
            }
        }, results)
    end)

    -- it("binding named parameters", function ()
    --     lucas.connect("127.0.0.1")
    --     local results = lucas.query("SELECT * FROM testing.data WHERE asset_type = :asset_id ALLOW FILTERING", {
    --         asset_id = {"CASS_VALUE_TYPE_INT", 1}
    --     })
    --     assert.are.same({
    --         {
    --             approval_status = 2,
    --             asset_id = "015e3714-a98b-11ec-9f51-0242ac150008",
    --             asset_type = 1,
    --             id = "9380816255dc45dfa1a57541db81df1d",
    --             id_type = 1,
    --             operator_name = "avantel"
    --         }
    --     }, results)
    -- end)
end)
