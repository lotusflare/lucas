require "busted.runner"()
local pretty = require "pl.pretty"
local lucas = require("luacassandra")

describe("luacassandra", function()
    it("can connect", function()
        local err = lucas.connect("127.0.0.1")
        assert.is_nil(err)
    end)

    it("error when cannot connect", function ()
        local err = lucas.connect("example.com")
        assert.has.errors(function() error("could not connect") end)
    end)
end)
