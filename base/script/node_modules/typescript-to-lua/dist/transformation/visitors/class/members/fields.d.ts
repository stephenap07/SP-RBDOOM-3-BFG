import * as ts from "typescript";
import * as lua from "../../../../LuaAST";
import { TransformationContext } from "../../../context";
export declare function createPropertyDecoratingExpression(context: TransformationContext, node: ts.PropertyDeclaration | ts.AccessorDeclaration, className: lua.Identifier): lua.Expression | undefined;
export declare function transformClassInstanceFields(context: TransformationContext, instanceFields: ts.PropertyDeclaration[]): lua.Statement[];
export declare function transformStaticPropertyDeclaration(context: TransformationContext, field: ts.PropertyDeclaration, className: lua.Identifier): lua.AssignmentStatement | undefined;
