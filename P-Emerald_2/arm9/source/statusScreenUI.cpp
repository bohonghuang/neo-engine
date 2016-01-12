/*
Pok�mon Emerald 2 Version
------------------------------

file        : statusScreenUI.cpp
author      : Philip Wellnitz
description :

Copyright (C) 2012 - 2016
Philip Wellnitz

This file is part of Pok�mon Emerald 2 Version.

Pok�mon Emerald 2 Version is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Pok�mon Emerald 2 Version is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Pok�mon Emerald 2 Version.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "statusScreenUI.h"
#include "defines.h"
#include "buffer.h"
#include "move.h"
#include "item.h"
#include "uio.h"
#include "sprite.h"
#include "fs.h"
#include "saveGame.h"
#include "ribbon.h"

#include <cstdio>

#include "memo.h"
#include "atks.h"
#include "Contest.h"

#include "anti_pokerus_icon.h"
#include "pokerus_icon.h"
#include "rare_icon.h"

#include "Back.h"
#include "A.h"
#include "Forward.h"
#include "Backward.h"
#include "Up.h"
#include "Down.h"
#include "Choice_1.h"
#include "Choice_2.h"
#include "Choice_3.h"

namespace STS {
    u8 borders[ 6 ][ 2 ] = {
        { 4, 2 }, { 18, 3 },
        { 4, 9 }, { 18, 10 },
        { 4, 17 }, { 18, 18 }
    };

    u8 positions[ 6 ][ 2 ] = {
        { 14, 2 }, { 16, 3 }, { 14, 9 },
        { 16, 10 }, { 14, 17 }, { 16, 18 }
    };

    regStsScreenUI::regStsScreenUI( u8 p_pageMax ) {
        m_pagemax = p_pageMax;
    }

    //OAMTop indices
#define ICON_IDX(a) (a)
#define SHINY_IDX 100
#define PKMN_SPRITE_START 6
#define PAGE_ICON_IDX  10
#define ITEM_ICON_IDX  11
#define TYPE_IDX 12
#define ATK_DMGTYPE_IDX(a) (16 + (u8(a)))
#define RIBBON_IDX 20

#define ICON_PAL(a) (a)
#define SHINY_PAL 5
#define PKMN_SPRITE_PAL 6
#define PAGE_ICON_PAL 7
#define ITEM_ICON_PAL 8
#define TYPE_PAL(a) (9+(a))
#define DMG_TYPE_PAL(a) (13 + (u8(a)))

    //OamSub indices
    //#define BACK_ID  0
#define SUB_PAGE_ICON_IDX(a) (1+(a))
#define SUB_TYPE_IDX(a) (4 + (a))
#define SUB_RARE_IDX 6
#define SUB_BALL_IDX 7
    //#define A_ID  8
#define SUB_PKRS_IDX 9
    //#define FWD_ID 13
    //#define BWD_ID 14

    void regStsScreenUI::initSub( const pokemon& p_pokemon ) {
        consoleSelect( &IO::Bottom );
        consoleSetWindow( &IO::Bottom, 16, 0, 16, 16 );
        consoleClear( );
        IO::initOAMTable( true );
        u16 nextAvailableTileIdx = 0;

        nextAvailableTileIdx = IO::loadSprite( BACK_ID, BACK_ID, nextAvailableTileIdx,
                                               SCREEN_WIDTH - 28, SCREEN_HEIGHT - 28, 32, 32, BackPal,
                                               BackTiles, BackTilesLen, false, false, false, OBJPRIORITY_0, true );
        nextAvailableTileIdx = IO::loadSprite( SUB_PAGE_ICON_IDX( 0 ), SUB_PAGE_ICON_IDX( 0 ), nextAvailableTileIdx,
                                               0, 0, 32, 32, memoPal,
                                               memoTiles, memoTilesLen, false, false, true, OBJPRIORITY_0, true );
        nextAvailableTileIdx = IO::loadSprite( SUB_PAGE_ICON_IDX( 1 ), SUB_PAGE_ICON_IDX( 1 ), nextAvailableTileIdx,
                                               0, 0, 32, 32, atksPal,
                                               atksTiles, atksTilesLen, false, false, true, OBJPRIORITY_0, true );
        nextAvailableTileIdx = IO::loadSprite( SUB_PAGE_ICON_IDX( 2 ), SUB_PAGE_ICON_IDX( 2 ), nextAvailableTileIdx,
                                               0, 0, 32, 32, ContestPal,
                                               ContestTiles, ContestTilesLen, false, false, true, OBJPRIORITY_0, true );

        nextAvailableTileIdx = IO::loadTypeIcon( Type( 0 ), 256 - 68, 24, SUB_TYPE_IDX( 0 ), SUB_TYPE_IDX( 0 ), nextAvailableTileIdx, true );
        nextAvailableTileIdx = IO::loadTypeIcon( Type( 0 ), 256 - 32, 24, SUB_TYPE_IDX( 1 ), SUB_TYPE_IDX( 1 ), nextAvailableTileIdx, true );
        IO::Oam->oamBuffer[ SUB_TYPE_IDX( 0 ) ].isHidden = true;
        IO::Oam->oamBuffer[ SUB_TYPE_IDX( 1 ) ].isHidden = true;

        nextAvailableTileIdx = IO::loadSprite( SUB_RARE_IDX, SUB_RARE_IDX, nextAvailableTileIdx, 176, 8, 8, 8, rare_iconPal,
                                               rare_iconTiles, rare_iconTilesLen, false, false, true, OBJPRIORITY_0, true );
        nextAvailableTileIdx = IO::loadItemIcon( "Pokeball", 256 - 108, 0, SUB_BALL_IDX, SUB_BALL_IDX, nextAvailableTileIdx, true );
        IO::Oam->oamBuffer[ SUB_BALL_IDX ].isHidden = true;
        nextAvailableTileIdx = IO::loadSprite( A_ID, A_ID, nextAvailableTileIdx,
                                               SCREEN_WIDTH / 2 - 16, SCREEN_HEIGHT / 2 - 16, 32, 32, APal,
                                               ATiles, ATilesLen, false, false, false, OBJPRIORITY_0, true );
        nextAvailableTileIdx = IO::loadSprite( SUB_PKRS_IDX, SUB_PKRS_IDX, nextAvailableTileIdx, 208, 4, 64, 32, pokerus_iconPal,
                                               pokerus_iconTiles, pokerus_iconTilesLen, false, false, true, OBJPRIORITY_0, true );


        nextAvailableTileIdx = IO::loadSprite( FWD_ID, FWD_ID, nextAvailableTileIdx,
                                               SCREEN_WIDTH - 28, SCREEN_HEIGHT - 28, 32, 32, DownPal,
                                               DownTiles, DownTilesLen, false, false, true, OBJPRIORITY_1, true );
        nextAvailableTileIdx = IO::loadSprite( BWD_ID, BWD_ID, nextAvailableTileIdx,
                                               SCREEN_WIDTH - 28, SCREEN_HEIGHT - 28, 32, 32, UpPal,
                                               UpTiles, UpTilesLen, false, false, true, OBJPRIORITY_1, true );
        u16 nextnext, nn2;

        for( u8 i = 0; i < 6; ++i ) {
            nextnext = IO::loadSprite( 2 * i + CHOICE_ID, CHOICE_ID, nextAvailableTileIdx,
                                       ( ( i % 2 ) ? 32 : 128 ), 68 + ( i / 2 ) * 32, 64, 32, Choice_1Pal,
                                       Choice_1Tiles, Choice_1TilesLen, false, false, true, OBJPRIORITY_2, true );

            nn2 = IO::loadSprite( 2 * i + CHOICE_ID + 1, CHOICE_ID, nextnext,
                                  ( ( i % 2 ) ? 62 : 160 ), 68 + ( i / 2 ) * 32, 64, 32, Choice_1Pal,
                                  Choice_3Tiles, Choice_3TilesLen, false, false, true, OBJPRIORITY_2, true );
        }
        for( u8 i = 0; i < 3; ++i ) {
            nextAvailableTileIdx = IO::loadSprite( i + CHOICE_ID + 12, CHOICE_ID, nn2,
                                                   96, 68 + (i) * 32, 64, 32, Choice_1Pal,
                                                   Choice_2Tiles, Choice_2TilesLen, false, false, true, OBJPRIORITY_2, true );
        }

        if( p_pokemon.m_boxdata.m_individualValues.m_isEgg ) {
            consoleSelect( &IO::Top );
            IO::updateOAM( true );
            return;
        }
        _showMoveCnt = 0;
        for( u8 i = 0; i < 4; ++i ) {
            if( p_pokemon.m_boxdata.m_moves[ i ] < MAXATTACK &&
                AttackList[ p_pokemon.m_boxdata.m_moves[ i ] ]->m_isFieldAttack ) {
                IO::Oam->oamBuffer[ CHOICE_ID + 2 * ( _showMoveCnt ) ].isHidden = false;
                IO::Oam->oamBuffer[ CHOICE_ID + 1 + 2 * ( _showMoveCnt ) ].isHidden = false;
                IO::Oam->oamBuffer[ CHOICE_ID + 2 * ( _showMoveCnt ) ].y = -7 + 24 * _showMoveCnt;
                IO::Oam->oamBuffer[ CHOICE_ID + 1 + 2 * ( _showMoveCnt ) ].y = -7 + 24 * _showMoveCnt;

                IO::Oam->oamBuffer[ CHOICE_ID + 2 * ( _showMoveCnt ) ].x = 152;
                IO::Oam->oamBuffer[ CHOICE_ID + 1 + 2 * ( _showMoveCnt ) ].x = 192 + 24;

                consoleSetWindow( &IO::Bottom, 16, 3 * _showMoveCnt + 1, 16, 16 );
                printf( "    %s", AttackList[ p_pokemon.m_boxdata.m_moves[ i ] ]->m_moveName.c_str( ) );
                ++_showMoveCnt;
            }
        }
        _showTakeItem = false;
        if( p_pokemon.m_boxdata.m_holdItem ) {
            _showTakeItem = true;

            IO::Oam->oamBuffer[ CHOICE_ID + 2 * ( _showMoveCnt ) ].isHidden = false;
            IO::Oam->oamBuffer[ CHOICE_ID + 1 + 2 * ( _showMoveCnt ) ].isHidden = false;
            IO::Oam->oamBuffer[ CHOICE_ID + 2 * ( _showMoveCnt ) ].y = -7 + 24 * _showMoveCnt;
            IO::Oam->oamBuffer[ CHOICE_ID + 1 + 2 * ( _showMoveCnt ) ].y = -7 + 24 * _showMoveCnt;

            IO::Oam->oamBuffer[ CHOICE_ID + 2 * ( _showMoveCnt ) ].x = 152;
            IO::Oam->oamBuffer[ CHOICE_ID + 1 + 2 * ( _showMoveCnt ) ].x = 192 + 24;

            consoleSetWindow( &IO::Bottom, 16, 3 * _showMoveCnt + 1, 16, 16 );
            printf( "    Item nehmen" );
        }
        u16 s = _showMoveCnt + _showTakeItem;
        IO::Oam->oamBuffer[ CHOICE_ID + 2 * ( s ) ].isHidden = false;
        IO::Oam->oamBuffer[ CHOICE_ID + 1 + 2 * ( s ) ].isHidden = false;
        IO::Oam->oamBuffer[ CHOICE_ID + 2 * ( s ) ].y = -7 + 24 * s;
        IO::Oam->oamBuffer[ CHOICE_ID + 1 + 2 * ( s ) ].y = -7 + 24 * s;

        IO::Oam->oamBuffer[ CHOICE_ID + 2 * ( s ) ].x = 152;
        IO::Oam->oamBuffer[ CHOICE_ID + 1 + 2 * ( s ) ].x = 192 + 24;
        IO::updateOAM( true );

        consoleSetWindow( &IO::Bottom, 16, 3 * s + 1, 16, 16 );
        printf( "    Dexeintrag" );
        consoleSelect( &IO::Top );
    }

    void regStsScreenUI::initTop( ) {
        IO::initOAMTable( false );
        consoleSelect( &IO::Top );
        printf( "\x1b[39m" );
        u16 tileCnt = 0;
        IO::regularFont->setColor( 0, 0 );
        IO::regularFont->setColor( BLACK_IDX, 1 );
        IO::regularFont->setColor( GRAY_IDX, 2 );

        BG_PALETTE[ WHITE_IDX ] = WHITE;
        BG_PALETTE[ GRAY_IDX ] = GRAY;
        BG_PALETTE[ BLACK_IDX ] = BLACK;

        for( size_t i = 0; i < FS::SAV->getTeamPkmnCount( ); i++ ) {
            if( !FS::SAV->m_pkmnTeam[ i ].m_boxdata.m_individualValues.m_isEgg ) {
                if( i % 2 == 0 ) {
                    tileCnt = IO::loadPKMNIcon( FS::SAV->m_pkmnTeam[ i ].m_boxdata.m_speciesId,
                                                u16( borders[ i ][ 0 ] * 8 - 28 ), u16( borders[ i ][ 1 ] * 8 ),
                                                ICON_IDX( i ), ICON_PAL( i ), tileCnt, false );
                    IO::displayHP( 100, 101, borders[ i ][ 0 ] * 8 - 13, borders[ i ][ 1 ] * 8 + 8 - ( i != 2 ? 4 : 0 ), 142 + 2 * i, 143 + 2 * i, false, true );
                    IO::displayHP( 100, 100 - FS::SAV->m_pkmnTeam[ i ].m_stats.m_acHP * 100 / FS::SAV->m_pkmnTeam[ i ].m_stats.m_maxHP, borders[ i ][ 0 ] * 8 - 13,
                                   borders[ i ][ 1 ] * 8 + 8 - ( i != 2 ? 4 : 0 ), 142 + 2 * i, 143 + 2 * i, false, true );
                } else {
                    tileCnt = IO::loadPKMNIcon( FS::SAV->m_pkmnTeam[ i ].m_boxdata.m_speciesId,
                                                u16( borders[ i ][ 0 ] * 8 + 76 ), u16( borders[ i ][ 1 ] * 8 ),
                                                ICON_IDX( i ), ICON_PAL( i ), tileCnt, false );
                    IO::displayHP( 100, 101, borders[ i ][ 0 ] * 8 + 63, borders[ i ][ 1 ] * 8 + 8 - ( i != 3 ? 4 : 0 ), 142 + 2 * i, 143 + 2 * i, false, true );
                    IO::displayHP( 100, 100 - FS::SAV->m_pkmnTeam[ i ].m_stats.m_acHP * 100 / FS::SAV->m_pkmnTeam[ i ].m_stats.m_maxHP, borders[ i ][ 0 ] * 8 + 63,
                                   borders[ i ][ 1 ] * 8 + 8 - ( i != 3 ? 4 : 0 ), 142 + 2 * i, 143 + 2 * i, false, true );
                }
                char buffer[ 100 ];

                u8 mval = 1 + ( ( i / 2 == 1 ) ? 4 : 8 );

                sprintf( buffer, "%ls", FS::SAV->m_pkmnTeam[ i ].m_boxdata.m_name );
                IO::regularFont->printString( buffer, borders[ i ][ 0 ] * 8, borders[ i ][ 1 ] * 8 - mval, false );
                sprintf( buffer, "%ls", getWDisplayName( FS::SAV->m_pkmnTeam[ i ].m_boxdata.m_speciesId ) );
                IO::regularFont->printString( buffer, borders[ i ][ 0 ] * 8, borders[ i ][ 1 ] * 8 + 14 - mval, false );

                IO::regularFont->setColor( 142 + 2 * i, 2 );
                if( FS::SAV->m_pkmnTeam[ i ].m_stats.m_acHP )
                    sprintf( buffer, "%hi/%hi KP", FS::SAV->m_pkmnTeam[ i ].m_stats.m_acHP, FS::SAV->m_pkmnTeam[ i ].m_stats.m_maxHP );
                else
                    sprintf( buffer, "Besiegt" );
                IO::regularFont->printString( buffer, borders[ i ][ 0 ] * 8, borders[ i ][ 1 ] * 8 + 28 - mval, false );
                IO::regularFont->setColor( GRAY_IDX, 2 );

                sprintf( buffer, "%s", ItemList[ FS::SAV->m_pkmnTeam[ i ].m_boxdata.getItem( ) ]->getDisplayName( true ).c_str( ) );
                IO::regularFont->printString( buffer, borders[ i ][ 0 ] * 8, borders[ i ][ 1 ] * 8 + 42 - mval, false );

            } else {
                consoleSetWindow( &IO::Top, borders[ i ][ 0 ], borders[ i ][ 1 ], 12, 6 );

                char buffer[ 100 ];
                int mval = 1 + ( ( i / 2 == 1 ) ? 4 : 8 );
                sprintf( buffer, "Ei" );
                IO::regularFont->printString( buffer, borders[ i ][ 0 ] * 8, borders[ i ][ 1 ] * 8 - mval, false );
                sprintf( buffer, "Ei" );
                IO::regularFont->printString( buffer, borders[ i ][ 0 ] * 8, borders[ i ][ 1 ] * 8 + 14 - mval, false );

                sprintf( buffer, "%s", ItemList[ FS::SAV->m_pkmnTeam[ i ].m_boxdata.getItem( ) ]->getDisplayName( true ).c_str( ) );
                IO::regularFont->printString( buffer, borders[ i ][ 0 ] * 8, borders[ i ][ 1 ] * 8 + 42 - mval, false );

                if( i % 2 == 0 ) {
                    tileCnt = IO::loadEggIcon( u16( borders[ i ][ 0 ] * 8 - 28 ), u16( borders[ i ][ 1 ] * 8 ), ICON_IDX( i ), ICON_PAL( i ), tileCnt, false );
                } else {
                    tileCnt = IO::loadEggIcon( u16( borders[ i ][ 0 ] * 8 + 76 ), u16( borders[ i ][ 1 ] * 8 ), ICON_IDX( i ), ICON_PAL( i ), tileCnt, false );
                }
            }
        }

        //Preload the page specific sprites
        auto currPkmn = FS::SAV->m_pkmnTeam[ _current ];

        IO::OamTop->oamBuffer[ PKMN_SPRITE_START ].gfxIndex = tileCnt;
        tileCnt += 144;

        tileCnt = IO::loadSprite( PAGE_ICON_IDX, PAGE_ICON_PAL, tileCnt,
                                  0, 0, 32, 32, memoPal, memoTiles, memoTilesLen, false, false, false, OBJPRIORITY_0, false );
        tileCnt = IO::loadItemIcon( ItemList[ currPkmn.m_boxdata.getItem( ) ]->m_itemName, 2, 152,
                                    ITEM_ICON_IDX, ITEM_ICON_PAL, tileCnt, false );

        for( u8 i = 0; i < 4; ++i ) {
            Type t = AttackList[ currPkmn.m_boxdata.m_moves[ i ] ]->m_moveType;
            tileCnt = IO::loadTypeIcon( t, 126, 43 + 32 * i, TYPE_IDX + i, TYPE_PAL( i ), tileCnt, false );
        }
        for( u8 i = 0; i < 4; ++i ) {
            tileCnt = IO::loadDamageCategoryIcon( ( move::moveHitTypes )( i % 3 ), 126, 43 + 32 * i,
                                                  ATK_DMGTYPE_IDX( i ), DMG_TYPE_PAL( i % 3 ), tileCnt, false );
        }

        for( u8 i = PKMN_SPRITE_START; i < RIBBON_IDX; ++i )
            IO::OamTop->oamBuffer[ i ].isHidden = true;
        IO::updateOAM( false );
    }

    void regStsScreenUI::init( u8 p_current, bool p_initTop ) {
        _current = p_current;
        if( p_initTop ) {
            IO::vramSetup( );
            swiWaitForVBlank( );
            IO::Top = *consoleInit( &IO::Top, 0, BgType_Text4bpp, BgSize_T_256x256, 2, 0, true, true );
            consoleSetFont( &IO::Top, IO::consoleFont );
            FS::readPictureData( bgGetGfxPtr( IO::bg3 ), "nitro:/PICS/", "PKMNScreen" );
            dmaFillWords( 0, bgGetGfxPtr( IO::bg2 ), 256 * 192 );
        }
        IO::Bottom = *consoleInit( &IO::Bottom, 0, BgType_Text4bpp, BgSize_T_256x256, 2, 0, false, true );
        consoleSetFont( &IO::Bottom, IO::consoleFont );

        consoleSelect( &IO::Top );
        consoleSetWindow( &IO::Top, 0, 0, 32, 24 );
        consoleClear( );
        consoleSetWindow( &IO::Bottom, 0, 0, 32, 24 );
        consoleSelect( &IO::Bottom );
        consoleClear( );

        bgUpdate( );

        IO::drawSub( );
        if( p_initTop )
            initTop( );
        initSub( FS::SAV->m_pkmnTeam[ _current ] );

        consoleSetWindow( &IO::Top, positions[ _current ][ 0 ], positions[ _current ][ 1 ], 2, 2 );
        if( _current & 1 )
            printf( ">" );
        else
            printf( "<" );
    }

    void hideSprites( bool p_bottom ) {
        auto Oam = p_bottom ? IO::Oam : IO::OamTop;
        for( u8 i = 0; i < 6; ++i )
            Oam->oamBuffer[ ICON_IDX( i ) ].isHidden = true;
        for( u8 i = 0; i < 4; ++i )
            Oam->oamBuffer[ TYPE_IDX + i ].isHidden = true;
        for( u8 i = 0; i < 4; ++i )
            Oam->oamBuffer[ ATK_DMGTYPE_IDX( i ) ].isHidden = true;
        for( u8 i = 0; i < 12; ++i )
            Oam->oamBuffer[ RIBBON_IDX + i ].isHidden = true;
    }

    pokemonData data;

    void drawPkmnInformation( const pokemon& p_pokemon, u8& p_page, bool p_newpok, bool p_bottom ) {
        dmaFillWords( 0, bgGetGfxPtr( p_bottom ? IO::bg2sub : IO::bg2 ), 256 * 192 );
        auto Oam = p_bottom ? IO::Oam : IO::OamTop;
        auto pal = BG_PAL( p_bottom );
        if( p_newpok )
            IO::loadItemIcon( !p_pokemon.m_boxdata.m_ball ? "Pokeball" : ItemList[ p_pokemon.m_boxdata.m_ball ]->m_itemName,
                              -6, 26, SHINY_IDX, SHINY_PAL, 1000, p_bottom );

        if( !p_pokemon.m_boxdata.m_individualValues.m_isEgg ) {
            pal[ RED_IDX ] = RED;
            pal[ BLUE_IDX ] = BLUE;
            pal[ RED2_IDX ] = RED2;
            pal[ BLUE2_IDX ] = BLUE2;
            pal[ WHITE_IDX ] = WHITE;
            IO::regularFont->setColor( WHITE_IDX, 1 );

            consoleSelect( p_bottom ? &IO::Bottom : &IO::Top );
            consoleSetWindow( p_bottom ? &IO::Bottom : &IO::Top, 20, 1, 13, 2 );

            IO::regularFont->printString( p_pokemon.m_boxdata.m_name, 150, 2, p_bottom );
            s8 G = p_pokemon.m_boxdata.gender( );

            IO::regularFont->printChar( '/', 234, 2, p_bottom );
            if( p_pokemon.m_boxdata.m_speciesId != 29 && p_pokemon.m_boxdata.m_speciesId != 32 ) {
                if( G == 1 ) {
                    IO::regularFont->setColor( BLUE_IDX, 1 );
                    IO::regularFont->setColor( BLUE2_IDX, 2 );
                    IO::regularFont->printChar( 136, 246, 8, p_bottom );
                } else if( G == -1 ) {
                    IO::regularFont->setColor( RED_IDX, 1 );
                    IO::regularFont->setColor( RED2_IDX, 2 );
                    IO::regularFont->printChar( 137, 246, 8, p_bottom );
                }
            }
            IO::regularFont->setColor( WHITE_IDX, 1 );
            IO::regularFont->setColor( GRAY_IDX, 2 );

            getAll( p_pokemon.m_boxdata.m_speciesId, data );
            IO::regularFont->printString( data.m_displayName, 160, 16, p_bottom );

            if( p_pokemon.m_boxdata.getItem( ) ) {
                IO::regularFont->printString( "Item", 2, 176, p_bottom );
                IO::regularFont->setColor( BLACK_IDX, 1 );
                IO::regularFont->setColor( 0, 2 );
                char buffer[ 200 ];
                sprintf( buffer, "%s: %s", ItemList[ p_pokemon.m_boxdata.getItem( ) ]->getDisplayName( true ).c_str( ),
                         ItemList[ p_pokemon.m_boxdata.getItem( ) ]->getShortDescription( ).c_str( ) );
                IO::regularFont->printString( buffer, 50, 159, p_bottom );
                if( p_newpok ) {
                    IO::loadItemIcon( ItemList[ p_pokemon.m_boxdata.getItem( ) ]->m_itemName, 2, 152,
                                      ITEM_ICON_IDX, ITEM_ICON_PAL, Oam->oamBuffer[ ITEM_ICON_IDX ].gfxIndex, p_bottom );
                }
            } else {
                IO::regularFont->setColor( BLACK_IDX, 1 );
                IO::regularFont->setColor( 0, 2 );
                IO::regularFont->printString( ItemList[ p_pokemon.m_boxdata.getItem( ) ]->getDisplayName( ).c_str( ), 56, 168, p_bottom );
                Oam->oamBuffer[ ITEM_ICON_IDX ].isHidden = true;
            }
            if( p_newpok ) {
                if( p_pokemon.m_boxdata.isShiny( ) ) {
                    IO::regularFont->setColor( RED_IDX, 1 );
                    IO::regularFont->setColor( RED2_IDX, 2 );
                    IO::regularFont->printString( "*", 1, 51, p_bottom );
                    IO::regularFont->setColor( WHITE_IDX, 1 );
                    IO::regularFont->setColor( GRAY_IDX, 2 );
                }
                if( p_pokemon.m_boxdata.isCloned( ) ) {
                    IO::regularFont->printString( "*", 20, 32, p_bottom );
                }
            }
            IO::regularFont->setColor( BLACK_IDX, 1 );
            IO::regularFont->setColor( GRAY_IDX, 2 );

            consoleSelect( p_bottom ? &IO::Bottom : &IO::Top );
            consoleSetWindow( p_bottom ? &IO::Bottom : &IO::Top, 4, 5, 12, 2 );

            if( p_newpok )
                if( !IO::loadPKMNSprite( "nitro:/PICS/SPRITES/PKMN/", p_pokemon.m_boxdata.m_speciesId, 16, 48,
                                         PKMN_SPRITE_START, PKMN_SPRITE_PAL, Oam->oamBuffer[ PKMN_SPRITE_START ].gfxIndex, p_bottom,
                                         p_pokemon.m_boxdata.isShiny( ), p_pokemon.m_boxdata.m_isFemale, true ) ) {
                    IO::loadPKMNSprite( "nitro:/PICS/SPRITES/PKMN/", p_pokemon.m_boxdata.m_speciesId, 16, 48,
                                        PKMN_SPRITE_START, PKMN_SPRITE_PAL, Oam->oamBuffer[ PKMN_SPRITE_START ].gfxIndex, p_bottom,
                                        p_pokemon.m_boxdata.isShiny( ), !p_pokemon.m_boxdata.m_isFemale, true );
                }

            u16 exptype = data.m_expType;

            printf( "EP %3lu%%\nKP %3i%%", ( p_pokemon.m_boxdata.m_experienceGained - EXP[ p_pokemon.m_Level - 1 ][ exptype ] ) * 100 /
                    ( EXP[ p_pokemon.m_Level ][ exptype ] - EXP[ p_pokemon.m_Level - 1 ][ exptype ] ),
                    p_pokemon.m_stats.m_acHP * 100 / p_pokemon.m_stats.m_maxHP );
            IO::displayHP( 100, 101, 46, 80, 97, 98, false, 50, 56, p_bottom );
            IO::displayHP( 100, 100 - p_pokemon.m_stats.m_acHP * 100 / p_pokemon.m_stats.m_maxHP, 46, 80, 97, 98, false, 50, 56, p_bottom );

            IO::displayEP( 100, 101, 46, 80, 99, 100, false, 59, 62, p_bottom );
            IO::displayEP( 0, ( p_pokemon.m_boxdata.m_experienceGained - EXP[ p_pokemon.m_Level - 1 ][ exptype ] ) * 100 /
                           ( EXP[ p_pokemon.m_Level ][ exptype ] - EXP[ p_pokemon.m_Level - 1 ][ exptype ] ), 46, 80, 99, 100, false, 59, 62, p_bottom );

        } else {
            p_page = -1;
            pal[ WHITE_IDX ] = WHITE;
            IO::regularFont->setColor( WHITE_IDX, 1 );

            IO::regularFont->printString( "Ei", 150, 2, p_bottom );
            IO::regularFont->printChar( '/', 234, 2, p_bottom );
            IO::regularFont->printString( "Ei", 160, 18, p_bottom );
            IO::regularFont->setColor( BLACK_IDX, 1 );

            for( u8 i = 0; i < 4; ++i )
                Oam->oamBuffer[ PKMN_SPRITE_START + i ].isHidden = true;
            Oam->oamBuffer[ ITEM_ICON_IDX ].isHidden = true;
        }
    }

    void drawPkmnStats( const pokemon& p_pokemon, bool p_bottom ) {
        auto currPkmn = p_pokemon;
        auto Oam = p_bottom ? IO::Oam : IO::OamTop;
        auto pal = BG_PAL( p_bottom );

        IO::loadSprite( PAGE_ICON_IDX, PAGE_ICON_PAL, Oam->oamBuffer[ PAGE_ICON_IDX ].gfxIndex,
                        0, 0, 32, 32, memoPal, memoTiles, memoTilesLen, false, false, false, OBJPRIORITY_0, p_bottom );

        IO::regularFont->printString( "Statuswerte", 36, 4, p_bottom );
        if( !( currPkmn.m_boxdata.m_individualValues.m_isEgg ) ) {

            pal[ RED_IDX ] = RED;
            pal[ BLUE_IDX ] = BLUE;
            pal[ RED2_IDX ] = RED2;
            pal[ BLUE2_IDX ] = BLUE2;

            char buffer[ 50 ];
            sprintf( buffer, "Status auf Lv.%3i:", currPkmn.m_Level );
            IO::regularFont->printString( buffer, 110, 34, p_bottom );

            sprintf( buffer, "KP                     %3i", currPkmn.m_stats.m_maxHP );
            IO::regularFont->printString( buffer, 130, 50, p_bottom );

            if( NatMod[ currPkmn.m_boxdata.getNature( ) ][ 0 ] == 1.1 ) {
                IO::regularFont->setColor( RED_IDX, 1 ); IO::regularFont->setColor( RED2_IDX, 2 );
            } else if( NatMod[ currPkmn.m_boxdata.getNature( ) ][ 0 ] == 0.9 ) {
                IO::regularFont->setColor( BLUE_IDX, 1 ); IO::regularFont->setColor( BLUE2_IDX, 2 );
            } else {
                IO::regularFont->setColor( BLACK_IDX, 1 ); IO::regularFont->setColor( GRAY_IDX, 2 );
            }
            sprintf( buffer, "ANG                   %3i", currPkmn.m_stats.m_Atk );
            IO::regularFont->printString( buffer, 130, 69, p_bottom );

            if( NatMod[ currPkmn.m_boxdata.getNature( ) ][ 1 ] == 1.1 ) {
                IO::regularFont->setColor( RED_IDX, 1 ); IO::regularFont->setColor( RED2_IDX, 2 );
            } else if( NatMod[ currPkmn.m_boxdata.getNature( ) ][ 1 ] == 0.9 ) {
                IO::regularFont->setColor( BLUE_IDX, 1 ); IO::regularFont->setColor( BLUE2_IDX, 2 );
            } else {
                IO::regularFont->setColor( BLACK_IDX, 1 ); IO::regularFont->setColor( GRAY_IDX, 2 );
            }
            sprintf( buffer, "VER                   %3i", currPkmn.m_stats.m_Def );
            IO::regularFont->printString( buffer, 130, 86, p_bottom );

            if( NatMod[ currPkmn.m_boxdata.getNature( ) ][ 3 ] == 1.1 ) {
                IO::regularFont->setColor( RED_IDX, 1 ); IO::regularFont->setColor( RED2_IDX, 2 );
            } else if( NatMod[ currPkmn.m_boxdata.getNature( ) ][ 3 ] == 0.9 ) {
                IO::regularFont->setColor( BLUE_IDX, 1 ); IO::regularFont->setColor( BLUE2_IDX, 2 );
            } else {
                IO::regularFont->setColor( BLACK_IDX, 1 ); IO::regularFont->setColor( GRAY_IDX, 2 );
            }
            sprintf( buffer, "SAN                   %3i", currPkmn.m_stats.m_SAtk );
            IO::regularFont->printString( buffer, 130, 103, p_bottom );

            if( NatMod[ currPkmn.m_boxdata.getNature( ) ][ 4 ] == 1.1 ) {
                IO::regularFont->setColor( RED_IDX, 1 ); IO::regularFont->setColor( RED2_IDX, 2 );
            } else if( NatMod[ currPkmn.m_boxdata.getNature( ) ][ 4 ] == 0.9 ) {
                IO::regularFont->setColor( BLUE_IDX, 1 ); IO::regularFont->setColor( BLUE2_IDX, 2 );
            } else {
                IO::regularFont->setColor( BLACK_IDX, 1 ); IO::regularFont->setColor( GRAY_IDX, 2 );
            }
            sprintf( buffer, "SVE                   %3i", currPkmn.m_stats.m_SDef );
            IO::regularFont->printString( buffer, 130, 120, p_bottom );

            if( NatMod[ currPkmn.m_boxdata.getNature( ) ][ 2 ] == 1.1 ) {
                IO::regularFont->setColor( RED_IDX, 1 ); IO::regularFont->setColor( RED2_IDX, 2 );
            } else if( NatMod[ currPkmn.m_boxdata.getNature( ) ][ 2 ] == 0.9 ) {
                IO::regularFont->setColor( BLUE_IDX, 1 ); IO::regularFont->setColor( BLUE2_IDX, 2 );
            } else {
                IO::regularFont->setColor( BLACK_IDX, 1 ); IO::regularFont->setColor( GRAY_IDX, 2 );
            }
            sprintf( buffer, "INI                   \xC3\xC3""%3i", currPkmn.m_stats.m_Spd );
            IO::regularFont->printString( buffer, 130, 137, p_bottom );

            IO::printRectangle( (u8) 158, (u8) 52, u8( 158 + 68 ), u8( 52 + 12 ), p_bottom, false, (u8) 251 );

            IO::printRectangle( (u8) 158, (u8) 52,
                                u8( 158 + ( 68.0 * currPkmn.m_boxdata.IVget( 0 ) / 31 ) ),
                                u8( 52 + 6 ), p_bottom, false, u8( 7 * 16 - 1 ) );
            IO::printRectangle( (u8) 158, u8( 52 + 6 ),
                                u8( 158 + ( 68.0*currPkmn.m_boxdata.m_effortValues[ 0 ] / 252 ) ),
                                u8( 52 + 12 ), p_bottom, false, u8( 7 * 16 - 1 ) );

            for( int i = 1; i < 6; ++i ) {
                IO::printRectangle( (u8) 158, u8( 54 + ( 17 * i ) ),
                                    u8( 158 + 68 ), u8( 54 + 12 + ( 17 * i ) ), p_bottom, false, (u8) 251 );
                IO::printRectangle( (u8) 158, u8( 54 + ( 17 * i ) ),
                                    u8( 158 + ( 68.0*currPkmn.m_boxdata.IVget( i ) / 31 ) ),
                                    u8( 54 + 6 + ( 17 * i ) ), p_bottom, false, u8( ( 7 + i ) * 16 - 1 ) );
                IO::printRectangle( (u8) 158, u8( 54 + 6 + ( 17 * i ) ),
                                    u8( 158 + ( 68.0*currPkmn.m_boxdata.m_effortValues[ i ] / 252 ) ),
                                    u8( 54 + 12 + ( 17 * i ) ), p_bottom, false, u8( ( 7 + i ) * 16 - 1 ) );
            }
        } else {
            if( currPkmn.m_boxdata.m_steps > 10 ) {
                IO::regularFont->printString( "Was da wohl", 16 * 8, 50, p_bottom );
                IO::regularFont->printString( "schl�pfen wird?", 16 * 8, 70, p_bottom );
                IO::regularFont->printString( "Es dauert wohl", 16 * 8, 100, p_bottom );
                IO::regularFont->printString( "noch lange.", 16 * 8, 120, p_bottom );
            } else if( currPkmn.m_boxdata.m_steps > 5 ) {
                IO::regularFont->printString( "Hat es sich", 16 * 8, 50, p_bottom );
                IO::regularFont->printString( "gerade bewegt?", 16 * 8, 70, p_bottom );
                IO::regularFont->printString( "Da tut sich", 16 * 8, 100, p_bottom );
                IO::regularFont->printString( "wohl bald was.", 16 * 8, 120, p_bottom );
            } else {
                IO::regularFont->printString( "Jetzt macht es", 16 * 8, 50, p_bottom );
                IO::regularFont->printString( "schon Ger�usche!", 16 * 8, 70, p_bottom );
                IO::regularFont->printString( "Bald ist es", 16 * 8, 100, p_bottom );
                IO::regularFont->printString( "wohl soweit.", 16 * 8, 120, p_bottom );
            }
        }
    }

    void drawPkmnMoves( const pokemon& p_pokemon, bool p_bottom ) {
        auto currPkmn = p_pokemon;
        auto Oam = p_bottom ? IO::Oam : IO::OamTop;
        auto pal = BG_PAL( p_bottom );

        IO::regularFont->printString( "Attacken", 36, 4, p_bottom );
        IO::loadSprite( PAGE_ICON_IDX, PAGE_ICON_PAL, Oam->oamBuffer[ PAGE_ICON_IDX ].gfxIndex,
                        0, 0, 32, 32, atksPal, atksTiles, atksTilesLen, false, false, false, OBJPRIORITY_0, p_bottom );

        consoleSelect( p_bottom ? &IO::Bottom : &IO::Top );
        consoleSetWindow( p_bottom ? &IO::Bottom : &IO::Top, 16, 5, 32, 24 );
        for( int i = 0; i < 4; i++ ) {
            if( !currPkmn.m_boxdata.m_moves[ i ] )
                continue;
            Type t = AttackList[ currPkmn.m_boxdata.m_moves[ i ] ]->m_moveType;
            IO::loadTypeIcon( t, 222, 42 + 30 * i, TYPE_IDX + i, TYPE_PAL( i ),
                              Oam->oamBuffer[ TYPE_IDX + i ].gfxIndex, p_bottom );

            pal[ COLOR_IDX ] = GREEN;
            if( t == data.m_types[ 0 ] || t == data.m_types[ 1 ] ) {
                IO::regularFont->setColor( COLOR_IDX, 1 );
                IO::regularFont->setColor( WHITE_IDX, 2 );
            } else {
                IO::regularFont->setColor( BLACK_IDX, 1 );
                IO::regularFont->setColor( GRAY_IDX, 2 );
            }

            IO::regularFont->printString( AttackList[ currPkmn.m_boxdata.m_moves[ i ] ]->m_moveName.c_str( ),
                                          128, 34 + 30 * i, p_bottom );

            IO::regularFont->setColor( GRAY_IDX, 1 );
            IO::regularFont->setColor( WHITE_IDX, 2 );
            sprintf( buffer, "AP %2hhu/%2hhu ", currPkmn.m_boxdata.m_acPP[ i ],
                     s8( AttackList[ currPkmn.m_boxdata.m_moves[ i ] ]->m_movePP * ( ( 5 + currPkmn.m_boxdata.PPupget( i ) ) / 5.0 ) ) );
            IO::regularFont->printString( buffer, 135, 49 + 30 * i, p_bottom );
        }
        IO::regularFont->setColor( BLACK_IDX, 1 );
        IO::regularFont->setColor( GRAY_IDX, 2 );
    }

    void drawPkmnRibbons( const pokemon& p_pokemon, bool p_bottom ) {
        auto currPkmn = p_pokemon;
        auto Oam = p_bottom ? IO::Oam : IO::OamTop;

        IO::loadSprite( PAGE_ICON_IDX, PAGE_ICON_PAL, Oam->oamBuffer[ PAGE_ICON_IDX ].gfxIndex,
                        0, 0, 32, 32, ContestPal, ContestTiles, ContestTilesLen, false, false, false, OBJPRIORITY_0, p_bottom );
        IO::regularFont->printString( "B�nder", 36, 4, p_bottom );

        auto rbs = ribbon::getRibbons( currPkmn );
        //Just draw the first 12 ribbons at max
        u16 tileCnt = Oam->oamBuffer[ TYPE_IDX ].gfxIndex;
        for( u8 i = 0; i < std::min( rbs.size( ), 12u ); ++i ) {
            u16 tmp = IO::loadRibbonIcon( rbs[ i ], 128 + 32 * ( i % 4 ), 40 + 40 * ( i / 4 ), RIBBON_IDX + i, TYPE_PAL( i ) % 16, tileCnt, p_bottom );
            if( !tmp ) //Draw an egg when something goes wrong
                tileCnt = IO::loadEggIcon( 128 + 32 * ( i % 4 ), 36 + 40 * ( i / 4 ), RIBBON_IDX + i, TYPE_PAL( i ) % 16, tileCnt, p_bottom );
            else
                tileCnt = tmp;
        }
        if( rbs.empty( ) ) {
            IO::regularFont->printString( "Keine B�nder", 148, 87, p_bottom );
        } else {
            sprintf( buffer, "(%u)", rbs.size( ) );
            IO::regularFont->printString( buffer, 88, 4, p_bottom );
        }
    }

    void drawPkmnGeneralData( const pokemon& p_pokemon, bool p_bottom ) {
        auto currPkmn = p_pokemon;
        auto Oam = p_bottom ? IO::Oam : IO::OamTop;

        IO::loadSprite( PAGE_ICON_IDX, PAGE_ICON_PAL, Oam->oamBuffer[ PAGE_ICON_IDX ].gfxIndex,
                        0, 0, 32, 32, memoPal, memoTiles, memoTilesLen, false, false, false, OBJPRIORITY_0, p_bottom );
        IO::regularFont->printString( "Trainer-Memo", 36, 4, p_bottom );

        if( data.m_types[ 0 ] == data.m_types[ 1 ] ) {
            IO::loadTypeIcon( data.m_types[ 0 ], 250 - 32, 54, TYPE_IDX, TYPE_PAL( 0 ),
                              Oam->oamBuffer[ TYPE_IDX ].gfxIndex, p_bottom );
            Oam->oamBuffer[ TYPE_IDX + 1 ].isHidden = true;
        } else {
            IO::loadTypeIcon( data.m_types[ 0 ], 250 - 64, 54, TYPE_IDX, TYPE_PAL( 0 ),
                              Oam->oamBuffer[ TYPE_IDX ].gfxIndex, p_bottom );
            IO::loadTypeIcon( data.m_types[ 1 ], 250 - 32, 54, TYPE_IDX + 1, TYPE_PAL( 1 ),
                              Oam->oamBuffer[ TYPE_IDX + 1 ].gfxIndex, p_bottom );
        }

        IO::regularFont->setColor( BLACK_IDX, 1 ); IO::regularFont->setColor( GRAY_IDX, 2 );

        IO::regularFont->printString( "OT", 110, 34, p_bottom );
        if( currPkmn.m_boxdata.m_oTisFemale ) {
            IO::regularFont->setColor( RED_IDX, 1 );
            IO::regularFont->setColor( RED2_IDX, 2 );
        } else {
            IO::regularFont->setColor( BLUE_IDX, 1 );
            IO::regularFont->setColor( BLUE2_IDX, 2 );
        }
        char buffer[ 50 ];
        sprintf( buffer, "%ls/%05d", currPkmn.m_boxdata.m_oT, currPkmn.m_boxdata.m_oTId );
        IO::regularFont->printString( buffer, 250 - IO::regularFont->stringWidth( buffer ), 34, p_bottom );

        if( !currPkmn.m_boxdata.isShiny( ) )
            IO::regularFont->setColor( BLACK_IDX, 1 ); IO::regularFont->setColor( GRAY_IDX, 2 );
        sprintf( buffer, "%03d", currPkmn.m_boxdata.m_speciesId );
        IO::regularFont->printString( buffer, 180 - IO::regularFont->stringWidth( buffer ), 55, p_bottom );
        IO::regularFont->setColor( BLACK_IDX, 1 ); IO::regularFont->setColor( GRAY_IDX, 2 );
        IO::regularFont->printString( "Nr.", 124, 55, p_bottom );

        bool plrOT = currPkmn.m_boxdata.m_oTId == FS::SAV->m_id && currPkmn.m_boxdata.m_oTSid == FS::SAV->m_sid;
        if( !currPkmn.m_boxdata.m_gotDate[ 0 ] )
            sprintf( buffer, "%s%d",
                     plrOT ? "Gef. auf Lv. " : "Off gef auf Lv.",
                     currPkmn.m_boxdata.m_gotLevel );
        else
            sprintf( buffer, "%s",
                     plrOT ? "Ei erhalten" : "Ei offenbar erh." );
        IO::regularFont->printString( buffer, 250 - IO::regularFont->stringWidth( buffer ), 80, p_bottom, 14 );
        sprintf( buffer, "am %02d.%02d.20%02d,",
                 currPkmn.m_boxdata.m_gotDate[ 0 ] ? currPkmn.m_boxdata.m_gotDate[ 0 ] : currPkmn.m_boxdata.m_hatchDate[ 0 ],
                 currPkmn.m_boxdata.m_gotDate[ 1 ] ? currPkmn.m_boxdata.m_gotDate[ 1 ] : currPkmn.m_boxdata.m_hatchDate[ 1 ],
                 currPkmn.m_boxdata.m_gotDate[ 2 ] ? currPkmn.m_boxdata.m_gotDate[ 2 ] : currPkmn.m_boxdata.m_hatchDate[ 2 ] );
        IO::regularFont->printString( buffer, 250 - IO::regularFont->stringWidth( buffer ), 94, p_bottom, 14 );
        sprintf( buffer, "%s.",
                 FS::getLoc( currPkmn.m_boxdata.m_gotPlace ) );
        IO::regularFont->printMaxString( buffer, std::max( (u32) 124, 250 - IO::regularFont->stringWidth( buffer ) ), 108, p_bottom, 254 );

        if( currPkmn.m_boxdata.m_gotDate[ 0 ] ) {
            sprintf( buffer, "Geschl. %02d.%02d.20%02d,",
                     currPkmn.m_boxdata.m_hatchDate[ 0 ],
                     currPkmn.m_boxdata.m_hatchDate[ 1 ],
                     currPkmn.m_boxdata.m_hatchDate[ 2 ] );
            IO::regularFont->printString( buffer, 250 - IO::regularFont->stringWidth( buffer ), 129, p_bottom, 14 );
            sprintf( buffer, "%s.",
                     FS::getLoc( currPkmn.m_boxdata.m_hatchPlace ) );
            IO::regularFont->printString( buffer, 250 - IO::regularFont->stringWidth( buffer ), 143, p_bottom, 14 );
        } else if( plrOT && currPkmn.m_boxdata.m_fateful )
            IO::regularFont->printString( "Schicksalhafte Begeg.", 102, 143, true );
        else if( currPkmn.m_boxdata.m_fateful )
            IO::regularFont->printString( "Off. schicksal. Begeg.", 102, 143, true );
    }

    void drawPkmnNature( const pokemon& p_pokemon, bool p_bottom ) {
        auto currPkmn = p_pokemon;
        auto Oam = p_bottom ? IO::Oam : IO::OamTop;

        IO::loadSprite( PAGE_ICON_IDX, PAGE_ICON_PAL, Oam->oamBuffer[ PAGE_ICON_IDX ].gfxIndex,
                        0, 0, 32, 32, memoPal, memoTiles, memoTilesLen, false, false, false, OBJPRIORITY_0, p_bottom );
        IO::regularFont->printString( "Pok�mon-Info", 36, 4, p_bottom );

        IO::regularFont->setColor( BLACK_IDX, 1 ); IO::regularFont->setColor( GRAY_IDX, 2 );

        sprintf( buffer, "Mag %s""e Pok�Rg.", TasteList[ currPkmn.m_boxdata.getTasteStr( ) ].c_str( ) );
        IO::regularFont->printString( buffer, 106, 34, p_bottom );

        sprintf( buffer, "Sehr %s; %s.", NatureList[ currPkmn.m_boxdata.getNature( ) ].c_str( ),
                 PersonalityList[ currPkmn.m_boxdata.getPersonality( ) ].c_str( ) );
        auto str = std::string( buffer );
        auto nStr = FS::breakString( str, IO::regularFont, 122 );
        IO::regularFont->printString( nStr.c_str( ), 122, 52, p_bottom, 14, -2 );

        auto acAbility = ability( currPkmn.m_boxdata.m_ability );
        u8 wd = IO::regularFont->stringWidth( acAbility.m_abilityName.c_str( ) );
        if( 253 - wd > 140 )
            IO::regularFont->printString( "F�h.", 126, 98, p_bottom );
        u8 nlCnt = 0;
        nStr = FS::breakString( acAbility.m_flavourText, IO::regularFont, 125 );
        for( auto c : nStr )
            if( c == '\n' )
                nlCnt++;
        IO::regularFont->printString( nStr.c_str( ), 124,
                                      112, p_bottom, u8( 16 - 2 * nlCnt ), 4 );

        if( currPkmn.m_boxdata.m_oTisFemale ) {
            IO::regularFont->setColor( RED_IDX, 1 );
            IO::regularFont->setColor( RED2_IDX, 2 );
        } else {
            IO::regularFont->setColor( BLUE_IDX, 1 );
            IO::regularFont->setColor( BLUE2_IDX, 2 );
        }
        IO::regularFont->printString( acAbility.m_abilityName.c_str( ),
                                      253 - wd,
                                      98, p_bottom );
        IO::regularFont->setColor( BLACK_IDX, 1 ); IO::regularFont->setColor( GRAY_IDX, 2 );
    }

    // Draw extra information about the specified move
    bool statusScreenUI::drawMove( const pokemon& p_pokemon, u8 p_moveIdx, bool p_bottom ) {
        auto currPkmn = p_pokemon;
        auto Oam = p_bottom ? IO::Oam : IO::OamTop;
        auto pal = BG_PAL( p_bottom );

        if( !currPkmn.m_boxdata.m_moves[ p_moveIdx ] )
            return false;
        if( currPkmn.m_boxdata.m_individualValues.m_isEgg )
            return false;

        for( u8 i = 0; i < 4; ++i )
            Oam->oamBuffer[ TYPE_IDX + i ].isHidden = true;
        for( u8 i = 0; i < 4; ++i )
            Oam->oamBuffer[ ATK_DMGTYPE_IDX( i ) ].isHidden = true;

        IO::setDefaultConsoleTextColors( pal, 6 );
        FS::readPictureData( bgGetGfxPtr( p_bottom ? IO::bg3sub : IO::bg3 ), "nitro:/PICS/", "PKMNInfoScreen", 128 );

        pal[ WHITE_IDX ] = WHITE;
        pal[ GRAY_IDX ] = RGB( 20, 20, 20 );
        pal[ BLACK_IDX ] = BLACK;
        IO::regularFont->setColor( 0, 0 );
        IO::regularFont->setColor( BLACK_IDX, 1 );
        IO::regularFont->setColor( GRAY_IDX, 2 );
        IO::boldFont->setColor( 0, 0 );
        IO::boldFont->setColor( GRAY_IDX, 1 );
        IO::boldFont->setColor( WHITE_IDX, 2 );

        consoleSelect( p_bottom ? &IO::Bottom : &IO::Top );
        consoleSetWindow( p_bottom ? &IO::Bottom : &IO::Top, 0, 0, 32, 24 );
        consoleClear( );

        u8 isNotEgg = 1;
        drawPkmnInformation( currPkmn, isNotEgg, false, p_bottom );
        if( !isNotEgg )
            return false; //This should never occur

        IO::regularFont->printString( "Attackeninfos", 36, 4, p_bottom );
        IO::loadSprite( PAGE_ICON_IDX, PAGE_ICON_PAL, Oam->oamBuffer[ PAGE_ICON_IDX ].gfxIndex,
                        0, 0, 32, 32, atksPal, atksTiles, atksTilesLen, false, false, false, OBJPRIORITY_0, p_bottom );

        move* currMove = AttackList[ currPkmn.m_boxdata.m_moves[ p_moveIdx ] ];

        pal[ COLOR_IDX ] = GREEN;
        if( currMove->m_moveType == data.m_types[ 0 ] || currMove->m_moveType == data.m_types[ 1 ] ) {
            IO::regularFont->setColor( COLOR_IDX, 1 );
            IO::regularFont->setColor( WHITE_IDX, 2 );
        } else {
            IO::regularFont->setColor( BLACK_IDX, 1 );
            IO::regularFont->setColor( GRAY_IDX, 2 );
        }
        IO::regularFont->printString( currMove->m_moveName.c_str( ),
                                      120, 36, p_bottom );
        IO::regularFont->setColor( GRAY_IDX, 1 );
        IO::regularFont->setColor( WHITE_IDX, 2 );

        IO::loadTypeIcon( currMove->m_moveType, 222, 34, TYPE_IDX + p_moveIdx, TYPE_PAL( p_moveIdx ),
                          Oam->oamBuffer[ TYPE_IDX + p_moveIdx ].gfxIndex, p_bottom );
        IO::loadDamageCategoryIcon( currMove->m_moveHitType, 222, 50, ATK_DMGTYPE_IDX( currMove->m_moveHitType ), DMG_TYPE_PAL( currMove->m_moveHitType ),
                                    Oam->oamBuffer[ ATK_DMGTYPE_IDX( currMove->m_moveHitType ) ].gfxIndex, p_bottom );
        char buffer[ 20 ];

        sprintf( buffer, "AP %2hhu""/""%2hhu ", currPkmn.m_boxdata.m_acPP[ p_moveIdx ],
                 currMove->m_movePP * ( ( 5 + ( ( currPkmn.m_boxdata.m_pPUps >> ( 2 * p_moveIdx ) ) % 4 ) ) / 5 ) );
        IO::regularFont->printString( buffer, 128, 51, p_bottom );

        IO::regularFont->printString( "St�rke", 128, 64, p_bottom );
        if( currMove->m_moveBasePower )
            sprintf( buffer, "%3i", currMove->m_moveBasePower );
        else
            sprintf( buffer, "---" );
        IO::regularFont->printString( buffer, 226, 64, p_bottom );

        IO::regularFont->printString( "Genauigkeit", 128, 76, p_bottom );
        if( currMove->m_moveAccuracy )
            sprintf( buffer, "%3i", currMove->m_moveAccuracy );
        else
            sprintf( buffer, "---" );
        IO::regularFont->printString( buffer, 226, 76, p_bottom );

        IO::regularFont->setColor( BLACK_IDX, 1 );
        IO::regularFont->setColor( GRAY_IDX, 2 );
        IO::regularFont->printString( FS::breakString( currMove->description( ), IO::regularFont, 120 ).c_str( ),
                                      128, 88, p_bottom, 11 );

        IO::updateOAM( p_bottom );
        return true;
    }

    // Draw extra information about the specified ribbon
    void statusScreenUI::drawRibbon( const pokemon& p_pokemon, u8 p_ribbonIdx, bool p_bottom )
    {
        auto currPkmn = p_pokemon;
        auto Oam = p_bottom ? IO::Oam : IO::OamTop;
        auto pal = BG_PAL( p_bottom );

        for( u8 i = 0; i < 4; ++i )
            Oam->oamBuffer[ TYPE_IDX + i ].isHidden = true;
        for( u8 i = 0; i < 4; ++i )
            Oam->oamBuffer[ ATK_DMGTYPE_IDX( i ) ].isHidden = true;
        for( u8 i = 0; i < 12; ++i )
            Oam->oamBuffer[ RIBBON_IDX + i ].isHidden = true;

        IO::setDefaultConsoleTextColors( pal, 6 );
        FS::readPictureData( bgGetGfxPtr( p_bottom ? IO::bg3sub : IO::bg3 ), "nitro:/PICS/", "PKMNInfoScreen", 128 );

        pal[ WHITE_IDX ] = WHITE;
        pal[ GRAY_IDX ] = RGB( 20, 20, 20 );
        pal[ BLACK_IDX ] = BLACK;
        IO::regularFont->setColor( 0, 0 );
        IO::regularFont->setColor( BLACK_IDX, 1 );
        IO::regularFont->setColor( GRAY_IDX, 2 );
        IO::boldFont->setColor( 0, 0 );
        IO::boldFont->setColor( GRAY_IDX, 1 );
        IO::boldFont->setColor( WHITE_IDX, 2 );

        consoleSelect( p_bottom ? &IO::Bottom : &IO::Top );
        consoleSetWindow( p_bottom ? &IO::Bottom : &IO::Top, 0, 0, 32, 24 );
        consoleClear( );

        u8 isNotEgg = 1;
        drawPkmnInformation( currPkmn, isNotEgg, false, p_bottom );
        if( !isNotEgg )
            return; //This should never occur

        IO::regularFont->printString( "Bandinfos", 36, 4, p_bottom );
        IO::loadSprite( PAGE_ICON_IDX, PAGE_ICON_PAL, Oam->oamBuffer[ PAGE_ICON_IDX ].gfxIndex,
                        0, 0, 32, 32, atksPal, atksTiles, atksTilesLen, false, false, false, OBJPRIORITY_0, p_bottom );

        ribbon currRb = RibbonList[ p_ribbonIdx ];

        u16 tileCnt = Oam->oamBuffer[ TYPE_IDX ].gfxIndex;

        if( !IO::loadRibbonIcon( p_ribbonIdx, 110, 32, RIBBON_IDX, TYPE_PAL( 0 ), tileCnt, p_bottom ) )
            tileCnt = IO::loadEggIcon( 110, 32, RIBBON_IDX, TYPE_PAL( 0 ), tileCnt, p_bottom );

        u8 nlCnt = 0;
        auto nStr = FS::breakString( currRb.m_name == "" ? ( "----" ) : currRb.m_name, IO::regularFont, 110 );
        for( auto c : nStr )
            if( c == '\n' )
                nlCnt++;

        if( currRb.m_name == "" )
            currRb.m_description = "----";

        IO::regularFont->printString( nStr.c_str( ), 142, 43 - 7 * nlCnt, p_bottom, 14 );
        IO::regularFont->printString( FS::breakString( ( currRb.m_description == "" ) ?
                                                       ( currPkmn.m_boxdata.m_fateful ? "Ein Band f�r Pok�mon-Freunde." : "Ein Gedenk-Band. An einem mysteri�sen Ort erhalten." )
                                                       : currRb.m_description, IO::regularFont, 120 ).c_str( ),
                                      128, 70, p_bottom, 14 );
        IO::updateOAM( p_bottom );
        return;
    }
    void regStsScreenUI::draw( const pokemon& p_pokemon, u8 p_page, bool p_newpok ) {
        hideSprites( false );
        IO::Oam->oamBuffer[ A_ID ].isHidden = true;
        for( u8 u = 1; u < 6; ++u ) {
            IO::Oam->oamBuffer[ CHOICE_ID + 2 * ( u ) ].isHidden = true;
            IO::Oam->oamBuffer[ CHOICE_ID + 1 + 2 * ( u ) ].isHidden = true;
        }

        IO::setDefaultConsoleTextColors( BG_PALETTE, 6 );
        FS::readPictureData( bgGetGfxPtr( IO::bg3 ), "nitro:/PICS/", "PKMNInfoScreen", 128 );
        IO::updateOAM( false );

        IO::regularFont->setColor( 0, 0 );
        { IO::regularFont->setColor( BLACK_IDX, 1 ); IO::regularFont->setColor( GRAY_IDX, 2 ); }
        IO::regularFont->setColor( GRAY_IDX, 2 );
        IO::boldFont->setColor( 0, 0 );
        IO::boldFont->setColor( GRAY_IDX, 1 );
        IO::boldFont->setColor( WHITE_IDX, 2 );

        BG_PALETTE[ WHITE_IDX ] = WHITE;
        BG_PALETTE[ GRAY_IDX ] = RGB( 20, 20, 20 );
        BG_PALETTE[ BLACK_IDX ] = BLACK;
        if( p_newpok ) {
            BG_PALETTE_SUB[ GRAY_IDX ] = RGB( 10, 10, 10 );
            BG_PALETTE_SUB[ WHITE_IDX ] = WHITE;
        }
        consoleSelect( &IO::Top );
        consoleSetWindow( &IO::Top, 0, 0, 32, 24 );
        consoleClear( );
        consoleSelect( &IO::Bottom );
        consoleSetWindow( &IO::Bottom, 0, 0, 32, 24 );
        if( p_newpok )
            consoleClear( );

        auto currPkmn = p_pokemon;

        drawPkmnInformation( currPkmn, p_page, p_newpok, false );
        if( p_page == (u8) -1 )
            p_page = 0;
        switch( p_page ) {
            case 0:
                drawPkmnStats( currPkmn, false );
                break;
            case 1:
                drawPkmnMoves( currPkmn, false );
                break;
            case 2:
                drawPkmnRibbons( currPkmn, false );
                break;
            default:
                return;
        }

        if( !currPkmn.m_boxdata.m_individualValues.m_isEgg ) {
            IO::Oam->oamBuffer[ SUB_PAGE_ICON_IDX( 0 ) ].isHidden = false;
            IO::Oam->oamBuffer[ SUB_PAGE_ICON_IDX( 0 ) ].y = static_cast<u16>( 256 - 4 );
            IO::Oam->oamBuffer[ SUB_PAGE_ICON_IDX( 0 ) ].x = static_cast<u16>( 512 - 8 );
            IO::Oam->oamBuffer[ SUB_PAGE_ICON_IDX( 0 ) ].priority = OBJPRIORITY_1;
            IO::Oam->oamBuffer[ SUB_PAGE_ICON_IDX( 1 ) ].isHidden = false;
            IO::Oam->oamBuffer[ SUB_PAGE_ICON_IDX( 1 ) ].y = 18;
            IO::Oam->oamBuffer[ SUB_PAGE_ICON_IDX( 1 ) ].x = static_cast<u16>( 512 - 8 );
            IO::Oam->oamBuffer[ SUB_PAGE_ICON_IDX( 1 ) ].priority = OBJPRIORITY_1;
            IO::Oam->oamBuffer[ SUB_PAGE_ICON_IDX( 2 ) ].isHidden = false;
            IO::Oam->oamBuffer[ SUB_PAGE_ICON_IDX( 2 ) ].y = static_cast<u16>( 256 - 8 );
            IO::Oam->oamBuffer[ SUB_PAGE_ICON_IDX( 2 ) ].x = 14;
            IO::Oam->oamBuffer[ SUB_PAGE_ICON_IDX( 2 ) ].priority = OBJPRIORITY_1;

            IO::Oam->oamBuffer[ SUB_PAGE_ICON_IDX( p_page ) ].isHidden = true;
        }
        if( p_newpok ) {
            consoleSelect( &IO::Bottom );
            printf( "\x1b[39m" );

            IO::Oam->oamBuffer[ BACK_ID ].x = 256 - 24;
            IO::Oam->oamBuffer[ BACK_ID ].y = 196 - 28;
            IO::Oam->oamBuffer[ FWD_ID ].isHidden = false;
            IO::Oam->oamBuffer[ BWD_ID ].isHidden = false;
            IO::Oam->oamBuffer[ BWD_ID ].x = 256 - 24;
            IO::Oam->oamBuffer[ BWD_ID ].y = 196 - 28 - 22;
            IO::Oam->oamBuffer[ FWD_ID ].x = 256 - 28 - 18;
            IO::Oam->oamBuffer[ FWD_ID ].y = 196 - 28;

            if( !currPkmn.m_boxdata.m_individualValues.m_isEgg ) {

                if( data.m_types[ 0 ] == data.m_types[ 1 ] ) {
                    IO::loadTypeIcon( data.m_types[ 0 ], 256 - 50, 24, SUB_TYPE_IDX( 0 ), SUB_TYPE_IDX( 0 ),
                                      IO::Oam->oamBuffer[ SUB_TYPE_IDX( 0 ) ].gfxIndex, true );
                    IO::Oam->oamBuffer[ SUB_TYPE_IDX( 1 ) ].isHidden = true;
                } else {
                    IO::loadTypeIcon( data.m_types[ 0 ], 256 - 68, 24, SUB_TYPE_IDX( 0 ), SUB_TYPE_IDX( 0 ),
                                      IO::Oam->oamBuffer[ SUB_TYPE_IDX( 0 ) ].gfxIndex, true );
                    IO::loadTypeIcon( data.m_types[ 1 ], 256 - 32, 24, SUB_TYPE_IDX( 1 ), SUB_TYPE_IDX( 1 ),
                                      IO::Oam->oamBuffer[ SUB_TYPE_IDX( 1 ) ].gfxIndex, true );
                }
                IO::loadItemIcon( !currPkmn.m_boxdata.m_ball ? "Pokeball" : ItemList[ currPkmn.m_boxdata.m_ball ]->m_itemName,
                                  256 - 108, 0, SUB_BALL_IDX, SUB_BALL_IDX, IO::Oam->oamBuffer[ SUB_BALL_IDX ].gfxIndex, true );

                IO::Oam->oamBuffer[ CHOICE_ID ].isHidden = false;
                IO::Oam->oamBuffer[ CHOICE_ID ].y = 0;
                IO::Oam->oamBuffer[ CHOICE_ID ].x = 256 - 96;
                IO::Oam->oamBuffer[ CHOICE_ID + 1 ].isHidden = false;
                IO::Oam->oamBuffer[ CHOICE_ID + 1 ].y = 0;
                IO::Oam->oamBuffer[ CHOICE_ID + 1 ].x = 256 - 32;

                consoleSetWindow( &IO::Bottom, 23, 1, 20, 5 );

                if( currPkmn.m_boxdata.isShiny( ) ) {
                    IO::loadSprite( SUB_RARE_IDX, SUB_RARE_IDX, IO::Oam->oamBuffer[ SUB_RARE_IDX ].gfxIndex, 176, 8, 8, 8, rare_iconPal,
                                    rare_iconTiles, rare_iconTilesLen, false, false, false, OBJPRIORITY_0, true );
                } else
                    IO::Oam->oamBuffer[ SUB_RARE_IDX ].isHidden = true;

                if( currPkmn.m_boxdata.m_pokerus ) {
                    IO::loadSprite( SUB_PKRS_IDX, SUB_PKRS_IDX, IO::Oam->oamBuffer[ SUB_PKRS_IDX ].gfxIndex, 208, 4, 64, 32, pokerus_iconPal,
                                    pokerus_iconTiles, pokerus_iconTilesLen, false, false, false, OBJPRIORITY_0, true );
                } else {
                    printf( "Nr. " );
                    IO::Oam->oamBuffer[ SUB_PKRS_IDX ].isHidden = true;
                }
                printf( "%03i", currPkmn.m_boxdata.m_speciesId );
            } else {
                IO::Oam->oamBuffer[ SUB_PAGE_ICON_IDX( 0 ) ].isHidden = true;
                IO::Oam->oamBuffer[ SUB_PAGE_ICON_IDX( 1 ) ].isHidden = true;
                IO::Oam->oamBuffer[ SUB_PAGE_ICON_IDX( 2 ) ].isHidden = true;

                IO::Oam->oamBuffer[ CHOICE_ID ].isHidden = false;
                IO::Oam->oamBuffer[ CHOICE_ID ].y = 0;
                IO::Oam->oamBuffer[ CHOICE_ID ].x = 256 - 32;
                IO::Oam->oamBuffer[ CHOICE_ID + 1 ].isHidden = true;

                IO::Oam->oamBuffer[ SUB_TYPE_IDX( 0 ) ].isHidden = true;
                IO::Oam->oamBuffer[ SUB_TYPE_IDX( 1 ) ].isHidden = true;
                IO::Oam->oamBuffer[ SUB_PKRS_IDX ].isHidden = true;
                IO::Oam->oamBuffer[ SUB_RARE_IDX ].isHidden = true;
                IO::loadItemIcon( !currPkmn.m_boxdata.m_ball ? "Pokeball" : ItemList[ currPkmn.m_boxdata.m_ball ]->m_itemName,
                                  256 - 32, 0, SUB_BALL_IDX, SUB_BALL_IDX, IO::Oam->oamBuffer[ SUB_BALL_IDX ].gfxIndex, true );
            }
            printf( "\x1b[33m" );

            consoleSetWindow( &IO::Bottom, 3, 3, 27, 15 );
            IO::drawSub( );

            IO::regularFont->setColor( 0, 0 );
            IO::regularFont->setColor( WHITE_IDX, 1 );
            IO::regularFont->setColor( BLACK_IDX, 2 );
            BG_PALETTE_SUB[ BLACK_IDX ] = BLACK;
            BG_PALETTE_SUB[ WHITE_IDX ] = WHITE;
            BG_PALETTE_SUB[ RED_IDX ] = RED;
            BG_PALETTE_SUB[ BLUE_IDX ] = BLUE;
            BG_PALETTE_SUB[ RED2_IDX ] = RED2;
            BG_PALETTE_SUB[ BLUE2_IDX ] = BLUE2;

            IO::regularFont->printString( "OT:", 28, 22, true );
            if( currPkmn.m_boxdata.m_oTisFemale ) {
                IO::regularFont->setColor( RED_IDX, 1 );
                IO::regularFont->setColor( RED2_IDX, 2 );
            } else {
                IO::regularFont->setColor( BLUE_IDX, 1 );
                IO::regularFont->setColor( BLUE2_IDX, 2 );
            }
            IO::regularFont->printString( currPkmn.m_boxdata.m_oT, 56, 16, true );
            IO::regularFont->setColor( WHITE_IDX, 1 );
            IO::regularFont->setColor( BLACK_IDX, 2 );

            if( !currPkmn.m_boxdata.m_individualValues.m_isEgg ) {
                auto acAbility = ability( currPkmn.m_boxdata.m_ability );

                BG_PALETTE_SUB[ WHITE_IDX ] = WHITE;
                IO::printRectangle( u8( 0 ), u8( 138 ), u8( 255 ), u8( 192 ), true, false, WHITE_IDX );
                if( currPkmn.m_boxdata.m_oTisFemale ) {
                    IO::regularFont->setColor( RED_IDX, 1 );
                    IO::regularFont->setColor( WHITE_IDX, 2 );
                } else {
                    IO::regularFont->setColor( BLUE_IDX, 1 );
                    IO::regularFont->setColor( WHITE_IDX, 2 );
                }
                u8 nlCnt = 0;
                auto nStr = FS::breakString( acAbility.m_flavourText, IO::regularFont, 220 );
                for( auto c : nStr )
                    if( c == '\n' )
                        nlCnt++;
                IO::regularFont->printString( nStr.c_str( ), 0, 138, true, u8( 16 - 2 * nlCnt ) );
                IO::regularFont->printString( acAbility.m_abilityName.c_str( ), 5, 176, true );
                IO::regularFont->setColor( WHITE_IDX, 1 );
                IO::regularFont->setColor( BLACK_IDX, 2 );
            }

            sprintf( buffer, "(%05i/%05i)", currPkmn.m_boxdata.m_oTId, currPkmn.m_boxdata.m_oTSid );
            IO::regularFont->printString( buffer, 50, 30, true );

            if( currPkmn.m_boxdata.m_oTId == FS::SAV->m_id && currPkmn.m_boxdata.m_oTSid == FS::SAV->m_sid ) //Trainer is OT
            {
                if( currPkmn.m_boxdata.m_fateful )
                    IO::regularFont->printString( "Schicksalhafte Begegnung.", 28, 120, true );
                if( !( currPkmn.m_boxdata.m_gotDate[ 0 ] ) ) {
                    if( FS::savMod == FS::SavMod::_NDS ) {
                        sprintf( buffer, "Gefangen am %02i.%02i.%02i mit Lv. %i",
                                 currPkmn.m_boxdata.m_hatchDate[ 0 ],
                                 currPkmn.m_boxdata.m_hatchDate[ 1 ],
                                 currPkmn.m_boxdata.m_hatchDate[ 2 ],
                                 currPkmn.m_boxdata.m_gotLevel );
                    } else
                        sprintf( buffer, "Gefangen mit Lv. %i", currPkmn.m_boxdata.m_gotLevel );
                    IO::regularFont->printString( buffer, 28, 44, true );
                    sprintf( buffer, "in/bei %s.", FS::getLoc( currPkmn.m_boxdata.m_gotPlace ) );
                    IO::regularFont->printString( buffer, 35, 58, true );
                    sprintf( buffer, "Besitzt ein %s""es Wesen,", NatureList[ currPkmn.m_boxdata.getNature( ) ].c_str( ) );
                    IO::regularFont->printString( buffer, 28, 76, true );
                    sprintf( buffer, "%s"".", PersonalityList[ currPkmn.m_boxdata.getPersonality( ) ].c_str( ) );
                    IO::regularFont->printString( buffer, 35, 90, true );

                    sprintf( buffer, "Mag %s""e Pok�riegel.", TasteList[ currPkmn.m_boxdata.getTasteStr( ) ].c_str( ) );
                    IO::regularFont->printString( buffer, 28, 104, true );
                } else {
                    if( FS::savMod == FS::SavMod::_NDS ) {
                        sprintf( buffer, "Als Ei erhalten am %02i.%02i.%02i",
                                 currPkmn.m_boxdata.m_gotDate[ 0 ],
                                 currPkmn.m_boxdata.m_gotDate[ 1 ],
                                 currPkmn.m_boxdata.m_gotDate[ 2 ] );
                    } else
                        sprintf( buffer, "Als Ei erhalten." );
                    IO::regularFont->printString( buffer, 28, 44, true );
                    sprintf( buffer, "in/bei %s.", FS::getLoc( currPkmn.m_boxdata.m_gotPlace ) );
                    IO::regularFont->printString( buffer, 35, 58, true );
                    if( !currPkmn.m_boxdata.m_individualValues.m_isEgg ) {

                        if( FS::savMod == FS::SavMod::_NDS ) {
                            sprintf( buffer, "Geschl�pft am %02i.%02i.%02i",
                                     currPkmn.m_boxdata.m_hatchDate[ 0 ],
                                     currPkmn.m_boxdata.m_hatchDate[ 1 ],
                                     currPkmn.m_boxdata.m_hatchDate[ 2 ] );
                            IO::regularFont->printString( buffer, 28, 72, true );
                            sprintf( buffer, "in/bei %s.", FS::getLoc( currPkmn.m_boxdata.m_hatchPlace ) );
                            IO::regularFont->printString( buffer, 35, 86, true );
                        } else {
                            sprintf( buffer, "Geschl�pft in/bei %s.", FS::getLoc( currPkmn.m_boxdata.m_hatchPlace ) );
                            IO::regularFont->printString( buffer, 28, 72, true );
                        }
                        if( !currPkmn.m_boxdata.m_fateful ) {
                            sprintf( buffer, "Besitzt ein %s""es Wesen,", NatureList[ currPkmn.m_boxdata.getNature( ) ].c_str( ) );
                            IO::regularFont->printString( buffer, 28, 100, true );
                            sprintf( buffer, "%s"".", PersonalityList[ currPkmn.m_boxdata.getPersonality( ) ].c_str( ) );
                            IO::regularFont->printString( buffer, 35, 114, true );

                            sprintf( buffer, "Mag %s""e Pok�riegel.", TasteList[ currPkmn.m_boxdata.getTasteStr( ) ].c_str( ) );
                            IO::regularFont->printString( buffer, 28, 128, true );
                        } else {
                            sprintf( buffer, "Besitzt ein %s""es Wesen.", NatureList[ currPkmn.m_boxdata.getNature( ) ].c_str( ) );
                            IO::regularFont->printString( buffer, 28, 100, true );
                        }
                    }
                }
            } else {
                if( !currPkmn.m_boxdata.m_gotDate[ 0 ] ) {
                    if( FS::savMod == FS::SavMod::_NDS ) {
                        sprintf( buffer, "Off. gef. am %02i.%02i.%02i mit Lv. %i",
                                 currPkmn.m_boxdata.m_hatchDate[ 0 ],
                                 currPkmn.m_boxdata.m_hatchDate[ 1 ],
                                 currPkmn.m_boxdata.m_hatchDate[ 2 ],
                                 currPkmn.m_boxdata.m_gotLevel );
                    } else
                        sprintf( buffer, "Offenbar gefangen mit Lv. %i", currPkmn.m_boxdata.m_gotLevel );
                    IO::regularFont->printString( buffer, 28, 44, true );
                    sprintf( buffer, "in/bei %s.", FS::getLoc( currPkmn.m_boxdata.m_gotPlace ) );
                    IO::regularFont->printString( buffer, 35, 58, true );
                    sprintf( buffer, "Besitzt ein %s""es Wesen,", NatureList[ currPkmn.m_boxdata.getNature( ) ].c_str( ) );
                    IO::regularFont->printString( buffer, 28, 76, true );
                    sprintf( buffer, "%s"".", PersonalityList[ currPkmn.m_boxdata.getPersonality( ) ].c_str( ) );
                    IO::regularFont->printString( buffer, 35, 90, true );

                    sprintf( buffer, "Mag %s""e Pok�riegel.", TasteList[ currPkmn.m_boxdata.getTasteStr( ) ].c_str( ) );
                    IO::regularFont->printString( buffer, 28, 104, true );
                } else {
                    if( FS::savMod == FS::SavMod::_NDS ) {
                        sprintf( buffer, "Off. Als Ei erh. am %02i.%02i.%02i",
                                 currPkmn.m_boxdata.m_gotDate[ 0 ],
                                 currPkmn.m_boxdata.m_gotDate[ 1 ],
                                 currPkmn.m_boxdata.m_gotDate[ 2 ] );
                    } else
                        sprintf( buffer, "Offenbar als Ei erhalten." );
                    IO::regularFont->printString( buffer, 28, 44, true );
                    sprintf( buffer, "in/bei %s.", FS::getLoc( currPkmn.m_boxdata.m_gotPlace ) );
                    IO::regularFont->printString( buffer, 35, 58, true );
                    if( !currPkmn.m_boxdata.m_individualValues.m_isEgg ) {
                        if( FS::savMod == FS::SavMod::_NDS ) {
                            sprintf( buffer, "Geschl�pft am %02i.%02i.%02i",
                                     currPkmn.m_boxdata.m_hatchDate[ 0 ],
                                     currPkmn.m_boxdata.m_hatchDate[ 1 ],
                                     currPkmn.m_boxdata.m_hatchDate[ 2 ] );
                            IO::regularFont->printString( buffer, 28, 72, true );
                            sprintf( buffer, "in/bei %s.", FS::getLoc( currPkmn.m_boxdata.m_hatchPlace ) );
                            IO::regularFont->printString( buffer, 35, 86, true );
                        } else {
                            sprintf( buffer, "Geschl�pft in/bei %s.", FS::getLoc( currPkmn.m_boxdata.m_hatchPlace ) );
                            IO::regularFont->printString( buffer, 28, 72, true );
                        }
                        if( !currPkmn.m_boxdata.m_fateful ) {
                            sprintf( buffer, "Besitzt ein %s""es Wesen,", NatureList[ currPkmn.m_boxdata.getNature( ) ].c_str( ) );
                            IO::regularFont->printString( buffer, 28, 100, true );
                            sprintf( buffer, "%s"".", PersonalityList[ currPkmn.m_boxdata.getPersonality( ) ].c_str( ) );
                            IO::regularFont->printString( buffer, 35, 114, true );

                            sprintf( buffer, "Mag %s""e Pok�riegel.", TasteList[ currPkmn.m_boxdata.getTasteStr( ) ].c_str( ) );
                            IO::regularFont->printString( buffer, 28, 128, true );
                        } else {
                            sprintf( buffer, "Besitzt ein %s""es Wesen.", NatureList[ currPkmn.m_boxdata.getNature( ) ].c_str( ) );
                            IO::regularFont->printString( buffer, 28, 100, true );
                        }
                    }
                }
                if( currPkmn.m_boxdata.m_fateful )
                    IO::regularFont->printString( "Off. schicks. Begegnung.", 28, 120, true );
            }
        }

        IO::updateOAM( true );
        IO::updateOAM( false );
    }

    void regStsScreenUI::draw( u8 p_current ) {
        consoleSetWindow( &IO::Top, positions[ _current ][ 0 ], positions[ _current ][ 1 ], 2, 2 );
        consoleClear( );

        _current = p_current;

        initSub( FS::SAV->m_pkmnTeam[ _current ] );

        consoleSetWindow( &IO::Top, positions[ _current ][ 0 ], positions[ _current ][ 1 ], 2, 2 );
        if( _current & 1 )
            printf( ">" );
        else
            printf( "<" );
    }

    void boxStsScreenUI::init( ) {
        //Remember: the storage sys swaps the screens.
        IO::Bottom = *consoleInit( &IO::Bottom, 0, BgType_Text4bpp, BgSize_T_256x256, 2, 0, false, true );
        consoleSetFont( &IO::Bottom, IO::consoleFont );
        consoleSetWindow( &IO::Bottom, 0, 0, 32, 24 );
        consoleSelect( &IO::Bottom );
        consoleClear( );

        IO::regularFont->setColor( 0, 0 );
        { IO::regularFont->setColor( BLACK_IDX, 1 ); IO::regularFont->setColor( GRAY_IDX, 2 ); }
        IO::regularFont->setColor( GRAY_IDX, 2 );
        IO::boldFont->setColor( 0, 0 );
        IO::boldFont->setColor( GRAY_IDX, 1 );
        IO::boldFont->setColor( WHITE_IDX, 2 );

        BG_PALETTE_SUB[ WHITE_IDX ] = WHITE;
        BG_PALETTE_SUB[ GRAY_IDX ] = RGB( 20, 20, 20 );
        BG_PALETTE_SUB[ BLACK_IDX ] = BLACK;

        u16 tileCnt = 0;
        IO::Oam->oamBuffer[ PKMN_SPRITE_START ].gfxIndex = tileCnt;
        tileCnt += 144;

        tileCnt = IO::loadSprite( PAGE_ICON_IDX, PAGE_ICON_PAL, tileCnt,
                                  0, 0, 32, 32, memoPal, memoTiles, memoTilesLen, false, false, false, OBJPRIORITY_0, true );
        tileCnt = IO::loadItemIcon( ItemList[ 0 ]->m_itemName, 2, 152,
                                    ITEM_ICON_IDX, ITEM_ICON_PAL, tileCnt );

        for( u8 i = 0; i < 4; ++i ) {
            Type t = AttackList[ 0 ]->m_moveType;
            tileCnt = IO::loadTypeIcon( t, 126, 43 + 32 * i, TYPE_IDX + i, TYPE_PAL( i ), tileCnt, true );
        }
        for( u8 i = 0; i < 4; ++i ) {
            tileCnt = IO::loadDamageCategoryIcon( ( move::moveHitTypes )( i % 3 ), 126, 43 + 32 * i,
                                                  ATK_DMGTYPE_IDX( i ), DMG_TYPE_PAL( i % 3 ), tileCnt, true );
        }

        for( u8 i = PKMN_SPRITE_START; i < RIBBON_IDX; ++i )
            IO::Oam->oamBuffer[ i ].isHidden = true;
        IO::setDefaultConsoleTextColors( BG_PALETTE_SUB, 6 );
        FS::readPictureData( bgGetGfxPtr( IO::bg3sub ), "nitro:/PICS/", "PKMNInfoScreen", 128, 49162, true );
    }

    void boxStsScreenUI::draw( const pokemon& p_pokemon, u8 p_page, bool p_newpok ) {
        //Remember: the storage sys swaps the screens.
        //Only drew on the sub screen
        hideSprites( true );
        consoleSetWindow( &IO::Bottom, 0, 0, 32, 24 );
        consoleSelect( &IO::Bottom );
        consoleClear( );

        IO::setDefaultConsoleTextColors( BG_PALETTE_SUB, 6 );
        drawPkmnInformation( p_pokemon, p_page, p_newpok, true );
        if( p_page == (u8) -1 )
            p_page = 2;
        switch( p_page ) {
            case 0:
                drawPkmnGeneralData( p_pokemon, true );
                break;
            case 1:
                drawPkmnNature( p_pokemon, true );
                break;
            case 2:
                drawPkmnStats( p_pokemon, true );
                break;
            case 3:
                drawPkmnMoves( p_pokemon, true );
                break;
            case 4:
                drawPkmnRibbons( p_pokemon, true );
                break;
            default:
                break;
        }
        IO::updateOAM( true );
    }
}