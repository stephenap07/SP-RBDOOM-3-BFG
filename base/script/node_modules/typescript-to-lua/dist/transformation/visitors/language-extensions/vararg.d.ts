import * as ts from "typescript";
import { TransformationContext } from "../../context";
import { Scope } from "../../utils/scope";
export declare function isGlobalVarargConstant(context: TransformationContext, symbol: ts.Symbol, scope: Scope): boolean;
export declare function isVarargConstantNode(context: TransformationContext, node: ts.Node): boolean;
