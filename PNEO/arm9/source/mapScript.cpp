/*
Pokémon neo
------------------------------

file        : mapScript.cpp
author      : Philip Wellnitz
description : Map script engine

Copyright (C) 2012 - 2023
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

#include <algorithm>

#include "bag/bagViewer.h"
#include "battle/battle.h"
#include "battle/battleTrainer.h"
#include "box/boxViewer.h"
#include "defines.h"
#include "fs/fs.h"
#include "gen/locationNames.h"
#include "io/choiceBox.h"
#include "io/counter.h"
#include "io/menuUI.h"
#include "io/message.h"
#include "io/screenFade.h"
#include "io/sprite.h"
#include "io/uio.h"
#include "map/mapDrawer.h"
#include "save/saveGame.h"
#include "sound/sound.h"
#include "spx/specials.h"
#include "sts/partyScreen.h"

namespace MAP {
#define MAX_SCRIPT_SIZE 128
    // opcode : u8, param1 : u8, param2 : u8, param3 : u8
    // opcode : u8, param1x : u11, param2x : u5, param3x : u8
    // opcode : u8, param1s : u5, param2s : u5, param3s : u14
    // opcode : u8, paramA : u12, paramB : u12
#define OPCODE( p_ins )  ( ( p_ins ) >> 24 )
#define PARAM1( p_ins )  ( ( ( p_ins ) >> 16 ) & 0xFF )
#define PARAM2( p_ins )  ( ( ( p_ins ) >> 8 ) & 0xFF )
#define PARAM3( p_ins )  ( ( p_ins ) & ( 0xFF ) )
#define PARAM1X( p_ins ) ( ( ( p_ins ) >> 13 ) & 0x7FF )
#define PARAM2X( p_ins ) ( ( ( p_ins ) >> 8 ) & 0x1F )
#define PARAM3X( p_ins ) ( ( p_ins ) & ( 0xFF ) )
#define PARAM1S( p_ins ) ( ( ( p_ins ) >> 19 ) & 0x1F )
#define PARAM2S( p_ins ) ( ( ( p_ins ) >> 14 ) & 0x1F )
#define PARAM3S( p_ins ) ( ( p_ins ) & ( 0x3FFF ) )
#define PARAMA( p_ins )  ( ( ( p_ins ) >> 12 ) & 0xFFF )
#define PARAMB( p_ins )  ( ( p_ins ) & ( 0xFFF ) )

    // special functions
    constexpr u8 CLL_HEAL_ENTIRE_TEAM     = 1;
    constexpr u8 CLL_RUN_POKE_MART        = 2;
    constexpr u8 CLL_GET_BADGE_COUNT      = 3; // (param1: region [0: Hoenn, 1: battle frontier])
    constexpr u8 CLL_INIT_GAME_ITEM_COUNT = 4;
    constexpr u8 CLL_GET_AND_REMOVE_INIT_GAME_ITEM = 5;
    constexpr u8 CLL_RUN_INITIAL_PKMN_SELECTION    = 6;
    constexpr u8 CLL_NAV_INIT                      = 7; // IO::init( ) from io/message.h
    constexpr u8 CLL_RUN_CATCHING_TUTORIAL         = 8;
    constexpr u8 CLL_AWARD_BADGE                   = 9; // (p1: region, p2: badge)
    constexpr u8 CLL_RUN_CHOICE_BOX                = 10;
    constexpr u8 CLL_GET_CURRENT_DAYTIME           = 11; // getCurrentDaytime( )
    constexpr u8 CLL_DAYCARE_BAA_SAN = 12; // (par: # of day care) (take/hand over pkmn)
    constexpr u8 CLL_DAYCARE_JII_SAN = 13; // (obtain egg)
    constexpr u8 CLL_SAVE_GAME       = 14;
    constexpr u8 CLL_HOURS_MOD       = 15; // current time (hours) % par1 (used for
                                           // shoal cave
    constexpr u8 CLL_PLAYTIME_HOURS = 16;
    constexpr u8 CLL_HALL_OF_FAME   = 17; // register current party in hall of fame, save
                                          // game, warp home

    // battle zone facilities
    constexpr u8 BTZ_BATTLE_FACTORY = 0;

    enum opCode : u8 {
        EOP = 0,  // end of program
        SMO = 1,  // set map object
        MMO = 2,  // move map object
        DMO = 3,  // destroy map object
        CFL = 4,  // check flag
        SFL = 5,  // set flag
        CRG = 6,  // check register
        SRG = 7,  // set register
        MRG = 8,  // move register value
        JMP = 9,  // jump
        JMB = 10, // jump backwards

        SMOR = 11, // set map object
        MMOR = 12, // move map object
        DMOR = 13, // destroy map object
        CFLR = 14, // check flag
        SFLR = 15, // set flag

        CRGL = 16, // check register lower
        CRGG = 17, // check register greater
        CRGN = 18, // check register not equal
        MPL  = 19, // move player
        CMO  = 20, // current map object id to reg 0
        GMO  = 21, // Get map object id of object at specified position to reg 0

        CPP = 22, // check player position

        LCKR = 23, // Lock map object
        ULKR = 24, // unlock map object

        BNK = 25, // warp player (bank, z) (needs to be before WRP)
        WRP = 26, // warp player (x, y)

        CVR  = 27, // check variable equal
        CVRN = 28, // check variable not equal
        CVRG = 29, // check variable greater
        CVRL = 30, // check variable lower

        MFO  = 31, // move map object fast
        MFOR = 32, // move map object fast

        GIT = 33, // return how many copies the player has of the specified item

        STF = 34, // set trainer flag
        CTF = 35, // check trainer flag

        ADD = 36, // reg[ par1 ] += par2
        ARG = 37, // reg[ par1 ] += reg[ par2 ]
        DIV = 38, // reg[ par1 ] /= par2
        DRG = 39, // reg[ par1 ] /= reg[ par2 ]

        HPL = 40, // hide player sprite
        SPL = 41, // show player sprite
        WPL = 42, // walk player (also through walls, etc)

        MINR = 43, // reg[ par3 ] = min( reg[ par1 ], reg[ par2 ] )
        MAXR = 44, // reg[ par3 ] = max( reg[ par1 ], reg[ par2 ] )

        GVR  = 45, // get value of variable and write it to register parB
        SVR  = 46, // set value of variable to parB
        SVRR = 47, // set value of variable to reg[ parB ]
        SUB  = 48, // reg[ par1 ] -= par2
        SUBR = 49, // reg[ par1 ] -= reg[ par2 ]

        FMM = 50, // force movement mode (player cannot change move mode themselves)
        UMM = 51, // unlock movement mode
        GMM = 52, // write current movement mode to reg 0
        CMM = 53, // change movement

        PRM  = 54, // check pkmn <param3s> at party slot <p1s>, skip <p2s> if true
        PRMA = 55, // check pkmn <param3s> appears at least <p1s> times in total in any party slot,
                   // skip <p2s> if true

        HPK = 60, // Hide following pkmn

        CMN = 70, // check money >=
        PMN = 71, // pay money

        EXM  = 87, // Exclamation mark
        EXMR = 88, // Exclamation mark (register)
        RDR  = 89, // Redraw objects
        ATT  = 90, // Attach player
        REM  = 91, // Remove player
        FIXR = 92, // Make map object to obtain same pos in map obj arr
        UFXR = 93, // Make map object to obtain same pos in map obj arr

        FNT  = 99,  // faint player
        BTR  = 100, // Battle trainer
        BPK  = 101, // Battle pkmn
        ITM  = 102, // Give item
        TTM  = 103, // Take item
        UTM  = 104, // Use item
        BTRR = 105, // Battle trainer
        BPKR = 106, // Battle pkmn
        ITMR = 107, // Give item
        TTMR = 108, // Take item
        UTMR = 109, // Use item

        SRT = 110, // set route
        CRT = 111, // check route

        COUR = 112, // Counter message (make player select number between 0 and reg[ parB ]
        MSC  = 113, // play music (temporary)
        RMS  = 114, // Reset music
        CRY  = 115, // Play cry
        SFX  = 116, // Play sound effect
        PMO  = 117, // Play music oneshot
        SMC  = 118, // Set music
        SLC  = 119, // Set location
        SWT  = 120, // Set weather to parA
        WAT  = 121, // Wait
        MBG  = 122, // Pokemart description begin
        MIT  = 123, // Mart item
        COU  = 124, // Counter message (make player select number between 0 and parB
        YNM  = 125, // yes no message
        CLL  = 126, // Call special function
        MSG  = 127, // message
        CBG  = 128, // choice box begin
        CIT  = 129, // choice item
        BTZ  = 130, // battle zone facility script

        MAP = 140, // redraw current map
        EQ  = 141, // earthquake animation

        DES = 150, // register pkmn as seen in pkdex

        SBC  = 196, // set block
        SBCC = 197, // set block, with player dir correction
        SMM  = 198, // set movement
    };

    std::string parseLogCmd( const std::string& p_cmd ) {
        u16 tmp = -1;

        if( p_cmd == "PLAYER" ) { return SAVE::SAV.getActiveFile( ).m_playername; }
        if( p_cmd == "RIVAL" ) {
            if( SAVE::SAV.getActiveFile( ).checkFlag( SAVE::F_RIVAL_APPEARANCE ) ) {
                return std::string( GET_STRING( 461 ) );
            } else {
                return std::string( GET_STRING( 460 ) );
            }
        }
        if( sscanf( p_cmd.c_str( ), "CRY:%hu", &tmp ) && tmp != u16( -1 ) ) {
            SOUND::playCry( tmp );
            return "";
        }
        if( sscanf( p_cmd.c_str( ), "VAR:%hu", &tmp ) && tmp != u16( -1 ) ) {
            return std::to_string( SAVE::SAV.getActiveFile( ).getVar( tmp ) );
        }
        if( sscanf( p_cmd.c_str( ), "TEAM:%hu", &tmp ) && tmp != u16( -1 ) ) {
            return FS::getDisplayName(
                SAVE::SAV.getActiveFile( ).getTeamPkmn( tmp )->getSpecies( ) );
        }
        return std::string( "[" ) + p_cmd + "]";
    }

    std::string convertMapString( const std::string& p_text, style p_style ) {
        std::string res = "";
        for( size_t i = 0; i < p_text.size( ); ++i ) {
            if( p_text[ i ] == '[' ) {
                std::string accmd = "";
                while( p_text[ ++i ] != ']' ) { accmd += p_text[ i ]; }
                res += parseLogCmd( accmd );
                continue;
            } else if( p_text[ i ] == '\r' ) {
                if( p_style == MSG_NORMAL ) {
                    IO::printMessage( res.c_str( ), MSG_NORMAL_CONT );
                } else if( p_style == MSG_INFO ) {
                    IO::printMessage( res.c_str( ), MSG_INFO_CONT );
                } else {
                    IO::printMessage( res.c_str( ), p_style );
                }
                res = "";
                continue;
            }
            res += p_text[ i ];
        }
        return res;
    }

    void printMapMessage( const std::string& p_text, style p_style ) {
        if( p_style == MSG_SIGN ) {
            // TODO: properly implement signs
            p_style = MSG_INFO;
        }
        IO::printMessage( convertMapString( p_text, p_style ).c_str( ), p_style );
    }

    void printMapYNMessage( const std::string& p_text, style p_style ) {

        if( p_style == MSG_SIGN ) {
            // TODO: properly implement signs
            p_style = MSG_INFO;
        }
        IO::printMessage( convertMapString( p_text, p_style ).c_str( ), p_style );
    }

    static u16 CURRENT_SCRIPT  = -1;
    u16        registers[ 10 ] = { 0 };

    bool mapDrawer::executeWarpScript( u16 p_scriptId, warpType& p_targetType,
                                       warpPos& p_targetPos ) {
        // run script
        executeScript( p_scriptId );

        // parse registers as returned data
        p_targetType = warpType( registers[ 0 ] );
        if( !p_targetType ) { return false; }

        p_targetPos = warpPos( registers[ 1 ], position( registers[ 3 ] * SIZE + registers[ 5 ],
                                                         registers[ 2 ] * SIZE + registers[ 4 ],
                                                         +registers[ 6 ] ) );

        if( !p_targetPos.first ) { return false; }
        return true;
    }

    void mapDrawer::executeScript( u16 p_scriptId, u8 p_mapObject, s16 p_mapX, s16 p_mapY ) {
        if( CURRENT_SCRIPT == p_scriptId ) { return; }
        CURRENT_SCRIPT = p_scriptId;

        FILE* f = FS::openScript( p_scriptId );
        if( !f ) {
            CURRENT_SCRIPT = -1;
            return;
        }

        bool srn       = _scriptRunning;
        _scriptRunning = true;

        u32 SCRIPT_INS[ MAX_SCRIPT_SIZE ];
        fread( SCRIPT_INS, sizeof( u32 ), MAX_SCRIPT_SIZE, f );
        FS::close( f );

        u8 pc = 0;

        std::vector<std::pair<u16, u32>> martItems;

        std::vector<u16> choiceBoxItems;
        std::vector<u16> choiceBoxPL;
        u16              choiceBoxMessage = 0;
        u8               choiceBoxMsgType = 0;

        char buffer[ 200 ] = { 0 };

        u8   pmartCurr = 0;
        bool martSell  = true;

        bool playerAttachedToObject = false;

        moveMode tmpmove = NO_MOVEMENT;

        u8 newbnk = 255;
        u8 newz   = 0;

        memset( registers, 0, sizeof( registers ) );

        while( SCRIPT_INS[ pc ] ) {
            u16 curx = SAVE::SAV.getActiveFile( ).m_player.m_pos.m_posX;
            u16 cury = SAVE::SAV.getActiveFile( ).m_player.m_pos.m_posY;
            u16 curz = SAVE::SAV.getActiveFile( ).m_player.m_pos.m_posZ;
            u16 mapX = curx / SIZE, mapY = cury / SIZE;
            if( p_mapX >= 0 ) { mapX = p_mapX; }
            if( p_mapY >= 0 ) { mapY = p_mapY; }
            auto ins   = opCode( OPCODE( SCRIPT_INS[ pc ] ) );
            u8   par1  = PARAM1( SCRIPT_INS[ pc ] );
            u8   par2  = PARAM2( SCRIPT_INS[ pc ] );
            u8   par3  = PARAM3( SCRIPT_INS[ pc ] );
            u16  par1x = PARAM1X( SCRIPT_INS[ pc ] );
            u8   par2x = PARAM2X( SCRIPT_INS[ pc ] );
            u8   par3x = PARAM3X( SCRIPT_INS[ pc ] );
            u8   par1s = PARAM1S( SCRIPT_INS[ pc ] );
            u8   par2s = PARAM2S( SCRIPT_INS[ pc ] );
            u16  par3s = PARAM3S( SCRIPT_INS[ pc ] );
            u16  parA  = PARAMA( SCRIPT_INS[ pc ] );
            u16  parB  = PARAMB( SCRIPT_INS[ pc ] );

#ifdef DESQUID_MORE
            IO::printMessage( ( std::to_string( pc ) + ": " + std::to_string( ins ) + " ( "
                                + std::to_string( par1 ) + " , " + std::to_string( par2 ) + " , "
                                + std::to_string( par3 ) + ")" )
                                  .c_str( ) );
#endif

            switch( ins ) {
            case DES: {
                SAVE::SAV.getActiveFile( ).registerSeenPkmn( parA );
                showPkmn( { parA, u8( parB ), false, false, false, DEFAULT_SPRITE_PID }, false );
                changeMoveMode( SAVE::SAV.getActiveFile( ).m_player.m_movement );
                swiWaitForVBlank( );
                break;
            }
            case HPK:
                removeFollowPkmn( );
                _forceNoFollow = true;
                break;
            case HPL: _mapSprites.setVisibility( _playerSprite, true ); break;
            case SPL: _mapSprites.setVisibility( _playerSprite, false ); break;
            case BNK: {
                newbnk = par1;
                newz   = par2;
                break;
            }
            case WRP: {
                warpPlayer( NO_SPECIAL, { newbnk, { parA, parB, newz } } );
                break;
            }
            case RDR: {
                constructAndAddNewMapObjects( currentData( ), mapX, mapY );
                break;
            }
            case EXM: {
                showExclamationAboveMapObject( par1 );
                break;
            }
            case EXMR: {
                if( par1 < 10 ) { showExclamationAboveMapObject( registers[ par1 ] ); }
                break;
            }
            case FIXR: {
                if( par1 < 10 ) { fixMapObject( registers[ par1 ] ); }
                break;
            }
            case UFXR: {
                unfixMapObject( );
                break;
            }
            case ATT: playerAttachedToObject = true; break;
            case REM: playerAttachedToObject = false; break;
            case EOP: {
                CURRENT_SCRIPT = -1;
                _scriptRunning = srn;
                return;
            }
            case GIT: {
                auto idata     = FS::getItemData( parA );
                registers[ 0 ] = SAVE::SAV.getActiveFile( ).m_bag.count(
                    BAG::toBagType( idata.m_itemType ), parA );
                break;
            }
            case SMO: {
                mapObject obj   = mapObject( );
                obj.m_pos       = { u16( mapX * SIZE + par1s ), u16( mapY * SIZE + par2s ), 3 };
                obj.m_picNum    = par3s;
                obj.m_movement  = NO_MOVEMENT;
                obj.m_range     = 0;
                obj.m_direction = DOWN;
                obj.m_currentMovement = movement{ obj.m_direction, 0 };

                std::pair<u8, mapObject> cur = { 0, obj };
                if( !loadMapObject( cur ) ) {
#ifdef DESQUID_MORE
                    IO::printMessage( "SMO fail" );
#endif
                }

                // Check if there is some unused map object

                u8 found = 255;
                for( u8 i = _fixedObjectCount; i < SAVE::SAV.getActiveFile( ).m_mapObjectCount;
                     ++i ) {
                    if( SAVE::SAV.getActiveFile( ).m_mapObjects[ i ].first == UNUSED_MAPOBJECT ) {
                        found = i;
                        break;
                    }
                }

                if( found < 255 ) {
                    registers[ 0 ] = found;
                } else {
#ifdef DESQUID_MORE
                    IO::printMessage( ( std::to_string( cur.first ) ).c_str( ) );
#endif
                    registers[ 0 ] = SAVE::SAV.getActiveFile( ).m_mapObjectCount++;
                }

                SAVE::SAV.getActiveFile( ).m_mapObjects[ registers[ 0 ] ] = cur;
                break;
            }
            case WPL: {
                redirectPlayer( direction( par2 ), false, true );
                for( u8 j = 0; j < par3; ++j ) { walkPlayer( direction( par2 ) ); }
                break;
            }
            case MPL: {
                redirectPlayer( direction( par2 ), false );
                for( u8 j = 0; j < par3; ++j ) { movePlayer( direction( par2 ) ); }
                break;
            }

            case PRM: {
                auto pkmn = SAVE::SAV.getActiveFile( ).getTeamPkmn( par1s );
                if( pkmn && pkmn->getSpecies( ) == par3s ) { pc += par2s; }
                break;
            }

            case PRMA: {
                u8 cnt = 0;
                for( u8 k = 0; k < SAVE::SAV.getActiveFile( ).getTeamPkmnCount( ); ++k ) {
                    auto pkmn = SAVE::SAV.getActiveFile( ).getTeamPkmn( k );
                    if( pkmn && pkmn->getSpecies( ) == par3s ) { cnt++; }
                }
                if( cnt >= par1s ) { pc += par2s; }
                break;
            }

            case CMM: {
                if( SAVE::SAV.getActiveFile( ).m_player.m_movement != parA ) {
                    changeMoveMode( moveMode( parA ) );
                }
                break;
            }
            case FMM: {
                SAVE::SAV.getActiveFile( ).m_forcedMovement
                    = SAVE::SAV.getActiveFile( ).m_player.m_movement;
                break;
            }
            case UMM: {
                SAVE::SAV.getActiveFile( ).m_forcedMovement = 0;
                break;
            }
            case GMM: {
                registers[ 0 ] = SAVE::SAV.getActiveFile( ).m_player.m_movement;
                break;
            }

            case MMO: {
                movement m = { direction( par2 ), 0 };
                _mapSprites.setFrameD( par1, direction( par2 ) );

                auto tmp = SAVE::SAV.getActiveFile( ).m_mapObjects[ par1 ].second.m_movement;
                SAVE::SAV.getActiveFile( ).m_mapObjects[ par1 ].second.m_movement = NO_MOVEMENT;

                for( u8 j = 0; j < par3; ++j ) {
                    for( u8 i = 0; i < 16; ++i ) {
                        moveMapObject( par1, m, playerAttachedToObject,
                                       SAVE::SAV.getActiveFile( ).m_player.m_direction );
                        m.m_frame = ( m.m_frame + 1 ) & 15;
                        swiWaitForVBlank( );
                    }
                    if( playerAttachedToObject ) {
                        SAVE::SAV.getActiveFile( ).m_player.m_direction = direction( par2 );
                    }
                }
                SAVE::SAV.getActiveFile( ).m_mapObjects[ par1 ].second.m_movement = tmp;
                break;
            }
            case MFO: {
                movement m = { direction( par2 ), 0 };
                _mapSprites.setFrameD( par1, direction( par2 ) );

                auto tmp = SAVE::SAV.getActiveFile( ).m_mapObjects[ par1 ].second.m_movement;
                SAVE::SAV.getActiveFile( ).m_mapObjects[ par1 ].second.m_movement = NO_MOVEMENT;

                for( u8 j = 0; j < par3; ++j ) {
                    for( u8 i = 0; i < 16; ++i ) {
                        moveMapObject( par1, m, playerAttachedToObject,
                                       SAVE::SAV.getActiveFile( ).m_player.m_direction );
                        m.m_frame = ( m.m_frame + 1 ) & 15;
                        if( i & 1 ) { swiWaitForVBlank( ); }
                    }
                    if( playerAttachedToObject ) {
                        SAVE::SAV.getActiveFile( ).m_player.m_direction = direction( par2 );
                    }
                }
                SAVE::SAV.getActiveFile( ).m_mapObjects[ par1 ].second.m_movement = tmp;
                break;
            }

            case DMO: {
                _mapSprites.destroySprite( SAVE::SAV.getActiveFile( ).m_mapObjects[ par1 ].first );
                SAVE::SAV.getActiveFile( ).m_mapObjects[ par1 ]
                    = { UNUSED_MAPOBJECT, mapObject( ) };
                break;
            }
            case CFL: {
                if( SAVE::SAV.getActiveFile( ).checkFlag( par1x ) == par2x ) { pc += par3x; }
                break;
            }
            case SFL: {
                SAVE::SAV.getActiveFile( ).setFlag( par1x, par2x );
                break;
            }
            case STF: {
                SAVE::SAV.getActiveFile( ).setFlag( SAVE::F_TRAINER_BATTLED( par1x ), par2x );
                break;
            }
            case CTF: {
                if( SAVE::SAV.getActiveFile( ).checkFlag( SAVE::F_TRAINER_BATTLED( par1x ) )
                    == par2x ) {
                    pc += par3x;
                }
                break;
            }
            case SRT: {
                SAVE::SAV.getActiveFile( ).m_route = par1;
                break;
            }
            case CRT: {
                if( SAVE::SAV.getActiveFile( ).m_route == par1 ) { pc += par2; }
                break;
            }

            case CRG: {
                if( registers[ par1 ] == par2 ) { pc += par3; }
                break;
            }

            case MINR: registers[ par3 ] = std::min( registers[ par1 ], registers[ par2 ] ); break;
            case MAXR: registers[ par3 ] = std::max( registers[ par1 ], registers[ par2 ] ); break;

            case SUB: registers[ parA ] -= parB; break;
            case ADD: registers[ parA ] += parB; break;
            case DIV:
                if( parB ) { registers[ parA ] /= parB; }
                break;
            case ARG: {
                registers[ par1 ] += registers[ par2 ];
#ifdef DESQUID_MORE
                std::string dstr = "";
                for( u8 q = 0; q < 10; ++q ) { dstr += std::to_string( registers[ q ] ) + " "; }
                IO::printMessage( dstr, MSG_INFO );

#endif

                break;
            }
            case SUBR: registers[ par1 ] -= registers[ par2 ]; break;
            case DRG:
                if( registers[ par2 ] ) { registers[ par1 ] /= registers[ par2 ]; }
                break;
            case SRG: registers[ parA ] = parB; break;
            case MRG: registers[ par2 ] = registers[ par1 ]; break;
            case JMP:
                if( pc + par1 < MAX_SCRIPT_SIZE ) {
                    pc += par1;
                } else {
                    CURRENT_SCRIPT = -1;
                    return;
                }
                break;
            case JMB:
                if( par1 > pc ) {
                    pc = 0;
                } else {
                    pc -= par1;
                }
                break;
            case SMOR: {
                mapObject obj         = mapObject( );
                obj.m_pos             = { u16( mapX * SIZE + par2 ), u16( mapY * SIZE + par3 ), 3 };
                obj.m_picNum          = registers[ par1 ];
                obj.m_movement        = NO_MOVEMENT;
                obj.m_range           = 0;
                obj.m_direction       = DOWN;
                obj.m_currentMovement = movement{ obj.m_direction, 0 };

                std::pair<u8, mapObject> cur = { 0, obj };
                loadMapObject( cur );

                // Check if there is some unused map object

                u8 found = 255;
                for( u8 i = _fixedObjectCount; i < SAVE::SAV.getActiveFile( ).m_mapObjectCount;
                     ++i ) {
                    if( SAVE::SAV.getActiveFile( ).m_mapObjects[ i ].first == UNUSED_MAPOBJECT ) {
                        found = i;
                        break;
                    }
                }

                if( found < 255 ) {
                    registers[ 0 ] = found;
                } else {
                    registers[ 0 ] = SAVE::SAV.getActiveFile( ).m_mapObjectCount++;
                }
                SAVE::SAV.getActiveFile( ).m_mapObjects[ registers[ 0 ] ] = cur;
                break;
            }
            case MMOR: {
#ifdef DESQUID_MORE
                IO::printMessage(
                    ( std::to_string(
                          SAVE::SAV.getActiveFile( ).m_mapObjects[ registers[ par1 ] ].first )
                      + " ( " + std::to_string( registers[ par1 ] ) + " , " + std::to_string( par2 )
                      + " , " + std::to_string( par3 ) + ")" )
                        .c_str( ) );
#endif
                auto tmp = SAVE::SAV.getActiveFile( )
                               .m_mapObjects[ registers[ par1 ] ]
                               .second.m_movement;
                SAVE::SAV.getActiveFile( ).m_mapObjects[ registers[ par1 ] ].second.m_movement
                    = NO_MOVEMENT;
                movement m = { direction( par2 ), 0 };
                _mapSprites.setFrameD(
                    SAVE::SAV.getActiveFile( ).m_mapObjects[ registers[ par1 ] ].first,
                    direction( par2 ) );

                for( u8 j = 0; j < par3; ++j ) {
                    for( u8 i = 0; i < 16; ++i ) {
                        moveMapObject( registers[ par1 ], m, playerAttachedToObject,
                                       SAVE::SAV.getActiveFile( ).m_player.m_direction );
                        m.m_frame = ( m.m_frame + 1 ) & 15;
                        swiWaitForVBlank( );
                    }

                    if( playerAttachedToObject ) {
                        SAVE::SAV.getActiveFile( ).m_player.m_direction = direction( par2 );
                    }
                }
                SAVE::SAV.getActiveFile( ).m_mapObjects[ registers[ par1 ] ].second.m_movement
                    = tmp;
                break;
            }
            case MFOR: {
#ifdef DESQUID_MORE
                IO::printMessage(
                    ( std::to_string(
                          SAVE::SAV.getActiveFile( ).m_mapObjects[ registers[ par1 ] ].first )
                      + " ( " + std::to_string( registers[ par1 ] ) + " , " + std::to_string( par2 )
                      + " , " + std::to_string( par3 ) + ")" )
                        .c_str( ) );
#endif
                auto tmp = SAVE::SAV.getActiveFile( )
                               .m_mapObjects[ registers[ par1 ] ]
                               .second.m_movement;
                SAVE::SAV.getActiveFile( ).m_mapObjects[ registers[ par1 ] ].second.m_movement
                    = NO_MOVEMENT;
                movement m = { direction( par2 ), 0 };
                _mapSprites.setFrameD(
                    SAVE::SAV.getActiveFile( ).m_mapObjects[ registers[ par1 ] ].first,
                    direction( par2 ) );

                for( u8 j = 0; j < par3; ++j ) {
                    for( u8 i = 0; i < 16; ++i ) {
                        moveMapObject( registers[ par1 ], m, playerAttachedToObject,
                                       SAVE::SAV.getActiveFile( ).m_player.m_direction );
                        m.m_frame = ( m.m_frame + 1 ) & 15;
                        if( i & 1 ) { swiWaitForVBlank( ); }
                    }

                    if( playerAttachedToObject ) {
                        SAVE::SAV.getActiveFile( ).m_player.m_direction = direction( par2 );
                    }
                }
                SAVE::SAV.getActiveFile( ).m_mapObjects[ registers[ par1 ] ].second.m_movement
                    = tmp;
                break;
            }

            case DMOR: {
                _mapSprites.destroySprite(
                    SAVE::SAV.getActiveFile( ).m_mapObjects[ registers[ par1 ] ].first );
                SAVE::SAV.getActiveFile( ).m_mapObjects[ registers[ par1 ] ]
                    = { UNUSED_MAPOBJECT, mapObject( ) };
                break;
            }
            case CFLR: {
                if( SAVE::SAV.getActiveFile( ).checkFlag( par1x ) == registers[ par2x ] ) {
                    pc += par3x;
                }
                break;
            }
            case SFLR: {
                SAVE::SAV.getActiveFile( ).setFlag( par1x, registers[ par2x ] );
                break;
            }

            case CRGL:
                if( registers[ par1 ] < par2 ) { pc += par3; }
                break;
            case CRGG:
                if( registers[ par1 ] > par2 ) { pc += par3; }
                break;
            case CRGN:
                if( registers[ par1 ] != par2 ) { pc += par3; }
                break;
            case CVRL:
                if( SAVE::SAV.getActiveFile( ).getVar( par1 ) < par2 ) { pc += par3; }
                break;
            case CVRG:
                if( SAVE::SAV.getActiveFile( ).getVar( par1 ) > par2 ) { pc += par3; }
                break;
            case CVRN:
                if( SAVE::SAV.getActiveFile( ).getVar( par1 ) != par2 ) { pc += par3; }
                break;
            case CVR:
                if( SAVE::SAV.getActiveFile( ).getVar( par1 ) == par2 ) { pc += par3; }
                break;

            case GVR: registers[ parB ] = SAVE::SAV.getActiveFile( ).getVar( parA ); break;
            case SVR: SAVE::SAV.getActiveFile( ).setVar( parA, parB ); break;
            case SVRR: SAVE::SAV.getActiveFile( ).setVar( parA, registers[ parB ] ); break;

            case CMN:
                if( SAVE::SAV.getActiveFile( ).m_money >= parA ) { pc += parB; }
                break;
            case PMN:
                SOUND::playSoundEffect( SFX_BUY_SUCCESSFUL );
                if( SAVE::SAV.getActiveFile( ).m_money >= parA ) {
                    SAVE::SAV.getActiveFile( ).m_money -= parA;
                } else {
                    SAVE::SAV.getActiveFile( ).m_money = 0;
                }
                break;

            case CMO: registers[ 0 ] = p_mapObject; break;
            case LCKR: {
                tmpmove = SAVE::SAV.getActiveFile( )
                              .m_mapObjects[ registers[ par1 ] ]
                              .second.m_movement;
                SAVE::SAV.getActiveFile( ).m_mapObjects[ registers[ par1 ] ].second.m_movement
                    = NO_MOVEMENT;
                break;
            }
            case ULKR: {
                SAVE::SAV.getActiveFile( ).m_mapObjects[ registers[ par1 ] ].second.m_movement
                    = tmpmove;
                break;
            }
            case GMO: {
                registers[ 0 ] = 255;
                for( u8 i = 0; i < SAVE::SAV.getActiveFile( ).m_mapObjectCount; ++i ) {
                    auto& o2 = SAVE::SAV.getActiveFile( ).m_mapObjects[ i ];
                    if( o2.second.m_pos.m_posX == par1 + mapX * SIZE
                        && o2.second.m_pos.m_posY == par2 + mapY * SIZE
                        && o2.second.m_pos.m_posZ == par3 ) {
                        registers[ 0 ] = i;
                        break;
                    }
                }
                break;
            }
            case CPP: {
                if( curx % SIZE == par1 && cury % SIZE == par2 && curz % SIZE == par3 ) {
                    registers[ 0 ] = 1;
                } else {
                    registers[ 0 ] = 0;
                }
                break;
            }
            case FNT: {
                faintPlayer( );
                break;
            }
            case MAP: {
                auto playerPrio = _mapSprites.getPriority( _playerSprite );
                FADE_TOP_DARK( );
                ANIMATE_MAP = false;
                swiWaitForVBlank( );

                IO::init( );
                draw( playerPrio );
                _mapSprites.setPriority( _playerSprite,
                                         SAVE::SAV.getActiveFile( ).m_playerPriority = playerPrio );
                ANIMATE_MAP = true;
                break;
            }
            case EQ: {
                SOUND::playSoundEffect( SFX_HM_STRENGTH );
                for( u8 k = 0; k < 2; ++k ) {
                    swiWaitForVBlank( );
                    moveCamera( RIGHT, false, false );
                    moveCamera( RIGHT, false, false );
                    swiWaitForVBlank( );
                    moveCamera( RIGHT, false, false );
                    moveCamera( RIGHT, false, false );
                    swiWaitForVBlank( );
                    moveCamera( LEFT, false, false );
                    moveCamera( LEFT, false, false );
                }
                for( u8 k = 0; k < 4; ++k ) {
                    swiWaitForVBlank( );
                    moveCamera( RIGHT, false, false );
                    moveCamera( RIGHT, false, false );
                    swiWaitForVBlank( );
                    moveCamera( LEFT, false, false );
                    moveCamera( LEFT, false, false );
                    swiWaitForVBlank( );
                    moveCamera( LEFT, false, false );
                    moveCamera( LEFT, false, false );
                }
                for( u8 k = 0; k < 2; ++k ) {
                    swiWaitForVBlank( );
                    moveCamera( RIGHT, false, false );
                    moveCamera( RIGHT, false, false );
                    swiWaitForVBlank( );
                    moveCamera( RIGHT, false, false );
                    moveCamera( RIGHT, false, false );
                    swiWaitForVBlank( );
                    moveCamera( LEFT, false, false );
                    moveCamera( LEFT, false, false );
                }
                swiWaitForVBlank( );
                swiWaitForVBlank( );

                break;
            }
            case BTR: {
                // If the player can't battle, they'll just lose
                if( !SAVE::SAV.getActiveFile( ).countAlivePkmn( ) ) {
                    registers[ 0 ] = 0;
                    break;
                }

                auto policy     = parB == 1 && SAVE::SAV.getActiveFile( ).countAlivePkmn( ) > 1
                                      ? BATTLE::DEFAULT_DOUBLE_TRAINER_POLICY
                                      : BATTLE::DEFAULT_TRAINER_POLICY;
                auto tr         = FS::getBattleTrainer( parA );
                auto playerPrio = _mapSprites.getPriority( _playerSprite );

                SOUND::playBGM( SOUND::BGMforTrainerBattle( tr.m_data.m_trainerClass ) );
                FADE_TOP_DARK( );
                ANIMATE_MAP = false;
                swiWaitForVBlank( );

                BATTLE::battle bt
                    = BATTLE::battle( SAVE::SAV.getActiveFile( ).m_pkmnTeam,
                                      SAVE::SAV.getActiveFile( ).getTeamPkmnCount( ), tr, policy );
                auto result = bt.start( );

                FADE_TOP_DARK( );
                IO::init( );
                draw( playerPrio );
                _mapSprites.setPriority( _playerSprite,
                                         SAVE::SAV.getActiveFile( ).m_playerPriority = playerPrio );
                SOUND::restartBGM( );
                ANIMATE_MAP = true;

                // set result register here to avoid weird interactions with the draw call
                // and level scripts.
                if( result == BATTLE::battle::BATTLE_OPPONENT_WON ) {
                    registers[ 0 ] = 0;
                } else {
                    registers[ 0 ] = 1;
                }
                break;
            }
            case BTRR:
                // If the player can't battle, they'll just lose
                if( !SAVE::SAV.getActiveFile( ).countAlivePkmn( ) ) {
                    registers[ 0 ] = 0;
                    break;
                }

                // TODO
                break;
            case BPK: {
                // If the player can't battle, they'll just lose
                if( !SAVE::SAV.getActiveFile( ).countAlivePkmn( ) ) {
                    registers[ 0 ] = 0;
                    break;
                }

                pokemon wildPkmn = pokemon( parA, parB );

                auto playerPrio = _mapSprites.getPriority( _playerSprite );
                ANIMATE_MAP     = false;
                DRAW_TIME       = false;
                swiWaitForVBlank( );
                auto res
                    = BATTLE::battle( SAVE::SAV.getActiveFile( ).m_pkmnTeam,
                                      SAVE::SAV.getActiveFile( ).getTeamPkmnCount( ), wildPkmn,
                                      currentData( ).m_battlePlat1, currentData( ).m_battlePlat2,
                                      currentData( ).m_battleBG, getBattlePolicy( true ) )
                          .start( );

                FADE_TOP_DARK( );
                draw( playerPrio );
                _mapSprites.setPriority( _playerSprite,
                                         SAVE::SAV.getActiveFile( ).m_playerPriority = playerPrio );
                ANIMATE_MAP = true;
                IO::init( );

                if( res == BATTLE::battle::BATTLE_OPPONENT_WON ) {
                    registers[ 0 ] = 0;
                } else {
                    registers[ 0 ] = 1;
                }

                if( res == BATTLE::battle::BATTLE_CAPTURE ) { registers[ 1 ] = 1; }

                break;
            }
            case BPKR:
                // If the player can't battle, they'll just lose
                if( !SAVE::SAV.getActiveFile( ).countAlivePkmn( ) ) {
                    registers[ 0 ] = 0;
                    break;
                }

                // TODO
                break;
            case ITM: IO::giveItemToPlayer( parA, parB ); break;
            case TTM: IO::takeItemFromPlayer( parA, parB ); break;
            case UTM: IO::useItemFromPlayer( parA, parB ); break;
            case ITMR: IO::giveItemToPlayer( registers[ par1 ], registers[ par1 + 1 ] ); break;
            case TTMR: {
                IO::takeItemFromPlayer( registers[ par1 ], registers[ par1 + 1 ] );
                break;
            }
            case UTMR: IO::useItemFromPlayer( registers[ par1 ], registers[ par1 + 1 ] ); break;
            case MSC: {
                SOUND::playBGM( parA, true );
                break;
            }
            case RMS: {
                SOUND::restartBGM( );
                break;
            }
            case CRY: {
                SOUND::playCry( parA, parB );
                break;
            }
            case SFX: {
                SOUND::playSoundEffect( parA );
                break;
            }
            case PMO: {
                SOUND::playBGMOneshot( parA );
                for( u16 i = 0; i < parB; ++i ) { swiWaitForVBlank( ); }
                SOUND::restartBGM( );
                break;
            }
            case SMC:
                // TODO
                break;
            case SLC:
                // TODO
                break;
            case SWT: changeWeather( mapWeather( parA ) ); break;
            case WAT:
                for( u8 i = 0; i < parA; ++i ) { swiWaitForVBlank( ); }
                break;
            case SBC: {
                setBlock( u16( mapX * SIZE + par1s ), u16( mapY * SIZE + par2s ), par3s );
                break;
            }
            case SBCC: {
                setBlock( u16( mapX * SIZE + par1s
                               + dir[ SAVE::SAV.getActiveFile( ).m_player.m_direction ][ 0 ] ),
                          u16( mapY * SIZE + par2s
                               + dir[ SAVE::SAV.getActiveFile( ).m_player.m_direction ][ 1 ] ),
                          par3s );
                break;
            }
            case SMM: {
                setMovement( u16( mapX * SIZE + par1s ), u16( mapY * SIZE + par2s ), par3s );
                break;
            }
            case CBG:
                choiceBoxItems.clear( );
                choiceBoxPL.clear( );
                choiceBoxMessage = parA;
                choiceBoxMsgType = parB;
                break;
            case CIT:
                choiceBoxItems.push_back( parA + FS::MAP_STRING );
                choiceBoxPL.push_back( parB );
                break;

            case BTZ: {
                // par 1: battle facility
                // par 2: ruleset (max level, num pkmn, battle mode, num battles, fixed
                // encounters, etc); each
                // ruleset has a corresponding set of possible teams/pkmn to pick from
                switch( par1 ) {
                case BTZ_BATTLE_FACTORY: {
                    runBattleFactory( FACILITY_RULE_SETS[ par2 ] );
                    break;
                }
                }
                break;
            }

            case MBG:
                pmartCurr = par1;
                martSell  = par2;
                martItems.clear( );
                break;
            case MIT:
                if( pmartCurr == 0 ) {
                    martItems.push_back( { parA, u16( parB ) * 10 } );
                } else {
                    martItems.push_back( { parA, parB } );
                }
                break;
            case CLL:
                switch( par1 ) {

                case CLL_HEAL_ENTIRE_TEAM: { // heal pkmn team
                    for( u8 i = 0; i < SAVE::SAV.getActiveFile( ).getTeamPkmnCount( ); ++i ) {
                        auto tmp = SAVE::SAV.getActiveFile( ).getTeamPkmn( i );
                        if( tmp ) { tmp->heal( ); }
                    }
                    break;
                }
                case CLL_RUN_POKE_MART: { // run pokemart
                    runPokeMart( martItems, 0, martSell, pmartCurr );
                    break;
                }
                case CLL_GET_BADGE_COUNT: {
                    registers[ 0 ] = SAVE::SAV.getActiveFile( ).getBadgeCount( par2 );
                    break;
                }
                case CLL_INIT_GAME_ITEM_COUNT: {
                    registers[ 0 ] = SAVE::SAV.getActiveFile( ).m_initGameItemCount;
                    break;
                }
                case CLL_GET_AND_REMOVE_INIT_GAME_ITEM: {
                    // At most 4 init game items
                    if( par2 <= 4 ) [[likely]] {
                        registers[ 0 ] = SAVE::SAV.getActiveFile( ).m_initGameItems[ par2 ];
                    } else {
                        registers[ 0 ] = 0;
                        break;
                    }
                    // remove first item from list of items yet to be handed out
                    for( u8 i = par2; i < SAVE::SAV.getActiveFile( ).m_initGameItemCount; ++i ) {
                        if( i < 4 ) [[likely]] {
                            SAVE::SAV.getActiveFile( ).m_initGameItems[ i ]
                                = SAVE::SAV.getActiveFile( ).m_initGameItems[ i + 1 ];
                        } else {
                            SAVE::SAV.getActiveFile( ).m_initGameItems[ i ] = 0;
                        }
                    }
                    SAVE::SAV.getActiveFile( ).m_initGameItemCount--;
                    break;
                }
                case CLL_RUN_INITIAL_PKMN_SELECTION: {
                    // init pkmn
                    SPX::runInitialPkmnSelection( );
                    break;
                }
                case CLL_NAV_INIT: {
                    IO::init( );
                    break;
                }
                case CLL_RUN_CATCHING_TUTORIAL: {
                    SPX::runCatchingTutorial( );
                    break;
                }
                case CLL_AWARD_BADGE: {
                    awardBadge( par2, par3 );
                    break;
                }
                case CLL_RUN_CHOICE_BOX: {
                    IO::choiceBox cb = IO::choiceBox( IO::choiceBox::MODE_UP_DOWN_LEFT_RIGHT );
                    registers[ 0 ]   = cb.getResult( GET_MAP_STRING( choiceBoxMessage ),
                                                     style( choiceBoxMsgType ), choiceBoxItems );
                    registers[ 1 ]   = choiceBoxPL[ registers[ 0 ] ];
                    IO::init( );
                    break;
                }
                case CLL_GET_CURRENT_DAYTIME: { // get current time
                    registers[ 0 ] = getCurrentDaytime( );
                    break;
                }
                case CLL_DAYCARE_BAA_SAN: {
                    // day care baa san
                    boxPokemon* dc1 = &SAVE::SAV.getActiveFile( ).m_dayCarePkmn[ par2 * 2 ];
                    boxPokemon* dc2 = &SAVE::SAV.getActiveFile( ).m_dayCarePkmn[ par2 * 2 + 1 ];
                    boxPokemon* dce = &SAVE::SAV.getActiveFile( ).m_dayCareEgg[ par2 ];

                    u8* dcl1 = &SAVE::SAV.getActiveFile( ).m_dayCareDepositLevel[ par2 * 2 ];
                    u8* dcl2 = &SAVE::SAV.getActiveFile( ).m_dayCareDepositLevel[ par2 * 2 + 1 ];

                    if( dce->getSpecies( ) ) {
                        // an egg spawned, redirect to jii san
                        printMapMessage( GET_MAP_STRING( 476 ), (style) 0 );
                        break;
                    }

                    if( !dc1->getSpecies( ) && dc2->getSpecies( ) ) {
                        std::swap( *dc1, *dc2 );
                        std::swap( *dcl1, *dcl2 );
                    }

                    u8 depositpkmn = false;

                    if( !dc1->getSpecies( ) ) {
                        // no pkmn deposited, ask if player wants to deposit a pkmn
                        if( IO::yesNoBox::YES
                            == IO::yesNoBox( ).getResult(
                                convertMapString( GET_MAP_STRING( 477 ), (style) 0 ).c_str( ),
                                (style) 0 ) ) {
                            IO::init( );
                            depositpkmn = true;
                        } else {
                            IO::init( );
                            printMapMessage( GET_MAP_STRING( 478 ), (style) 0 );
                            break;
                        }
                    } else {
                        snprintf( buffer, 199, GET_MAP_STRING( 479 ), dc1->m_name );
                        printMapMessage( buffer, (style) 0 );

                        if( !dc2->getSpecies( ) ) {
                            // ask if player wants to deposit a second pkmn
                            if( IO::yesNoBox::YES
                                == IO::yesNoBox( ).getResult(
                                    convertMapString( GET_MAP_STRING( 480 ), (style) 0 ).c_str( ),
                                    (style) 0 ) ) {
                                IO::init( );
                                depositpkmn = 2;
                            } else {
                                IO::init( );
                            }
                        }

                        if( !depositpkmn ) {
                            // ask if he player wants to take a pkmn back
                            printMapMessage( GET_MAP_STRING( 483 ), MSG_NOCLOSE );
                            loop( ) {
                                u8 takeback = IO::chooseDaycarePkmn( par2 );
                                IO::init( );

                                if( takeback > 1 ) {
                                    // player doesn't want to get pkmn back
                                    printMapMessage( GET_MAP_STRING( 478 ), (style) 0 );
                                    break;
                                }

                                // check if there is space in the player's team
                                if( SAVE::SAV.getActiveFile( ).getTeamPkmnCount( ) >= 6 ) {
                                    printMapMessage( GET_MAP_STRING( 487 ), (style) 0 );
                                    break;
                                }

                                pokemon pk   = pokemon( dc1[ takeback ] );
                                u32     cost = ( pk.m_level - dcl1[ takeback ] + 1 ) * 100;
                                snprintf( buffer, 199, GET_MAP_STRING( 488 ),
                                          dc1[ takeback ].m_name, cost );

                                if( IO::yesNoBox::YES
                                    == IO::yesNoBox( ).getResult(
                                        convertMapString( buffer, (style) 0 ).c_str( ),
                                        (style) 0 ) ) {
                                    IO::init( );
                                    // check if the player has enough money
                                    if( SAVE::SAV.getActiveFile( ).m_money >= cost ) {
                                        SOUND::playSoundEffect( SFX_BUY_SUCCESSFUL );
                                        SAVE::SAV.getActiveFile( ).m_money -= cost;
                                        snprintf( buffer, 199, GET_MAP_STRING( 490 ),
                                                  dc1[ takeback ].m_name );
                                        printMapMessage( buffer, (style) 0 );

                                        snprintf( buffer, 199, GET_MAP_STRING( 491 ),
                                                  dc1[ takeback ].m_name );
                                        printMapMessage( buffer, (style) 1 );

                                        SAVE::SAV.getActiveFile( ).setTeamPkmn(
                                            SAVE::SAV.getActiveFile( ).getTeamPkmnCount( ), &pk );

                                        dc1[ takeback ]  = boxPokemon( );
                                        dcl1[ takeback ] = 0;

                                        if( !takeback ) {
                                            std::swap( *dc1, *dc2 );
                                            std::swap( *dcl1, *dcl2 );
                                        }

                                        // check if the player wants to take back the other
                                        // pkmn as well
                                        if( dc1->getSpecies( ) ) {
                                            printMapMessage( GET_MAP_STRING( 492 ), MSG_NOCLOSE );
                                            continue;
                                        }
                                        printMapMessage( GET_MAP_STRING( 482 ), (style) 0 );
                                        break;
                                    } else {
                                        printMapMessage( GET_MAP_STRING( 489 ), (style) 0 );
                                        break;
                                    }
                                } else {
                                    IO::init( );
                                    printMapMessage( GET_MAP_STRING( 482 ), (style) 0 );
                                    break;
                                }
                            }
                            break;
                        }
                    }

                    while( depositpkmn && depositpkmn <= 2 ) {
                        // check if the player has at least 2 pkmn

                        u8 plyerpkmncnt = 0;
                        for( u8 i = 0; i < SAVE::SAV.getActiveFile( ).getTeamPkmnCount( ); ++i ) {
                            if( !SAVE::SAV.getActiveFile( ).getTeamPkmn( i )->isEgg( ) ) {
                                plyerpkmncnt++;
                            }
                        }

                        if( plyerpkmncnt < 2 ) {
                            // player has only 1 pkmn
                            printMapMessage( GET_MAP_STRING( 484 ), (style) 0 );
                            break;
                        }

                        // make player select a pkmn
                        printMapMessage( GET_MAP_STRING( 481 ), (style) 0 );

                        ANIMATE_MAP = false;
                        IO::clearScreen( false );
                        videoSetMode( MODE_5_2D );
                        bgUpdate( );

                        STS::partyScreen sts
                            = STS::partyScreen( SAVE::SAV.getActiveFile( ).m_pkmnTeam,
                                                SAVE::SAV.getActiveFile( ).getTeamPkmnCount( ),
                                                false, false, false, 1, true, true, false );

                        SOUND::dimVolume( );

                        auto res = sts.run( );

                        FADE_TOP_DARK( );
                        FADE_SUB_DARK( );
                        IO::clearScreen( false );
                        videoSetMode( MODE_5_2D );
                        IO::resetScale( true, false );
                        bgUpdate( );

                        ANIMATE_MAP = true;
                        SOUND::restoreVolume( );

                        IO::init( );
                        MAP::curMap->draw( );

                        // check if the player has another pkmn that can battle

                        u8 selpkmn = res.getSelectedPkmn( );

                        if( selpkmn >= SAVE::SAV.getActiveFile( ).getTeamPkmnCount( ) ) {
                            // player aborted
                            printMapMessage( GET_MAP_STRING( 482 ), (style) 0 );
                            break;
                        }

                        plyerpkmncnt = 0;
                        for( u8 i = 0; i < SAVE::SAV.getActiveFile( ).getTeamPkmnCount( ); ++i ) {
                            if( i == selpkmn ) { continue; }
                            if( SAVE::SAV.getActiveFile( ).getTeamPkmn( i )->canBattle( ) ) {
                                plyerpkmncnt++;
                            }
                        }

                        if( !plyerpkmncnt ) {
                            // no remaining pkmn
                            printMapMessage( GET_MAP_STRING( 485 ), (style) 0 );
                            break;
                        }

                        // actually deposit the pkmn

                        if( SAVE::SAV.getActiveFile( ).getTeamPkmn( selpkmn ) != nullptr )
                            [[likely]] {
                            dc1[ depositpkmn - 1 ]
                                = SAVE::SAV.getActiveFile( ).getTeamPkmn( selpkmn )->m_boxdata;
                            dcl1[ depositpkmn - 1 ]
                                = SAVE::SAV.getActiveFile( ).getTeamPkmn( selpkmn )->m_level;
                            SAVE::SAV.getActiveFile( ).setTeamPkmn( selpkmn,
                                                                    (boxPokemon*) nullptr );
                            SAVE::SAV.getActiveFile( ).consolidatePkmn( );

                            if( selpkmn == 0 ) { MAP::curMap->removeFollowPkmn( ); }

                            snprintf( buffer, 199, GET_MAP_STRING( 486 ),
                                      dc1[ depositpkmn - 1 ].m_name );
                            printMapMessage( buffer, (style) 0 );
                        } else {
                            break;
                        }

                        if( depositpkmn < 2 ) {
                            // ask if player wants to deposit a second pkmn
                            if( IO::yesNoBox::YES
                                == IO::yesNoBox( ).getResult(
                                    convertMapString( GET_MAP_STRING( 480 ), (style) 0 ).c_str( ),
                                    (style) 0 ) ) {
                                IO::init( );
                                depositpkmn = 2;
                            } else {
                                IO::init( );
                                printMapMessage( GET_MAP_STRING( 482 ), (style) 0 );
                                break;
                            }
                        } else {
                            break;
                        }
                    }

                    break;
                }
                case CLL_DAYCARE_JII_SAN: {
                    // day care jii san

                    boxPokemon* dc1 = &SAVE::SAV.getActiveFile( ).m_dayCarePkmn[ par2 * 2 ];
                    boxPokemon* dc2 = &SAVE::SAV.getActiveFile( ).m_dayCarePkmn[ par2 * 2 + 1 ];
                    boxPokemon* dce = &SAVE::SAV.getActiveFile( ).m_dayCareEgg[ par2 ];

                    u8* dcl1 = &SAVE::SAV.getActiveFile( ).m_dayCareDepositLevel[ par2 * 2 ];
                    u8* dcl2 = &SAVE::SAV.getActiveFile( ).m_dayCareDepositLevel[ par2 * 2 + 1 ];

                    if( dce->getSpecies( ) ) {
                        // an egg spawned
                        // ask player if they want to obtain the egg
                        if( IO::yesNoBox::NO
                            == IO::yesNoBox( ).getResult(
                                convertMapString( GET_MAP_STRING( 464 ), (style) 0 ).c_str( ),
                                (style) 0 ) ) {
                            IO::init( );
                            // ask if they really don't want the egg
                            if( IO::yesNoBox::YES
                                == IO::yesNoBox( ).getResult(
                                    convertMapString( GET_MAP_STRING( 465 ), (style) 0 ).c_str( ),
                                    (style) 0 ) ) {
                                IO::init( );
                                // throw away the egg
                                *dce = boxPokemon( );
                                SAVE::SAV.getActiveFile( ).setFlag(
                                    SAVE::F_HOENN_DAYCARE_EGG + par2, false );
                                printMapMessage( GET_MAP_STRING( 466 ), (style) 0 );
                                break;
                            }
                        }
                        IO::init( );
                        // hand egg to player

                        // check if they have space for an egg
                        auto teampkmncnt = SAVE::SAV.getActiveFile( ).getTeamPkmnCount( );
                        if( teampkmncnt >= 6 ) {
                            // player has no space left
                            printMapMessage( GET_MAP_STRING( 473 ), (style) 0 );
                            break;
                        }

                        SOUND::playSoundEffect( SFX_OBTAIN_EGG );
                        printMapMessage( GET_MAP_STRING( 474 ), (style) 1 );
                        printMapMessage( GET_MAP_STRING( 475 ), (style) 0 );

                        dce->m_gotPlace = L_DAY_CARE_COUPLE;

                        SAVE::SAV.getActiveFile( ).setTeamPkmn( teampkmncnt, dce );
                        SAVE::SAV.getActiveFile( ).setFlag( SAVE::F_HOENN_DAYCARE_EGG + par2,
                                                            false );
                        *dce = boxPokemon( );
                    } else {
                        // no egg
                        if( !dc1->getSpecies( ) && dc2->getSpecies( ) ) {
                            std::swap( *dc1, *dc2 );
                            std::swap( *dcl1, *dcl2 );
                        }

                        if( !dc1->getSpecies( ) ) {
                            printMapMessage( GET_MAP_STRING( 463 ), (style) 0 );
                            break;
                        } else {
                            if( !dc2->getSpecies( ) ) {
                                snprintf( buffer, 199, GET_MAP_STRING( 467 ), dc1->m_name );
                                printMapMessage( buffer, (style) 0 );
                            } else {
                                snprintf( buffer, 199, GET_MAP_STRING( 468 ), dc1->m_name,
                                          dc2->m_name );
                                printMapMessage( buffer, (style) 0 );

                                u8 comp = dc1->getCompatibility( *dc2 );
                                printMapMessage( GET_MAP_STRING( 469 + comp ), (style) 0 );
                            }
                            break;
                        }
                    }

                    break;
                }
                case CLL_SAVE_GAME: {
                    // save game, writes 1 to eval reg if successful

                    IO::yesNoBox yn;
                    if( par2 == 1
                        || yn.getResult( GET_STRING( 92 ), MSG_INFO_NOCLOSE )
                               == IO::yesNoBox::YES ) {
                        IO::init( );
                        ANIMATE_MAP = false;
                        u16 lst     = -1;
                        if( FS::writeSave( ARGV[ 0 ], [ & ]( u16 p_perc, u16 p_total ) {
                                u16 stat = p_perc * 18 / p_total;
                                if( stat != lst ) {
                                    lst = stat;
                                    IO::printMessage( 0, MSG_INFO_NOCLOSE );
                                    std::string buf2 = "";
                                    for( u8 i = 0; i < stat; ++i ) {
                                        buf2 += "\x03";
                                        if( i % 3 == 2 ) { buf2 += " "; }
                                    }
                                    for( u8 i = stat; i < 18; ++i ) {
                                        buf2 += "\x04";
                                        if( i % 3 == 2 ) { buf2 += " "; }
                                    }
                                    snprintf( buffer, 99, GET_STRING( 93 ), buf2.c_str( ) );
                                    IO::printMessage( buffer, MSG_INFO_NOCLOSE, true );
                                }
                            } ) ) {
                            IO::printMessage( 0, MSG_INFO_NOCLOSE );
                            SOUND::playSoundEffect( SFX_SAVE );
                            IO::printMessage( GET_STRING( 94 ), MSG_INFO );
                            registers[ 0 ] = 1;
                        } else {
                            IO::printMessage( 0, MSG_INFO_NOCLOSE );
                            IO::printMessage( GET_STRING( 95 ), MSG_INFO );
                            registers[ 0 ] = 0;
                        }
                        ANIMATE_MAP = true;
                    } else {
                        registers[ 0 ] = 0;
                        IO::init( );
                    }
                    break;
                }
                case CLL_HOURS_MOD: {
                    registers[ 0 ] = SAVE::CURRENT_TIME.m_hours % par2;
                    break;
                }
                case CLL_PLAYTIME_HOURS: {
                    registers[ 0 ] = SAVE::SAV.getActiveFile( ).m_playTime.m_hours;
                    break;
                }
                case CLL_HALL_OF_FAME: {
                    // fade screen
                    // set current player position to position home
                    removeFollowPkmn( );
                    ANIMATE_MAP = false;

                    SAVE::SAV.getActiveFile( ).m_currentMap         = 20;
                    SAVE::SAV.getActiveFile( ).m_player.m_direction = MAP::DOWN;

                    if( SAVE::SAV.getActiveFile( ).checkFlag( SAVE::F_RIVAL_APPEARANCE ) ) {
                        // TODO: move to FSINFO
                        SAVE::SAV.getActiveFile( ).m_player = MAP::mapPlayer(
                            { 0x2b, 0x89, 3 }, u16( 10 * SAVE::SAV.getActiveFile( ).m_appearance ),
                            MAP::moveMode::WALK );
                    } else {
                        // TODO: move to FSINFO
                        SAVE::SAV.getActiveFile( ).m_player = MAP::mapPlayer(
                            { 0x31, 0xa9, 3 }, u16( 10 * SAVE::SAV.getActiveFile( ).m_appearance ),
                            MAP::moveMode::WALK );
                    }

                    // heal party pkmn
                    for( u8 i = 0; i < SAVE::SAV.getActiveFile( ).getTeamPkmnCount( ); ++i ) {
                        auto tmp = SAVE::SAV.getActiveFile( ).getTeamPkmn( i );
                        if( tmp ) {
                            tmp->heal( );
                            // award champion ribbon
                            tmp->m_boxdata.awardRibbon( 20 );
                        }
                    }

                    // add star to trainers card (if it doesn't exist already)
                    SAVE::SAV.getActiveFile( ).registerAchievement(
                        SAVE::saveGame::playerInfo::ACHIEVEMENT_HALL_OF_FAME );

                    // add achievement
                    SAVE::SAV.getActiveFile( ).m_lastAchievementDate  = SAVE::CURRENT_DATE;
                    SAVE::SAV.getActiveFile( ).m_lastAchievementEvent = 9; // hall of fame
                                                                           // message

                    // save game
                    u16 lst = -1;
                    if( FS::writeSave( ARGV[ 0 ], [ & ]( u16 p_perc, u16 p_total ) {
                            u16 stat = p_perc * 18 / p_total;
                            if( stat != lst ) {
                                lst = stat;
                                IO::printMessage( 0, MSG_INFO_NOCLOSE );
                                std::string buf2 = "";
                                for( u8 i = 0; i < stat; ++i ) {
                                    buf2 += "\x03";
                                    if( i % 3 == 2 ) { buf2 += " "; }
                                }
                                for( u8 i = stat; i < 18; ++i ) {
                                    buf2 += "\x04";
                                    if( i % 3 == 2 ) { buf2 += " "; }
                                }
                                snprintf( buffer, 99, GET_STRING( 93 ), buf2.c_str( ) );
                                IO::printMessage( buffer, MSG_INFO_NOCLOSE, true );
                            }
                        } ) ) {
                        IO::printMessage( 0, MSG_INFO_NOCLOSE );
                        SOUND::playSoundEffect( SFX_SAVE );
                        IO::printMessage( GET_STRING( 94 ), MSG_INFO );
                    } else {
                        IO::printMessage( 0, MSG_INFO_NOCLOSE );
                        IO::printMessage( GET_STRING( 95 ), MSG_INFO );
                    }

                    IO::fadeScreen( IO::CLEAR_DARK_IMMEDIATE, true, true );
                    videoSetMode( MODE_5_2D );
                    IO::clearScreen( true, true, true );
                    IO::initVideo( true );
                    IO::initOAMTable( true );
                    IO::initOAMTable( false );
                    bgUpdate( );

                    // show hall of fame screen
                    // show credits
                    // reset game
                    RESET_GAME = true;
                    return;
                }
                default: break;
                }
                break;
            case COU: {
                style st       = MSG_INFO_NOCLOSE;
                registers[ 0 ] = IO::counter( 0, parB ).getResult(
                    convertMapString( GET_MAP_STRING( parA ), st ).c_str( ), st );
                IO::init( );
                break;
            }
            case COUR: {
                style st = MSG_INFO_NOCLOSE;
                registers[ 0 ]
                    = IO::counter( 0, registers[ parB ] )
                          .getResult( convertMapString( GET_MAP_STRING( parA ), st ).c_str( ), st );
                IO::init( );
#ifdef DESQUID_MORE
                std::string dstr = "";
                for( u8 q = 0; q < 10; ++q ) { dstr += std::to_string( registers[ q ] ) + " "; }
                IO::printMessage( dstr, MSG_INFO );
#endif
                break;
            }

            case YNM: {
                style st        = (style) parB;
                bool  showMoney = st & MSG_SHOW_MONEY_FLAG;
                st              = (style) ( st & 127 );

                if( st == MSG_NORMAL ) { st = MSG_NOCLOSE; }
                if( st == MSG_INFO ) { st = MSG_INFO_NOCLOSE; }

                registers[ 0 ]
                    = IO::yesNoBox::YES
                      == IO::yesNoBox( ).getResult(
                          convertMapString( GET_MAP_STRING( parA ), st ).c_str( ), st, showMoney );
                IO::init( );
                break;
            }
            case MSG: printMapMessage( GET_MAP_STRING( parA ), (style) parB ); break;
            default: break;
            }
            ++pc;
        }
        CURRENT_SCRIPT = -1;
        _scriptRunning = srn;

#ifdef DESQUID_MORE
        IO::printMessage( "SCRIPT END" );
#endif
    }

    void mapDrawer::interactFollowPkmn( ) {
        if( _followPkmnData == nullptr ) { return; }

        // rotate pkmn sprite to face player
        direction d = direction( ( SAVE::SAV.getActiveFile( ).m_player.m_direction + 2 ) % 4 );
        _mapSprites.setFrameD( _playerFollowPkmnSprite, d );

        // if follow pkmn is disguised, undisguise it
        if( _followPkmnIsDisguised ) {
            _followPkmnIsDisguised    = false;
            _followPkmnDisguiseBusted = true;
            updateFollowPkmn( );
            _followPkmnIsDisguised    = true;
            _followPkmnDisguiseBusted = false;
        }

        // play pkmn cry
        SOUND::playCry( _followPkmnData->getSpecies( ), _followPkmnData->getForme( ) );

        u8   emotion       = 0;
        char buffer[ 200 ] = { 0 };

        // first pick a random message type out of:
        //
        // 0 - special map location interaction (TODO)
        // 1  - item found (TODO)
        // 2 - general message
        // 3 - special map tile interaction (standing on sand/water/etc) (TODO)
        // 4 - special pkmn message (TODO)
        // 5 - hp / status condition message
        // 6 - special type interaction (some message specific to the pkmn's type) (TODO)
        // 7 - special weather interaction (TODO)
        // 8 - happiness message
        u8 msgtype = rand( ) % 9;

        switch( msgtype ) {
        case 0: {
            u8 msg = 0;
            switch( getCurrentLocationId( ) ) {
            case L_FIERY_DEN:
            case L_FIERY_PATH:
            case L_MT_PYRE:
            case L_TEAM_MAGMA_HIDEOUT:
            case L_SCORCHED_SLAB:
                if( _followPkmnSpeciesData ) {
                    if( _followPkmnSpeciesData->m_baseForme.m_types[ 0 ] == BATTLE::TYPE_FIRE
                        || _followPkmnSpeciesData->m_baseForme.m_types[ 1 ] == BATTLE::TYPE_FIRE ) {
                        msg     = 50 + ( rand( ) & 1 );
                        emotion = 2;
                        break;
                    }
                    if( _followPkmnSpeciesData->m_baseForme.m_types[ 0 ] == BATTLE::TYPE_WATER
                        || _followPkmnSpeciesData->m_baseForme.m_types[ 1 ]
                               == BATTLE::TYPE_WATER ) {
                        msg     = 52;
                        emotion = 0;
                        break;
                    }
                    if( _followPkmnSpeciesData->m_baseForme.m_types[ 0 ] == BATTLE::TYPE_ICE
                        || _followPkmnSpeciesData->m_baseForme.m_types[ 1 ] == BATTLE::TYPE_ICE ) {
                        msg     = 52 + ( rand( ) & 1 );
                        emotion = 8;
                        break;
                    }
                }
            default: break;
            }
            if( msg ) {
                snprintf( buffer, PKMNPHRS_LEN, FS::getPkmnPhrase( msg ),
                          _followPkmnData->m_boxdata.m_name );
                break;
            }

            [[fallthrough]];
        }
        case 1:
        case 2:
        default: {
            emotion = rand( ) % 12;
            while( emotion == 10 ) { emotion = rand( ) % 12; } // ignore poison
            u8 msg = rand( ) % 3;
            snprintf( buffer, PKMNPHRS_LEN, FS::getPkmnPhrase( 3 * emotion + msg ),
                      _followPkmnData->m_boxdata.m_name );
            break;
        }

        case 3:
        case 4:
        case 5: {
            u8 msg = 0;
            if( _followPkmnData->m_statusint ) {
                if( _followPkmnData->m_status.m_isPoisoned
                    || _followPkmnData->m_status.m_isBadlyPoisoned ) {
                    emotion = 10;
                    msg     = 36;
                } else if( _followPkmnData->m_status.m_isAsleep ) {
                    emotion = 1;
                    msg     = 37;
                } else if( _followPkmnData->m_status.m_isBurned ) {
                    emotion = 11;
                    msg     = 38;
                } else if( _followPkmnData->m_status.m_isFrozen ) {
                    emotion = 4;
                    msg     = 39;
                } else if( _followPkmnData->m_status.m_isParalyzed ) {
                    emotion = 11;
                    msg     = 40;
                }
            } else {
                if( _followPkmnData->m_stats.m_curHP * 8 < _followPkmnData->m_stats.m_maxHP ) {
                    emotion = 11;
                    msg     = 41;
                } else if( _followPkmnData->m_stats.m_curHP * 4
                           < _followPkmnData->m_stats.m_maxHP ) {
                    emotion = 11;
                    msg     = 42;
                } else if( _followPkmnData->m_stats.m_curHP * 2
                           < _followPkmnData->m_stats.m_maxHP ) {
                    emotion = 11;
                    msg     = 40;
                } else if( _followPkmnData->m_stats.m_curHP * 4
                           < _followPkmnData->m_stats.m_maxHP * 3 ) {
                    emotion = 3;
                    msg     = 43;
                } else {
                    emotion = 7;
                    msg     = 44;
                }
            }
            snprintf( buffer, PKMNPHRS_LEN, FS::getPkmnPhrase( msg ),
                      _followPkmnData->m_boxdata.m_name );
            break;
        }

        case 6:
        case 7:
        case 8: {
            u8 msg = 0;
            // happiness
            if( _followPkmnData->m_boxdata.m_steps < 60 ) {
                emotion = 8;
                msg     = 45;
            } else if( _followPkmnData->m_boxdata.m_steps < 120 ) {
                emotion = 1;
                msg     = 46;
            } else if( _followPkmnData->m_boxdata.m_steps < 185 ) {
                emotion = 3;
                msg     = 47;
            } else if( _followPkmnData->m_boxdata.m_steps < 250 ) {
                emotion = 7;
                msg     = 48;
            } else {
                emotion = 6;
                msg     = 49;
            }
            snprintf( buffer, PKMNPHRS_LEN, FS::getPkmnPhrase( msg ),
                      _followPkmnData->m_boxdata.m_name );
            break;
        }
        }

        if( emotion ) {
            // Show the emotion icon
            _mapSprites.showExclamation( _playerFollowPkmnSprite, 2 * emotion );
            for( u8 i = 0; i < 15; ++i ) { swiWaitForVBlank( ); }
            _mapSprites.showExclamation( _playerFollowPkmnSprite, 2 * emotion + 1 );
            for( u8 i = 0; i < 15; ++i ) { swiWaitForVBlank( ); }
            _mapSprites.hideExclamation( );
        }

        // print message
        printMapMessage( buffer, style( 0 ) );
    }

    void mapDrawer::interact( ) {
        u16  px = SAVE::SAV.getActiveFile( ).m_player.m_pos.m_posX;
        u16  py = SAVE::SAV.getActiveFile( ).m_player.m_pos.m_posY;
        u16  pz = SAVE::SAV.getActiveFile( ).m_player.m_pos.m_posZ;
        auto d  = SAVE::SAV.getActiveFile( ).m_player.m_direction;
        u16  tx = px + dir[ d ][ 0 ];
        u16  ty = py + dir[ d ][ 1 ];

        // Check for follow pkmn
        if( _pkmnFollowsPlayer && _followPkmn.m_pos.m_posX == tx
            && _followPkmn.m_pos.m_posY == ty ) {
            interactFollowPkmn( );
            return;
        }

        // Check for special blocks

        u8 behave = at( tx, ty ).m_bottombehave;

        switch( behave ) {
        case 0x83: { // PC
            SOUND::playSoundEffect( SFX_PC_OPEN );
            atom( tx, ty ).m_blockidx = 0x5;
            loadBlock( at( tx, ty ), ( _lastcol + NUM_COLS / 2 ) % NUM_COLS,
                       ( _lastrow + NUM_ROWS / 2 ) % NUM_ROWS );
            bgUpdate( );
            for( u8 i = 0; i < 3; ++i ) { swiWaitForVBlank( ); }
            IO::printMessage( GET_STRING( 559 ), MSG_INFO );

            BOX::boxViewer bxv;
            ANIMATE_MAP = false;
            videoSetMode( MODE_5_2D );
            bgUpdate( );
            SOUND::dimVolume( );

            bxv.run( );

            FADE_TOP_DARK( );
            FADE_SUB_DARK( );
            IO::clearScreen( false );
            videoSetMode( MODE_5_2D );
            bgUpdate( );

            IO::initVideoSub( );
            IO::resetScale( true, false );
            ANIMATE_MAP = true;
            SOUND::restoreVolume( );
            IO::init( );
            draw( );

            SOUND::playSoundEffect( SFX_PC_CLOSE );
            atom( tx, ty ).m_blockidx = 0x4;
            loadBlock( at( tx, ty ), ( _lastcol + NUM_COLS / 2 ) % NUM_COLS,
                       ( _lastrow + NUM_ROWS / 2 ) % NUM_ROWS );
            bgUpdate( );
            for( u8 i = 0; i < 3; ++i ) { swiWaitForVBlank( ); }

            return;
        }
        case 0x85: { // Map
            // TODO
            printMapMessage( GET_STRING( 560 ), MSG_NORMAL );
            return;
        }
        case 0x86: { // TV
            printMapMessage( GET_MAP_STRING( 26 ), MSG_NORMAL );
            return;
        }

        case 0xe0: { // picture books
            printMapMessage( GET_MAP_STRING( 133 ), MSG_NORMAL );
            break;
        }
        case 0xe1: { // picture books
            printMapMessage( GET_MAP_STRING( 134 ), MSG_NORMAL );
            break;
        }
        case 0xe2: { // PokeCenter magazines
            printMapMessage( GET_MAP_STRING( 30 ), MSG_NORMAL );
            break;
        }
        case 0xe4: { // trash bin is empty
            if( !currentData( ).hasEvent( EVENT_ITEM, px, py, pz ) ) {
                printMapMessage( GET_MAP_STRING( 404 ), MSG_NORMAL );
            }
            break;
        }
        case 0xe5: { // Pokemart shelves
            printMapMessage( GET_MAP_STRING( 127 ), MSG_NORMAL );
            break;
        }
        case 0xe6: { // blue prints
            printMapMessage( GET_MAP_STRING( 396 ), MSG_NORMAL );
            break;
        }
        case 0x80: { // load script one block behind
            handleEvents( tx, ty, pz, d );
            return;
        }
            [[likely]] default : break;
        }
        handleEvents( px, py, pz, d );
    }

    void mapDrawer::runEvent( mapData::event p_event, u8 p_objectId, s16 p_mapX, s16 p_mapY ) {
        u16 curx = SAVE::SAV.getActiveFile( ).m_player.m_pos.m_posX;
        u16 cury = SAVE::SAV.getActiveFile( ).m_player.m_pos.m_posY;
        u16 mapX = curx / SIZE, mapY = cury / SIZE;

        auto oldforce = _forceNoFollow;

        switch( p_event.m_type ) {
        case EVENT_MESSAGE:
            printMapMessage( GET_MAP_STRING( p_event.m_data.m_message.m_msgId ),
                             (style) p_event.m_data.m_message.m_msgType );
            break;
        case EVENT_NPC_MESSAGE: {
            u8 tp = p_event.m_data.m_npc.m_scriptType & 127;
            if( tp == 10 || tp == 11 ) { tp = 0; }
            printMapMessage( GET_MAP_STRING( p_event.m_data.m_npc.m_scriptId ), (style) tp );
            if( p_event.m_data.m_npc.m_scriptType & 128 ) {
                SAVE::SAV.getActiveFile( ).setFlag( p_event.m_deactivateFlag, true );
            }
            break;
        }
        case EVENT_OW_PKMN: {
            if( !SAVE::SAV.getActiveFile( ).checkFlag( p_event.m_deactivateFlag ) ) {
                ANIMATE_MAP = false;
                DRAW_TIME   = false;
                resetTracerChain( true );
                u16  pkmnIdx = p_event.m_data.m_owPkmn.m_speciesId;
                u8   level   = p_event.m_data.m_owPkmn.m_level;
                u8   forme   = p_event.m_data.m_owPkmn.m_forme & ( ~( ( 1 << 6 ) | ( 1 << 7 ) ) );
                bool female  = !!( p_event.m_data.m_owPkmn.m_forme & ( 1 << 6 ) );
                // bool genderless = !!( p_event.m_data.m_owPkmn.m_forme & ( 1 << 7 ) );

                u8   shiny    = p_event.m_data.m_owPkmn.m_shiny & ( ~( ( 1 << 6 ) | ( 1 << 7 ) ) );
                bool hiddenab = !!( p_event.m_data.m_owPkmn.m_shiny & ( 1 << 6 ) );
                bool fateful  = !!( p_event.m_data.m_owPkmn.m_shiny & ( 1 << 7 ) );

                SOUND::playCry( pkmnIdx, forme, female );
                swiWaitForVBlank( );
                swiWaitForVBlank( );

                bool luckyenc = SAVE::SAV.getActiveFile( ).m_bag.count(
                                    BAG::toBagType( BAG::ITEMTYPE_KEYITEM ), I_WISHING_CHARM )
                                    ? !( rand( ) & 127 )
                                    : !( rand( ) & 2047 );
                bool charm    = SAVE::SAV.getActiveFile( ).m_bag.count(
                    BAG::toBagType( BAG::ITEMTYPE_KEYITEM ), I_SHINY_CHARM );

                wildPkmnType btType
                    = SAVE::SAV.getActiveFile( ).m_player.m_movement == SURF ? WATER : GRASS;

                prepareBattleWildPkmn( btType, pkmnIdx, luckyenc );

                WILD_PKMN = pokemon( pkmnIdx, level, forme, 0,
                                     shiny ? shiny : ( luckyenc ? 255 : ( charm ? 3 : 0 ) ),
                                     hiddenab || luckyenc, false, 0, 0, fateful || luckyenc );

                SAVE::SAV.getActiveFile( ).setFlag( p_event.m_deactivateFlag, 1 );
                auto battleres = battleWildPkmn( btType );
                if( battleres == BATTLE::battle::BATTLE_OPPONENT_WON ) {
                    SAVE::SAV.getActiveFile( ).setFlag( p_event.m_deactivateFlag, 0 );
                    return;
                }
                if( battleres != BATTLE::battle::BATTLE_CAPTURE ) {
                    char buffer[ 100 ];
                    snprintf( buffer, 99, GET_STRING( 672 ), WILD_PKMN.m_boxdata.m_name );
                    printMapMessage( std::string( buffer ), MSG_NORMAL );
                }
            }
            break;
        }
        case EVENT_TRAINER: {
            auto tr = FS::getBattleTrainer( p_event.m_data.m_trainer.m_trainerId );
            if( !SAVE::SAV.getActiveFile( ).checkFlag(
                    SAVE::F_TRAINER_BATTLED( p_event.m_data.m_trainer.m_trainerId ) ) ) {
                // player did not defeat the trainer yet

                printMapMessage( tr.m_strings.m_message1, MSG_NORMAL );

                auto playerPrio = _mapSprites.getPriority( _playerSprite );

                SOUND::playBGM( SOUND::BGMforTrainerBattle( tr.m_data.m_trainerClass ) );

                // Check the trainer class, for some classes start a double battle
                auto policy = BATTLE::isDoubleBattleTrainerClass( tr.m_data.m_trainerClass )
                                  ? BATTLE::DEFAULT_DOUBLE_TRAINER_POLICY
                                  : BATTLE::DEFAULT_TRAINER_POLICY;

                FADE_TOP_DARK( );
                ANIMATE_MAP = false;
                swiWaitForVBlank( );

                BATTLE::battle bt
                    = BATTLE::battle( SAVE::SAV.getActiveFile( ).m_pkmnTeam,
                                      SAVE::SAV.getActiveFile( ).getTeamPkmnCount( ), tr, policy );
                if( bt.start( ) == BATTLE::battle::BATTLE_OPPONENT_WON ) {
                    faintPlayer( );
                    return;
                } else {
                    SAVE::SAV.getActiveFile( ).setFlag(
                        SAVE::F_TRAINER_BATTLED( p_event.m_data.m_trainer.m_trainerId ), true );
                }
                FADE_TOP_DARK( );
                IO::init( );
                draw( playerPrio );
                SOUND::restartBGM( );
                ANIMATE_MAP = true;
            } else {
                printMapMessage( tr.m_strings.m_message2, MSG_NORMAL );
            }
            break;
        }
        case EVENT_HMOBJECT: {
            switch( p_event.m_data.m_hmObject.m_hmType ) {
            case mapSpriteManager::SPR_STRENGTH:
                if( _strengthUsed ) {
                    IO::printMessage( GET_STRING( 558 ), MSG_NORMAL );
                } else {
                    IO::printMessage( GET_STRING( 318 ), MSG_NORMAL );
                }
                break;
            case mapSpriteManager::SPR_ROCKSMASH:
                IO::printMessage( GET_STRING( 314 ), MSG_NORMAL );
                break;
            case mapSpriteManager::SPR_CUT:
                IO::printMessage( GET_STRING( 313 ), MSG_NORMAL );
                break;
            default: break;
            }
            break;
        }
        case EVENT_NPC: {
            executeScript( p_event.m_data.m_npc.m_scriptId, p_objectId );
            break;
        }
        case EVENT_GENERIC: {
            executeScript( p_event.m_data.m_generic.m_scriptId, 0, p_mapX, p_mapY );
            break;
        }
        case EVENT_ITEM: {
            IO::giveItemToPlayer( p_event.m_data.m_item.m_itemId, 1 );
            SAVE::SAV.getActiveFile( ).setFlag( p_event.m_deactivateFlag, 1 );
            constructAndAddNewMapObjects( currentData( ), mapX, mapY );
        }
        default: break;
        }
        _forceNoFollow = oldforce;
    }

    void mapDrawer::handleEvents( u16 p_globX, u16 p_globY, u8 p_z ) {
        u8 x = p_globX % SIZE;
        u8 y = p_globY % SIZE;
        u8 z = p_z;

        auto mdata = currentData( );

        for( u8 i = 0; i < MAX_EVENTS_PER_SLICE; ++i ) {
            if( mdata.m_events[ i ].m_posX != x || mdata.m_events[ i ].m_posY != y
                || mdata.m_events[ i ].m_posZ != z ) {
                continue;
            }
            if( mdata.m_events[ i ].m_activateFlag
                && !SAVE::SAV.getActiveFile( ).checkFlag( mdata.m_events[ i ].m_activateFlag ) ) {
                continue;
            }
            if( mdata.m_events[ i ].m_deactivateFlag
                && SAVE::SAV.getActiveFile( ).checkFlag( mdata.m_events[ i ].m_deactivateFlag ) ) {
                continue;
            }
            if( ( mdata.m_events[ i ].m_route )
                && mdata.m_events[ i ].m_route != SAVE::SAV.getActiveFile( ).m_route ) {
                continue;
            }

            if( mdata.m_events[ i ].m_type == EVENT_FLY_POS ) {
                // register fly pos
                SAVE::SAV.getActiveFile( ).registerFlyPos(
                    flyPos{ mdata.m_events[ i ].m_data.m_flyPos.m_location,
                            SAVE::SAV.getActiveFile( ).m_currentMap, p_z, p_globX, p_globY } );
                continue;
            }

            if( mdata.m_events[ i ].m_type != EVENT_MESSAGE
                && mdata.m_events[ i ].m_type != EVENT_GENERIC ) {
                // These events have associated map objects
                continue;
            }
            if( mdata.m_events[ i ].m_type == EVENT_GENERIC ) {
                if( mdata.m_events[ i ].m_data.m_generic.m_scriptType == 11
                    && SAVE::SAV.getActiveFile( ).checkFlag( SAVE::F_RIVAL_APPEARANCE ) ) {
                    continue;
                }
                if( mdata.m_events[ i ].m_data.m_generic.m_scriptType == 10
                    && !SAVE::SAV.getActiveFile( ).checkFlag( SAVE::F_RIVAL_APPEARANCE ) ) {
                    continue;
                }
            }
            if( mdata.m_events[ i ].m_trigger == TRIGGER_STEP_ON ) {
                runEvent( mdata.m_events[ i ] );
            }
        }

        // check if player moved to different position; may need to check for events at
        // new position
        if( p_globX != SAVE::SAV.getActiveFile( ).m_player.m_pos.m_posX
            || p_globY != SAVE::SAV.getActiveFile( ).m_player.m_pos.m_posY
            || p_z != SAVE::SAV.getActiveFile( ).m_player.m_pos.m_posZ ) {
            handleEvents( SAVE::SAV.getActiveFile( ).m_player.m_pos.m_posX,
                          SAVE::SAV.getActiveFile( ).m_player.m_pos.m_posY,
                          SAVE::SAV.getActiveFile( ).m_player.m_pos.m_posZ );
        }
    }

    void mapDrawer::handleEvents( u16 p_globX, u16 p_globY, u8 p_z, direction p_dir ) {
        p_globX += dir[ p_dir ][ 0 ];
        p_globY += dir[ p_dir ][ 1 ];

        u8 x = p_globX % SIZE;
        u8 y = p_globY % SIZE;
        u8 z = p_z;

        u16 mapX = p_globX / SIZE, mapY = p_globY / SIZE;

        auto mdata = currentData( p_globX, p_globY );
        for( u8 i = 0; i < MAX_EVENTS_PER_SLICE; ++i ) {

            if( mdata.m_events[ i ].m_posX != x || mdata.m_events[ i ].m_posY != y
                || mdata.m_events[ i ].m_posZ != z ) {
                continue;
            }
            if( mdata.m_events[ i ].m_activateFlag
                && !SAVE::SAV.getActiveFile( ).checkFlag( mdata.m_events[ i ].m_activateFlag ) ) {
                continue;
            }
            if( mdata.m_events[ i ].m_type != EVENT_TRAINER && mdata.m_events[ i ].m_deactivateFlag
                && SAVE::SAV.getActiveFile( ).checkFlag( mdata.m_events[ i ].m_deactivateFlag ) ) {
                continue;
            }
            if( ( mdata.m_events[ i ].m_route )
                && mdata.m_events[ i ].m_route != SAVE::SAV.getActiveFile( ).m_route ) {
                continue;
            }

            if( mdata.m_events[ i ].m_type != EVENT_MESSAGE
                && mdata.m_events[ i ].m_type != EVENT_GENERIC
                && mdata.m_events[ i ].m_type != EVENT_BERRYTREE ) {
                // These events have associated map objects
                continue;
            }

            if( mdata.m_events[ i ].m_type == EVENT_BERRYTREE ) {
                if( !SAVE::SAV.getActiveFile( ).berryIsAlive(
                        mdata.m_events[ i ].m_data.m_berryTree.m_treeIdx ) ) {
                    SAVE::SAV.getActiveFile( ).harvestBerry(
                        mdata.m_events[ i ].m_data.m_berryTree.m_treeIdx );
                }

                u8 berryType = SAVE::SAV.getActiveFile( ).getBerry(
                    mdata.m_events[ i ].m_data.m_berryTree.m_treeIdx );

                if( !berryType ) {
                    //  ask if player wants to plant a berry
                    if( IO::yesNoBox( ).getResult( GET_STRING( 571 ), MSG_INFO_NOCLOSE )
                        == IO::yesNoBox::YES ) {

                        FADE_TOP_DARK( );
                        ANIMATE_MAP = false;
                        swiWaitForVBlank( );

                        IO::clearScreen( false );
                        videoSetMode( MODE_5_2D );
                        bgUpdate( );

                        auto playerPrio = _mapSprites.getPriority( _playerSprite );

                        // Make player choose berry
                        SOUND::dimVolume( );
                        BAG::bagViewer bv  = BAG::bagViewer( SAVE::SAV.getActiveFile( ).m_pkmnTeam,
                                                             BAG::bagViewer::CHOOSE_BERRY );
                        u16            itm = bv.getItem( true );

                        FADE_TOP_DARK( );
                        FADE_SUB_DARK( );
                        IO::clearScreen( false );
                        videoSetMode( MODE_5_2D );
                        bgUpdate( );

                        if( itm ) {
                            // plant the berry
                            SAVE::SAV.getActiveFile( ).plantBerry(
                                mdata.m_events[ i ].m_data.m_berryTree.m_treeIdx, itm );
                        }

                        FADE_TOP_DARK( );
                        IO::init( );
                        draw( playerPrio );
                        _mapSprites.setPriority( _playerSprite,
                                                 SAVE::SAV.getActiveFile( ).m_playerPriority
                                                 = playerPrio );
                        SOUND::restartBGM( );
                        ANIMATE_MAP = true;
                        SOUND::restoreVolume( );
                        if( itm ) {
                            char buffer[ 100 ];
                            snprintf( buffer, 99, GET_STRING( 572 ),
                                      FS::getItemName( itm ).c_str( ) );
                            IO::printMessage( buffer, MSG_INFO );
                        }
                        return;
                    } else {
                        IO::init( );
                    }
                    continue;
                }
            }

            if( mdata.m_events[ i ].m_type == EVENT_GENERIC ) {
                if( mdata.m_events[ i ].m_data.m_generic.m_scriptType == 11
                    && SAVE::SAV.getActiveFile( ).checkFlag( SAVE::F_RIVAL_APPEARANCE ) ) {
                    continue;
                }
                if( mdata.m_events[ i ].m_data.m_generic.m_scriptType == 10
                    && !SAVE::SAV.getActiveFile( ).checkFlag( SAVE::F_RIVAL_APPEARANCE ) ) {
                    continue;
                }
            }

            if( mdata.m_events[ i ].m_trigger == TRIGGER_NONE ) { continue; }
            if( mdata.m_events[ i ].m_trigger & dirToEventTrigger( p_dir ) ) {
                runEvent( mdata.m_events[ i ] );
            }
        }

        for( u8 i = 0; i < SAVE::SAV.getActiveFile( ).m_mapObjectCount; ++i ) {
            auto& o = SAVE::SAV.getActiveFile( ).m_mapObjects[ i ];

            if( o.second.m_pos.m_posX != p_globX || o.second.m_pos.m_posY != p_globY
                || o.second.m_pos.m_posZ != z ) {
                continue;
            }

            if( o.second.m_movement != NO_MOVEMENT && o.second.m_currentMovement.m_frame ) {
                continue;
            }

            auto old = o.second.m_movement;

            if( o.second.m_event.m_type == EVENT_BERRYTREE ) {
                u8 stage = SAVE::SAV.getActiveFile( ).getBerryStage(
                    o.second.m_event.m_data.m_berryTree.m_treeIdx );
                u8 berryType = SAVE::SAV.getActiveFile( ).getBerry(
                    o.second.m_event.m_data.m_berryTree.m_treeIdx );
                u8 yield = SAVE::SAV.getActiveFile( ).getBerryYield(
                    o.second.m_event.m_data.m_berryTree.m_treeIdx );

                if( !berryType ) {
                    // handled later
                    continue;
                }

                char buffer[ 100 ];

                switch( stage ) {
                case 0:
                case 1:
                case 2:
                case 3:
                    snprintf( buffer, 99, GET_STRING( 565 + stage ),
                              FS::getItemName( BAG::berryToItem( berryType ) ).c_str( ) );
                    break;
                case 4:
                    snprintf( buffer, 99, GET_STRING( 569 ),
                              FS::getItemName( BAG::berryToItem( berryType ) ).c_str( ), yield );
                    break;
                default: continue;
                }
                IO::printMessage( buffer, MSG_INFO_CONT );
                if( stage == 4 ) {
                    // Berries can be harvested
                    if( IO::yesNoBox( ).getResult( GET_STRING( 570 ), MSG_INFO_NOCLOSE )
                        == IO::yesNoBox::YES ) {
                        IO::init( );
                        IO::giveItemToPlayer( BAG::berryToItem( berryType ), yield );
                        SAVE::SAV.getActiveFile( ).harvestBerry(
                            o.second.m_event.m_data.m_berryTree.m_treeIdx );
                        _mapSprites.destroySprite( o.first );
                        o.first = 255;
                        constructAndAddNewMapObjects( currentData( p_globX, p_globY ), mapX, mapY );
                    }
                    IO::init( );
                } else {
                    // Ask player if they want to water the berry
                    // (only if they have a watering can, though)
                    if( SAVE::SAV.getActiveFile( ).m_bag.count(
                            BAG::toBagType( BAG::ITEMTYPE_KEYITEM ), I_SPRAYDUCK )
                        + SAVE::SAV.getActiveFile( ).m_bag.count(
                            BAG::toBagType( BAG::ITEMTYPE_KEYITEM ), I_SPRINKLOTAD )
                        + SAVE::SAV.getActiveFile( ).m_bag.count(
                            BAG::toBagType( BAG::ITEMTYPE_KEYITEM ), I_WAILMER_PAIL )
                        + SAVE::SAV.getActiveFile( ).m_bag.count(
                            BAG::toBagType( BAG::ITEMTYPE_KEYITEM ), I_SQUIRT_BOTTLE ) ) {

                        if( IO::yesNoBox( ).getResult( GET_STRING( 574 ), MSG_INFO_NOCLOSE )
                            == IO::yesNoBox::YES ) {
                            IO::init( );
                            IO::printMessage( GET_STRING( 573 ), MSG_INFO );
                            SAVE::SAV.getActiveFile( ).waterBerry(
                                o.second.m_event.m_data.m_berryTree.m_treeIdx );
                        } else {
                            IO::init( );
                        }
                    } else {
                        IO::init( );
                    }
                }
                continue;
            } else {
                // rotate sprite to player
                if( o.second.m_picNum > 1000 || ( o.second.m_picNum & 0xff ) <= 240 ) {
                    o.second.m_movement = NO_MOVEMENT;
                    _mapSprites.setFrameD( o.first, direction( ( u8( p_dir ) + 2 ) % 4 ) );
                }
            }

            if( o.second.m_event.m_type == EVENT_TRAINER ) {
                // Check for exclamation mark / music change

                if( !SAVE::SAV.getActiveFile( ).checkFlag( SAVE::F_TRAINER_BATTLED(
                        o.second.m_event.m_data.m_trainer.m_trainerId ) ) ) {
                    // player did not defeat the trainer yet
                    showExclamationAboveMapObject( i );
                    auto tr = FS::getBattleTrainer( o.second.m_event.m_data.m_trainer.m_trainerId );
                    SOUND::playBGM( SOUND::BGMforTrainerEncounter( tr.m_data.m_trainerClass ) );
                }
            }

            u16 curx = SAVE::SAV.getActiveFile( ).m_player.m_pos.m_posX;
            u16 cury = SAVE::SAV.getActiveFile( ).m_player.m_pos.m_posY;

            runEvent( o.second.m_event, i );
            if( curx == SAVE::SAV.getActiveFile( ).m_player.m_pos.m_posX
                && cury == SAVE::SAV.getActiveFile( ).m_player.m_pos.m_posY ) {
                o.second.m_movement = old;
            }
        }
    }

    bool mapData::hasEvent( eventType p_type, u8 p_x, u8 p_y, u8 p_z ) const {
        for( u8 i = 0; i < MAX_EVENTS_PER_SLICE; ++i ) {
            if( m_events[ i ].m_posX == p_x && m_events[ i ].m_posY == p_y
                && m_events[ i ].m_posZ == p_z && m_events[ i ].m_type == p_type ) {
                if( m_events[ i ].m_activateFlag
                    && !SAVE::SAV.getActiveFile( ).checkFlag( m_events[ i ].m_activateFlag ) ) {
                    continue;
                }
                if( m_events[ i ].m_deactivateFlag
                    && SAVE::SAV.getActiveFile( ).checkFlag( m_events[ i ].m_deactivateFlag ) ) {
                    continue;
                }
                if( ( m_events[ i ].m_route )
                    && m_events[ i ].m_route != SAVE::SAV.getActiveFile( ).m_route ) {
                    continue;
                }

                return true;
            }
        }
        return false;
    }

    void mapDrawer::runLevelScripts( const mapData& p_data, u16 p_mapX, u16 p_mapY ) {
        for( u8 i = 0; i < MAX_EVENTS_PER_SLICE; ++i ) {
            if( p_data.m_events[ i ].m_trigger != TRIGGER_ON_MAP_ENTER ) { continue; }

            if( p_data.m_events[ i ].m_activateFlag
                && !SAVE::SAV.getActiveFile( ).checkFlag( p_data.m_events[ i ].m_activateFlag ) ) {
                continue;
            }
            if( p_data.m_events[ i ].m_deactivateFlag
                && SAVE::SAV.getActiveFile( ).checkFlag( p_data.m_events[ i ].m_deactivateFlag ) ) {
                continue;
            }
            if( ( p_data.m_events[ i ].m_route )
                && p_data.m_events[ i ].m_route != SAVE::SAV.getActiveFile( ).m_route ) {
                continue;
            }

            runEvent( p_data.m_events[ i ], u8( 0 ), s16( p_mapX ), s16( p_mapY ) );
        }
    }

    void mapDrawer::executeMoveTriggerScript( u16 p_move ) {
        u16  curx = SAVE::SAV.getActiveFile( ).m_player.m_pos.m_posX;
        u16  cury = SAVE::SAV.getActiveFile( ).m_player.m_pos.m_posY;
        u16  curz = SAVE::SAV.getActiveFile( ).m_player.m_pos.m_posZ;
        auto data = currentData( curx, cury );

        auto mx = curx / SIZE;
        auto my = cury / SIZE;

        curx %= SIZE;
        cury %= SIZE;

        for( u8 i = 0; i < MAX_EVENTS_PER_SLICE; ++i ) {
            if( data.m_events[ i ].m_type == EVENT_GENERIC
                && data.m_events[ i ].m_trigger == TRIGGER_ON_MOVE_AT_POS
                && data.m_events[ i ].m_posX == curx && data.m_events[ i ].m_posY == cury
                && data.m_events[ i ].m_posZ == curz
                && data.m_events[ i ].m_data.m_generic.m_triggerMove == p_move ) {
                if( data.m_events[ i ].m_activateFlag
                    && !SAVE::SAV.getActiveFile( ).checkFlag(
                        data.m_events[ i ].m_activateFlag ) ) {
                    continue;
                }
                if( data.m_events[ i ].m_deactivateFlag
                    && SAVE::SAV.getActiveFile( ).checkFlag(
                        data.m_events[ i ].m_deactivateFlag ) ) {
                    continue;
                }
                if( ( data.m_events[ i ].m_route )
                    && data.m_events[ i ].m_route != SAVE::SAV.getActiveFile( ).m_route ) {
                    continue;
                }

                executeScript( data.m_events[ i ].m_data.m_generic.m_scriptId, 0, mx, my );
                return;
            }
        }
    }

    std::vector<u16> mapDrawer::getTriggerMovesForCurPos( ) const {
        std::vector<u16> res{ };

        u16  curx = SAVE::SAV.getActiveFile( ).m_player.m_pos.m_posX;
        u16  cury = SAVE::SAV.getActiveFile( ).m_player.m_pos.m_posY;
        u16  curz = SAVE::SAV.getActiveFile( ).m_player.m_pos.m_posZ;
        auto data = currentData( curx, cury );

        curx %= SIZE;
        cury %= SIZE;

        for( u8 i = 0; i < MAX_EVENTS_PER_SLICE; ++i ) {
            if( data.m_events[ i ].m_type == EVENT_GENERIC
                && data.m_events[ i ].m_trigger == TRIGGER_ON_MOVE_AT_POS
                && data.m_events[ i ].m_posX == curx && data.m_events[ i ].m_posY == cury
                && data.m_events[ i ].m_posZ == curz ) {
                if( data.m_events[ i ].m_activateFlag
                    && !SAVE::SAV.getActiveFile( ).checkFlag(
                        data.m_events[ i ].m_activateFlag ) ) {
                    continue;
                }
                if( data.m_events[ i ].m_deactivateFlag
                    && SAVE::SAV.getActiveFile( ).checkFlag(
                        data.m_events[ i ].m_deactivateFlag ) ) {
                    continue;
                }
                if( ( data.m_events[ i ].m_route )
                    && data.m_events[ i ].m_route != SAVE::SAV.getActiveFile( ).m_route ) {
                    continue;
                }

                res.push_back( data.m_events[ i ].m_data.m_generic.m_triggerMove );
            }
        }
        return res;
    }

    std::vector<u16> mapDrawer::getTriggerBattleMovesForCurPos( ) const {
        std::vector<u16> res{ };

        u16  curx = SAVE::SAV.getActiveFile( ).m_player.m_pos.m_posX;
        u16  cury = SAVE::SAV.getActiveFile( ).m_player.m_pos.m_posY;
        auto data = currentData( curx, cury );

        curx %= SIZE;
        cury %= SIZE;

        for( u8 i = 0; i < MAX_EVENTS_PER_SLICE; ++i ) {
            if( data.m_events[ i ].m_type == EVENT_GENERIC
                && data.m_events[ i ].m_trigger == TRIGGER_ON_MOVE_IN_BATTLE ) {
                if( data.m_events[ i ].m_activateFlag
                    && !SAVE::SAV.getActiveFile( ).checkFlag(
                        data.m_events[ i ].m_activateFlag ) ) {
                    continue;
                }
                if( data.m_events[ i ].m_deactivateFlag
                    && SAVE::SAV.getActiveFile( ).checkFlag(
                        data.m_events[ i ].m_deactivateFlag ) ) {
                    continue;
                }
                if( ( data.m_events[ i ].m_route )
                    && data.m_events[ i ].m_route != SAVE::SAV.getActiveFile( ).m_route ) {
                    continue;
                }

                res.push_back( data.m_events[ i ].m_data.m_generic.m_triggerMove );
            }
        }
        return res;
    }

    std::pair<bool, mapData::event::data> mapDrawer::getWarpData( u16 p_globX, u16 p_globY,
                                                                  u8 p_z ) {
        auto data = currentData( p_globX, p_globY );
        p_globX %= SIZE;
        p_globY %= SIZE;

        for( u8 i = 0; i < MAX_EVENTS_PER_SLICE; ++i ) {
            if( data.m_events[ i ].m_type == EVENT_WARP && data.m_events[ i ].m_posX == p_globX
                && data.m_events[ i ].m_posY == p_globY && data.m_events[ i ].m_posZ == p_z ) {
                if( data.m_events[ i ].m_activateFlag
                    && !SAVE::SAV.getActiveFile( ).checkFlag(
                        data.m_events[ i ].m_activateFlag ) ) {
                    continue;
                }
                if( data.m_events[ i ].m_deactivateFlag
                    && SAVE::SAV.getActiveFile( ).checkFlag(
                        data.m_events[ i ].m_deactivateFlag ) ) {
                    continue;
                }
                if( ( data.m_events[ i ].m_route )
                    && data.m_events[ i ].m_route != SAVE::SAV.getActiveFile( ).m_route ) {
                    continue;
                }

                return { true, data.m_events[ i ].m_data };
            }
        }
        return { false, mapData::event::data( ) };
    }

    void mapDrawer::handleWarp( warpType p_type, warpPos p_source ) {
        warpPos tg;
        u16     curx = SAVE::SAV.getActiveFile( ).m_player.m_pos.m_posX;
        u16     cury = SAVE::SAV.getActiveFile( ).m_player.m_pos.m_posY;
        u16     curz = SAVE::SAV.getActiveFile( ).m_player.m_pos.m_posZ;

        auto wdata = getWarpData( curx, cury, curz );
        curx %= SIZE;
        cury %= SIZE;
        if( !wdata.first ) { return; }

        if( wdata.second.m_warp.m_warpType != NO_SPECIAL ) {
            p_type = (warpType) wdata.second.m_warp.m_warpType;
            if( wdata.second.m_warp.m_warpType == SCRIPT ) {
                auto scriptId = wdata.second.m_warp.m_posZ;
                if( !executeWarpScript( scriptId, p_type, tg ) ) {
                    // warp script failed / didn't return
                    return;
                }
                if( p_type == LAST_VISITED ) { tg = SAVE::SAV.getActiveFile( ).m_lastWarp; }
            } else {
                if( wdata.second.m_warp.m_warpType == LAST_VISITED ) {
                    tg = SAVE::SAV.getActiveFile( ).m_lastWarp;
                } else {
                    tg = warpPos(
                        wdata.second.m_warp.m_bank,
                        position( wdata.second.m_warp.m_mapX * SIZE + wdata.second.m_warp.m_posX,
                                  wdata.second.m_warp.m_mapY * SIZE + wdata.second.m_warp.m_posY,
                                  +wdata.second.m_warp.m_posZ ) );
                }
            }
        }

        if( tg.first == WARP_TO_LAST_ENTRY ) { tg = SAVE::SAV.getActiveFile( ).m_lastWarp; }
        if( !tg.first && !tg.second.m_posY && !tg.second.m_posZ && !tg.second.m_posX ) return;

        SAVE::SAV.getActiveFile( ).m_lastWarp = p_source;
        warpPlayer( p_type, tg );
    }

    void mapDrawer::handleWarp( warpType p_type ) {
        warpPos current = warpPos{ SAVE::SAV.getActiveFile( ).m_currentMap,
                                   SAVE::SAV.getActiveFile( ).m_player.m_pos };
        if( p_type == LAST_VISITED ) {
            warpPos target = SAVE::SAV.getActiveFile( ).m_lastWarp;
            if( !target.first && !target.second.m_posX && !target.second.m_posY
                && !target.second.m_posZ )
                return;
            SAVE::SAV.getActiveFile( ).m_lastWarp = current;

            warpPlayer( p_type, target );
        } else {
            handleWarp( p_type, current );
        }
    }

    bool mapDrawer::currentPosAllowsDirectFieldMove( ) const {
        u16 gx = SAVE::SAV.getActiveFile( ).m_player.m_pos.m_posX;
        u16 gy = SAVE::SAV.getActiveFile( ).m_player.m_pos.m_posY;

        auto beh = at( gx, gy ).m_bottombehave;

        if( beh == BEH_NO_DIRECT_FIELD_MOVE ) { return false; }

        return true;
    }
} // namespace MAP
