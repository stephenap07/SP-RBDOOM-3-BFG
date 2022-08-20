import * as ts from "typescript";
import * as lua from "../../../LuaAST";
import { TransformationContext } from "../../context";
export declare function isRangeFunction(context: TransformationContext, expression: ts.CallExpression): boolean;
export declare function isRangeFunctionNode(context: TransformationContext, node: ts.Node): boolean;
export declare function transformRangeStatement(context: TransformationContext, statement: ts.ForOfStatement, block: lua.Block): lua.Statement;
