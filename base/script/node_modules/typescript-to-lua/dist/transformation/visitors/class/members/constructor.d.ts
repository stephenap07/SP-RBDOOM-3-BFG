import * as ts from "typescript";
import * as lua from "../../../../LuaAST";
import { TransformationContext } from "../../../context";
export declare function createPrototypeName(className: lua.Identifier): lua.TableIndexExpression;
export declare function createConstructorName(className: lua.Identifier): lua.TableIndexExpression;
export declare function transformConstructorDeclaration(context: TransformationContext, statement: ts.ConstructorDeclaration, className: lua.Identifier, instanceFields: ts.PropertyDeclaration[], classDeclaration: ts.ClassLikeDeclaration): lua.Statement | undefined;
