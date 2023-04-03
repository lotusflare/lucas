local lucas = require("lucas")

local function testQueryLucasFromLua()
    lucas.connect({
        contact_points = 'cassandra2',
        port = 9042,
        credentials = {
            username = 'cassandra',
            password = 'cassandra',
        },
    })

    local results = lucas.query('SELECT * FROM testing.data', {})
    for index, data in ipairs(results) do
        print(index)
    
        for key, value in pairs(data) do
            print('\t', key, value)
        end
    end

    results = lucas.query('SELECT * FROM testing.data WHERE id_type = ? AND operator_name = ? ALLOW FILTERING', { lucas.int(1), lucas.varchar('avantel') })
    for index, data in ipairs(results) do
        print(index)
    
        for key, value in pairs(data) do
            print('\t', key, value)
        end
    end
end

testQueryLucasFromLua()