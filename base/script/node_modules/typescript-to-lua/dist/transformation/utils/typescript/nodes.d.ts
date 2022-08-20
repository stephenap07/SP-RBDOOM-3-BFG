import * as ts from "typescript";
import { TransformationContext } from "../../context";
export declare function isAssignmentPattern(node: ts.Node): node is ts.AssignmentPattern;
export declare function isDestructuringAssignment(node: ts.Node): node is ts.DestructuringAssignment;
export declare function isAmbientNode(node: ts.Declaration): boolean;
export declare function isInDestructingAssignment(node: ts.Node): boolean;
export declare function isInAsyncFunction(node: ts.Node): boolean;
export declare function isInGeneratorFunction(node: ts.Node): boolean;
/**
 * Quite hacky, avoid unless absolutely necessary!
 */
export declare function getSymbolOfNode(context: TransformationContext, node: ts.Node): ts.Symbol | undefined;
export declare function isFirstDeclaration(context: TransformationContext, node: ts.Node): boolean;
