"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.createDiagnosticReporter = exports.prepareDiagnosticForFormatting = void 0;
const ts = require("typescript");
const prepareDiagnosticForFormatting = (diagnostic) => diagnostic.source === "typescript-to-lua" ? { ...diagnostic, code: "TL" } : diagnostic;
exports.prepareDiagnosticForFormatting = prepareDiagnosticForFormatting;
function createDiagnosticReporter(pretty, system = ts.sys) {
    const reporter = ts.createDiagnosticReporter(system, pretty);
    return diagnostic => reporter((0, exports.prepareDiagnosticForFormatting)(diagnostic));
}
exports.createDiagnosticReporter = createDiagnosticReporter;
//# sourceMappingURL=report.js.map