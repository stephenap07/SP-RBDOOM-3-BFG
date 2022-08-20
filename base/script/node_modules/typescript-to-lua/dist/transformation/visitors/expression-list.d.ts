import * as ts from "typescript";
import * as lua from "../../LuaAST";
import { TransformationContext } from "../context";
export declare function shouldMoveToTemp(context: TransformationContext, expression: lua.Expression, tsOriginal?: ts.Node): boolean;
export declare function moveToPrecedingTemp(context: TransformationContext, expression: lua.Expression, tsOriginal?: ts.Node): lua.Expression;
export declare function transformExpressionList(context: TransformationContext, expressions: readonly ts.Expression[]): lua.Expression[];
export declare function transformOrderedExpressions(context: TransformationContext, expressions: readonly ts.Expression[]): lua.Expression[];
