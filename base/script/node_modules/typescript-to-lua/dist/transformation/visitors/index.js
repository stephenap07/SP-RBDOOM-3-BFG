"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.standardVisitors = void 0;
const ts = require("typescript");
const access_1 = require("./access");
const binary_expression_1 = require("./binary-expression");
const block_1 = require("./block");
const break_continue_1 = require("./break-continue");
const call_1 = require("./call");
const spread_1 = require("./spread");
const class_1 = require("./class");
const new_1 = require("./class/new");
const conditional_1 = require("./conditional");
const delete_1 = require("./delete");
const enum_1 = require("./enum");
const errors_1 = require("./errors");
const expression_statement_1 = require("./expression-statement");
const function_1 = require("./function");
const identifier_1 = require("./identifier");
const literal_1 = require("./literal");
const do_while_1 = require("./loops/do-while");
const for_1 = require("./loops/for");
const for_in_1 = require("./loops/for-in");
const for_of_1 = require("./loops/for-of");
const export_1 = require("./modules/export");
const import_1 = require("./modules/import");
const namespace_1 = require("./namespace");
const return_1 = require("./return");
const sourceFile_1 = require("./sourceFile");
const switch_1 = require("./switch");
const template_1 = require("./template");
const typeof_1 = require("./typeof");
const typescript_1 = require("./typescript");
const unary_expression_1 = require("./unary-expression");
const variable_declaration_1 = require("./variable-declaration");
const async_await_1 = require("./async-await");
const void_1 = require("./void");
const transformEmptyStatement = () => undefined;
const transformParenthesizedExpression = (node, context) => context.transformExpression(node.expression);
exports.standardVisitors = {
    ...literal_1.literalVisitors,
    ...typescript_1.typescriptVisitors,
    [ts.SyntaxKind.ArrowFunction]: function_1.transformFunctionLikeDeclaration,
    [ts.SyntaxKind.AwaitExpression]: async_await_1.transformAwaitExpression,
    [ts.SyntaxKind.BinaryExpression]: binary_expression_1.transformBinaryExpression,
    [ts.SyntaxKind.Block]: block_1.transformBlock,
    [ts.SyntaxKind.BreakStatement]: break_continue_1.transformBreakStatement,
    [ts.SyntaxKind.CallExpression]: call_1.transformCallExpression,
    [ts.SyntaxKind.ClassDeclaration]: class_1.transformClassDeclaration,
    [ts.SyntaxKind.ClassExpression]: class_1.transformClassAsExpression,
    [ts.SyntaxKind.ConditionalExpression]: conditional_1.transformConditionalExpression,
    [ts.SyntaxKind.ContinueStatement]: break_continue_1.transformContinueStatement,
    [ts.SyntaxKind.DeleteExpression]: delete_1.transformDeleteExpression,
    [ts.SyntaxKind.DoStatement]: do_while_1.transformDoStatement,
    [ts.SyntaxKind.ElementAccessExpression]: access_1.transformElementAccessExpression,
    [ts.SyntaxKind.EmptyStatement]: transformEmptyStatement,
    [ts.SyntaxKind.EnumDeclaration]: enum_1.transformEnumDeclaration,
    [ts.SyntaxKind.ExportAssignment]: export_1.transformExportAssignment,
    [ts.SyntaxKind.ExportDeclaration]: export_1.transformExportDeclaration,
    [ts.SyntaxKind.ExpressionStatement]: expression_statement_1.transformExpressionStatement,
    [ts.SyntaxKind.ExternalModuleReference]: import_1.transformExternalModuleReference,
    [ts.SyntaxKind.ForInStatement]: for_in_1.transformForInStatement,
    [ts.SyntaxKind.ForOfStatement]: for_of_1.transformForOfStatement,
    [ts.SyntaxKind.ForStatement]: for_1.transformForStatement,
    [ts.SyntaxKind.FunctionDeclaration]: function_1.transformFunctionDeclaration,
    [ts.SyntaxKind.FunctionExpression]: function_1.transformFunctionLikeDeclaration,
    [ts.SyntaxKind.Identifier]: identifier_1.transformIdentifierExpression,
    [ts.SyntaxKind.IfStatement]: conditional_1.transformIfStatement,
    [ts.SyntaxKind.ImportDeclaration]: import_1.transformImportDeclaration,
    [ts.SyntaxKind.ImportEqualsDeclaration]: import_1.transformImportEqualsDeclaration,
    [ts.SyntaxKind.ModuleDeclaration]: namespace_1.transformModuleDeclaration,
    [ts.SyntaxKind.NewExpression]: new_1.transformNewExpression,
    [ts.SyntaxKind.ParenthesizedExpression]: transformParenthesizedExpression,
    [ts.SyntaxKind.PostfixUnaryExpression]: unary_expression_1.transformPostfixUnaryExpression,
    [ts.SyntaxKind.PrefixUnaryExpression]: unary_expression_1.transformPrefixUnaryExpression,
    [ts.SyntaxKind.PropertyAccessExpression]: access_1.transformPropertyAccessExpression,
    [ts.SyntaxKind.QualifiedName]: access_1.transformQualifiedName,
    [ts.SyntaxKind.ReturnStatement]: return_1.transformReturnStatement,
    [ts.SyntaxKind.SourceFile]: sourceFile_1.transformSourceFileNode,
    [ts.SyntaxKind.SpreadElement]: spread_1.transformSpreadElement,
    [ts.SyntaxKind.SuperKeyword]: class_1.transformSuperExpression,
    [ts.SyntaxKind.SwitchStatement]: switch_1.transformSwitchStatement,
    [ts.SyntaxKind.TaggedTemplateExpression]: template_1.transformTaggedTemplateExpression,
    [ts.SyntaxKind.TemplateExpression]: template_1.transformTemplateExpression,
    [ts.SyntaxKind.ThisKeyword]: class_1.transformThisExpression,
    [ts.SyntaxKind.ThrowStatement]: errors_1.transformThrowStatement,
    [ts.SyntaxKind.TryStatement]: errors_1.transformTryStatement,
    [ts.SyntaxKind.TypeOfExpression]: typeof_1.transformTypeOfExpression,
    [ts.SyntaxKind.VariableStatement]: variable_declaration_1.transformVariableStatement,
    [ts.SyntaxKind.WhileStatement]: do_while_1.transformWhileStatement,
    [ts.SyntaxKind.YieldExpression]: function_1.transformYieldExpression,
    [ts.SyntaxKind.VoidExpression]: void_1.transformVoidExpression,
};
//# sourceMappingURL=index.js.map