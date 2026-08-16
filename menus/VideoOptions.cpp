/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "Framework.h"
#include "Bitmap.h"
#include "PicButton.h"
#include "Slider.h"
#include "CheckBox.h"
#include "YesNoMessageBox.h"
#include "SpinControl.h"
#include "StringArrayModel.h"

#define ART_BANNER	  	"gfx/shell/head_vidoptions"

#define LEGACY_VIEWSIZE 0

// options for the ray traced renderer
static const char *pNvDlssNames[] =
{
	L( "Off" ),
	L( "Quality" ),
	L( "Balanced" ),
	L( "Performance" ),
	L( "Ultra Performance" ),
};

// must match the rt_upscale_fsr3 cvar modes from gl_opengl.c
static const char *pFsr3Names[] =
{
	L( "Off" ),
	L( "Native AA" ),
	L( "Quality" ),
	L( "Balanced" ),
	L( "Performance" ),
	L( "Ultra Performance" ),
};

// must match RT_VINTAGE enum from gl_rmain.c
static const char *pVintageNames[] =
{
	L( "Off" ),
	L( "CRT" ),
	L( "200p" ),
	L( "480p" ),
	L( "720p" ),
};

class CMenuVidOptions : public CMenuFramework
{
private:
	void _Init() override;
	void Reload() override;

public:
	CMenuVidOptions() : CMenuFramework( "CMenuVidOptions" ) { }
	void SaveAndPopMenu() override;

	CMenuPicButton	done;

#if LEGACY_VIEWSIZE
	CMenuSlider	screenSize;
#endif
	CMenuCheckBox	detailtex;
	CMenuCheckBox	hudscale;
	CMenuYesNoMessageBox	msgBox;

	// ray traced renderer options
	CMenuCheckBox vsync;
	CMenuCheckBox muzzleFlash;
	CMenuCheckBox nearestTextureFiltering;
	CMenuCheckBox classic;
	CMenuCheckBox noServerVis;
	CMenuCheckBox volumetric;
	CMenuCheckBox bloom;
	CMenuCheckBox dirt;
	CMenuCheckBox fsr3FrameGen;
	CMenuSpinControl nvDlss;
	CMenuSpinControl fsr3;
	CMenuSpinControl vintage;
	CMenuYesNoMessageBox frameGenMsgBox;
};

/*
=================
CMenuVidOptions::SaveAndPopMenu
=================
*/
void CMenuVidOptions::SaveAndPopMenu( void )
{
#if LEGACY_VIEWSIZE
	screenSize.WriteCvar();
#endif
	detailtex.WriteCvar();
	hudscale.WriteCvar();

	CMenuFramework::SaveAndPopMenu();
}

