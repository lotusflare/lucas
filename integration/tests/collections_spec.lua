local pretty = require("pl.pretty")
local lucas = require("lucas")
local helper = require("test_helper")

describe("collections", function()
	it("select map", function()
		helper.connect()
		local results =
			lucas.query("SELECT * FROM testing.collections WHERE id = ? ALLOW FILTERING", { lucas.bigint(2) })
		assert.are.same({
			{
				id = 2,
				int_set = { 50 },
				tinyint_list = { 10, 20 },
				uuid_to_text_map = {
					["0f854625-9bfd-4407-8c28-c4cdb67106f8"] = "foo",
					["e00e3562-9b62-40dc-ac40-0b6834ffc5e5"] = "bar",
				},
			},
		}, results)
	end)

	it("insert named args typed collection", function()
		helper.connect()
		local insert_results =
			lucas.query("INSERT INTO testing.collections (id, int_set) VALUES (:id, :int_set) IF NOT EXISTS", {
				id = lucas.bigint(10),
				int_set = lucas.set({
					lucas.int(1),
					lucas.int(3),
					lucas.int(5),
				}),
			})
		local select_results =
			lucas.query("SELECT * FROM testing.collections WHERE id = ? ALLOW FILTERING", { lucas.bigint(10) })
		assert.are.same({ {
			id = 10,
			int_set = { 1, 3, 5 },
		} }, select_results)
	end)

	it("insert positional args typed collection", function()
		helper.connect()
		lucas.query("INSERT INTO testing.collections (id, uuid_to_text_map, int_set, tinyint_list) VALUES (?, ?, ?, ?)", {
			lucas.bigint(30),
			lucas.map({
				[lucas.uuid("7778132f-64e0-4db3-8d30-3cf40b93c5fc")] = lucas.text("hello world"),
			}),
			lucas.set({ lucas.int(1), lucas.int(2) }),
			lucas.list({ lucas.tinyint(5), lucas.tinyint(8) }),
		})
		local results =
			lucas.query("SELECT * FROM testing.collections WHERE id = ? ALLOW FILTERING", { lucas.bigint(30) })
		assert.are.same({
			{
				id = 30,
				int_set = { 1, 2 },
				tinyint_list = { 5, 8 },
				uuid_to_text_map = {
					["7778132f-64e0-4db3-8d30-3cf40b93c5fc"] = "hello world",
				},
			},
		}, results)
	end)
end)
