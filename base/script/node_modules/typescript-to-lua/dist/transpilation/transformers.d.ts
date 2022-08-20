import * as ts from "typescript";
export declare function getTransformers(program: ts.Program, diagnostics: ts.Diagnostic[], customTransformers: ts.CustomTransformers, onSourceFile: (sourceFile: ts.SourceFile) => void): ts.CustomTransformers;
export declare const noImplicitSelfTransformer: ts.TransformerFactory<ts.SourceFile | ts.Bundle>;
export declare const stripParenthesisExpressionsTransformer: ts.TransformerFactory<ts.SourceFile>;
