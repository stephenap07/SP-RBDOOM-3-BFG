import * as ts from "typescript";
import { Plugin } from "./plugins";
import { EmitHost, ProcessedFile } from "./utils";
export interface TranspileOptions {
    program: ts.Program;
    sourceFiles?: ts.SourceFile[];
    customTransformers?: ts.CustomTransformers;
    plugins?: Plugin[];
}
export interface TranspileResult {
    diagnostics: ts.Diagnostic[];
    transpiledFiles: ProcessedFile[];
}
export declare function getProgramTranspileResult(emitHost: EmitHost, writeFileResult: ts.WriteFileCallback, { program, sourceFiles: targetSourceFiles, customTransformers, plugins }: TranspileOptions): TranspileResult;
