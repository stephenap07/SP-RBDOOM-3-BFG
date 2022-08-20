"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.createDecoratingExpression = exports.transformDecoratorExpression = void 0;
const ts = require("typescript");
const lua = require("../../../LuaAST");
const diagnostics_1 = require("../../utils/diagnostics");
const function_context_1 = require("../../utils/function-context");
const lualib_1 = require("../../utils/lualib");
function transformDecoratorExpression(context, decorator) {
    const expression = decorator.expression;
    const type = context.checker.getTypeAtLocation(expression);
    const callContext = (0, function_context_1.getFunctionContextType)(context, type);
    if (callContext === function_context_1.ContextType.Void) {
        context.diagnostics.push((0, diagnostics_1.decoratorInvalidContext)(decorator));
    }
    return context.transformExpression(expression);
}
exports.transformDecoratorExpression = transformDecoratorExpression;
function createDecoratingExpression(context, kind, decorators, targetTableName, targetFieldExpression) {
    const decoratorTable = lua.createTableExpression(decorators.map(e => lua.createTableFieldExpression(e)));
    const trailingExpressions = [decoratorTable, targetTableName];
    if (targetFieldExpression) {
        trailingExpressions.push(targetFieldExpression);
        const isMethodOrAccessor = kind === ts.SyntaxKind.MethodDeclaration ||
            kind === ts.SyntaxKind.GetAccessor ||
            kind === ts.SyntaxKind.SetAccessor;
        trailingExpressions.push(isMethodOrAccessor ? lua.createBooleanLiteral(true) : lua.createNilLiteral());
    }
    return (0, lualib_1.transformLuaLibFunction)(context, lualib_1.LuaLibFeature.Decorate, undefined, ...trailingExpressions);
}
exports.createDecoratingExpression = createDecoratingExpression;
//# sourceMappingURL=decorators.js.map