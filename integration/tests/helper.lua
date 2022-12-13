local lucas = require("lucas")

function convert(arg)
    local t = type(arg)

    if t == "number" then
        return lucas.int(arg)
    elseif t == "boolean" then
        return lucas.boolean(arg)
    elseif t == "string" then
        return lucas.text(arg)
    elseif t == "table" then
        return handle_table(arg)
    else
        print("error")
    end
end

function handle_table(arg)

end