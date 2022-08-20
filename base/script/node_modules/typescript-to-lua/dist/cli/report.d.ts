import * as ts from "typescript";
export declare const prepareDiagnosticForFormatting: (diagnostic: ts.Diagnostic) => {
    code: any;
    reportsUnnecessary?: {} | undefined;
    reportsDeprecated?: {} | undefined;
    source?: string | undefined;
    relatedInformation?: ts.DiagnosticRelatedInformation[] | undefined;
    category: ts.DiagnosticCategory;
    file: ts.SourceFile | undefined;
    start: number | undefined;
    length: number | undefined;
    messageText: string | ts.DiagnosticMessageChain;
};
export declare function createDiagnosticReporter(pretty: boolean, system?: ts.System): ts.DiagnosticReporter;
