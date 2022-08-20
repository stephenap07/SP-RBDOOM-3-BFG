import * as ts from "typescript";
import { CompilerOptions } from "../CompilerOptions";
import { TranspiledFile } from "./output-collector";
import { EmitResult } from "./transpiler";
export { Plugin } from "./plugins";
export * from "./transpile";
export * from "./transpiler";
export { EmitHost } from "./utils";
export { TranspiledFile };
export declare function transpileFiles(rootNames: string[], options?: CompilerOptions, writeFile?: ts.WriteFileCallback): EmitResult;
export declare function transpileProject(configFileName: string, optionsToExtend?: CompilerOptions, writeFile?: ts.WriteFileCallback): EmitResult;
export interface TranspileVirtualProjectResult {
    diagnostics: ts.Diagnostic[];
    transpiledFiles: TranspiledFile[];
}
export declare function transpileVirtualProject(files: Record<string, string>, options?: CompilerOptions): TranspileVirtualProjectResult;
export interface TranspileStringResult {
    diagnostics: ts.Diagnostic[];
    file?: TranspiledFile;
}
export declare function transpileString(main: string, options?: CompilerOptions): TranspileStringResult;
