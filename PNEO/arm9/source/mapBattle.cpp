/*
Pokémon neo
------------------------------

file        : mapBattle.cpp
author      : Philip Wellnitz
description : Map drawing engine: functions related to wild and trainer battles

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

#include <algorithm>

#include "bagViewer.h"
#include "battle.h"
#include "battleDefines.h"
#include "battleTrainer.h"
#include "defines.h"
#include "fs.h"
#include "gameStart.h"
#include "mapDrawer.h"
#include "nav.h"
#include "pokemonNames.h"
#include "saveGame.h"
#include "screenFade.h"
#include "sound.h"
#include "sprite.h"
#include "uio.h"

#define DESQUID_LOG( _ ) \
    {}

namespace MAP {
    void mapDrawer::disablePkmn( s16 p_steps ) {
        SAVE::SAV.getActiveFile( ).m_repelSteps = p_steps;
    }

    void mapDrawer::enablePkmn( ) {
        SAVE::SAV.getActiveFile( ).m_repelSteps = 0;
    }

    void mapDrawer::handleWildPkmn( u16 p_globX, u16 p_globY ) {
        u8 moveData = atom( p_globX, p_globY ).m_movedata;
        u8 behave   = at( p_globX, p_globY ).m_bottombehave;

        if( SAVE::SAV.getActiveFile( ).m_repelSteps ) return;
        // handle Pkmn stuff
        if( moveData == MVD_SURF && behave != BEH_WATERFALL )
            handleWildPkmn( WATER );
        else if( behave == BEH_GRASS || behave == BEH_GRASS_ASH || behave == BEH_GRASS_UNDERWATER
                 || behave == BEH_SAND_WITH_ENCOUNTER_AND_FISH
                 || behave == BEH_CAVE_WITH_ENCOUNTER )
            handleWildPkmn( GRASS );
        else if( behave == BEH_LONG_GRASS )
            handleWildPkmn( HIGH_GRASS );
        //        else if( currentData( ).m_mapType & CAVE )
        //            handleWildPkmn( GRASS );
    }

    u8 mapDrawer::getWildPkmnLevel( u16 p_rnd ) {
        u8 tier;
        if( p_rnd < 2 )
            tier = 4;
        else if( p_rnd < 6 )
            tier = 3;
        else if( p_rnd < 14 )
            tier = 2;
        else if( p_rnd < 26 )
            tier = 1;
        else
            tier = 0;
        return SAVE::SAV.getActiveFile( ).getEncounterLevel( tier );
    }

    bool mapDrawer::getWildPkmnSpecies( wildPkmnType p_type, u16& p_pkmnId, u8& p_pkmnForme ) {
        p_pkmnId    = 0;
        p_pkmnForme = 0;

        s8 availmod = ( SAVE::SAV.getActiveFile( ).m_options.getDifficulty( ) - 3 ) / 3;

        u8 total = 0;
        for( u8 i = 0; i < currentData( ).m_pokemonDescrCount; ++i ) {
            if( currentData( ).m_pokemon[ i ].m_encounterType == p_type ) {
                s8 ownedbadge = SAVE::SAV.getActiveFile( ).getBadgeCount( ) + availmod;
                if( ownedbadge < 0 ) { ownedbadge = 0; }

                if( ownedbadge >= currentData( ).m_pokemon[ i ].m_slot ) {

                    if( currentData( ).m_pokemon[ i ].m_daytime
                        & ( 1 << ( getCurrentDaytime( ) % 4 ) ) ) {
                        total += currentData( ).m_pokemon[ i ].m_encounterRate;
                    }
                }
            }
        }
        if( !total ) {
            // there are no wild pkmn for the current map
            return false;
        }

        u16 backup      = 0;
        u8  backupForme = 0;

        u8 res = rand( ) % total;
        total  = 0;
        for( u8 i = 0; i < currentData( ).m_pokemonDescrCount; ++i ) {
            if( currentData( ).m_pokemon[ i ].m_encounterType == p_type ) {
                s8 ownedbadge = SAVE::SAV.getActiveFile( ).getBadgeCount( ) + availmod;
                if( ownedbadge < 0 ) { ownedbadge = 0; }

                if( ownedbadge >= currentData( ).m_pokemon[ i ].m_slot
                    && ( currentData( ).m_pokemon[ i ].m_daytime
                         & ( 1 << ( getCurrentDaytime( ) % 4 ) ) ) ) {
                    total += currentData( ).m_pokemon[ i ].m_encounterRate;

                    // if the player hasn't obtained the nat dex yet, they should only see
                    // pkmn that are in the local dex
                    if( SAVE::SAV.getActiveFile( ).getPkmnDisplayDexId(
                            currentData( ).m_pokemon[ i ].m_speciesId )
                        != u16( -1 ) ) {
                        backup      = currentData( ).m_pokemon[ i ].m_speciesId;
                        backupForme = currentData( ).m_pokemon[ i ].m_forme;
                    } else {
                        continue;
                    }

                    if( total > res ) {
                        p_pkmnId    = currentData( ).m_pokemon[ i ].m_speciesId;
                        p_pkmnForme = currentData( ).m_pokemon[ i ].m_forme;
                        break;
                    }
                }
            }
        }

        if( !p_pkmnId ) {
            p_pkmnId    = backup;
            p_pkmnForme = backupForme;
        }
        if( !p_pkmnId ) { return false; }

        if( p_pkmnId == PKMN_PIKACHU && !( rand( ) & PIKACHU_IS_MIMIKYU_MOD ) ) {
            p_pkmnId = PKMN_MIMIKYU;
        }

        return true;
    }

    pokemon WILD_PKMN;
    void    mapDrawer::prepareBattleWildPkmn( wildPkmnType p_type, u16 p_pkmnId, bool p_luckyEnc ) {
        (void) p_type;

        ANIMATE_MAP = false;
        DRAW_TIME   = false;
        if( p_luckyEnc ) {
            SOUND::playBGM( MOD_BATTLE_WILD_ALT );
        } else {
            SOUND::playBGM( SOUND::BGMforWildBattle( p_pkmnId ) );
        }
        _playerIsFast = false;
        _fastBike     = false;
        _mapSprites.setFrameD( _playerSprite, SAVE::SAV.getActiveFile( ).m_player.m_direction );

        IO::fadeScreen( IO::BATTLE );
        IO::BG_PAL( true )[ 0 ] = 0;
        IO::fadeScreen( IO::CLEAR_DARK_IMMEDIATE, true, true );
        dmaFillWords( 0, bgGetGfxPtr( IO::bg2sub ), 256 * 192 );
        dmaFillWords( 0, bgGetGfxPtr( IO::bg3sub ), 256 * 192 );
    }

    bool mapDrawer::battleWildPkmn( wildPkmnType p_type ) {
        u8 platform = 0, plat2 = 0;
        u8 battleBack = p_type == WATER ? currentData( ).m_surfBattleBG : currentData( ).m_battleBG;
        switch( p_type ) {
        case WATER:
            platform = currentData( ).m_surfBattlePlat1;
            plat2    = currentData( ).m_surfBattlePlat2;
            break;
        case OLD_ROD:
        case GOOD_ROD:
        case SUPER_ROD:
            platform = currentData( ).m_battlePlat1;
            plat2    = currentData( ).m_surfBattlePlat2;
            break;

        default:
            platform = currentData( ).m_battlePlat1;
            plat2    = currentData( ).m_battlePlat2;
            break;
        }

        auto playerPrio = _mapSprites.getPriority( _playerSprite );
        swiWaitForVBlank( );
        auto result = BATTLE::battle( SAVE::SAV.getActiveFile( ).m_pkmnTeam,
                                      SAVE::SAV.getActiveFile( ).getTeamPkmnCount( ), WILD_PKMN,
                                      platform, plat2, battleBack, getBattlePolicy( true ) )
                          .start( );

        // check if a tracer chain needs to be continued
        if( _tracerChain ) {
            if( ( result != BATTLE::battle::BATTLE_PLAYER_WON
                  && result != BATTLE::battle::BATTLE_CAPTURE )
                || !continueTracerChain( ) ) {
                // chain breaks if player does not defeat or capture
                // or if no new tracer spots can be generated.
                resetTracerChain( true );
            }
        }

        if( result == BATTLE::battle::BATTLE_OPPONENT_WON ) {
            faintPlayer( );
            return true;
        }
        SOUND::restartBGM( );
        FADE_TOP_DARK( );
        draw( playerPrio );
        _mapSprites.setPriority( _playerSprite,
                                 SAVE::SAV.getActiveFile( ).m_playerPriority = playerPrio );
        NAV::init( );

        ANIMATE_MAP = true;
        DRAW_TIME   = true;

        return true;
    }

    bool mapDrawer::handleTracerPkmn( u8 p_tracerSlot ) {
        if( !_tracerSpecies ) { return false; }
        u16 rn
            = rand( ) % ( 512 + 3 * SAVE::SAV.getActiveFile( ).m_options.m_encounterRateModifier );
        u8 level = getWildPkmnLevel( rn );

        bool luckyenc = tracerSlotLucky( p_tracerSlot );
        u8   shiny    = tracerSlotShiny( p_tracerSlot );

        prepareBattleWildPkmn( POKE_TORE, _tracerSpecies, luckyenc );
        WILD_PKMN = pokemon( _tracerSpecies, level, _tracerForme, 0, 2 * shiny, luckyenc, false, 0,
                             0, luckyenc );
        return battleWildPkmn( POKE_TORE );
    }

    bool mapDrawer::handleWildPkmn( wildPkmnType p_type, bool p_forceEncounter ) {
        u16 rn
            = rand( ) % ( 512 + 3 * SAVE::SAV.getActiveFile( ).m_options.m_encounterRateModifier );
        if( p_type == OLD_ROD || p_type == GOOD_ROD || p_type == SUPER_ROD ) rn /= 8;
        if( p_forceEncounter ) rn %= 40;

        u8 level = getWildPkmnLevel( rn );

        if( rn > 40 || !level ) {
            if( p_type == OLD_ROD || p_type == GOOD_ROD || p_type == SUPER_ROD ) {
                _playerIsFast = false;
                NAV::printMessage( GET_STRING( 5 ) );
            }
            return false;
        }
        if( p_type == OLD_ROD || p_type == GOOD_ROD || p_type == SUPER_ROD ) {
            _playerIsFast = false;
            NAV::printMessage( GET_STRING( 6 ) );
        } else if( SAVE::SAV.getActiveFile( ).m_repelSteps && !p_forceEncounter ) {
            return false;
        }

        u16 pkmnId    = 0;
        u8  pkmnForme = 0;

        if( !getWildPkmnSpecies( p_type, pkmnId, pkmnForme ) ) { return false; }

        bool luckyenc = SAVE::SAV.getActiveFile( ).m_bag.count(
                            BAG::toBagType( ITEM::ITEMTYPE_KEYITEM ), I_WISHING_CHARM )
                            ? !( rand( ) & 127 )
                            : !( rand( ) & 2047 );
        bool charm    = SAVE::SAV.getActiveFile( ).m_bag.count(
               BAG::toBagType( ITEM::ITEMTYPE_KEYITEM ), I_SHINY_CHARM );

        resetTracerChain( );
        prepareBattleWildPkmn( p_type, pkmnId, luckyenc );
        WILD_PKMN = pokemon( pkmnId, level, pkmnForme, 0, luckyenc ? 255 : ( charm ? 3 : 0 ),
                             luckyenc, false, 0, 0, luckyenc );
        return battleWildPkmn( p_type );
    }

    BATTLE::battlePolicy mapDrawer::getBattlePolicy( bool p_isWildBattle, BATTLE::battleMode p_mode,
                                                     bool p_distributeEXP ) {
        BATTLE::battlePolicy res = p_isWildBattle
                                       ? BATTLE::battlePolicy( BATTLE::DEFAULT_WILD_POLICY )
                                       : BATTLE::battlePolicy( BATTLE::DEFAULT_TRAINER_POLICY );

        res.m_mode               = p_mode;
        res.m_distributeEXP      = p_distributeEXP;
        res.m_allowMegaEvolution = SAVE::SAV.getActiveFile( ).checkFlag( SAVE::F_MEGA_EVOLUTION );

        res.m_weather = BATTLE::weather::NO_WEATHER;
        switch( getWeather( ) ) {
        case SUNNY: res.m_weather = BATTLE::weather::SUN; break;
        case RAINY:
        case THUNDERSTORM: res.m_weather = BATTLE::weather::RAIN; break;
        case SNOW:
        case BLIZZARD: res.m_weather = BATTLE::weather::HAIL; break;
        case SANDSTORM: res.m_weather = BATTLE::weather::SANDSTORM; break;
        case FOG: res.m_weather = BATTLE::weather::FOG; break;
        case HEAVY_SUNLIGHT: res.m_weather = BATTLE::weather::HEAVY_SUNSHINE; break;
        case HEAVY_RAIN: res.m_weather = BATTLE::weather::HEAVY_RAIN; break;
        default: break;
        }

        return res;
    }

    bool mapDrawer::requestWildPkmn( bool p_forceHighGrass ) {
        u8 moveData = atom( SAVE::SAV.getActiveFile( ).m_player.m_pos.m_posX,
                            SAVE::SAV.getActiveFile( ).m_player.m_pos.m_posY )
                          .m_movedata;
        u8 behave = at( SAVE::SAV.getActiveFile( ).m_player.m_pos.m_posX,
                        SAVE::SAV.getActiveFile( ).m_player.m_pos.m_posY )
                        .m_bottombehave;

        u8 tracerSlot = getTracerPkmn( SAVE::SAV.getActiveFile( ).m_player.m_pos.m_posX,
                                       SAVE::SAV.getActiveFile( ).m_player.m_pos.m_posY );

        if( tracerSlot != NO_TRACER_PKMN ) {
            return handleTracerPkmn( tracerSlot );
        } else if( moveData == MVD_SURF && behave != BEH_WATERFALL ) {
            return handleWildPkmn( WATER, true );
        } else if( currentData( ).m_mapType & CAVE ) {
            return handleWildPkmn( GRASS, true );
        } else if( behave == BEH_LONG_GRASS || p_forceHighGrass ) {
            return handleWildPkmn( HIGH_GRASS, true );
        } else if( behave == BEH_GRASS_ASH || behave == BEH_SAND_WITH_ENCOUNTER_AND_FISH
                   || behave == BEH_GRASS ) {
            return handleWildPkmn( GRASS, true );
        }
        return false;
    }

    void mapDrawer::resetTracerChain( bool p_updateMusic ) {
        // reset any remains of any previous chain
        std::memset( _tracerPositions, 0, sizeof( _tracerPositions ) );
        _tracerLuckyShiny = 0;
        _tracerLastPos    = { 0, 0, 0 };
        _tracerChain      = 0;

        _tracerSpecies = 0;
        _tracerForme   = 0;
        SOUND::setTracerStatus( false, !p_updateMusic );
    }

    static constexpr u16 STR_TRACER_FAIL = 696;

    bool mapDrawer::startTracerChain( ) {
        SOUND::setTracerStatus( true );
        bool specialTracerPkmn = rand( ) & 1;

        if( ( !specialTracerPkmn || !getWildPkmnSpecies( POKE_TORE, _tracerSpecies, _tracerForme ) )
            && !getWildPkmnSpecies( GRASS, _tracerSpecies, _tracerForme ) ) {
            // couldn't find suitable pkmn, no chain will start
            NAV::printMessage( GET_STRING( STR_TRACER_FAIL ) );
            return false;
        }
        return true;
    }

    u8 mapDrawer::getTracerPkmn( u16 p_globX, u16 p_globY ) {
        if( !_tracerChain ) { return NO_TRACER_PKMN; }

        // compute tracer slot
        u16 slot = dist( p_globX, p_globY, _tracerLastPos.m_posX, _tracerLastPos.m_posY );
        if( slot > TRACER_AREA ) { return NO_TRACER_PKMN; }

        // check if position actually contains a pkmn
        u16 px = TRACER_AREA + _tracerLastPos.m_posX - p_globX;
        u16 py = TRACER_AREA + _tracerLastPos.m_posY - p_globY;

        if( _tracerPositions[ py ] & ( 1 << px ) ) { return slot; }
        return NO_TRACER_PKMN;
    }

    bool mapDrawer::updateTracerChain( direction p_dir ) {
        if( ++_tracerCharge > TRACER_CHARGED ) { _tracerCharge = TRACER_CHARGED; }
        if( !_tracerChain ) { return false; }
        if( !tracerUsable( SAVE::SAV.getActiveFile( ).m_player.m_pos ) ) {
            // player stepped outside of the grass, breaking the chain
            return false;
        }

        // rotate the tracer positions

        switch( p_dir ) {
        case UP: {
            for( u8 i = 1; i < 2 * TRACER_AREA + 1; ++i ) {
                _tracerPositions[ i ] = _tracerPositions[ i - 1 ];
            }
            _tracerPositions[ 0 ] = 0;
            break;
        }
        case DOWN: {
            for( u8 i = 2 * TRACER_AREA + 1; i; --i ) {
                _tracerPositions[ i - 1 ] = _tracerPositions[ i ];
            }
            _tracerPositions[ 2 * TRACER_AREA ] = 0;
            break;
        }
        case LEFT: {
            for( u8 i = 0; i < 2 * TRACER_AREA + 1; ++i ) { _tracerPositions[ i ] >>= 1; }
            break;
        }
        case RIGHT: {
            for( u8 i = 0; i < 2 * TRACER_AREA + 1; ++i ) { _tracerPositions[ i ] <<= 1; }
            break;
        }
        default: return false;
        }

        // check if any position remains
        u16 active = 0;
        for( u8 i = 0; i < 2 * TRACER_AREA + 1; ++i ) { active |= _tracerPositions[ i ]; }
        return !!active;
    }

    bool mapDrawer::continueTracerChain( ) {
        DESQUID_LOG( "continueTracerChain" );

        // store current position
        std::memset( _tracerPositions, 0, sizeof( _tracerPositions ) );
        _tracerLastPos    = SAVE::SAV.getActiveFile( ).m_player.m_pos;
        _tracerLuckyShiny = 0;
        _tracerLastPos    = { 0, 0, 0 };

        ++_tracerChain;
        if( _tracerChain > 40 ) { _tracerChain = 39; }

        u8 luckyMod    = SAVE::SAV.getActiveFile( ).m_bag.count(
                             BAG::toBagType( ITEM::ITEMTYPE_KEYITEM ), I_WISHING_CHARM )
                             ? 5
                             : 10;
        u8 shinyFactor = 75;
        if( SAVE::SAV.getActiveFile( ).m_bag.count( BAG::toBagType( ITEM::ITEMTYPE_KEYITEM ),
                                                    I_SHINY_CHARM ) ) {
            shinyFactor = 125;
        }

        bool spotFound = false;

        for( u8 d = 1; d <= TRACER_AREA; ++d ) {
            s8 rx = d * ( ( rand( ) % 3 ) - 1 );
            s8 ry = d * ( ( rand( ) % 3 ) - 1 );

            if( !rx && !ry ) { continue; }

            u16 nx = _tracerLastPos.m_posX + rx;
            u16 ny = _tracerLastPos.m_posX + ry;

            // check if a pkmn can spawn at position ( nx, ny )
            if( !tracerUsable( { nx, ny, 0 } ) ) { continue; }

            spotFound = true;

            u16 shiny = rand( );

            if( !( _tracerChain % luckyMod ) ) {
                setTracerSlotLucky( d );
            } else if( shiny < _tracerChain * shinyFactor ) {
                setTracerSlotShiny( d );
            }

            _tracerPositions[ ny + TRACER_AREA ] |= ( 1 << ( nx + TRACER_AREA ) );
        }

        if( !spotFound ) {
            NAV::printMessage( GET_STRING( STR_TRACER_FAIL ) );
            return false;
        } else {
            return true;
        }
    }

    void mapDrawer::useTracer( ) {
        DESQUID_LOG( "useTracer" );

        resetTracerChain( );
        _tracerCharge = 0;
        if( startTracerChain( ) && continueTracerChain( ) ) {
            animateTracer( );
        } else {
            resetTracerChain( true );
        }
    }

} // namespace MAP
