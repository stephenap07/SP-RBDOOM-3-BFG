import * as ts from "typescript";
import { EmitHost, ProcessedFile } from "./utils";
interface ResolutionResult {
    resolvedFiles: ProcessedFile[];
    diagnostics: ts.Diagnostic[];
}
export declare function resolveDependencies(program: ts.Program, files: ProcessedFile[], emitHost: EmitHost): ResolutionResult;
export {};
