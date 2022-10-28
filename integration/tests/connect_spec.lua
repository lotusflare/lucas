require"busted.runner"()
local lucas = require("lucas")

describe("connect", function()
    it("no error when server is available", function()
        local err = lucas.connect(os.getenv("CASSANDRA_HOST"))
    end)

    it("error when server is not available", function()
        assert.has.errors(function()
            lucas.connect("example.com")
        end)
    end)
end)