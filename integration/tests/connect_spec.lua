local lucas = require("lucas")

describe("connect", function()
    it("no error when server is available", function()
        lucas.connect({
            contact_points = os.getenv("CASSANDRA_HOST"),
            port = os.getenv("CASSANDRA_PORT"),
        })
    end)

    -- it("error when server is not available", function()
    --     assert.has.errors(function()
    --         local err = lucas.connect({
    --             contact_points = "example.com",
    --             reconnect = true,
    --         })
    --     end)
    -- end)

    it("reconnect flag works", function()
        local err = lucas.connect({
            contact_points = os.getenv("CASSANDRA_HOST"),
            port = os.getenv("CASSANDRA_PORT"),
            reconnect = true,
        })
        local err = lucas.connect({
            contact_points = "example.com",
            reconnect = false,
        })
    end)
end)