/*
=================
CMenuVidOptions::Init
=================
*/
void CMenuVidOptions::_Init( void )
{
	banner.SetPicture( ART_BANNER );

	int height = 280;

#if LEGACY_VIEWSIZE
	screenSize.szName = L( "Screen size" );
	screenSize.SetCoord( 72, height );
	screenSize.Setup( 30, 120, 10 );
	screenSize.onChanged = CMenuEditable::WriteCvarCb;

	height += 60;
#endif

	detailtex.szName = L( "Detail textures" );
	detailtex.SetCoord( 72, height );
	height += 50;

	hudscale.szName = L( "Auto scale HUD" );
	hudscale.SetCoord( 72, height );
	height += 50;

	done.szName = L( "GameUI_OK" );
	done.SetCoord( 72, height );
	done.SetPicture( PC_DONE );
	done.onReleased = VoidCb( &CMenuVidOptions::SaveAndPopMenu );
	height += 60;

	// ray traced renderer options
	static CStringArrayModel nvDlssModel( pNvDlssNames, V_ARRAYSIZE( pNvDlssNames ) );
	nvDlss.szName = "NVIDIA DLSS";
	nvDlss.SetRect( 390, 280, 250, 32 );
	nvDlss.SetCharSize( QM_SMALLFONT );
	nvDlss.Setup( &nvDlssModel );
	nvDlss.LinkCvar( "rt_upscale_dlss", CMenuEditable::CVAR_VALUE );
	nvDlss.bUpdateImmediately = true;
	nvDlss.onCvarWrite = []( CMenuBaseItem* pSelf, void* pExtra ) {
		int val = int( static_cast< CMenuEditable* >( pSelf )->CvarValue() );
		EngFuncs::CvarSetValue( "rt_upscale_dlss", float( val ) );
		if( val != 0 )
		{
			EngFuncs::CvarSetValue( "rt_ef_vintage", 0 );
		}
	};

	static CStringArrayModel fsr3Model( pFsr3Names, V_ARRAYSIZE( pFsr3Names ) );
	fsr3.szName = "AMD FSR3";
	fsr3.SetRect( 660, 280, 250, 32 );
	fsr3.SetCharSize( QM_SMALLFONT );
	fsr3.Setup( &fsr3Model );
	fsr3.LinkCvar( "rt_upscale_fsr3", CMenuEditable::CVAR_VALUE );
	fsr3.bUpdateImmediately = true;
	fsr3.onCvarWrite = []( CMenuBaseItem* pSelf, void* pExtra ) {
		int val = int( static_cast< CMenuEditable* >( pSelf )->CvarValue() );
		EngFuncs::CvarSetValue( "rt_upscale_fsr3", float( val ) );
		if( val != 0 )
		{
			EngFuncs::CvarSetValue( "rt_ef_vintage", 0 );
		}
	};

	static CStringArrayModel vintageModel( pVintageNames, V_ARRAYSIZE( pVintageNames ) );
	vintage.szName = "Vintage";
	vintage.SetRect( 390, 340, 250, 32 );
	vintage.SetCharSize( QM_SMALLFONT );
	vintage.Setup( &vintageModel );
	vintage.LinkCvar( "rt_ef_vintage", CMenuEditable::CVAR_VALUE );
	vintage.bUpdateImmediately = true;
	vintage.onCvarWrite = []( CMenuBaseItem* pSelf, void* pExtra ) {
		int val = int( static_cast< CMenuEditable* >( pSelf )->CvarValue() );
		EngFuncs::CvarSetValue( "rt_ef_vintage", float( val ) );
		if( val != 0 )
		{
			EngFuncs::CvarSetValue( "rt_upscale_dlss", 0 );
			EngFuncs::CvarSetValue( "rt_upscale_fsr3", 0 );
		}
	};

	vsync.szName = L( "VSync" );
	vsync.SetCoord( 390, 400 );

	muzzleFlash.szName = L( "Muzzle flash" );
	muzzleFlash.SetCoord( 390, 445 );

	nearestTextureFiltering.szName = L( "Pixelated textures" );
	nearestTextureFiltering.SetCoord( 390, 490 );

	classic.szName = L( "Classic lighting" );
	classic.SetCoord( 390, 535 );

	noServerVis.szName = L( "No server cull" );
	noServerVis.SetCoord( 390, 580 );

	volumetric.szName = L( "Volumetrics" );
	volumetric.SetCoord( 390, 625 );

	bloom.szName = L( "Bloom" );
	bloom.SetCoord( 390, 670 );

	dirt.szName = L( "Lens dirt" );
	dirt.SetCoord( 390, 715 );

	fsr3FrameGen.szName = L( "FSR3 Frame Gen" );
	fsr3FrameGen.SetCoord( 660, 340 );

	frameGenMsgBox.SetMessage( L( "^3FSR3 Frame Generation setting has been changed.^7\n\nPlease restart the game for this change to take effect." ) );

	SET_EVENT_MULTI( fsr3FrameGen.onChanged,
	{
		CMenuVidOptions *parent = (CMenuVidOptions *)pSelf->Parent();
		parent->frameGenMsgBox.Show();
	});

	msgBox.SetMessage( L( "^1WARNING: This is an experimental option and turning it on might break mods!^7\n\nReload the game or reconnect to the server to apply the settings."));
	msgBox.onNegative.pExtra = &hudscale;
	SET_EVENT_MULTI( msgBox.onNegative,
	{
		CMenuCheckBox *cb = (CMenuCheckBox *)pExtra;

		cb->bChecked = false;
		cb->SetCvarValue( 0.0f );
	});

	SET_EVENT_MULTI( hudscale.onChanged,
	{
		CMenuVidOptions *parent = (CMenuVidOptions *)pSelf->Parent();

		if( EngFuncs::ClientInGame( ))
		{
			// bring up warning message box
			// FIXME: try to save the game, apply the settings, and then load it back. This will be useful when changing resolution too.
			parent->msgBox.Show();
		}
	});

	SET_EVENT_MULTI( hudscale.onCvarWrite,
	{
		CMenuCheckBox *cb = (CMenuCheckBox *)pSelf;

		if( cb->bChecked )
		{
			// automatically scale HUD as if we have 1024x768 screen
			// (saving aspect ratio)
			// FIXME: allow configuring this value?
			EngFuncs::CvarSetValue( cb->CvarName(), 1024.0f );
		}
		else
		{
			EngFuncs::CvarSetValue( cb->CvarName(), 0.0f );
		}
	});

	SET_EVENT_MULTI( hudscale.onCvarGet,
	{
		CMenuCheckBox *cb = (CMenuCheckBox *)pSelf;

		cb->bChecked = EngFuncs::GetCvarFloat( cb->CvarName() ) >= 640.0f;
	});

	AddItem( banner );
	AddItem( done );
#if LEGACY_VIEWSIZE
	AddItem( screenSize );
#endif
	AddItem( detailtex );
	AddItem( hudscale );
	AddItem( nvDlss );
	AddItem( fsr3 );
	AddItem( fsr3FrameGen );
	AddItem( vintage );
	AddItem( vsync );
	AddItem( muzzleFlash );
	AddItem( nearestTextureFiltering );
	AddItem( classic );
	AddItem( noServerVis );
	AddItem( volumetric );
	AddItem( bloom );
	AddItem( dirt );

#if LEGACY_VIEWSIZE
	screenSize.LinkCvar( "viewsize" );
#endif

	detailtex.LinkCvar( "r_detailtextures" );
	hudscale.LinkCvar( "hud_scale" );

	// ray traced renderer options
	vsync.LinkCvar( "rt_vsync" );
	vsync.bUpdateImmediately = true;
	muzzleFlash.LinkCvar( "rt_mzlflash" );
	muzzleFlash.bUpdateImmediately = true;
	nearestTextureFiltering.LinkCvar( "rt_texture_nearest" );
	nearestTextureFiltering.bUpdateImmediately = true;
	classic.LinkCvar( "rt_classic" );
	classic.bUpdateImmediately = true;
	noServerVis.LinkCvar( "sv_novis" );
	noServerVis.bUpdateImmediately = true;
	volumetric.LinkCvar( "rt_volume_type" );
	volumetric.bUpdateImmediately = true;
	bloom.LinkCvar( "rt_bloom" );
	bloom.bUpdateImmediately = true;
	dirt.LinkCvar( "rt_bloom_dirt_enable" );
	dirt.bUpdateImmediately = true;
	fsr3FrameGen.LinkCvar( "rt_fsr3_framegen" );
	fsr3FrameGen.bUpdateImmediately = true;
}

