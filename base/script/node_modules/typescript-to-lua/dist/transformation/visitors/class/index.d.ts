import * as ts from "typescript";
import * as lua from "../../../LuaAST";
import { FunctionVisitor, TransformationContext } from "../../context";
export declare const transformClassDeclaration: FunctionVisitor<ts.ClassLikeDeclaration>;
export declare const transformThisExpression: FunctionVisitor<ts.ThisExpression>;
export declare function transformClassAsExpression(expression: ts.ClassLikeDeclaration, context: TransformationContext): lua.Expression;
export declare const transformSuperExpression: FunctionVisitor<ts.SuperExpression>;
