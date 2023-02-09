local lucas = require("lucas")

describe("connect", function()
	it("no error when server is available", function()
		lucas.connect({
			contact_points = os.getenv("CASSANDRA_HOST"),
			port = os.getenv("CASSANDRA_PORT"),
			reconnect = true,
			credentials = {
				username = os.getenv("CASSANDRA_USERNAME"),
				password = os.getenv("CASSANDRA_PASSWORD"),
			},
		})
	end)

	it("error when server is not available", function()
		assert.has.errors(function()
			lucas.connect({
				contact_points = "example.com",
				reconnect = true,
				connect_timeout = 100,
				credentials = {
					username = os.getenv("CASSANDRA_USERNAME"),
					password = os.getenv("CASSANDRA_PASSWORD"),
				},
			})
		end)
	end)

	it("reconnect flag works", function()
		lucas.connect({
			contact_points = os.getenv("CASSANDRA_HOST"),
			port = os.getenv("CASSANDRA_PORT"),
			reconnect = true,
			credentials = {
				username = os.getenv("CASSANDRA_USERNAME"),
				password = os.getenv("CASSANDRA_PASSWORD"),
			},
		})
		lucas.connect({
			contact_points = "example.com",
			reconnect = false,
		})
	end)

	it("connect with SSL", function()
		lucas.connect({
			contact_points = os.getenv("CASSANDRA_HOST"),
			port = os.getenv("CASSANDRA_PORT_SSL"),
			reconnect = true,
			credentials = {
				username = os.getenv("CASSANDRA_USERNAME"),
				password = os.getenv("CASSANDRA_PASSWORD"),
			},
			ssl = {
				certificate = os.getenv("CASSANDRA_SSL_CERT"),
				private_key = os.getenv("CASSANDRA_SSL_KEY"),
				password = "cassandra",
			},
		})
	end)
end)
