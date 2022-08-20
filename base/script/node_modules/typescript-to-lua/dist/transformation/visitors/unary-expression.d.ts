import * as ts from "typescript";
import * as lua from "../../LuaAST";
import { FunctionVisitor, TransformationContext } from "../context";
export declare function transformUnaryExpressionStatement(context: TransformationContext, node: ts.ExpressionStatement): lua.Statement[] | undefined;
export declare const transformPostfixUnaryExpression: FunctionVisitor<ts.PostfixUnaryExpression>;
export declare const transformPrefixUnaryExpression: FunctionVisitor<ts.PrefixUnaryExpression>;
