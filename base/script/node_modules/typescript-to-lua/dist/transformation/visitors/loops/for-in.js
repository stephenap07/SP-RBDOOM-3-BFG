"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.transformForInStatement = void 0;
const lua = require("../../../LuaAST");
const diagnostics_1 = require("../../utils/diagnostics");
const typescript_1 = require("../../utils/typescript");
const utils_1 = require("./utils");
const transformForInStatement = (statement, context) => {
    if ((0, typescript_1.isArrayType)(context, context.checker.getTypeAtLocation(statement.expression))) {
        context.diagnostics.push((0, diagnostics_1.forbiddenForIn)(statement));
    }
    // Transpile expression
    const pairsIdentifier = lua.createIdentifier("pairs");
    const expression = context.transformExpression(statement.expression);
    const pairsCall = lua.createCallExpression(pairsIdentifier, [expression]);
    const body = lua.createBlock((0, utils_1.transformLoopBody)(context, statement));
    const valueVariable = (0, utils_1.transformForInitializer)(context, statement.initializer, body);
    return lua.createForInStatement(body, [valueVariable], [pairsCall], statement);
};
exports.transformForInStatement = transformForInStatement;
//# sourceMappingURL=for-in.js.map