"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.transformPrefixUnaryExpression = exports.transformPostfixUnaryExpression = exports.transformUnaryExpressionStatement = void 0;
const ts = require("typescript");
const lua = require("../../LuaAST");
const utils_1 = require("../../utils");
const bit_1 = require("./binary-expression/bit");
const compound_1 = require("./binary-expression/compound");
function transformUnaryExpressionStatement(context, node) {
    const expression = ts.isExpressionStatement(node) ? node.expression : node;
    if (ts.isPrefixUnaryExpression(expression) &&
        (expression.operator === ts.SyntaxKind.PlusPlusToken || expression.operator === ts.SyntaxKind.MinusMinusToken)) {
        // ++i, --i
        const replacementOperator = expression.operator === ts.SyntaxKind.PlusPlusToken ? ts.SyntaxKind.PlusToken : ts.SyntaxKind.MinusToken;
        return (0, compound_1.transformCompoundAssignmentStatement)(context, expression, expression.operand, ts.factory.createNumericLiteral(1), replacementOperator);
    }
    else if (ts.isPostfixUnaryExpression(expression)) {
        // i++, i--
        const replacementOperator = expression.operator === ts.SyntaxKind.PlusPlusToken ? ts.SyntaxKind.PlusToken : ts.SyntaxKind.MinusToken;
        return (0, compound_1.transformCompoundAssignmentStatement)(context, expression, expression.operand, ts.factory.createNumericLiteral(1), replacementOperator);
    }
}
exports.transformUnaryExpressionStatement = transformUnaryExpressionStatement;
const transformPostfixUnaryExpression = (expression, context) => {
    switch (expression.operator) {
        case ts.SyntaxKind.PlusPlusToken:
            return (0, compound_1.transformCompoundAssignmentExpression)(context, expression, expression.operand, ts.factory.createNumericLiteral(1), ts.SyntaxKind.PlusToken, true);
        case ts.SyntaxKind.MinusMinusToken:
            return (0, compound_1.transformCompoundAssignmentExpression)(context, expression, expression.operand, ts.factory.createNumericLiteral(1), ts.SyntaxKind.MinusToken, true);
        default:
            (0, utils_1.assertNever)(expression.operator);
    }
};
exports.transformPostfixUnaryExpression = transformPostfixUnaryExpression;
const transformPrefixUnaryExpression = (expression, context) => {
    switch (expression.operator) {
        case ts.SyntaxKind.PlusPlusToken:
            return (0, compound_1.transformCompoundAssignmentExpression)(context, expression, expression.operand, ts.factory.createNumericLiteral(1), ts.SyntaxKind.PlusToken, false);
        case ts.SyntaxKind.MinusMinusToken:
            return (0, compound_1.transformCompoundAssignmentExpression)(context, expression, expression.operand, ts.factory.createNumericLiteral(1), ts.SyntaxKind.MinusToken, false);
        case ts.SyntaxKind.PlusToken:
            // TODO: Wrap with `Number`
            return context.transformExpression(expression.operand);
        case ts.SyntaxKind.MinusToken:
            return lua.createUnaryExpression(context.transformExpression(expression.operand), lua.SyntaxKind.NegationOperator);
        case ts.SyntaxKind.ExclamationToken:
            return lua.createUnaryExpression(context.transformExpression(expression.operand), lua.SyntaxKind.NotOperator);
        case ts.SyntaxKind.TildeToken:
            return (0, bit_1.transformUnaryBitOperation)(context, expression, context.transformExpression(expression.operand), lua.SyntaxKind.BitwiseNotOperator);
        default:
            (0, utils_1.assertNever)(expression.operator);
    }
};
exports.transformPrefixUnaryExpression = transformPrefixUnaryExpression;
//# sourceMappingURL=unary-expression.js.map