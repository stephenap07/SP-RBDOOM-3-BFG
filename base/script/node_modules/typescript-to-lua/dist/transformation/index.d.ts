import * as ts from "typescript";
import * as lua from "../LuaAST";
import { VisitorMap, Visitors } from "./context";
export declare function createVisitorMap(customVisitors: Visitors[]): VisitorMap;
export declare function transformSourceFile(program: ts.Program, sourceFile: ts.SourceFile, visitorMap: VisitorMap): {
    file: lua.File;
    diagnostics: ts.Diagnostic[];
};
