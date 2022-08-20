"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.transformEnumDeclaration = exports.tryGetConstEnumValue = void 0;
const ts = require("typescript");
const lua = require("../../LuaAST");
const annotations_1 = require("../utils/annotations");
const export_1 = require("../utils/export");
const lua_ast_1 = require("../utils/lua-ast");
const typescript_1 = require("../utils/typescript");
const identifier_1 = require("./identifier");
const literal_1 = require("./literal");
function tryGetConstEnumValue(context, node) {
    const value = context.checker.getConstantValue(node);
    if (typeof value === "string") {
        return lua.createStringLiteral(value, node);
    }
    else if (typeof value === "number") {
        return lua.createNumericLiteral(value, node);
    }
}
exports.tryGetConstEnumValue = tryGetConstEnumValue;
const transformEnumDeclaration = (node, context) => {
    if (ts.getCombinedModifierFlags(node) & ts.ModifierFlags.Const && !context.options.preserveConstEnums) {
        return undefined;
    }
    const type = context.checker.getTypeAtLocation(node);
    const membersOnly = (0, annotations_1.getTypeAnnotations)(type).has(annotations_1.AnnotationKind.CompileMembersOnly);
    const result = [];
    if (!membersOnly && (0, typescript_1.isFirstDeclaration)(context, node)) {
        const name = (0, identifier_1.transformIdentifier)(context, node.name);
        const table = lua.createBinaryExpression(lua.cloneIdentifier(name), lua.createTableExpression(), lua.SyntaxKind.OrOperator);
        result.push(...(0, lua_ast_1.createLocalOrExportedOrGlobalDeclaration)(context, name, table, node));
    }
    const enumReference = context.transformExpression(node.name);
    for (const member of node.members) {
        const memberName = (0, literal_1.transformPropertyName)(context, member.name);
        let valueExpression;
        const constEnumValue = tryGetConstEnumValue(context, member);
        if (constEnumValue) {
            valueExpression = constEnumValue;
        }
        else if (member.initializer) {
            if (ts.isIdentifier(member.initializer)) {
                const symbol = context.checker.getSymbolAtLocation(member.initializer);
                if ((symbol === null || symbol === void 0 ? void 0 : symbol.valueDeclaration) &&
                    ts.isEnumMember(symbol.valueDeclaration) &&
                    symbol.valueDeclaration.parent === node) {
                    const otherMemberName = (0, literal_1.transformPropertyName)(context, symbol.valueDeclaration.name);
                    valueExpression = lua.createTableIndexExpression(enumReference, otherMemberName);
                }
            }
            if (!valueExpression) {
                valueExpression = context.transformExpression(member.initializer);
            }
        }
        else {
            valueExpression = lua.createNilLiteral();
        }
        if (membersOnly) {
            const enumSymbol = context.checker.getSymbolAtLocation(node.name);
            const exportScope = enumSymbol ? (0, export_1.getSymbolExportScope)(context, enumSymbol) : undefined;
            result.push(...(0, lua_ast_1.createLocalOrExportedOrGlobalDeclaration)(context, lua.isIdentifier(memberName)
                ? memberName
                : lua.createIdentifier(member.name.getText(), member.name), valueExpression, node, exportScope));
        }
        else {
            const memberAccessor = lua.createTableIndexExpression(enumReference, memberName);
            result.push(lua.createAssignmentStatement(memberAccessor, valueExpression, member));
            if (!lua.isStringLiteral(valueExpression) && !lua.isNilLiteral(valueExpression)) {
                const reverseMemberAccessor = lua.createTableIndexExpression(enumReference, memberAccessor);
                result.push(lua.createAssignmentStatement(reverseMemberAccessor, memberName, member));
            }
        }
    }
    return result;
};
exports.transformEnumDeclaration = transformEnumDeclaration;
//# sourceMappingURL=enum.js.map