import * as ts from "typescript";
import * as lua from "../../../../LuaAST";
import { TransformationContext } from "../../../context";
export declare function transformMemberExpressionOwnerName(node: ts.PropertyDeclaration | ts.MethodDeclaration | ts.AccessorDeclaration, className: lua.Identifier): lua.Expression;
export declare function transformMethodName(context: TransformationContext, node: ts.MethodDeclaration): lua.Expression;
export declare function transformMethodDeclaration(context: TransformationContext, node: ts.MethodDeclaration, className: lua.Identifier): lua.Statement | undefined;
export declare function createMethodDecoratingExpression(context: TransformationContext, node: ts.MethodDeclaration, className: lua.Identifier): lua.Statement | undefined;
