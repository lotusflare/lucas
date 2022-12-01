require("busted.runner")()
local pretty = require("pl.pretty")
local lucas = require("lucas")

describe("collections", function()
    it("select map", function()
        lucas.connect({
            contact_points = os.getenv("CASSANDRA_HOST"),
            port = os.getenv("CASSANDRA_PORT"),
        })
        local results = lucas.query("SELECT * FROM testing.collections WHERE id = ? ALLOW FILTERING", {
            lucas.bigint(2),
        })
        assert.are.same(
            { {
                id = 2,
                int_set = { 50 },
                tinyint_list = { 10, 20 },
                uuid_to_text_map = {
                    ["0f854625-9bfd-4407-8c28-c4cdb67106f8"] = "foo",
                    ["e00e3562-9b62-40dc-ac40-0b6834ffc5e5"] = "bar",
                },
            } },
            results
        )
    end)

    it("insert implicitly typed collection", function()
        lucas.connect({
            contact_points = os.getenv("CASSANDRA_HOST"),
            port = os.getenv("CASSANDRA_PORT"),
        })
        lucas.query(
            "INSERT INTO testing.collections (id, int_set) VALUES (?, ?)",
            { lucas.bigint(10), lucas.set({ 1, 2, 3, 4, 5 }) }
        )
    end)

    it("insert explicitly typed collection", function()
        lucas.connect({
            contact_points = os.getenv("CASSANDRA_HOST"),
            port = os.getenv("CASSANDRA_PORT"),
        })
        lucas.query(
            "INSERT INTO testing.collections (id, uuid_to_text_map, int_set, tinyint_list) VALUES (?, ?, ?, ?)",
            {
                lucas.bigint(30),
                lucas.map({
                    [lucas.uuid(
                        "7778132f-64e0-4db3-8d30-3cf40b93c5fc"
                    )] = lucas.text("hello world"),
                }),
                lucas.set({lucas.int(1), lucas.int(2)}),
                lucas.list({lucas.tinyint(5), lucas.tinyint(8)}),
            }
        )
        local results = lucas.query("SELECT * FROM testing.collections WHERE id = ? ALLOW FILTERING", { lucas.bigint(30) })
        assert.are.same({ {
            id = 30,
            int_set = { 1, 2 },
            tinyint_list = { 5, 8 },
            uuid_to_text_map = {
                ["7778132f-64e0-4db3-8d30-3cf40b93c5fc"] = "hello world",
            },
        } }, results)
    end)
end)