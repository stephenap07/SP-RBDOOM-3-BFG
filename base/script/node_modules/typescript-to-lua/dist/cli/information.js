"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.getHelpString = exports.versionString = exports.version = void 0;
const parse_1 = require("./parse");
exports.version = require("../../package.json").version;
exports.versionString = `Version ${exports.version}`;
const helpString = `
Syntax:   tstl [options] [files...]

Examples: tstl path/to/file.ts [...]
          tstl -p path/to/tsconfig.json

In addition to the options listed below you can also pass options
for the typescript compiler (For a list of options use tsc -h).
Some tsc options might have no effect.
`.trim();
function getHelpString() {
    var _a;
    let result = helpString + "\n\n";
    result += "Options:\n";
    for (const option of parse_1.optionDeclarations) {
        const aliasStrings = ((_a = option.aliases) !== null && _a !== void 0 ? _a : []).map(a => "-" + a);
        const optionString = [...aliasStrings, "--" + option.name].join("|");
        const valuesHint = option.type === "enum" ? option.choices.join("|") : option.type;
        const spacing = " ".repeat(Math.max(1, 45 - optionString.length - valuesHint.length));
        result += `\n ${optionString} <${valuesHint}>${spacing}${option.description}\n`;
    }
    return result;
}
exports.getHelpString = getHelpString;
//# sourceMappingURL=information.js.map