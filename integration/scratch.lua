local pretty = require "pl.pretty"
local lucas = require("luacassandra")



-- local results = lucas.query("SELECT * FROM testing.data WHERE asset_type = ? ALLOW FILTERING", {
--     {lucas.type_int(), 1},
-- })

lucas.connect("127.0.0.1")
local results = lucas.query("SELECT * FROM testing.data WHERE asset_type = ?", {
    {lucas.int(), 1}
})
pretty.dump(results)



-- print('hello')


-- local results2 = lucas.query("SELECT operator_name FROM testing.data ", {

-- })
-- pretty.dump(results2)

-- local results2 = lucas.query("SELECT * FROM testing.data WHERE asset_type = :asset_type ALLOW FILTERING", {
--     asset_type = {lucas.type_int(), 1},
-- })
-- pretty.dump(results2)

-- lucas.named_args("", {
--     {5, "foo"},
--     somekey={9, "someval"}
-- })

-- assert.are.same({
--     {
--         approval_status = 2,
--         asset_id = "015e3714-a98b-11ec-9f51-0242ac150008",
--         asset_type = 1,
--         id = "9380816255dc45dfa1a57541db81df1d",
--         id_type = 1,
--         operator_name = "avantel"
--     }
-- }, results)
