import * as ts from "typescript";
import { TransformationContext } from "../context";
export declare function validateAssignment(context: TransformationContext, node: ts.Node, fromType: ts.Type, toType: ts.Type, toName?: string): void;
