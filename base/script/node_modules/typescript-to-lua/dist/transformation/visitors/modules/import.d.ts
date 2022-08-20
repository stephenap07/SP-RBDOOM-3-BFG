import * as ts from "typescript";
import * as lua from "../../../LuaAST";
import { FunctionVisitor, TransformationContext } from "../../context";
export declare function createModuleRequire(context: TransformationContext, moduleSpecifier: ts.Expression, tsOriginal?: ts.Node): lua.CallExpression;
export declare const transformImportDeclaration: FunctionVisitor<ts.ImportDeclaration>;
export declare const transformExternalModuleReference: FunctionVisitor<ts.ExternalModuleReference>;
export declare const transformImportEqualsDeclaration: FunctionVisitor<ts.ImportEqualsDeclaration>;
export declare const transformImportExpression: FunctionVisitor<ts.CallExpression>;
