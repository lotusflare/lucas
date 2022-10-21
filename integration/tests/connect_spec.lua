require "busted.runner"()
local lucas = require("luacassandra")

describe("connect", function()
    it("no error when server is available", function()
        local err = lucas.connect("127.0.0.1")
    end)

    it("error when server is not available", function ()
        assert.has.errors(function ()
            lucas.connect("example.com")
        end)
    end)
end)
