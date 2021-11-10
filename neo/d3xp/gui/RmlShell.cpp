/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.

This file is part of the Doom 3 BFG Edition GPL Source Code ("Doom 3 BFG Edition Source Code").

Doom 3 BFG Edition Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 BFG Edition Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 BFG Edition Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 BFG Edition Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 BFG Edition Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

#include "precompiled.h"
#pragma hdrstop

#include "RmlShell.h"

#include "../Game_local.h"

#include "rmlui/RmlUserInterfaceLocal.h"

#include "RmlUi/Core/EventListener.h"
#include "RmlUi/Core/EventListenerInstancer.h"
#include "RmlUi/Core/TransformPrimitive.h"

#include "rmlui/GlobalRmlEventListener.h"

class UI_Shell;

// UI Code

UI_Shell::UI_Shell()
	: _ui(nullptr)
{
}

bool UI_Shell::Init( )
{
	_ui = rmlManager->Find( "shell", true );

	if( !_ui )
	{ 
		return false;
	}

	_ui->SetInhibitsControl(true);
	_ui->Activate(true, 0);
	_ui->SetIsPausingGame(false);

	return SetNextScreen("startmenu");
}

Rml::ElementDocument* UI_Shell::SetNextScreen( const char* name )
{
	idStr path( va( "guis/rml/shell/%s.rml", name ) );

	Rml::ElementDocument* document = _ui->SetNextScreen( path.c_str() );

	if (document && idStr::Icmp(name, "start"))
	{
		// Initialize the start screen
		auto el = document->GetElementById("start_game");
		if (el)
		{
			auto p1 = Rml::Transform::MakeProperty({ Rml::Transforms::Rotate2D{10.f}, Rml::Transforms::TranslateX{100.f} });
			auto p2 = Rml::Transform::MakeProperty({ Rml::Transforms::Scale2D{3.f} });
			el->Animate("transform", p1, 1.8f, Rml::Tween{ Rml::Tween::Elastic, Rml::Tween::InOut }, -1, true);
			el->AddAnimationKey("transform", p2, 1.3f, Rml::Tween{ Rml::Tween::Elastic, Rml::Tween::InOut });
		}
	}

	// When this object frees its resources after destructing, it frees itself using the overriden delete method.
	// Originally this was allocated with the non-overriden 'new' function. Annoying.
	Rml::StringList textureNames = Rml::GetTextureSourceList();

	for (const auto& texturePath : textureNames)
	{
		const idMaterial* material = declManager->FindMaterial(texturePath.c_str());
		if (material)
		{
			if (idLib::IsMainThread())
			{
				material->ReloadImages(false);
			}
		}
		else
		{
			common->Warning("Failed to load rml texture %s", texturePath.c_str());
		}
	}

	return document;
}