import * as ts from "typescript";
import * as lua from "../../LuaAST";
import { TransformationContext, Visitors } from "../context";
export declare function transformPropertyName(context: TransformationContext, node: ts.PropertyName): lua.Expression;
export declare function createShorthandIdentifier(context: TransformationContext, valueSymbol: ts.Symbol | undefined, propertyIdentifier: ts.Identifier): lua.Expression;
export declare const literalVisitors: Visitors;
