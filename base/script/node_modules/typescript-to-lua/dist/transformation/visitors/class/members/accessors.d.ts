import * as lua from "../../../../LuaAST";
import { AllAccessorDeclarations, TransformationContext } from "../../../context";
export declare function transformAccessorDeclarations(context: TransformationContext, { firstAccessor, getAccessor, setAccessor }: AllAccessorDeclarations, className: lua.Identifier): lua.Statement | undefined;
