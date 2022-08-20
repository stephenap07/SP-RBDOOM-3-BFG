import * as ts from "typescript";
import * as lua from "../../../LuaAST";
import { TransformationContext } from "../../context";
export declare function transformLoopBody(context: TransformationContext, loop: ts.WhileStatement | ts.DoStatement | ts.ForStatement | ts.ForOfStatement | ts.ForInOrOfStatement): lua.Statement[];
export declare function getVariableDeclarationBinding(context: TransformationContext, node: ts.VariableDeclarationList): ts.BindingName;
export declare function transformForInitializer(context: TransformationContext, initializer: ts.ForInitializer, block: lua.Block): lua.Identifier;
export declare function invertCondition(expression: lua.Expression): lua.Expression;
