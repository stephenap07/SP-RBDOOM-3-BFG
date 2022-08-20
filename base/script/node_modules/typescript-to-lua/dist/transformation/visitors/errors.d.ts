import * as ts from "typescript";
import { FunctionVisitor } from "../context";
export declare const transformTryStatement: FunctionVisitor<ts.TryStatement>;
export declare const transformThrowStatement: FunctionVisitor<ts.ThrowStatement>;
