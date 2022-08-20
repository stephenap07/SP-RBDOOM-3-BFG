import * as ts from "typescript";
import * as lua from "../../LuaAST";
import { FunctionVisitor, TransformationContext } from "../context";
import { Scope, ScopeType } from "../utils/scope";
export declare function transformBlockOrStatement(context: TransformationContext, statement: ts.Statement): lua.Statement[];
export declare function transformScopeBlock(context: TransformationContext, node: ts.Block, scopeType: ScopeType): [lua.Block, Scope];
export declare const transformBlock: FunctionVisitor<ts.Block>;
