import * as ts from "typescript";
import { FunctionVisitor, TransformationContext } from "../../context";
export declare const transformExportAssignment: FunctionVisitor<ts.ExportAssignment>;
export declare const getExported: (context: TransformationContext, exportSpecifiers: ts.NamedExports) => ts.ExportSpecifier[];
export declare const transformExportDeclaration: FunctionVisitor<ts.ExportDeclaration>;
