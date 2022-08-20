import * as ts from "typescript";
import * as lua from "../../../LuaAST";
import { TransformationContext } from "../../context";
export declare function isArrayLength(context: TransformationContext, expression: ts.Expression): expression is ts.PropertyAccessExpression | ts.ElementAccessExpression;
export declare function transformDestructuringAssignment(context: TransformationContext, node: ts.DestructuringAssignment, root: lua.Expression, rightHasPrecedingStatements: boolean): lua.Statement[];
export declare function transformAssignmentPattern(context: TransformationContext, node: ts.AssignmentPattern, root: lua.Expression, rightHasPrecedingStatements: boolean): lua.Statement[];
