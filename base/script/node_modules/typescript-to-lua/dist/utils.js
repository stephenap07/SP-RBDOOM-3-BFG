"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.assume = exports.assertNever = exports.assert = exports.cast = exports.isNonNull = exports.getOrUpdate = exports.formatPathToLuaPath = exports.trimExtension = exports.normalizeSlashes = exports.createSerialDiagnosticFactory = exports.createDiagnosticFactoryWithCode = exports.intersection = exports.union = exports.intersperse = exports.castArray = void 0;
const ts = require("typescript");
const nativeAssert = require("assert");
const path = require("path");
function castArray(value) {
    return Array.isArray(value) ? value : [value];
}
exports.castArray = castArray;
const intersperse = (values, separator) => values.flatMap((value, index) => (index === 0 ? [value] : [separator, value]));
exports.intersperse = intersperse;
const union = (...values) => [...new Set(...values)];
exports.union = union;
const intersection = (first, ...rest) => (0, exports.union)(first).filter(x => rest.every(r => r.includes(x)));
exports.intersection = intersection;
const createDiagnosticFactoryWithCode = (code, create) => Object.assign((...args) => ({
    file: undefined,
    start: undefined,
    length: undefined,
    category: ts.DiagnosticCategory.Error,
    code,
    source: "typescript-to-lua",
    ...create(...args),
}), { code });
exports.createDiagnosticFactoryWithCode = createDiagnosticFactoryWithCode;
let serialDiagnosticCodeCounter = 100000;
const createSerialDiagnosticFactory = (create) => (0, exports.createDiagnosticFactoryWithCode)(serialDiagnosticCodeCounter++, create);
exports.createSerialDiagnosticFactory = createSerialDiagnosticFactory;
const normalizeSlashes = (filePath) => filePath.replace(/\\/g, "/");
exports.normalizeSlashes = normalizeSlashes;
const trimExtension = (filePath) => filePath.slice(0, -path.extname(filePath).length);
exports.trimExtension = trimExtension;
function formatPathToLuaPath(filePath) {
    filePath = filePath.replace(/\.json$/, "");
    if (process.platform === "win32") {
        // Windows can use backslashes
        filePath = filePath.replace(/\.\\/g, "").replace(/\\/g, ".");
    }
    return filePath.replace(/\.\//g, "").replace(/\//g, ".");
}
exports.formatPathToLuaPath = formatPathToLuaPath;
function getOrUpdate(
// eslint-disable-next-line @typescript-eslint/ban-types
map, key, getDefaultValue) {
    if (!map.has(key)) {
        map.set(key, getDefaultValue());
    }
    return map.get(key);
}
exports.getOrUpdate = getOrUpdate;
// eslint-disable-next-line @typescript-eslint/ban-types
function isNonNull(value) {
    return value != null;
}
exports.isNonNull = isNonNull;
function cast(item, cast) {
    if (cast(item)) {
        return item;
    }
    else {
        throw new Error(`Failed to cast value to expected type using ${cast.name}.`);
    }
}
exports.cast = cast;
function assert(value, message) {
    nativeAssert(value, message);
}
exports.assert = assert;
function assertNever(_value) {
    throw new Error("Value is expected to be never");
}
exports.assertNever = assertNever;
function assume(_value) { }
exports.assume = assume;
//# sourceMappingURL=utils.js.map