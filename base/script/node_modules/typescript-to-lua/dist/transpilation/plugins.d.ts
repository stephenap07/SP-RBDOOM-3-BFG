import * as ts from "typescript";
import { EmitHost } from "..";
import { CompilerOptions } from "../CompilerOptions";
import { Printer } from "../LuaPrinter";
import { Visitors } from "../transformation/context";
import { EmitFile, ProcessedFile } from "./utils";
export interface Plugin {
    /**
     * An augmentation to the map of visitors that transform TypeScript AST to Lua AST.
     *
     * Key is a `SyntaxKind` of a processed node.
     */
    visitors?: Visitors;
    /**
     * A function that converts Lua AST to a string.
     *
     * At most one custom printer can be provided across all plugins.
     */
    printer?: Printer;
    /**
     * This function is called before transpilation of the TypeScript program starts.
     */
    beforeTransform?: (program: ts.Program, options: CompilerOptions, emitHost: EmitHost) => ts.Diagnostic[] | void;
    /**
     * This function is called after translating the input program to Lua, but before resolving dependencies or bundling.
     */
    afterPrint?: (program: ts.Program, options: CompilerOptions, emitHost: EmitHost, result: ProcessedFile[]) => ts.Diagnostic[] | void;
    /**
     * This function is called after translating the input program to Lua, after resolving dependencies and after bundling.
     */
    beforeEmit?: (program: ts.Program, options: CompilerOptions, emitHost: EmitHost, result: EmitFile[]) => ts.Diagnostic[] | void;
}
export declare function getPlugins(program: ts.Program): {
    diagnostics: ts.Diagnostic[];
    plugins: Plugin[];
};
