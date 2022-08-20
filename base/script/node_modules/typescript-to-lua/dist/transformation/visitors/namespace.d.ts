import * as ts from "typescript";
import * as lua from "../../LuaAST";
import { FunctionVisitor, TransformationContext } from "../context";
export declare function createModuleLocalNameIdentifier(context: TransformationContext, declaration: ts.ModuleDeclaration): lua.Identifier;
export declare const transformModuleDeclaration: FunctionVisitor<ts.ModuleDeclaration>;
