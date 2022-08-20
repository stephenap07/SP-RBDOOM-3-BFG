import * as ts from "typescript";
import { CompilerOptions } from "../CompilerOptions";
import { ParsedCommandLine } from "./parse";
export declare function locateConfigFile(commandLine: ParsedCommandLine): ts.Diagnostic | string | undefined;
export declare function parseConfigFileWithSystem(configFileName: string, commandLineOptions?: CompilerOptions, system?: ts.System): ParsedCommandLine;
export declare function createConfigFileUpdater(optionsToExtend: CompilerOptions): (options: ts.CompilerOptions) => ts.Diagnostic[];
