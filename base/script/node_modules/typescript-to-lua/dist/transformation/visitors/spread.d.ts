import * as ts from "typescript";
import { FunctionVisitor, TransformationContext } from "../context";
export declare function isOptimizedVarArgSpread(context: TransformationContext, symbol: ts.Symbol, identifier: ts.Identifier): boolean | undefined;
export declare const transformSpreadElement: FunctionVisitor<ts.SpreadElement>;
