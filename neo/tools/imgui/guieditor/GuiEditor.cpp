/*
===========================================================================

Doom 3 GPL Source Code
Copyright (C) 1999-2011 id Software LLC, a ZeniMax Media company.
Copyright (C) 2022 Stephen Pridham

This file is part of the Doom 3 GPL Source Code ("Doom 3 Source Code").

Doom 3 Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

#include "precompiled.h"
#pragma hdrstop

#include "GuiEditor.h"

#include "../imgui/BFGimgui.h"
#include "../util/Imgui_IdWidgets.h"

#include "../renderer/RenderCommon.h"

#include "../ui/DeviceContext.h"
#include "sys/DeviceManager.h"

extern idGuiModel*		tr_guiModel;
extern DeviceManager*	deviceManager;

namespace ImGuiTools
{

GuiEditor::GuiEditor()
	: isInitialized( false )
	, openGui( 0 )
	, imgSize( VIRTUAL_WIDTH, VIRTUAL_HEIGHT )
	, gui( nullptr )
	, mat( nullptr )
{
}

GuiEditor::~GuiEditor()
{
}

void GuiEditor::Init()
{
	mat = declManager->FindMaterial( "_guiEdit" );
}

void GuiEditor::Draw()
{
	bool editorEnabled = com_editors & EDITOR_GUI;
	bool showTool = editorEnabled;

	if( !isInitialized )
	{
		Init();
		isInitialized = true;
		impl::SetReleaseToolMouse( true );
	}

	if( ImGui::Begin( "GUI Editor", &showTool, ImGuiWindowFlags_MenuBar ) )
	{
		if( ImGui::BeginMenuBar() )
		{
			bool clickedNew = false;
			bool clickedOpen = false;

			if( ImGui::BeginMenu( "File" ) )
			{
				clickedNew = ImGui::MenuItem( "New", "Ctrl+N" );
				clickedOpen = ImGui::MenuItem( "Open..", "Ctrl+O" );

				if( ImGui::MenuItem( "Save", "Ctrl+S" ) )
				{
				}

				// When the editor is closed. it should also set g_editEntities with ~= EDITOR_AF.
				if( ImGui::MenuItem( "Close", "Ctrl+W" ) )
				{
					showTool = false;
				}

				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();


			if( clickedOpen )
			{
				guiFiles.Clear();
				ImGui::OpenPopup( "Open##popup" );
			}

			if( ImGui::BeginPopupModal( "Open##popup" ) )
			{
				Browse();

				ImGui::Text( "Selected gui: %s", selectedGui.c_str() );

				if( ImGui::Button( "Open" ) )
				{
					gui = uiManager->Alloc();

					if( gui )
					{
						gui->InitFromFile( selectedGui.c_str() );
					}

					ImGui::CloseCurrentPopup();
				}

				ImGui::SameLine();

				if( ImGui::Button( "Cancel" ) )
				{
					ImGui::CloseCurrentPopup();
				}

				ImGui::EndPopup();
			}
		}

		if( gui )
		{
			tr.guiModel->EmitFullScreen();
			tr.guiModel->Clear();

			HandleEvent();
			gui->Redraw( Sys_Milliseconds() / 1000.f );

			tr.guiModel->EmitFullScreen( globalFramebuffers.guiRenderTargetFBO );
			tr.guiModel->Clear();

			ImGui::Image( ( ImTextureID )mat, ImVec2( 640, 480 ) );
		}

	}

	ImGui::End();

	if( !showTool )
	{
		isInitialized = false;
		impl::SetReleaseToolMouse( false );
		com_editors &= ~EDITOR_GUI;
	}
}

void GuiEditor::Browse()
{
	if( guiFiles.Num() == 0 )
	{
		idFileList* files = fileSystem->ListFiles( "guis", ".gui", true, true );
		for( int i = 0; i < files->GetNumFiles(); i++ )
		{
			guiFiles.Append( files->GetFile( i ) );
		}
		fileSystem->FreeFileList( files );

		if( guiFiles.Num() > 0 )
		{
			selectedGui = guiFiles[ 0 ];
		}
	}

	if( ImGui::Combo( "Select", &openGui, StringListItemGetter, &guiFiles, guiFiles.Num() ) )
	{
		selectedGui = guiFiles[ openGui ];
	}
}

void GuiEditor::HandleEvent()
{
	if( !gui )
	{
		return;
	}

	ImVec2 mousePos = ImGui::GetMouseDragDelta();

	if( mousePos.x != 0.f || mousePos.y != 0.f )
	{
		sysEvent_t ev = sys->GenerateMouseMoveEvent( mousePos.x, mousePos.y );
		gui->HandleEvent( &ev, Sys_Milliseconds() / 1000.f );
	}
}

GuiEditor& GuiEditor::Instance()
{
	static GuiEditor instance;
	return instance;
}

}