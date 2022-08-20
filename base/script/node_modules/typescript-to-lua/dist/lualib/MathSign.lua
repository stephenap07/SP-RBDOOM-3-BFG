local function __TS__MathSign(val)
    if val > 0 then
        return 1
    elseif val < 0 then
        return -1
    end
    return 0
end
