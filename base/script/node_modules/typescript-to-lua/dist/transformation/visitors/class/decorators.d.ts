import * as ts from "typescript";
import * as lua from "../../../LuaAST";
import { TransformationContext } from "../../context";
export declare function transformDecoratorExpression(context: TransformationContext, decorator: ts.Decorator): lua.Expression;
export declare function createDecoratingExpression(context: TransformationContext, kind: ts.SyntaxKind, decorators: lua.Expression[], targetTableName: lua.Expression, targetFieldExpression?: lua.Expression): lua.Expression;
