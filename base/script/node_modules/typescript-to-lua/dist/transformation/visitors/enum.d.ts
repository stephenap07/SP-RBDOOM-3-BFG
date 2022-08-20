import * as ts from "typescript";
import * as lua from "../../LuaAST";
import { FunctionVisitor, TransformationContext } from "../context";
export declare function tryGetConstEnumValue(context: TransformationContext, node: ts.EnumMember | ts.PropertyAccessExpression | ts.ElementAccessExpression): lua.Expression | undefined;
export declare const transformEnumDeclaration: FunctionVisitor<ts.EnumDeclaration>;
