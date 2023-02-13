local lucas = require("lucas")
local pretty = require("pl.pretty")

local helper = {}

function helper.connect()
	lucas.connect({
		contact_points = os.getenv("CASSANDRA_HOST"),
		port = os.getenv("CASSANDRA_PORT"),
		credentials = {
			username = os.getenv("CASSANDRA_USER"),
			password = os.getenv("CASSANDRA_PASS"),
		},
	})
end

return helper
