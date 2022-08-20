import * as ts from "typescript";
import * as lua from "../../../LuaAST";
import { TransformationContext } from "../../context";
export declare function transformAssignmentLeftHandSideExpression(context: TransformationContext, node: ts.Expression, rightHasPrecedingStatements?: boolean): lua.AssignmentLeftHandSideExpression;
export declare function transformAssignment(context: TransformationContext, lhs: ts.Expression, right: lua.Expression, rightHasPrecedingStatements?: boolean, parent?: ts.Expression): lua.Statement[];
export declare function transformAssignmentWithRightPrecedingStatements(context: TransformationContext, lhs: ts.Expression, right: lua.Expression, rightPrecedingStatements: lua.Statement[], parent?: ts.Expression): lua.Statement[];
export declare function transformAssignmentExpression(context: TransformationContext, expression: ts.AssignmentExpression<ts.EqualsToken>): lua.Expression;
export declare function transformAssignmentStatement(context: TransformationContext, expression: ts.AssignmentExpression<ts.EqualsToken>): lua.Statement[];
