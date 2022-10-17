require "busted.runner"()
local lua_cassandra = require("luacassandra")

describe("basic query", function()
    it("runs", function()
        assert.is_true(1 == 1)
    end)

    it("lua_cassandra", function()
        lua_cassandra.connect("127.0.0.1")
        -- lua_cassandra.query("", {})
    end)
end)
