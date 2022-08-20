import * as ts from "typescript";
import { FunctionVisitor } from "../context";
export declare const transformBreakStatement: FunctionVisitor<ts.BreakStatement>;
export declare const transformContinueStatement: FunctionVisitor<ts.ContinueStatement>;
