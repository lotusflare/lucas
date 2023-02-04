local lucas = require("lucas")

describe("connect", function()
	it("no error when server is available", function()
		lucas.connect({
			contact_points = os.getenv("CASSANDRA_HOST"),
			port = os.getenv("CASSANDRA_PORT"),
			reconnect = true,
		})
	end)

	it("error when server is not available", function()
		assert.has.errors(function()
			lucas.connect({
				contact_points = "example.com",
				reconnect = true,
				connect_timeout = 100,
			})
		end)
	end)

	it("reconnect flag works", function()
		lucas.connect({
			contact_points = os.getenv("CASSANDRA_HOST"),
			port = os.getenv("CASSANDRA_PORT"),
			reconnect = true,
		})
		lucas.connect({
			contact_points = "example.com",
			reconnect = false,
		})
	end)

	-- it("connect with SSL", function()
	-- 	lucas.connect({
	-- 		contact_points = os.getenv("CASSANDRA_HOST"),
	-- 		port = os.getenv("CASSANDRA_PORT_SSL"),
	-- 		reconnect = true,
	-- 		ssl = {
	-- 			certificate = os.getenv("CASSANDRA_SSL_CERT"),
	-- 			private_key = os.getenv("CASSANDRA_SSL_KEY"),
	-- 		},
	-- 	})
	-- end)
end)