void CMenuVidOptions::Reload()
{
	bool gl_active = !strnicmp( EngFuncs::GetCvarString( "r_refdll_loaded" ), "gl", 2 );

	detailtex.SetGrayed( !gl_active );
	detailtex.SetInactive( !gl_active );

	// ray traced renderer options are only meaningful with the GL renderer
	bool rt_active = gl_active;
	nvDlss.SetGrayed( !rt_active || !(int)EngFuncs::GetCvarFloat( "_rt_dlss_available" ));
	nvDlss.SetInactive( !rt_active );
	fsr3.SetGrayed( !rt_active || !(int)EngFuncs::GetCvarFloat( "_rt_fsr3_available" ));
	fsr3.SetInactive( !rt_active );
	fsr3FrameGen.SetGrayed( !rt_active || !(int)EngFuncs::GetCvarFloat( "_rt_fsr3_framegen_available" ));
	fsr3FrameGen.SetInactive( !rt_active );
	vintage.SetGrayed( !rt_active );
	vintage.SetInactive( !rt_active );
	vsync.SetGrayed( !rt_active );
	vsync.SetInactive( !rt_active );
	muzzleFlash.SetGrayed( !rt_active );
	muzzleFlash.SetInactive( !rt_active );
	nearestTextureFiltering.SetGrayed( !rt_active );
	nearestTextureFiltering.SetInactive( !rt_active );
	classic.SetGrayed( !rt_active );
	classic.SetInactive( !rt_active );
	noServerVis.SetGrayed( !rt_active );
	noServerVis.SetInactive( !rt_active );
	volumetric.SetGrayed( !rt_active );
	volumetric.SetInactive( !rt_active );
	bloom.SetGrayed( !rt_active );
	bloom.SetInactive( !rt_active );
	dirt.SetGrayed( !rt_active || !(int)EngFuncs::GetCvarFloat( "rt_bloom" ));
	dirt.SetInactive( !rt_active );

	CMenuFramework::Reload();
}

ADD_MENU( menu_vidoptions, CMenuVidOptions, UI_VidOptions_Menu );
