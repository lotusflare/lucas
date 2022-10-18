require "busted.runner"()
local pretty = require "pl.pretty"
local lucas = require("luacassandra")

describe("luacassandra", function()
    it("can connect", function()
        local err = lucas.connect("127.0.0.1")
        assert.is_nil(err)
    end)

    -- it("error when cannot connect", function ()
    --     local ok, err = pcall(function ()
    --         lucas.connect("example.com")
    --     end)
    --     assert.is_false(ok)
    -- end)

    it("select statement", function ()
        lucas.connect("127.0.0.1")
        local results = lucas.execute_query("SELECT * FROM testing.data", {})
        pretty.dump(results)
    end)
end)
