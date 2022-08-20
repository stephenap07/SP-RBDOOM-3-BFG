import * as ts from "typescript";
import * as lua from "../../LuaAST";
import { FunctionVisitor, TransformationContext } from "../context";
export declare function transformExpressionBodyToReturnStatement(context: TransformationContext, node: ts.Expression): lua.Statement;
export declare const transformReturnStatement: FunctionVisitor<ts.ReturnStatement>;
export declare function createReturnStatement(context: TransformationContext, values: lua.Expression[], node: ts.Node): lua.ReturnStatement;
