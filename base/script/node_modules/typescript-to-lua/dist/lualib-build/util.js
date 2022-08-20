"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.isExportsReturn = exports.isImport = exports.isRequire = exports.isExportAssignment = exports.isExportAlias = exports.isExportTableIndex = exports.isExportTable = exports.isExportTableDeclaration = void 0;
const tstl = require("..");
function isExportTableDeclaration(node) {
    return tstl.isVariableDeclarationStatement(node) && isExportTable(node.left[0]);
}
exports.isExportTableDeclaration = isExportTableDeclaration;
function isExportTable(node) {
    return tstl.isIdentifier(node) && node.text === "____exports";
}
exports.isExportTable = isExportTable;
function isExportTableIndex(node) {
    return tstl.isTableIndexExpression(node) && isExportTable(node.table) && tstl.isStringLiteral(node.index);
}
exports.isExportTableIndex = isExportTableIndex;
function isExportAlias(node) {
    return tstl.isVariableDeclarationStatement(node) && node.right !== undefined && isExportTableIndex(node.right[0]);
}
exports.isExportAlias = isExportAlias;
function isExportAssignment(node) {
    return tstl.isAssignmentStatement(node) && isExportTableIndex(node.left[0]);
}
exports.isExportAssignment = isExportAssignment;
function isRequire(node) {
    return (tstl.isVariableDeclarationStatement(node) &&
        node.right &&
        tstl.isCallExpression(node.right[0]) &&
        tstl.isIdentifier(node.right[0].expression) &&
        node.right[0].expression.text === "require");
}
exports.isRequire = isRequire;
function isImport(node, importNames) {
    return tstl.isVariableDeclarationStatement(node) && importNames.has(node.left[0].text);
}
exports.isImport = isImport;
function isExportsReturn(node) {
    return (tstl.isReturnStatement(node) &&
        tstl.isIdentifier(node.expressions[0]) &&
        node.expressions[0].text === "____exports");
}
exports.isExportsReturn = isExportsReturn;
//# sourceMappingURL=util.js.map