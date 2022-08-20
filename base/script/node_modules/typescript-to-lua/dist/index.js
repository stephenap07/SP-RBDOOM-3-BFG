"use strict";
var __createBinding = (this && this.__createBinding) || (Object.create ? (function(o, m, k, k2) {
    if (k2 === undefined) k2 = k;
    var desc = Object.getOwnPropertyDescriptor(m, k);
    if (!desc || ("get" in desc ? !m.__esModule : desc.writable || desc.configurable)) {
      desc = { enumerable: true, get: function() { return m[k]; } };
    }
    Object.defineProperty(o, k2, desc);
}) : (function(o, m, k, k2) {
    if (k2 === undefined) k2 = k;
    o[k2] = m[k];
}));
var __exportStar = (this && this.__exportStar) || function(m, exports) {
    for (var p in m) if (p !== "default" && !Object.prototype.hasOwnProperty.call(exports, p)) __createBinding(exports, m, p);
};
Object.defineProperty(exports, "__esModule", { value: true });
exports.LuaLibFeature = exports.parseConfigFileWithSystem = exports.updateParsedConfigFile = exports.parseCommandLine = exports.version = void 0;
var information_1 = require("./cli/information");
Object.defineProperty(exports, "version", { enumerable: true, get: function () { return information_1.version; } });
var parse_1 = require("./cli/parse");
Object.defineProperty(exports, "parseCommandLine", { enumerable: true, get: function () { return parse_1.parseCommandLine; } });
Object.defineProperty(exports, "updateParsedConfigFile", { enumerable: true, get: function () { return parse_1.updateParsedConfigFile; } });
__exportStar(require("./cli/report"), exports);
var tsconfig_1 = require("./cli/tsconfig");
Object.defineProperty(exports, "parseConfigFileWithSystem", { enumerable: true, get: function () { return tsconfig_1.parseConfigFileWithSystem; } });
__exportStar(require("./CompilerOptions"), exports);
__exportStar(require("./LuaAST"), exports);
var LuaLib_1 = require("./LuaLib");
Object.defineProperty(exports, "LuaLibFeature", { enumerable: true, get: function () { return LuaLib_1.LuaLibFeature; } });
__exportStar(require("./LuaPrinter"), exports);
__exportStar(require("./transformation/context"), exports);
__exportStar(require("./transpilation"), exports);
//# sourceMappingURL=index.js.map