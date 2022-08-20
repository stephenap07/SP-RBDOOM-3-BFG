"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.transformDeleteExpression = void 0;
const ts = require("typescript");
const lua = require("../../LuaAST");
const lualib_1 = require("../utils/lualib");
const diagnostics_1 = require("../utils/diagnostics");
const typescript_1 = require("../utils/typescript");
const lua_ast_1 = require("../utils/lua-ast");
const optional_chaining_1 = require("./optional-chaining");
const transformDeleteExpression = (node, context) => {
    if (ts.isOptionalChain(node.expression)) {
        return (0, optional_chaining_1.transformOptionalDeleteExpression)(context, node, node.expression);
    }
    let ownerExpression;
    let propertyExpression;
    if (ts.isPropertyAccessExpression(node.expression)) {
        if (ts.isPrivateIdentifier(node.expression.name))
            throw new Error("PrivateIdentifier is not supported");
        ownerExpression = context.transformExpression(node.expression.expression);
        propertyExpression = lua.createStringLiteral(node.expression.name.text);
    }
    else if (ts.isElementAccessExpression(node.expression)) {
        ownerExpression = context.transformExpression(node.expression.expression);
        propertyExpression = context.transformExpression(node.expression.argumentExpression);
        const type = context.checker.getTypeAtLocation(node.expression.expression);
        const argumentType = context.checker.getTypeAtLocation(node.expression.argumentExpression);
        if ((0, typescript_1.isArrayType)(context, type) && (0, typescript_1.isNumberType)(context, argumentType)) {
            propertyExpression = (0, lua_ast_1.addToNumericExpression)(propertyExpression, 1);
        }
    }
    if (!ownerExpression || !propertyExpression) {
        context.diagnostics.push((0, diagnostics_1.unsupportedProperty)(node, "delete", ts.SyntaxKind[node.kind]));
        return lua.createNilLiteral();
    }
    return (0, lualib_1.transformLuaLibFunction)(context, lualib_1.LuaLibFeature.Delete, node, ownerExpression, propertyExpression);
};
exports.transformDeleteExpression = transformDeleteExpression;
//# sourceMappingURL=delete.js.map