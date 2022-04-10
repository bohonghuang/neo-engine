/*
Pokémon neo
------------------------------

file        : navApp.cpp
author      : Philip Wellnitz
description : Implementation of applications for the PNav.

Copyright (C) 2012 - 2022
Philip Wellnitz

This file is part of Pokémon neo.

Pokémon neo is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Pokémon neo is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Pokémon neo.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "fs/data.h"
#include "fs/fs.h"
#include "io/sprite.h"
#include "io/uio.h"
#include "map/mapDefines.h"
#include "nav/nav.h"
#include "nav/navApp.h"

namespace NAV {
    void mapNavApp::drawIcon( u8 p_oamSlot, bool p_bottom ) {
        SpriteEntry* oam = ( p_bottom ? IO::Oam : IO::OamTop )->oamBuffer;
        IO::loadSprite( "NV/app01", p_oamSlot, oam[ p_oamSlot ].palette, oam[ p_oamSlot ].gfxIndex,
                        oam[ p_oamSlot ].x, oam[ p_oamSlot ].y, 64, 64, false, false, false,
                        OBJPRIORITY_1, p_bottom );
    }

    void mapNavApp::load( bool p_bottom ) {
        char buffer[ 100 ];

        auto ptr3 = !p_bottom ? bgGetGfxPtr( IO::bg3 ) : bgGetGfxPtr( IO::bg3sub );
        auto pal  = !p_bottom ? BG_PALETTE : BG_PALETTE_SUB;

        SpriteEntry* oam = ( p_bottom ? IO::Oam : IO::OamTop )->oamBuffer;

        for( u8 i = 0; i < MAX_NAV_APPS; ++i ) { oam[ SPR_NAV_APP_ICON_SUB( i ) ].isHidden = true; }

        oam[ SPR_NAV_APP_RSV_SUB + 2 ]          = oam[ SPR_X_OAM_SUB ];
        oam[ SPR_NAV_APP_RSV_SUB + 2 ].isHidden = false;
        oam[ SPR_NAV_APP_RSV_SUB + 2 ].x        = 200;
        oam[ SPR_NAV_APP_RSV_SUB + 2 ].y        = 166;

        computePlayerPosition( );
        _cursorX = _playerX;
        _cursorY = _playerY;
        computeCursorLocationId( );

        // load player icon
        u16 tileCnt = 0;

        if( !SAVE::SAV.getActiveFile( ).m_appearance ) {
            tileCnt
                = IO::loadSprite( "NV/player0", SPR_NAV_APP_RSV_SUB + 1, SPR_NAV_APP_RSV1_PAL_SUB,
                                  tileCnt, _playerX - 8, _playerY + MAP_TOP_Y - 8, 16, 16, false,
                                  false, false, OBJPRIORITY_3, p_bottom );
        } else {
            tileCnt
                = IO::loadSprite( "NV/player1", SPR_NAV_APP_RSV_SUB + 1, SPR_NAV_APP_RSV1_PAL_SUB,
                                  tileCnt, _playerX - 8, _playerY + MAP_TOP_Y - 8, 16, 16, false,
                                  false, false, OBJPRIORITY_3, p_bottom );
        }

        // load cursor icon
        tileCnt = IO::loadSprite( "BX/box_arrow", SPR_NAV_APP_RSV_SUB, SPR_NAV_APP_RSV2_PAL_SUB,
                                  tileCnt, _cursorX, _cursorY + MAP_TOP_Y - 14, 16, 16, false,
                                  false, false, OBJPRIORITY_3, true );

        // load bg

        FS::readPictureData( ptr3, "nitro:/PICS/NAV_APP/", "app01bg", 192 * 2, 192 * 256,
                             p_bottom );
        pal[ 0 ] = IO::BLACK;

        if( _cursorLocationId ) {
            snprintf( buffer, 99, "%s: %s", FS::getLocation( 2005 ).c_str( ),
                      FS::getLocation( _cursorLocationId ).c_str( ) );
        } else {
            snprintf( buffer, 99, "%s", FS::getLocation( 2005 ).c_str( ) );
        }

        IO::regularFont->printStringC( buffer, 12, 10, p_bottom, IO::font::LEFT );

        IO::updateOAM( p_bottom );
    }

    void mapNavApp::computePlayerPosition( ) {
        if( SAVE::SAV.getActiveFile( ).m_currentMap == MAP::OW_MAP ) {
            // player is in OW
            _playerX = SAVE::SAV.getActiveFile( ).m_player.m_pos.m_posX / MAP_IMG_RES;
            _playerY = SAVE::SAV.getActiveFile( ).m_player.m_pos.m_posY / MAP_IMG_RES;
        } else if( SAVE::SAV.getActiveFile( ).m_lastOWPos.first == MAP::OW_MAP ) {
            _playerX = SAVE::SAV.getActiveFile( ).m_lastOWPos.second.m_posX / MAP_IMG_RES;
            _playerY = SAVE::SAV.getActiveFile( ).m_lastOWPos.second.m_posY / MAP_IMG_RES;
        } else {
            _playerX = 0;
            _playerY = 0;
        }
    }

    void mapNavApp::computeCursorLocationId( ) {
        _cursorLocationId = MAP::BANK_10_MAP_LOCATIONS[ _cursorY / 2 ][ _cursorX / 2 ];
    }

    bool mapNavApp::tick( bool p_bottom ) {
        char buffer[ 100 ];

        // update player position
        auto oldx = _playerX, oldy = _playerY;
        auto oldl = _cursorLocationId;
        computePlayerPosition( );
        SpriteEntry* oam = ( p_bottom ? IO::Oam : IO::OamTop )->oamBuffer;

        bool change    = _playerX != oldx || _playerY != oldy;
        bool locChange = false;

        // update player sprite

        oam[ SPR_NAV_APP_RSV_SUB + 1 ].x = _playerX - 8;
        oam[ SPR_NAV_APP_RSV_SUB + 1 ].y = _playerY + MAP_TOP_Y - 8;
        if( change ) {
            locChange |= ( _playerX != _cursorX || _playerY != _cursorY );
            _cursorX = _playerX;
            _cursorY = _playerY;
        }

        // check for (touch) input

        if( touch.px >= MAP_TOP_X && touch.px <= MAP_BOT_X && touch.py >= MAP_TOP_Y
            && touch.py <= MAP_BOT_Y ) {
            // move cursor
            locChange |= ( touch.px != _cursorX || touch.py != _cursorY );

            _cursorX = touch.px;
            _cursorY = touch.py - MAP_TOP_Y;
        }

        if( locChange ) {
            oam[ SPR_NAV_APP_RSV_SUB ].x = _cursorX;
            oam[ SPR_NAV_APP_RSV_SUB ].y = _cursorY + MAP_TOP_Y - 14;

            computeCursorLocationId( );
            if( _cursorLocationId != oldl ) {
                if( _cursorLocationId ) {
                    snprintf( buffer, 99, "%s: %s", FS::getLocation( 2005 ).c_str( ),
                              FS::getLocation( _cursorLocationId ).c_str( ) );
                } else {
                    snprintf( buffer, 99, "%s", FS::getLocation( 2005 ).c_str( ) );
                }
                IO::printRectangle( 43, 10, 200, 30, p_bottom, 0 );
                IO::regularFont->printStringC( buffer, 12, 10, p_bottom, IO::font::LEFT );
            }
        }

        if( touch.px >= oam[ SPR_NAV_APP_RSV_SUB + 2 ].x
            && touch.px <= oam[ SPR_NAV_APP_RSV_SUB + 2 ].x + 20
            && touch.py >= oam[ SPR_NAV_APP_RSV_SUB + 2 ].y
            && touch.py <= oam[ SPR_NAV_APP_RSV_SUB + 2 ].y + 20 ) {
            bool suc = true;
            while( touch.px || touch.py ) {
                swiWaitForVBlank( );
                scanKeys( );

                if( !( touch.px >= oam[ SPR_NAV_APP_RSV_SUB + 2 ].x
                       && touch.px <= oam[ SPR_NAV_APP_RSV_SUB + 2 ].x + 20
                       && touch.py >= oam[ SPR_NAV_APP_RSV_SUB + 2 ].y
                       && touch.py <= oam[ SPR_NAV_APP_RSV_SUB + 2 ].y + 20 ) ) {
                    suc = false;
                    break;
                }
                touchRead( &touch );
                swiWaitForVBlank( );
            }
            if( suc ) { return true; }
        }

        if( change || locChange ) { IO::updateOAM( p_bottom ); }
        return false;
    }
} // namespace NAV