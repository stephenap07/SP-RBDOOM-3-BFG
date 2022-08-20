"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.resolvePlugin = exports.getConfigDirectory = void 0;
const path = require("path");
const resolve = require("resolve");
// TODO: Don't depend on CLI?
const cliDiagnostics = require("../cli/diagnostics");
const diagnosticFactories = require("./diagnostics");
const getConfigDirectory = (options) => options.configFilePath ? path.dirname(options.configFilePath) : process.cwd();
exports.getConfigDirectory = getConfigDirectory;
const getTstlDirectory = () => path.dirname(__dirname);
function resolvePlugin(kind, optionName, basedir, query, importName = "default") {
    if (typeof query !== "string") {
        return { error: cliDiagnostics.compilerOptionRequiresAValueOfType(optionName, "string") };
    }
    const isModuleNotFoundError = (error) => error.code === "MODULE_NOT_FOUND";
    let resolved;
    try {
        resolved = resolve.sync(query, { basedir, extensions: [".js", ".ts", ".tsx"] });
    }
    catch (err) {
        if (!isModuleNotFoundError(err))
            throw err;
        return { error: diagnosticFactories.couldNotResolveFrom(kind, query, basedir) };
    }
    const hasNoRequireHook = require.extensions[".ts"] === undefined;
    if (hasNoRequireHook && (resolved.endsWith(".ts") || resolved.endsWith(".tsx"))) {
        try {
            const tsNodePath = resolve.sync("ts-node", { basedir: getTstlDirectory() });
            const tsNode = require(tsNodePath);
            tsNode.register({ transpileOnly: true });
        }
        catch (err) {
            if (!isModuleNotFoundError(err))
                throw err;
            return { error: diagnosticFactories.toLoadItShouldBeTranspiled(kind, query) };
        }
    }
    const commonjsModule = require(resolved);
    const factoryModule = commonjsModule.__esModule ? commonjsModule : { default: commonjsModule };
    const result = factoryModule[importName];
    if (result === undefined) {
        return { error: diagnosticFactories.shouldHaveAExport(kind, query, importName) };
    }
    return { result };
}
exports.resolvePlugin = resolvePlugin;
//# sourceMappingURL=utils.js.map