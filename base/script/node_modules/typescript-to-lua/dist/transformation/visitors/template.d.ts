import * as ts from "typescript";
import { FunctionVisitor } from "../context";
export declare const transformTemplateExpression: FunctionVisitor<ts.TemplateExpression>;
export declare const transformTaggedTemplateExpression: FunctionVisitor<ts.TaggedTemplateExpression>;
