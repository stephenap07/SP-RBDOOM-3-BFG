import * as ts from "typescript";
import * as lua from "../../LuaAST";
import { FunctionVisitor, TransformationContext } from "../context";
export declare const transformTypeOfExpression: FunctionVisitor<ts.TypeOfExpression>;
export declare function transformTypeOfBinaryExpression(context: TransformationContext, node: ts.BinaryExpression): lua.Expression | undefined;
