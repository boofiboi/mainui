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

static const char *pFsr3Names[] =
{
	L( "Off" ),
	L( "Native AA" ),
	L( "Quality" ),
	L( "Balanced" ),
	L( "Performance" ),
	L( "Ultra Performance" ),
};

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
	void Draw() override;

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

	CMenuCheckBox vsync;
	CMenuCheckBox muzzleFlash;
	CMenuCheckBox nearestTextureFiltering;
	CMenuCheckBox noServerVis;
	CMenuCheckBox volumetric;
	CMenuCheckBox bloom;
	CMenuCheckBox dirt;
	CMenuCheckBox antiFirefly;
	CMenuCheckBox indirect2Bounces;
	CMenuCheckBox waterWarp;
	CMenuCheckBox fsr3FrameGen;
	CMenuSpinControl fsr3;
	CMenuSpinControl vintage;
	CMenuYesNoMessageBox frameGenMsgBox;
};

void CMenuVidOptions::SaveAndPopMenu( void )
{
#if LEGACY_VIEWSIZE
	screenSize.WriteCvar();
#endif
	detailtex.WriteCvar();
	hudscale.WriteCvar();

	CMenuFramework::SaveAndPopMenu();
}

void CMenuVidOptions::_Init( void )
{
	banner.SetPicture( ART_BANNER );

	done.SetNameAndStatus( L( "GameUI_OK" ), L( "Save and close options" ) );
	done.SetCoord( 72, 380 );
	done.SetPicture( PC_DONE );
	done.onReleased = VoidCb( &CMenuVidOptions::SaveAndPopMenu );

	static CStringArrayModel fsr3Model( pFsr3Names, V_ARRAYSIZE( pFsr3Names ) );
	fsr3.SetNameAndStatus( "AMD FSR3", L( "AMD FidelityFX Super Resolution 3 upscaling" ) );
	fsr3.SetRect( 260, 265, 230, 32 );
	fsr3.SetCharSize( QM_SMALLFONT );
	fsr3.Setup( &fsr3Model );
	fsr3.LinkCvar( "rt_upscale_fsr3", CMenuEditable::CVAR_VALUE );
	fsr3.bUpdateImmediately = true;
	fsr3.onCvarWrite = []( CMenuBaseItem* pSelf, void* pExtra ) {
		int val = int( static_cast< CMenuEditable* >( pSelf )->CvarValue() );
		EngFuncs::CvarSetValue( "rt_upscale_fsr3", float( val ) );
		if( val != 0 && pSelf->Parent() )
		{
			CMenuVidOptions* parent = static_cast< CMenuVidOptions* >( pSelf->Parent() );
			parent->vintage.SetCurrentValue( 0.0f );
		}
	};

	static CStringArrayModel vintageModel( pVintageNames, V_ARRAYSIZE( pVintageNames ) );
	vintage.SetNameAndStatus( "Vintage", L( "Pixelated vintage rendering filter" ) );
	vintage.SetRect( 260, 315, 230, 32 );
	vintage.SetCharSize( QM_SMALLFONT );
	vintage.Setup( &vintageModel );
	vintage.LinkCvar( "rt_ef_vintage", CMenuEditable::CVAR_VALUE );
	vintage.bUpdateImmediately = true;
	vintage.onCvarWrite = []( CMenuBaseItem* pSelf, void* pExtra ) {
		int val = int( static_cast< CMenuEditable* >( pSelf )->CvarValue() );
		EngFuncs::CvarSetValue( "rt_ef_vintage", float( val ) );
		if( val != 0 && pSelf->Parent() )
		{
			CMenuVidOptions* parent = static_cast< CMenuVidOptions* >( pSelf->Parent() );
			parent->fsr3.SetCurrentValue( 0.0f );
		}
	};

	fsr3FrameGen.SetNameAndStatus( L( "FSR3 Frame Gen" ), L( "Frame generation is best used when FPS>60, utilizing it with any less FPS will result in massive amounts of frame delay" ) );
	fsr3FrameGen.SetCoord( 600, 315 );

	vsync.SetNameAndStatus( L( "VSync" ), L( "Enable vertical synchronization" ) );
	vsync.SetCoord( 260, 375 );

	nearestTextureFiltering.SetNameAndStatus( L( "Pixelated textures" ), L( "Disable texture filtering for pixelated retro texture look" ) );
	nearestTextureFiltering.SetCoord( 260, 415 );

	volumetric.SetNameAndStatus( L( "Volumetrics" ), L( "Enable ray-traced volumetric lighting and fog" ) );
	volumetric.SetCoord( 260, 455 );

	dirt.SetNameAndStatus( L( "Lens dirt" ), L( "Lens dirt effect on bright light sources" ) );
	dirt.SetCoord( 260, 495 );

	antiFirefly.SetNameAndStatus( L( "Anti-firefly" ), L( "Suppress high-energy specular firefly noise" ) );
	antiFirefly.SetCoord( 260, 535 );

	hudscale.SetNameAndStatus( L( "Auto scale HUD" ), L( "Automatically scale HUD to match screen resolution" ) );
	hudscale.SetCoord( 260, 575 );

	muzzleFlash.SetNameAndStatus( L( "Muzzle flash" ), L( "Ray-traced dynamic light for weapon muzzle flashes" ) );
	muzzleFlash.SetCoord( 600, 375 );

	noServerVis.SetNameAndStatus( L( "No server cull" ), L( "Disable server-side PVS culling so entire map geometry is ray-traced" ) );
	noServerVis.SetCoord( 600, 415 );

	bloom.SetNameAndStatus( L( "Bloom" ), L( "Bloom post-processing on glowing and bright surfaces" ) );
	bloom.SetCoord( 600, 455 );

	detailtex.SetNameAndStatus( L( "Detail textures" ), L( "Enable high-frequency detail textures overlay" ) );
	detailtex.SetCoord( 600, 495 );

	indirect2Bounces.SetNameAndStatus( L( "2nd indirect bounce" ), L( "Calculate second bounce of indirect ray-traced lighting" ) );
	indirect2Bounces.SetCoord( 600, 535 );

	waterWarp.SetNameAndStatus( L( "Underwater warp" ), L( "Screen distortion effect when submerged underwater" ) );
	waterWarp.SetCoord( 600, 575 );

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
			parent->msgBox.Show();
		}
	});

	SET_EVENT_MULTI( hudscale.onCvarWrite,
	{
		CMenuCheckBox *cb = (CMenuCheckBox *)pSelf;

		if( cb->bChecked )
		{
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
	AddItem( fsr3 );
	AddItem( fsr3FrameGen );
	AddItem( vintage );
	AddItem( vsync );
	AddItem( muzzleFlash );
	AddItem( nearestTextureFiltering );
	AddItem( noServerVis );
	AddItem( volumetric );
	AddItem( bloom );
	AddItem( dirt );
	AddItem( antiFirefly );
	AddItem( detailtex );
	AddItem( indirect2Bounces );
	AddItem( waterWarp );
	AddItem( hudscale );

#if LEGACY_VIEWSIZE
	screenSize.LinkCvar( "viewsize" );
#endif

	detailtex.LinkCvar( "r_detailtextures" );
	hudscale.LinkCvar( "hud_scale" );

	vsync.LinkCvar( "rt_vsync" );
	vsync.bUpdateImmediately = true;
	muzzleFlash.LinkCvar( "rt_mzlflash" );
	muzzleFlash.bUpdateImmediately = true;
	nearestTextureFiltering.LinkCvar( "rt_texture_nearest" );
	nearestTextureFiltering.bUpdateImmediately = true;
	noServerVis.LinkCvar( "sv_novis" );
	noServerVis.bUpdateImmediately = true;
	volumetric.LinkCvar( "rt_volume_type" );
	volumetric.bUpdateImmediately = true;
	bloom.LinkCvar( "rt_bloom" );
	bloom.bUpdateImmediately = true;
	dirt.LinkCvar( "rt_bloom_dirt_enable" );
	dirt.bUpdateImmediately = true;
	antiFirefly.LinkCvar( "rt_antifirefly" );
	antiFirefly.bUpdateImmediately = true;
	indirect2Bounces.LinkCvar( "rt_indir2bounces" );
	indirect2Bounces.bUpdateImmediately = true;
	waterWarp.LinkCvar( "rt_ef_water" );
	waterWarp.bUpdateImmediately = true;
	fsr3FrameGen.LinkCvar( "rt_fsr3_framegen" );
	fsr3FrameGen.bUpdateImmediately = true;
}

void CMenuVidOptions::Draw( void )
{
	CMenuFramework::Draw();

	UI_DrawRectangleExt( 260 * uiStatic.scaleX, 358 * uiStatic.scaleY, 630 * uiStatic.scaleX, 2 * uiStatic.scaleY, uiColorHelp, 1, QM_TOP | QM_BOTTOM );
}

void CMenuVidOptions::Reload()
{
	bool gl_active = !strnicmp( EngFuncs::GetCvarString( "r_refdll_loaded" ), "gl", 2 );

	detailtex.SetGrayed( !gl_active );
	detailtex.SetInactive( !gl_active );

	bool rt_active = gl_active;
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
	noServerVis.SetGrayed( !rt_active );
	noServerVis.SetInactive( !rt_active );
	volumetric.SetGrayed( !rt_active );
	volumetric.SetInactive( !rt_active );
	bloom.SetGrayed( !rt_active );
	bloom.SetInactive( !rt_active );
	dirt.SetGrayed( !rt_active || !(int)EngFuncs::GetCvarFloat( "rt_bloom" ));
	dirt.SetInactive( !rt_active );
	antiFirefly.SetGrayed( !rt_active );
	antiFirefly.SetInactive( !rt_active );
	indirect2Bounces.SetGrayed( !rt_active );
	indirect2Bounces.SetInactive( !rt_active );
	waterWarp.SetGrayed( !rt_active );
	waterWarp.SetInactive( !rt_active );

	CMenuFramework::Reload();
}

ADD_MENU( menu_vidoptions, CMenuVidOptions, UI_VidOptions_Menu );
