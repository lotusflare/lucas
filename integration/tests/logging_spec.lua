local lucas = require("lucas")
local spy = require("luassert.spy")
local pretty = require("pl.pretty")
local helper = require("test_helper")

assert:register("matcher", "contains", function(state, args)
	local needle = args[1]
	return function(haystack)
		if string.find(haystack, needle, 0, true) then
			return true
		else
			return false
		end
	end
end)

describe("logging", function()
	it("callback hook works", function()
		local s = spy.new(function() end)
		lucas.logger(function(message, source, severity, timestamp)
			s(message, source, severity, timestamp)
		end)
		helper.connect()
		helper.connect()
		assert
			.spy(s).was
			.called_with(match.contains("already connected"), "lucas", match.is_number(), match.is_number())
	end)
end)

describe("metrics", function()
	it("collects general metrics", function()
		helper.connect()
		local metrics = lucas.metrics()
		assert.array.has({ "errors", "requests", "stats" }, metrics)
	end)

	it("collects speculative execution metrics", function()
		helper.connect()
		local metrics = lucas.speculative_execution_metrics()
		assert.array.has({
			"count",
			"max",
			"mean",
			"median",
			"min",
			"percentage",
			"percentile_75th",
			"percentile_95th",
			"percentile_98th",
			"percentile_99th",
			"percentile_999th",
		}, metrics)
	end)
end)
