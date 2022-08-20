import { SourceNode } from "source-map";
import * as ts from "typescript";
import { EmitFile, ProcessedFile } from "./utils";
export declare const sourceMapTracebackBundlePlaceholder = "{#SourceMapTracebackBundle}";
export declare function printStackTraceBundleOverride(rootNode: SourceNode): string;
export declare function getBundleResult(program: ts.Program, files: ProcessedFile[]): [ts.Diagnostic[], EmitFile];
