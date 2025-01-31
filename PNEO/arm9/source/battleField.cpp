/*
Pokémon neo
------------------------------

file        : battleField.cpp
author      : Philip Wellnitz
description :

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
#include <vector>

#include "battle/ability.h"
#include "battle/battle.h"
#include "battle/battleField.h"
#include "battle/battleSide.h"
#include "battle/battleSlot.h"
#include "battle/battleUI.h"
#include "battle/move.h"
#include "battle/type.h"
#include "defines.h"
#include "gen/abilityNames.h"
#include "gen/itemNames.h"
#include "gen/pokemonNames.h"
#include "save/saveGame.h"

/*

   TODO

- entry hazards
- (most) slot conditions
- binding moves
-

items:
adrenaline orb: increase speed when intimidate
binding band, grip claw
blunder policy: inc speed by 2 when move misses
destiny knot: if user infatuated, also attacker
eject pack: swap out once stat is lowered
heavy-duty boots: unaffected by entry hazards
red card
throat spray: inc sp atk by 2 after sound-based move
utility umbrella

seeds / when enter terrain
   */

namespace BATTLE {
    field::field( battleMode p_battleMode, bool p_isWildBattle, weather p_initialWeather,
                  pseudoWeather p_initialPseudoWeather, terrain p_initialTerrain ) {
        _mode         = p_battleMode;
        _weather      = p_initialWeather;
        _weatherTimer = u8( -1 ); // Initial weather stays forever
        _isWildBattle = p_isWildBattle;

        std::memset( _pseudoWeatherTimer, 0, sizeof( _pseudoWeatherTimer ) );
        for( u8 i = 0; i < MAX_PSEUDO_WEATHER; ++i ) {
            if( p_initialPseudoWeather & ( 1LLU << i ) ) [[unlikely]] {
                _pseudoWeatherTimer[ i ] = u8( -1 ); // Initial pseudo weather stays forever
            }
        }

        _terrain      = p_initialTerrain;
        _terrainTimer = u8( -1 ); // Initial terrain stays forever
    }

    void field::checkOnEatBerry( battleUI* p_ui, bool p_opponent, u8 p_slot, u16 p_berry ) {
        (void) p_berry;

        auto pkmn = getPkmn( p_opponent, p_slot );
        if( pkmn == nullptr ) { return; }
        if( !suppressesAbilities( ) ) {
            switch( pkmn->getAbility( ) ) {
            [[unlikely]] case A_CHEEK_POUCH:
                p_ui->logAbility( getPkmnOrDisguise( p_opponent, p_slot ), p_opponent );
                healPokemon( p_ui, p_opponent, p_slot, pkmn->m_stats.m_maxHP / 3 );
                break;
            [[likely]] default:
                break;
            }
        }
    }

    void field::checkItemAfterAttack( battleUI* p_ui, bool p_opponent, u8 p_pos ) {
        auto pkmn = getPkmn( p_opponent, p_pos );
        if( pkmn == nullptr ) { return; }
        constexpr u8 TMP_BUFFER_SIZE = 100;
        char         buffer[ TMP_BUFFER_SIZE + 10 ];

        bool items = canUseItem( p_opponent, p_pos );
        if( !items || !pkmn->getItem( ) ) { return; }

        bool supprAbs    = suppressesAbilities( );
        auto userVolStat = getVolatileStatus( p_opponent, p_pos );

        bool lowhptrigger = pkmn->m_stats.m_curHP * 4 <= pkmn->m_stats.m_maxHP;

        switch( pkmn->getItem( ) ) {
        case I_BERRY_JUICE:
            if( lowhptrigger ) {
                p_ui->logItem( getPkmnOrDisguise( p_opponent, p_pos ), p_opponent );
                auto fmt = std::string( GET_STRING( IO::STR_UI_BATTLE_DRINK_ITEM ) );
                snprintf( buffer, TMP_BUFFER_SIZE, fmt.c_str( ),
                          p_ui->getPkmnName( getPkmnOrDisguise( p_opponent, p_pos ), p_opponent )
                              .c_str( ),
                          FS::getItemName( pkmn->getItem( ) ).c_str( ) );
                p_ui->log( buffer );
                healPokemon( p_ui, p_opponent, p_pos, pkmn->m_stats.m_maxHP / 4 );
                removeItem( p_ui, p_opponent, p_pos );
                checkOnEatBerry( p_ui, p_opponent, p_pos, pkmn->getItem( ) );
            }
            break;
        case I_WHITE_HERB: {
            boosts bs = getBoosts( p_opponent, p_pos );
            if( bs.negative( ) != boosts( ) ) {
                bs       = bs.negative( ).invert( );
                auto res = addBoosts( p_opponent, p_pos, bs );
                if( res != boosts( ) ) {
                    p_ui->logItem( getPkmnOrDisguise( p_opponent, p_pos ), p_opponent );
                    auto fmt = std::string( GET_STRING( IO::STR_UI_BATTLE_ACTIVATE_ITEM ) );
                    snprintf(
                        buffer, TMP_BUFFER_SIZE, fmt.c_str( ),
                        p_ui->getPkmnName( getPkmnOrDisguise( p_opponent, p_pos ), p_opponent )
                            .c_str( ),
                        FS::getItemName( pkmn->getItem( ) ).c_str( ) );
                    p_ui->log( buffer );
                    p_ui->logBoosts( getPkmnOrDisguise( p_opponent, p_pos ), p_opponent, p_pos, bs,
                                     res );
                    removeItem( p_ui, p_opponent, p_pos );
                }
            }
            break;
        }
        case I_MENTAL_HERB:
            if( userVolStat & VS_ATTRACT ) {
                p_ui->logItem( getPkmnOrDisguise( p_opponent, p_pos ), p_opponent );
                auto fmt = std::string( GET_STRING( IO::STR_UI_BATTLE_ACTIVATE_ITEM ) );
                snprintf( buffer, TMP_BUFFER_SIZE, fmt.c_str( ),
                          p_ui->getPkmnName( getPkmnOrDisguise( p_opponent, p_pos ), p_opponent )
                              .c_str( ),
                          FS::getItemName( pkmn->getItem( ) ).c_str( ) );
                p_ui->log( buffer );

                removeVolatileStatus( p_ui, p_opponent, p_pos, VS_ATTRACT );
                removeItem( p_ui, p_opponent, p_pos );
            }
            break;

        default: break;
        }

        if( supprAbs
            || !_sides[ p_opponent ? PLAYER_SIDE : OPPONENT_SIDE ].anyHasAbility( A_UNNERVE ) ) {

            if( !supprAbs && pkmn->getAbility( ) == A_GLUTTONY ) {
                lowhptrigger = pkmn->m_stats.m_curHP * 2 <= pkmn->m_stats.m_maxHP;
            }

            bool ripen = !supprAbs && pkmn->getAbility( ) == A_RIPEN;
            auto fmt   = std::string( GET_STRING( IO::STR_UI_BATTLE_EAT_ITEM ) );

            switch( pkmn->getItem( ) ) {
            case I_NION_BERRY: {
                boosts bs = getBoosts( p_opponent, p_pos );
                if( bs.negative( ) != boosts( ) ) {
                    boosts tmp = bs.negative( ).invert( );
                    bs         = tmp;
                    if( ripen ) { bs.addBoosts( tmp ); };
                    auto res = addBoosts( p_opponent, p_pos, bs );
                    if( res != boosts( ) ) {
                        p_ui->logItem( getPkmnOrDisguise( p_opponent, p_pos ), p_opponent );
                        snprintf(
                            buffer, TMP_BUFFER_SIZE, fmt.c_str( ),
                            p_ui->getPkmnName( getPkmnOrDisguise( p_opponent, p_pos ), p_opponent )
                                .c_str( ),
                            FS::getItemName( pkmn->getItem( ) ).c_str( ) );
                        p_ui->log( buffer );
                        p_ui->logBoosts( getPkmnOrDisguise( p_opponent, p_pos ), p_opponent, p_pos,
                                         bs, res );
                        removeItem( p_ui, p_opponent, p_pos );
                        checkOnEatBerry( p_ui, p_opponent, p_pos, pkmn->getItem( ) );
                    }
                }
                break;
            }
            case I_CHERI_BERRY:
                if( hasStatusCondition( p_opponent, p_pos, PARALYSIS ) ) {
                    p_ui->logItem( getPkmnOrDisguise( p_opponent, p_pos ), p_opponent );
                    snprintf(
                        buffer, TMP_BUFFER_SIZE, fmt.c_str( ),
                        p_ui->getPkmnName( getPkmnOrDisguise( p_opponent, p_pos ), p_opponent )
                            .c_str( ),
                        FS::getItemName( pkmn->getItem( ) ).c_str( ) );
                    p_ui->log( buffer );
                    removeStatusCondition( p_opponent, p_pos );
                    p_ui->updatePkmnStats( p_opponent, p_pos,
                                           getPkmnOrDisguise( p_opponent, p_pos ) );
                    removeItem( p_ui, p_opponent, p_pos );
                    checkOnEatBerry( p_ui, p_opponent, p_pos, pkmn->getItem( ) );
                }
                break;
            case I_CHESTO_BERRY:
                if( hasStatusCondition( p_opponent, p_pos, SLEEP ) ) {
                    p_ui->logItem( getPkmnOrDisguise( p_opponent, p_pos ), p_opponent );
                    snprintf(
                        buffer, TMP_BUFFER_SIZE, fmt.c_str( ),
                        p_ui->getPkmnName( getPkmnOrDisguise( p_opponent, p_pos ), p_opponent )
                            .c_str( ),
                        FS::getItemName( pkmn->getItem( ) ).c_str( ) );
                    p_ui->log( buffer );
                    removeStatusCondition( p_opponent, p_pos );
                    p_ui->updatePkmnStats( p_opponent, p_pos,
                                           getPkmnOrDisguise( p_opponent, p_pos ) );
                    removeItem( p_ui, p_opponent, p_pos );
                    checkOnEatBerry( p_ui, p_opponent, p_pos, pkmn->getItem( ) );
                }
                break;
            case I_DRASH_BERRY:
            case I_PECHA_BERRY:
                if( hasStatusCondition( p_opponent, p_pos, POISON ) ) {
                    p_ui->logItem( getPkmnOrDisguise( p_opponent, p_pos ), p_opponent );
                    snprintf(
                        buffer, TMP_BUFFER_SIZE, fmt.c_str( ),
                        p_ui->getPkmnName( getPkmnOrDisguise( p_opponent, p_pos ), p_opponent )
                            .c_str( ),
                        FS::getItemName( pkmn->getItem( ) ).c_str( ) );
                    p_ui->log( buffer );
                    removeStatusCondition( p_opponent, p_pos );
                    p_ui->updatePkmnStats( p_opponent, p_pos,
                                           getPkmnOrDisguise( p_opponent, p_pos ) );
                    removeItem( p_ui, p_opponent, p_pos );
                    checkOnEatBerry( p_ui, p_opponent, p_pos, pkmn->getItem( ) );
                }
                break;
            case I_CHRO_BERRY:
            case I_RAWST_BERRY:
                if( hasStatusCondition( p_opponent, p_pos, BURN ) ) {
                    p_ui->logItem( getPkmnOrDisguise( p_opponent, p_pos ), p_opponent );
                    snprintf(
                        buffer, TMP_BUFFER_SIZE, fmt.c_str( ),
                        p_ui->getPkmnName( getPkmnOrDisguise( p_opponent, p_pos ), p_opponent )
                            .c_str( ),
                        FS::getItemName( pkmn->getItem( ) ).c_str( ) );
                    p_ui->log( buffer );
                    removeStatusCondition( p_opponent, p_pos );
                    p_ui->updatePkmnStats( p_opponent, p_pos,
                                           getPkmnOrDisguise( p_opponent, p_pos ) );
                    removeItem( p_ui, p_opponent, p_pos );
                    checkOnEatBerry( p_ui, p_opponent, p_pos, pkmn->getItem( ) );
                }
                break;
            case I_PUMKIN_BERRY:
            case I_ASPEAR_BERRY:
                if( hasStatusCondition( p_opponent, p_pos, FROZEN ) ) {
                    p_ui->logItem( getPkmnOrDisguise( p_opponent, p_pos ), p_opponent );
                    snprintf(
                        buffer, TMP_BUFFER_SIZE, fmt.c_str( ),
                        p_ui->getPkmnName( getPkmnOrDisguise( p_opponent, p_pos ), p_opponent )
                            .c_str( ),
                        FS::getItemName( pkmn->getItem( ) ).c_str( ) );
                    p_ui->log( buffer );
                    removeStatusCondition( p_opponent, p_pos );
                    p_ui->updatePkmnStats( p_opponent, p_pos,
                                           getPkmnOrDisguise( p_opponent, p_pos ) );
                    removeItem( p_ui, p_opponent, p_pos );
                    checkOnEatBerry( p_ui, p_opponent, p_pos, pkmn->getItem( ) );
                }
                break;
            case I_ORAN_BERRY:
                if( lowhptrigger ) {
                    p_ui->logItem( getPkmnOrDisguise( p_opponent, p_pos ), p_opponent );
                    snprintf(
                        buffer, TMP_BUFFER_SIZE, fmt.c_str( ),
                        p_ui->getPkmnName( getPkmnOrDisguise( p_opponent, p_pos ), p_opponent )
                            .c_str( ),
                        FS::getItemName( pkmn->getItem( ) ).c_str( ) );
                    p_ui->log( buffer );
                    healPokemon( p_ui, p_opponent, p_pos, 10 * ( 1 + ripen ) );
                    removeItem( p_ui, p_opponent, p_pos );
                    checkOnEatBerry( p_ui, p_opponent, p_pos, pkmn->getItem( ) );
                }
                break;
            case I_RIE_BERRY:
            case I_PERSIM_BERRY:
                if( userVolStat & VS_CONFUSION ) {
                    p_ui->logItem( getPkmnOrDisguise( p_opponent, p_pos ), p_opponent );
                    fmt = std::string( GET_STRING( IO::STR_UI_BATTLE_EAT_ITEM_HEAL_CONFUSION ) );
                    snprintf(
                        buffer, TMP_BUFFER_SIZE, fmt.c_str( ),
                        p_ui->getPkmnName( getPkmnOrDisguise( p_opponent, p_pos ), p_opponent )
                            .c_str( ),
                        FS::getItemName( pkmn->getItem( ) ).c_str( ) );
                    p_ui->log( buffer );

                    removeVolatileStatus( p_ui, p_opponent, p_pos, VS_CONFUSION );
                    removeItem( p_ui, p_opponent, p_pos );
                    checkOnEatBerry( p_ui, p_opponent, p_pos, pkmn->getItem( ) );
                }
                break;
            case I_GARC_BERRY:
                if( userVolStat & VS_ATTRACT ) {
                    p_ui->logItem( getPkmnOrDisguise( p_opponent, p_pos ), p_opponent );
                    fmt = std::string( GET_STRING( IO::STR_UI_BATTLE_EAT_ITEM ) );
                    snprintf(
                        buffer, TMP_BUFFER_SIZE, fmt.c_str( ),
                        p_ui->getPkmnName( getPkmnOrDisguise( p_opponent, p_pos ), p_opponent )
                            .c_str( ),
                        FS::getItemName( pkmn->getItem( ) ).c_str( ) );
                    p_ui->log( buffer );

                    removeVolatileStatus( p_ui, p_opponent, p_pos, VS_ATTRACT );
                    removeItem( p_ui, p_opponent, p_pos );
                    checkOnEatBerry( p_ui, p_opponent, p_pos, pkmn->getItem( ) );
                }
                break;
            case I_LUM_BERRY:
                if( pkmn->m_statusint || ( userVolStat & VS_CONFUSION ) ) {
                    p_ui->logItem( getPkmnOrDisguise( p_opponent, p_pos ), p_opponent );
                    snprintf(
                        buffer, TMP_BUFFER_SIZE, fmt.c_str( ),
                        p_ui->getPkmnName( getPkmnOrDisguise( p_opponent, p_pos ), p_opponent )
                            .c_str( ),
                        FS::getItemName( pkmn->getItem( ) ).c_str( ) );
                    p_ui->log( buffer );
                    removeStatusCondition( p_opponent, p_pos );
                    removeVolatileStatus( p_ui, p_opponent, p_pos, VS_CONFUSION );
                    p_ui->updatePkmnStats( p_opponent, p_pos,
                                           getPkmnOrDisguise( p_opponent, p_pos ) );
                    removeItem( p_ui, p_opponent, p_pos );
                    checkOnEatBerry( p_ui, p_opponent, p_pos, pkmn->getItem( ) );
                }
                break;
            case I_SITRUS_BERRY:
                if( lowhptrigger ) {
                    p_ui->logItem( getPkmnOrDisguise( p_opponent, p_pos ), p_opponent );
                    snprintf(
                        buffer, TMP_BUFFER_SIZE, fmt.c_str( ),
                        p_ui->getPkmnName( getPkmnOrDisguise( p_opponent, p_pos ), p_opponent )
                            .c_str( ),
                        FS::getItemName( pkmn->getItem( ) ).c_str( ) );
                    p_ui->log( buffer );
                    healPokemon( p_ui, p_opponent, p_pos,
                                 pkmn->m_stats.m_maxHP * ( 1 + ripen ) / 4 );
                    removeItem( p_ui, p_opponent, p_pos );
                    checkOnEatBerry( p_ui, p_opponent, p_pos, pkmn->getItem( ) );
                }
                break;
            case I_FIGY_BERRY:
                if( lowhptrigger ) {
                    p_ui->logItem( getPkmnOrDisguise( p_opponent, p_pos ), p_opponent );
                    snprintf(
                        buffer, TMP_BUFFER_SIZE, fmt.c_str( ),
                        p_ui->getPkmnName( getPkmnOrDisguise( p_opponent, p_pos ), p_opponent )
                            .c_str( ),
                        FS::getItemName( pkmn->getItem( ) ).c_str( ) );
                    p_ui->log( buffer );
                    healPokemon( p_ui, p_opponent, p_pos,
                                 pkmn->m_stats.m_maxHP * ( 1 + ripen ) / 4 );
                    removeItem( p_ui, p_opponent, p_pos );
                    checkOnEatBerry( p_ui, p_opponent, p_pos, pkmn->getItem( ) );
                }
                break;
            case I_WIKI_BERRY:
                if( lowhptrigger ) {
                    p_ui->logItem( getPkmnOrDisguise( p_opponent, p_pos ), p_opponent );
                    snprintf(
                        buffer, TMP_BUFFER_SIZE, fmt.c_str( ),
                        p_ui->getPkmnName( getPkmnOrDisguise( p_opponent, p_pos ), p_opponent )
                            .c_str( ),
                        FS::getItemName( pkmn->getItem( ) ).c_str( ) );
                    p_ui->log( buffer );
                    healPokemon( p_ui, p_opponent, p_pos,
                                 pkmn->m_stats.m_maxHP * ( 1 + ripen ) / 4 );
                    removeItem( p_ui, p_opponent, p_pos );
                    checkOnEatBerry( p_ui, p_opponent, p_pos, pkmn->getItem( ) );
                }
                break;
            case I_MAGO_BERRY:
                if( lowhptrigger ) {
                    p_ui->logItem( getPkmnOrDisguise( p_opponent, p_pos ), p_opponent );
                    snprintf(
                        buffer, TMP_BUFFER_SIZE, fmt.c_str( ),
                        p_ui->getPkmnName( getPkmnOrDisguise( p_opponent, p_pos ), p_opponent )
                            .c_str( ),
                        FS::getItemName( pkmn->getItem( ) ).c_str( ) );
                    p_ui->log( buffer );
                    healPokemon( p_ui, p_opponent, p_pos,
                                 pkmn->m_stats.m_maxHP * ( 1 + ripen ) / 4 );
                    removeItem( p_ui, p_opponent, p_pos );
                    checkOnEatBerry( p_ui, p_opponent, p_pos, pkmn->getItem( ) );
                }
                break;
            case I_AGUAV_BERRY:
                if( lowhptrigger ) {
                    p_ui->logItem( getPkmnOrDisguise( p_opponent, p_pos ), p_opponent );
                    snprintf(
                        buffer, TMP_BUFFER_SIZE, fmt.c_str( ),
                        p_ui->getPkmnName( getPkmnOrDisguise( p_opponent, p_pos ), p_opponent )
                            .c_str( ),
                        FS::getItemName( pkmn->getItem( ) ).c_str( ) );
                    p_ui->log( buffer );
                    healPokemon( p_ui, p_opponent, p_pos,
                                 pkmn->m_stats.m_maxHP * ( 1 + ripen ) / 4 );
                    removeItem( p_ui, p_opponent, p_pos );
                    checkOnEatBerry( p_ui, p_opponent, p_pos, pkmn->getItem( ) );
                }
                break;
            case I_IAPAPA_BERRY:
                if( lowhptrigger ) {
                    p_ui->logItem( getPkmnOrDisguise( p_opponent, p_pos ), p_opponent );
                    snprintf(
                        buffer, TMP_BUFFER_SIZE, fmt.c_str( ),
                        p_ui->getPkmnName( getPkmnOrDisguise( p_opponent, p_pos ), p_opponent )
                            .c_str( ),
                        FS::getItemName( pkmn->getItem( ) ).c_str( ) );
                    p_ui->log( buffer );
                    healPokemon( p_ui, p_opponent, p_pos,
                                 pkmn->m_stats.m_maxHP * ( 1 + ripen ) / 4 );
                    removeItem( p_ui, p_opponent, p_pos );
                    checkOnEatBerry( p_ui, p_opponent, p_pos, pkmn->getItem( ) );
                }
                break;
            case I_LIECHI_BERRY: {
                if( lowhptrigger ) {
                    boosts bs = boosts( );
                    bs.setBoost( ATK, 3 * ( 1 + ripen ) );

                    auto res = addBoosts( p_opponent, p_pos, bs );
                    if( res != boosts( ) ) {
                        p_ui->logItem( getPkmnOrDisguise( p_opponent, p_pos ), p_opponent );
                        snprintf(
                            buffer, TMP_BUFFER_SIZE, fmt.c_str( ),
                            p_ui->getPkmnName( getPkmnOrDisguise( p_opponent, p_pos ), p_opponent )
                                .c_str( ),
                            FS::getItemName( pkmn->getItem( ) ).c_str( ) );
                        p_ui->log( buffer );
                        p_ui->logBoosts( getPkmnOrDisguise( p_opponent, p_pos ), p_opponent, p_pos,
                                         bs, res );
                        removeItem( p_ui, p_opponent, p_pos );
                        checkOnEatBerry( p_ui, p_opponent, p_pos, pkmn->getItem( ) );
                    }
                }
                break;
            }
            case I_GANLON_BERRY: {
                if( lowhptrigger ) {
                    boosts bs = boosts( );
                    bs.setBoost( DEF, 3 * ( 1 + ripen ) );

                    auto res = addBoosts( p_opponent, p_pos, bs );
                    if( res != boosts( ) ) {
                        p_ui->logItem( getPkmnOrDisguise( p_opponent, p_pos ), p_opponent );
                        snprintf(
                            buffer, TMP_BUFFER_SIZE, fmt.c_str( ),
                            p_ui->getPkmnName( getPkmnOrDisguise( p_opponent, p_pos ), p_opponent )
                                .c_str( ),
                            FS::getItemName( pkmn->getItem( ) ).c_str( ) );
                        p_ui->log( buffer );
                        p_ui->logBoosts( getPkmnOrDisguise( p_opponent, p_pos ), p_opponent, p_pos,
                                         bs, res );
                        removeItem( p_ui, p_opponent, p_pos );
                        checkOnEatBerry( p_ui, p_opponent, p_pos, pkmn->getItem( ) );
                    }
                }
                break;
            }
            case I_SALAC_BERRY: {
                if( lowhptrigger ) {
                    boosts bs = boosts( );
                    bs.setBoost( SPEED, 3 * ( 1 + ripen ) );

                    auto res = addBoosts( p_opponent, p_pos, bs );
                    if( res != boosts( ) ) {
                        p_ui->logItem( getPkmnOrDisguise( p_opponent, p_pos ), p_opponent );
                        snprintf(
                            buffer, TMP_BUFFER_SIZE, fmt.c_str( ),
                            p_ui->getPkmnName( getPkmnOrDisguise( p_opponent, p_pos ), p_opponent )
                                .c_str( ),
                            FS::getItemName( pkmn->getItem( ) ).c_str( ) );
                        p_ui->log( buffer );
                        p_ui->logBoosts( getPkmnOrDisguise( p_opponent, p_pos ), p_opponent, p_pos,
                                         bs, res );
                        removeItem( p_ui, p_opponent, p_pos );
                        checkOnEatBerry( p_ui, p_opponent, p_pos, pkmn->getItem( ) );
                    }
                }
                break;
            }
            case I_PETAYA_BERRY: {
                if( lowhptrigger ) {
                    boosts bs = boosts( );
                    bs.setBoost( SATK, 3 * ( 1 + ripen ) );

                    auto res = addBoosts( p_opponent, p_pos, bs );
                    if( res != boosts( ) ) {
                        p_ui->logItem( getPkmnOrDisguise( p_opponent, p_pos ), p_opponent );
                        snprintf(
                            buffer, TMP_BUFFER_SIZE, fmt.c_str( ),
                            p_ui->getPkmnName( getPkmnOrDisguise( p_opponent, p_pos ), p_opponent )
                                .c_str( ),
                            FS::getItemName( pkmn->getItem( ) ).c_str( ) );
                        p_ui->log( buffer );
                        p_ui->logBoosts( getPkmnOrDisguise( p_opponent, p_pos ), p_opponent, p_pos,
                                         bs, res );
                        removeItem( p_ui, p_opponent, p_pos );
                        checkOnEatBerry( p_ui, p_opponent, p_pos, pkmn->getItem( ) );
                    }
                }
                break;
            }
            case I_APICOT_BERRY: {
                if( lowhptrigger ) {
                    boosts bs = boosts( );
                    bs.setBoost( SDEF, 3 * ( 1 + ripen ) );

                    auto res = addBoosts( p_opponent, p_pos, bs );
                    if( res != boosts( ) ) {
                        p_ui->logItem( getPkmnOrDisguise( p_opponent, p_pos ), p_opponent );
                        snprintf(
                            buffer, TMP_BUFFER_SIZE, fmt.c_str( ),
                            p_ui->getPkmnName( getPkmnOrDisguise( p_opponent, p_pos ), p_opponent )
                                .c_str( ),
                            FS::getItemName( pkmn->getItem( ) ).c_str( ) );
                        p_ui->log( buffer );
                        p_ui->logBoosts( getPkmnOrDisguise( p_opponent, p_pos ), p_opponent, p_pos,
                                         bs, res );
                        removeItem( p_ui, p_opponent, p_pos );
                        checkOnEatBerry( p_ui, p_opponent, p_pos, pkmn->getItem( ) );
                    }
                }
                break;
            }
            case I_STARF_BERRY: {
                if( lowhptrigger ) {
                    boosts bs  = getBoosts( p_opponent, p_pos );
                    boosts res = boosts( );
                    u8     rs  = 1 + rand( ) % 5;

                    bool gd = false;
                    for( u8 i2 = 1; i2 < 6; ++i2 ) {
                        if( bs.getBoost( i2 ) < 6 ) { gd = true; }
                    }
                    while( gd && bs.getBoost( rs ) >= 6 ) { rs = 1 + rand( ) % 5; }
                    res.setBoost( rs, 3 * ( 1 + ripen ) );

                    auto res2 = addBoosts( p_opponent, p_pos, res );
                    if( res2 != boosts( ) ) {
                        p_ui->logItem( getPkmnOrDisguise( p_opponent, p_pos ), p_opponent );
                        snprintf(
                            buffer, TMP_BUFFER_SIZE, fmt.c_str( ),
                            p_ui->getPkmnName( getPkmnOrDisguise( p_opponent, p_pos ), p_opponent )
                                .c_str( ),
                            FS::getItemName( pkmn->getItem( ) ).c_str( ) );
                        p_ui->log( buffer );

                        p_ui->logBoosts( getPkmnOrDisguise( p_opponent, p_pos ), p_opponent, p_pos,
                                         res, res2 );

                        removeItem( p_ui, p_opponent, p_pos );
                        checkOnEatBerry( p_ui, p_opponent, p_pos, pkmn->getItem( ) );
                    }
                }
                break;
            }
                [[likely]] default : break;
            }
        }
        return;
    }

    void field::checkOnTakeDamage( battleUI* p_ui, battleMove p_move, fieldPosition p_target,
                                   u16 p_damage, u8 p_effectiveness ) {
        if( !p_damage ) { return; }

        auto user   = getPkmn( p_move.m_user.first, p_move.m_user.second );
        auto target = getPkmn( p_target.first, p_target.second );
        if( user == nullptr || target == nullptr ) [[unlikely]] { return; }

        constexpr u8 TMP_BUFFER_SIZE = 100;
        char         buffer[ TMP_BUFFER_SIZE + 10 ];
        bool         supprAbs = suppressesAbilities( );

        // Check for disguise of A_ILLUSION that breaks
        if( p_damage ) {
            if( getSlot( p_target.first, p_target.second )->isDisguised( ) ) {
                setSlotDisguise( p_target.first, p_target.second, nullptr );
                p_ui->logAbility( getPkmnOrDisguise( p_target.first, p_target.second ),
                                  p_target.first );
                p_ui->updatePkmn( p_target.first, p_target.second,
                                  getPkmnOrDisguise( p_target.first, p_target.second ) );
            }
        }

        // Check for M_FOCUS_PUNCH and M_SHELL_TRAP
        auto tgvol = getVolatileStatus( p_target.first, p_target.second );
        if( p_damage && ( tgvol & VS_FOCUSPUNCH ) ) {
            removeVolatileStatus( p_ui, p_target.first, p_target.second, VS_FOCUSPUNCH );
        }
        if( p_move.m_moveData.m_category == MH_PHYSICAL && p_damage && ( tgvol & VS_SHELLTRAP ) ) {
            removeVolatileStatus( p_ui, p_target.first, p_target.second, VS_SHELLTRAP );
        }

        if( canUseItem( p_target.first, p_target.second ) ) {
            auto fmt = std::string( GET_STRING( IO::STR_UI_BATTLE_ACTIVATE_ITEM ) );
            switch( target->getItem( ) ) {
            case I_WEAKNESS_POLICY:
                if( p_effectiveness > 100 && target->canBattle( )
                    && target->m_stats.m_curHP < target->m_stats.m_maxHP ) {
                    boosts bs = boosts( );
                    bs.setBoost( ATK, 2 );
                    bs.setBoost( SATK, 2 );

                    auto res = addBoosts( p_target.first, p_target.second, bs );
                    if( res != boosts( ) ) {
                        p_ui->logItem( getPkmnOrDisguise( p_target.first, p_target.second ),
                                       p_target.first );
                        snprintf(
                            buffer, TMP_BUFFER_SIZE, fmt.c_str( ),
                            p_ui->getPkmnName( getPkmnOrDisguise( p_target.first, p_target.second ),
                                               p_target.first )
                                .c_str( ),
                            FS::getItemName( target->getItem( ) ).c_str( ) );
                        p_ui->log( buffer );

                        p_ui->logBoosts( getPkmnOrDisguise( p_target.first, p_target.second ),
                                         p_target.first, p_target.second, bs, res );

                        removeItem( p_ui, p_target.first, p_target.second );
                    }
                }
                break;
            case I_SNOWBALL:
                if( p_move.m_moveData.m_type == TYPE_ICE && target->canBattle( )
                    && target->m_stats.m_curHP < target->m_stats.m_maxHP ) {
                    boosts bs = boosts( );
                    bs.setBoost( ATK, 1 );

                    auto res = addBoosts( p_target.first, p_target.second, bs );
                    if( res != boosts( ) ) {
                        p_ui->logItem( getPkmnOrDisguise( p_target.first, p_target.second ),
                                       p_target.first );
                        snprintf(
                            buffer, TMP_BUFFER_SIZE, fmt.c_str( ),
                            p_ui->getPkmnName( getPkmnOrDisguise( p_target.first, p_target.second ),
                                               p_target.first )
                                .c_str( ),
                            FS::getItemName( target->getItem( ) ).c_str( ) );
                        p_ui->log( buffer );

                        p_ui->logBoosts( getPkmnOrDisguise( p_target.first, p_target.second ),
                                         p_target.first, p_target.second, bs, res );

                        removeItem( p_ui, p_target.first, p_target.second );
                    }
                }
                break;
            case I_ABSORB_BULB:
                if( p_move.m_moveData.m_type == TYPE_WATER && target->canBattle( )
                    && target->m_stats.m_curHP < target->m_stats.m_maxHP ) {
                    boosts bs = boosts( );
                    bs.setBoost( SATK, 1 );

                    auto res = addBoosts( p_target.first, p_target.second, bs );
                    if( res != boosts( ) ) {
                        p_ui->logItem( getPkmnOrDisguise( p_target.first, p_target.second ),
                                       p_target.first );
                        snprintf(
                            buffer, TMP_BUFFER_SIZE, fmt.c_str( ),
                            p_ui->getPkmnName( getPkmnOrDisguise( p_target.first, p_target.second ),
                                               p_target.first )
                                .c_str( ),
                            FS::getItemName( target->getItem( ) ).c_str( ) );
                        p_ui->log( buffer );

                        p_ui->logBoosts( getPkmnOrDisguise( p_target.first, p_target.second ),
                                         p_target.first, p_target.second, bs, res );

                        removeItem( p_ui, p_target.first, p_target.second );
                    }
                }
                break;

            case I_LUMINOUS_MOSS:
                if( p_move.m_moveData.m_type == TYPE_WATER && target->canBattle( )
                    && target->m_stats.m_curHP < target->m_stats.m_maxHP ) {
                    boosts bs = boosts( );
                    bs.setBoost( SDEF, 1 );

                    auto res = addBoosts( p_target.first, p_target.second, bs );
                    if( res != boosts( ) ) {
                        p_ui->logItem( getPkmnOrDisguise( p_target.first, p_target.second ),
                                       p_target.first );
                        snprintf(
                            buffer, TMP_BUFFER_SIZE, fmt.c_str( ),
                            p_ui->getPkmnName( getPkmnOrDisguise( p_target.first, p_target.second ),
                                               p_target.first )
                                .c_str( ),
                            FS::getItemName( target->getItem( ) ).c_str( ) );
                        p_ui->log( buffer );

                        p_ui->logBoosts( getPkmnOrDisguise( p_target.first, p_target.second ),
                                         p_target.first, p_target.second, bs, res );

                        removeItem( p_ui, p_target.first, p_target.second );
                    }
                }
                break;
            case I_CELL_BATTERY:
                if( p_move.m_moveData.m_type == TYPE_LIGHTNING && target->canBattle( )
                    && target->m_stats.m_curHP < target->m_stats.m_maxHP ) {
                    boosts bs = boosts( );
                    bs.setBoost( ATK, 1 );

                    auto res = addBoosts( p_target.first, p_target.second, bs );
                    if( res != boosts( ) ) {
                        p_ui->logItem( getPkmnOrDisguise( p_target.first, p_target.second ),
                                       p_target.first );
                        snprintf(
                            buffer, TMP_BUFFER_SIZE, fmt.c_str( ),
                            p_ui->getPkmnName( getPkmnOrDisguise( p_target.first, p_target.second ),
                                               p_target.first )
                                .c_str( ),
                            FS::getItemName( target->getItem( ) ).c_str( ) );
                        p_ui->log( buffer );

                        p_ui->logBoosts( getPkmnOrDisguise( p_target.first, p_target.second ),
                                         p_target.first, p_target.second, bs, res );

                        removeItem( p_ui, p_target.first, p_target.second );
                    }
                }
                break;

            case I_AIR_BALLOON: {
                fmt = std::string( GET_STRING( IO::STR_UI_BATTLE_ACTIVATE_AIR_BALLON ) );
                snprintf( buffer, TMP_BUFFER_SIZE, fmt.c_str( ),
                          p_ui->getPkmnName( getPkmnOrDisguise( p_target.first, p_target.second ),
                                             p_target.first )
                              .c_str( ) );
                p_ui->log( buffer );
                removeItem( p_ui, p_target.first, p_target.second );
                break;
            }
            case I_EJECT_BUTTON:
                p_ui->logItem( getPkmnOrDisguise( p_target.first, p_target.second ),
                               p_target.first );
                snprintf( buffer, TMP_BUFFER_SIZE, fmt.c_str( ),
                          p_ui->getPkmnName( getPkmnOrDisguise( p_target.first, p_target.second ),
                                             p_target.first )
                              .c_str( ),
                          FS::getItemName( target->getItem( ) ).c_str( ) );
                p_ui->log( buffer );
                removeItem( p_ui, p_target.first, p_target.second );
                // TODO: should only work if opponent has > 1 pkmn that can battle
                recallPokemon( p_ui, p_target.first, p_target.second );
                break;
            default: break;
            }
        }

        // berries
        if( supprAbs
            || !_sides[ !p_move.m_user.first ? PLAYER_SIDE : OPPONENT_SIDE ].anyHasAbility(
                A_UNNERVE ) ) {
            if( canUseItem( p_target.first, p_target.second ) ) {
                auto fmt = std::string( GET_STRING( IO::STR_UI_BATTLE_EAT_ITEM ) );
                switch( target->getItem( ) ) {
                case I_ENIGMA_BERRY:
                    if( p_effectiveness > 100 && target->canBattle( )
                        && target->m_stats.m_curHP < target->m_stats.m_maxHP ) {
                        p_ui->logItem( getPkmnOrDisguise( p_target.first, p_target.second ),
                                       p_target.first );
                        snprintf(
                            buffer, TMP_BUFFER_SIZE, fmt.c_str( ),
                            p_ui->getPkmnName( getPkmnOrDisguise( p_target.first, p_target.second ),
                                               p_target.first )
                                .c_str( ),
                            FS::getItemName( target->getItem( ) ).c_str( ) );
                        p_ui->log( buffer );

                        if( supprAbs || target->getAbility( ) != A_RIPEN ) {
                            healPokemon( p_ui, p_target.first, p_target.second,
                                         target->m_stats.m_maxHP / 4 );
                        } else {
                            healPokemon( p_ui, p_target.first, p_target.second,
                                         target->m_stats.m_maxHP / 2 );
                        }

                        removeItem( p_ui, p_target.first, p_target.second );
                        checkOnEatBerry( p_ui, p_target.first, p_target.second,
                                         target->getItem( ) );
                    }
                    break;
                case I_JABOCA_BERRY:
                    if( user->canBattle( ) && p_move.m_moveData.m_category == MH_PHYSICAL ) {
                        p_ui->logItem( getPkmnOrDisguise( p_target.first, p_target.second ),
                                       p_target.first );
                        snprintf(
                            buffer, TMP_BUFFER_SIZE, fmt.c_str( ),
                            p_ui->getPkmnName( getPkmnOrDisguise( p_target.first, p_target.second ),
                                               p_target.first )
                                .c_str( ),
                            FS::getItemName( target->getItem( ) ).c_str( ) );
                        p_ui->log( buffer );

                        if( supprAbs || target->getAbility( ) != A_RIPEN ) {
                            damagePokemon( p_ui, p_move.m_user.first, p_move.m_user.second,
                                           target->m_stats.m_maxHP / 8 );
                        } else {
                            damagePokemon( p_ui, p_move.m_user.first, p_move.m_user.second,
                                           target->m_stats.m_maxHP / 4 );
                        }

                        removeItem( p_ui, p_target.first, p_target.second );
                        checkOnEatBerry( p_ui, p_target.first, p_target.second,
                                         target->getItem( ) );
                    }
                    break;
                case I_KEE_BERRY:
                    if( user->canBattle( ) && p_move.m_moveData.m_category == MH_PHYSICAL ) {
                        boosts bs = boosts( );
                        if( supprAbs || target->getAbility( ) != A_RIPEN ) {
                            bs.setBoost( DEF, 1 );
                        } else {
                            bs.setBoost( DEF, 2 );
                        }

                        auto res = addBoosts( p_target.first, p_target.second, bs );
                        if( res != boosts( ) ) {
                            p_ui->logItem( getPkmnOrDisguise( p_target.first, p_target.second ),
                                           p_target.first );
                            snprintf( buffer, TMP_BUFFER_SIZE, fmt.c_str( ),
                                      p_ui->getPkmnName(
                                              getPkmnOrDisguise( p_target.first, p_target.second ),
                                              p_target.first )
                                          .c_str( ),
                                      FS::getItemName( target->getItem( ) ).c_str( ) );
                            p_ui->log( buffer );

                            p_ui->logBoosts( getPkmnOrDisguise( p_target.first, p_target.second ),
                                             p_target.first, p_target.second, bs, res );

                            removeItem( p_ui, p_target.first, p_target.second );
                            checkOnEatBerry( p_ui, p_target.first, p_target.second,
                                             target->getItem( ) );
                        }
                    }
                    break;
                case I_MARANGA_BERRY:
                    if( user->canBattle( ) && p_move.m_moveData.m_category == MH_SPECIAL ) {
                        boosts bs = boosts( );
                        if( supprAbs || target->getAbility( ) != A_RIPEN ) {
                            bs.setBoost( SDEF, 1 );
                        } else {
                            bs.setBoost( SDEF, 2 );
                        }

                        auto res = addBoosts( p_target.first, p_target.second, bs );
                        if( res != boosts( ) ) {
                            p_ui->logItem( getPkmnOrDisguise( p_target.first, p_target.second ),
                                           p_target.first );
                            snprintf( buffer, TMP_BUFFER_SIZE, fmt.c_str( ),
                                      p_ui->getPkmnName(
                                              getPkmnOrDisguise( p_target.first, p_target.second ),
                                              p_target.first )
                                          .c_str( ),
                                      FS::getItemName( target->getItem( ) ).c_str( ) );
                            p_ui->log( buffer );

                            p_ui->logBoosts( getPkmnOrDisguise( p_target.first, p_target.second ),
                                             p_target.first, p_target.second, bs, res );

                            removeItem( p_ui, p_target.first, p_target.second );
                            checkOnEatBerry( p_ui, p_target.first, p_target.second,
                                             target->getItem( ) );
                        }
                    }
                    break;

                case I_ROWAP_BERRY:
                    if( user->canBattle( ) && p_move.m_moveData.m_category == MH_SPECIAL ) {
                        p_ui->logItem( getPkmnOrDisguise( p_target.first, p_target.second ),
                                       p_target.first );
                        snprintf(
                            buffer, TMP_BUFFER_SIZE, fmt.c_str( ),
                            p_ui->getPkmnName( getPkmnOrDisguise( p_target.first, p_target.second ),
                                               p_target.first )
                                .c_str( ),
                            FS::getItemName( target->getItem( ) ).c_str( ) );
                        p_ui->log( buffer );

                        if( supprAbs || target->getAbility( ) != A_RIPEN ) {
                            damagePokemon( p_ui, p_move.m_user.first, p_move.m_user.second,
                                           target->m_stats.m_maxHP / 8 );
                        } else {
                            damagePokemon( p_ui, p_move.m_user.first, p_move.m_user.second,
                                           target->m_stats.m_maxHP / 4 );
                        }

                        removeItem( p_ui, p_target.first, p_target.second );
                        checkOnEatBerry( p_ui, p_target.first, p_target.second,
                                         target->getItem( ) );
                    }
                    break;

                default: break;
                }
            }
        }
    }

    void field::checkOnSendOut( battleUI* p_ui, bool p_opponent, u8 p_slot ) {
        auto pkmn = getPkmn( p_opponent, p_slot );
        if( pkmn == nullptr ) { return; }

        bool items = canUseItem( p_opponent, p_slot );

        switch( pkmn->getAbility( ) ) {
        // abilities that cannot be suppressed
        case A_COMATOSE:
        case A_NEUTRALIZING_GAS:
            p_ui->logAbility( getPkmnOrDisguise( p_opponent, p_slot ), p_opponent );
            break;
        case A_ZEN_MODE:
            if( pkmn->m_stats.m_curHP * 2 < pkmn->m_stats.m_maxHP && !( pkmn->getForme( ) & 1 ) ) {
                if( pkmn->getSpecies( ) == PKMN_DARMANITAN ) {
                    p_ui->logAbility( pkmn, p_opponent );
                    pkmn->battleTransform( );
                    p_ui->updatePkmn( p_opponent, p_slot, pkmn );
                }
            }
            break;
        case A_SCHOOLING: {
            if( pkmn->m_level >= 20 && pkmn->m_stats.m_curHP * 4 > pkmn->m_stats.m_maxHP
                && !pkmn->getForme( ) ) {
                p_ui->logAbility( pkmn, p_opponent );
                pkmn->battleTransform( );
                p_ui->updatePkmn( p_opponent, p_slot, pkmn );
            }
            break;
        }

        default:
            if( !suppressesAbilities( ) ) {
                switch( pkmn->getAbility( ) ) {
                case A_ARENA_TRAP:
                case A_SHADOW_TAG:
                case A_MAGNET_PULL:
                case A_CLOUD_NINE:
                case A_AIR_LOCK:
                case A_MOLD_BREAKER:
                case A_TERAVOLT:
                case A_TURBOBLAZE:
                case A_PRESSURE:
                case A_UNNERVE:
                    p_ui->logAbility( getPkmnOrDisguise( p_opponent, p_slot ), p_opponent );
                    break;
                case A_DISGUISE: {
                    if( pkmn->getForme( ) == 0 ) {
                        p_ui->logAbility( getPkmnOrDisguise( p_opponent, p_slot ), p_opponent );
                    }
                    break;
                }
                // Stat changing abilities
                case A_DAUNTLESS_SHIELD: {
                    p_ui->logAbility( getPkmnOrDisguise( p_opponent, p_slot ), p_opponent );
                    boosts bt = boosts( );
                    bt.setBoost( DEF, 1 );
                    auto res = addBoosts( p_opponent, p_slot, bt );
                    p_ui->logBoosts( getPkmnOrDisguise( p_opponent, p_slot ), p_opponent, p_slot,
                                     bt, res );
                    break;
                }
                case A_INTREPID_SWORD: {
                    p_ui->logAbility( getPkmnOrDisguise( p_opponent, p_slot ), p_opponent );
                    boosts bt = boosts( );
                    bt.setBoost( ATK, 1 );
                    auto res = addBoosts( p_opponent, p_slot, bt );
                    p_ui->logBoosts( getPkmnOrDisguise( p_opponent, p_slot ), p_opponent, p_slot,
                                     bt, res );
                    break;
                }
                case A_INTIMIDATE: {
                    p_ui->logAbility( getPkmnOrDisguise( p_opponent, p_slot ), p_opponent );
                    boosts bt = boosts( );
                    bt.setBoost( ATK, -1 );
                    for( u8 i = 0; i < getBattlingPKMNCount( _mode ); ++i ) {
                        if( getPkmn( !p_opponent, i ) == nullptr ) { continue; }
                        auto res = addBoosts( !p_opponent, i, bt );
                        p_ui->logBoosts( getPkmnOrDisguise( !p_opponent, i ), !p_opponent, i, bt,
                                         res );
                    }
                    break;
                }
                case A_DOWNLOAD: {
                    p_ui->logAbility( getPkmnOrDisguise( p_opponent, p_slot ), p_opponent );
                    boosts bt = boosts( );

                    u16 def = 0, sdef = 0;
                    for( u8 i = 0; i < getBattlingPKMNCount( _mode ); ++i ) {
                        def += getStat( !p_opponent, i, DEF );
                        sdef += getStat( !p_opponent, i, SDEF );
                    }
                    bt.setBoost( def < sdef ? ATK : SATK, 1 );
                    auto res = addBoosts( p_opponent, p_slot, bt );
                    p_ui->logBoosts( getPkmnOrDisguise( p_opponent, p_slot ), p_opponent, p_slot,
                                     bt, res );
                    break;
                }
                // Weather abilities
                case A_DRIZZLE:
                    p_ui->logAbility( getPkmnOrDisguise( p_opponent, p_slot ), p_opponent );
                    setWeather( p_ui, WE_RAIN, items && pkmn->getItem( ) == I_DAMP_ROCK );
                    break;
                case A_DROUGHT:
                    p_ui->logAbility( getPkmnOrDisguise( p_opponent, p_slot ), p_opponent );
                    setWeather( p_ui, WE_SUN, items && pkmn->getItem( ) == I_HEAT_ROCK );
                    break;
                case A_SAND_STREAM:
                    p_ui->logAbility( getPkmnOrDisguise( p_opponent, p_slot ), p_opponent );
                    setWeather( p_ui, WE_SANDSTORM, items && pkmn->getItem( ) == I_SMOOTH_ROCK );
                    break;
                case A_SNOW_WARNING:
                    p_ui->logAbility( getPkmnOrDisguise( p_opponent, p_slot ), p_opponent );
                    setWeather( p_ui, WE_HAIL, items && pkmn->getItem( ) == I_ICY_ROCK );
                    break;
                case A_PRIMORDIAL_SEA:
                    p_ui->logAbility( getPkmnOrDisguise( p_opponent, p_slot ), p_opponent );
                    setWeather( p_ui, WE_HEAVY_RAIN );
                    break;
                case A_DESOLATE_LAND:
                    p_ui->logAbility( getPkmnOrDisguise( p_opponent, p_slot ), p_opponent );
                    setWeather( p_ui, WE_HEAVY_SUNSHINE );
                    break;
                case A_DELTA_STREAM:
                    p_ui->logAbility( getPkmnOrDisguise( p_opponent, p_slot ), p_opponent );
                    setWeather( p_ui, WE_HEAVY_WINDS );
                    break;

                // Terrain abilities
                case A_ELECTRIC_SURGE:
                    p_ui->logAbility( getPkmnOrDisguise( p_opponent, p_slot ), p_opponent );
                    setTerrain( p_ui, TR_ELECTRICTERRAIN,
                                items && pkmn->getItem( ) == I_TERRAIN_EXTENDER );
                    break;
                case A_PSYCHIC_SURGE:
                    p_ui->logAbility( getPkmnOrDisguise( p_opponent, p_slot ), p_opponent );
                    setTerrain( p_ui, TR_PSYCHICTERRAIN,
                                items && pkmn->getItem( ) == I_TERRAIN_EXTENDER );
                    break;
                case A_GRASSY_SURGE:
                    p_ui->logAbility( getPkmnOrDisguise( p_opponent, p_slot ), p_opponent );
                    setTerrain( p_ui, TR_GRASSYTERRAIN,
                                items && pkmn->getItem( ) == I_TERRAIN_EXTENDER );
                    break;
                case A_MISTY_SURGE:
                    p_ui->logAbility( getPkmnOrDisguise( p_opponent, p_slot ), p_opponent );
                    setTerrain( p_ui, TR_MISTYTERRAIN,
                                items && pkmn->getItem( ) == I_TERRAIN_EXTENDER );
                    break;

                // Form change abilities
                case A_IMPOSTER:
                    if( transformPkmn(
                            p_opponent, p_slot,
                            getSlot( !p_opponent, getOpposingPkmn( p_slot, _mode ) ) ) ) {
                        p_ui->logAbility( getPkmnOrDisguise( p_opponent, p_slot ), p_opponent );
                        p_ui->updatePkmn( p_opponent, p_slot, getPkmn( p_opponent, p_slot ) );
                        if( getPkmn( p_opponent, p_slot )->getAbility( ) != A_IMPOSTER ) {
                            checkOnSendOut( p_ui, p_opponent, p_slot );
                        }
                    }
                    break;

                case A_TRACE: {
                    p_ui->logAbility( getPkmnOrDisguise( p_opponent, p_slot ), p_opponent );
                    auto tmp = getPkmn( !p_opponent, getOpposingPkmn( p_slot, _mode ) );
                    if( tmp != nullptr && allowsCopy( tmp->getAbility( ) )
                        && changeAbility( p_opponent, p_slot, tmp->getAbility( ) ) ) {
                        p_ui->logAbility( getPkmnOrDisguise( p_opponent, p_slot ), p_opponent );

                        if( getPkmn( p_opponent, p_slot )->getAbility( ) != A_TRACE ) {
                            checkOnSendOut( p_ui, p_opponent, p_slot );
                        }
                    }
                    break;
                }

                case A_ANTICIPATION: {
                    p_ui->logAbility( getPkmnOrDisguise( p_opponent, p_slot ), p_opponent );
                    bool warn = false;
                    for( u8 i = 0; !warn && i < getBattlingPKMNCount( _mode ); ++i ) {
                        auto tmp = getPkmn( !p_opponent, i );
                        if( tmp == nullptr ) { continue; }
                        for( u8 j = 0; j < 4; ++j ) {
                            if( tmp->getMove( j ) ) [[likely]] {
                                auto mdata = FS::getMoveData( tmp->getMove( j ) );
                                if( ( mdata.m_flags & MF_OHKO ) ) [[unlikely]] {
                                    warn = true;
                                    break;
                                }
                                u16 eff = 1;
                                for( auto tp : getTypes( p_opponent, p_slot ) ) {
                                    eff *= getTypeEffectiveness( mdata.m_type, tp );
                                }
                                if( eff > 1 ) {
                                    warn = true;
                                    break;
                                }
                            }
                        }
                    }
                    if( warn ) {
                        p_ui->logAnticipation( getPkmnOrDisguise( p_opponent, p_slot ),
                                               p_opponent );
                    }
                    break;
                }

                case A_FOREWARN: {
                    p_ui->logAbility( getPkmnOrDisguise( p_opponent, p_slot ), p_opponent );
                    std::vector<std::pair<u8, u16>> moves = std::vector<std::pair<u8, u16>>( );
                    for( u8 i = 0; i < getBattlingPKMNCount( _mode ); ++i ) {
                        auto tmp = getPkmn( !p_opponent, i );
                        if( tmp == nullptr ) { continue; }
                        for( u8 j = 0; j < 4; ++j ) {
                            if( tmp->getMove( j ) ) {
                                moves.push_back( std::pair<u8, u16>(
                                    255 - FS::getMoveData( tmp->getMove( j ) ).m_basePower,
                                    tmp->getMove( j ) ) );
                            }
                        }
                    }
                    std::sort( moves.begin( ), moves.end( ) );
                    p_ui->logForewarn( getPkmnOrDisguise( p_opponent, p_slot ), p_opponent,
                                       moves[ 0 ].second );
                    break;
                }
                case A_FRISK: {
                    std::vector<u16> itms = std::vector<u16>( );

                    for( u8 i = 0; i < getBattlingPKMNCount( _mode ); ++i ) {
                        auto tmp = getPkmn( !p_opponent, i );
                        if( tmp == nullptr ) { continue; }
                        if( tmp->getItem( ) ) { itms.push_back( tmp->getItem( ) ); }
                    }
                    if( itms.size( ) ) {
                        p_ui->logAbility( getPkmnOrDisguise( p_opponent, p_slot ), p_opponent );
                        p_ui->logFrisk( getPkmnOrDisguise( p_opponent, p_slot ), p_opponent, itms );
                    }
                    break;
                }

                case A_SCREEN_CLEANER: {
                    p_ui->logAbility( getPkmnOrDisguise( p_opponent, p_slot ), p_opponent );
                    for( u8 i = 0; i < getBattlingPKMNCount( _mode ); ++i ) {
                        _sides[ i ].removeSideCondition(
                            p_ui, sideCondition( SC_LIGHTSCREEN | SC_REFLECT | SC_AURORAVEIL ) );
                    }
                    break;
                }
                case A_FLOWER_GIFT: {
                    if( pkmn->getSpecies( ) == PKMN_CHERRIM ) {
                        if( !suppressesWeather( ) ) {
                            if( getWeather( ) == WE_SUN || getWeather( ) == WE_HEAVY_SUNSHINE ) {
                                pkmn->setBattleForme( 1 );
                                p_ui->updatePkmn( p_opponent, p_slot, pkmn );
                                break;
                            }
                        }
                        pkmn->setBattleForme( 0 );
                        p_ui->updatePkmn( p_opponent, p_slot, pkmn );
                    }

                    break;
                }
                case A_FORECAST: {
                    if( pkmn->getSpecies( ) == PKMN_CASTFORM ) {
                        if( !suppressesWeather( ) ) {
                            if( getWeather( ) == WE_RAIN || getWeather( ) == WE_HEAVY_RAIN ) {
                                pkmn->setBattleForme( 2 );
                                p_ui->updatePkmn( p_opponent, p_slot, pkmn );
                                break;
                            }
                            if( getWeather( ) == WE_SUN || getWeather( ) == WE_HEAVY_SUNSHINE ) {
                                pkmn->setBattleForme( 1 );
                                p_ui->updatePkmn( p_opponent, p_slot, pkmn );
                                break;
                            }
                            if( getWeather( ) == WE_HAIL ) {
                                pkmn->setBattleForme( 3 );
                                p_ui->updatePkmn( p_opponent, p_slot, pkmn );
                                break;
                            }
                        }
                        pkmn->setBattleForme( 0 );
                        p_ui->updatePkmn( p_opponent, p_slot, pkmn );
                    }
                    break;
                }
                default: break;
                }
                break;
            }
        }
    }

    void field::init( battleUI* p_ui ) {
        (void) p_ui;
    }

    void field::age( battleUI* p_ui ) {
        for( u8 i = 0; i < 2; ++i ) { _sides[ i ].age( p_ui ); }
        if( _weatherTimer > 0 ) {
            if( _weatherTimer < 240 && !--_weatherTimer ) {
                // Weather ended
                removeWeather( p_ui );
            } else {
                continueWeather( p_ui );
            }
        }
        if( _terrainTimer > 0 && _terrainTimer < 240 ) {
            if( !--_terrainTimer ) {
                // terrain ended
                removeTerrain( p_ui );
            }
        }

        for( u8 i = 0; i < MAX_PSEUDO_WEATHER; ++i ) {
            if( _pseudoWeatherTimer[ i ] > 0 && _pseudoWeatherTimer[ i ] < 240 ) {
                if( !--_pseudoWeatherTimer[ i ] ) {
                    // pseudo weather ended
                    p_ui->removePseudoWeather( i );
                }
            }
        }

        // abilities
        bool supprA = suppressesAbilities( );
        if( !supprA ) {
            for( u8 i = 0; i < 2; ++i ) {
                for( u8 j = 0; j < getBattlingPKMNCount( _mode ); ++j ) {
                    auto pkmn = getPkmn( i, j );
                    if( pkmn == nullptr || supprA ) { continue; }

                    auto adj = getPkmn( i, !j );

                    if( adj != nullptr && adj->getAbility( ) == A_HEALER ) {
                        if( pkmn->m_statusint && !( rand( ) % 3 ) ) {
                            if( removeStatusCondition( i, j ) ) {
                                p_ui->logAbility( getPkmnOrDisguise( i, !j ), i );
                                p_ui->updatePkmnStats( i, j, getPkmnOrDisguise( i, j ), true );
                            }
                        }
                    }

                    switch( pkmn->getAbility( ) ) {
                    case A_SHED_SKIN:
                        if( pkmn->m_statusint && !( rand( ) % 3 ) ) {
                            if( removeStatusCondition( i, j ) ) {
                                p_ui->logAbility( getPkmnOrDisguise( i, j ), i );
                                p_ui->updatePkmnStats( i, j, getPkmnOrDisguise( i, j ), true );
                            }
                        }
                        break;
                    case A_SPEED_BOOST: {
                        boosts bs  = getBoosts( i, j );
                        boosts res = boosts( );
                        if( bs.getBoost( SPEED ) < 6 ) {
                            res.setBoost( SPEED, 1 );
                            p_ui->logAbility( getPkmnOrDisguise( i, j ), i );
                            auto res2 = addBoosts( i, j, res );
                            p_ui->logBoosts( getPkmnOrDisguise( i, j ), i, j, res, res2 );
                        }
                        break;
                    }
                    case A_MOODY: {
                        boosts bs  = getBoosts( i, j );
                        boosts res = boosts( );
                        u8     rs = 1 + rand( ) % 5, lw = 1 + rand( ) % 5;

                        bool gd = false;
                        for( u8 i2 = 1; i2 < 6; ++i2 ) {
                            if( bs.getBoost( i2 ) < 6 ) { gd = true; }
                        }
                        while( gd && bs.getBoost( rs ) >= 6 ) { rs = 1 + rand( ) % 5; }
                        res.setBoost( rs, 2 );
                        gd = false;
                        for( u8 i2 = 1; i2 < 6; ++i2 ) {
                            if( bs.getBoost( i2 ) > -6 ) { gd = true; }
                        }
                        while( gd && ( bs.getBoost( lw ) <= -6 || rs == lw ) ) {
                            lw = 1 + rand( ) % 5;
                        }
                        res.setBoost( lw, -1 );

                        if( res != boosts( ) ) {
                            p_ui->logAbility( getPkmnOrDisguise( i, j ), i );
                            auto res2 = addBoosts( i, j, res );
                            p_ui->logBoosts( getPkmnOrDisguise( i, j ), i, j, res, res2 );
                        }
                        break;
                    }
                    case A_ZEN_MODE:
                        if( pkmn->m_stats.m_curHP * 2 < pkmn->m_stats.m_maxHP
                            && !( pkmn->getForme( ) & 1 ) ) {
                            if( pkmn->getSpecies( ) == PKMN_DARMANITAN ) {
                                p_ui->logAbility( getPkmnOrDisguise( i, j ), i );
                                pkmn->battleTransform( );
                                p_ui->updatePkmn( i, j, getPkmnOrDisguise( i, j ) );
                            }
                        }
                        break;
                    case A_POWER_CONSTRUCT:
                        if( pkmn->m_stats.m_curHP * 2 < pkmn->m_stats.m_maxHP
                            && pkmn->getForme( ) < 2 ) {
                            if( pkmn->getSpecies( ) == PKMN_ZYGARDE ) {
                                p_ui->logAbility( getPkmnOrDisguise( i, j ), i );
                                pkmn->battleTransform( );
                                p_ui->updatePkmn( i, j, getPkmnOrDisguise( i, j ) );
                            }
                        }
                        break;
                    case A_SHIELDS_DOWN:
                        if( pkmn->m_stats.m_curHP * 2 < pkmn->m_stats.m_maxHP
                            && pkmn->getForme( ) < 7 ) {
                            if( pkmn->getSpecies( ) == PKMN_MINIOR ) {
                                p_ui->logAbility( getPkmnOrDisguise( i, j ), i );
                                // TODO: implement minior battle transform
                                pkmn->battleTransform( );
                                p_ui->updatePkmn( i, j, getPkmnOrDisguise( i, j ) );
                            }
                        }
                        break;

                    case A_SCHOOLING: {
                        if( pkmn->m_level >= 20 && pkmn->m_stats.m_curHP * 4 > pkmn->m_stats.m_maxHP
                            && !pkmn->getForme( ) ) {
                            p_ui->logAbility( getPkmnOrDisguise( i, j ), i );
                            pkmn->battleTransform( );
                            p_ui->updatePkmn( i, j, getPkmnOrDisguise( i, j ) );
                        }
                        break;
                    }

                    default: break;
                    }
                }
            }
        }

        constexpr u8 TMP_BUFFER_SIZE = 100;
        char         buffer[ TMP_BUFFER_SIZE + 10 ];

        // TODO: add log messages for volatile status?

        // volatile status
        for( u8 i = 0; i < 2; ++i ) {
            for( u8 j = 0; j < getBattlingPKMNCount( _mode ); ++j ) {
                auto pkmn = getPkmn( i, j );
                if( pkmn == nullptr || supprA ) { continue; }

                auto volst = getVolatileStatus( i, j );
                if( volst & VS_NIGHTMARE ) {
                    auto fmt = std::string( GET_STRING( IO::STR_UI_BATTLE_HARMED_BY_NIGHTMARE ) );
                    if( hasStatusCondition( i, j, SLEEP ) ) {
                        snprintf( buffer, TMP_BUFFER_SIZE, fmt.c_str( ),
                                  p_ui->getPkmnName( getPkmnOrDisguise( i, j ), i ).c_str( ) );
                        p_ui->log( buffer );
                        u16 amount = pkmn->m_stats.m_maxHP / 4;
                        if( !amount ) { amount = 1; }
                        damagePokemon( p_ui, i, j, amount );
                    } else {
                        _sides[ i ].removeVolatileStatus( p_ui, j, VS_NIGHTMARE );
                    }
                }

                if( volst & VS_YAWN ) {
                    if( getVolatileStatusCounter( i, j, VS_YAWN ) == 1 ) {
                        if( setStatusCondition( i, j, SLEEP, 4 + ( rand( ) & 3 ) ) ) {
                            p_ui->animateGetStatusCondition( getPkmnOrDisguise( i, j ), i, j,
                                                             SLEEP );
                            p_ui->updatePkmnStats( i, j, getPkmnOrDisguise( i, j ), true );
                        }

                        _sides[ i ].removeVolatileStatus( p_ui, j, VS_YAWN );
                    }
                }
                if( volst & VS_AQUARING ) {
                    if( !( volst & VS_HEALBLOCK ) ) {
                        auto fmt
                            = std::string( GET_STRING( IO::STR_UI_BATTLE_HEALED_BY_AQUA_RING ) );
                        snprintf( buffer, TMP_BUFFER_SIZE, fmt.c_str( ),
                                  p_ui->getPkmnName( getPkmnOrDisguise( i, j ), i ).c_str( ) );
                        p_ui->log( buffer );
                        u16 amount = pkmn->m_stats.m_maxHP / 16;
                        if( !amount ) { amount = 1; }
                        if( canUseItem( i, j ) && pkmn->getItem( ) == I_BIG_ROOT ) {
                            amount = ( amount * 13 ) / 10;
                        }
                        healPokemon( p_ui, i, j, amount );
                    }
                }
                if( volst & VS_INGRAIN ) {
                    if( !( volst & VS_HEALBLOCK ) ) {
                        auto fmt = std::string( GET_STRING( IO::STR_UI_BATTLE_HEALED_BY_INGRAIN ) );
                        snprintf( buffer, TMP_BUFFER_SIZE, fmt.c_str( ),
                                  p_ui->getPkmnName( getPkmnOrDisguise( i, j ), i ).c_str( ) );
                        p_ui->log( buffer );
                        u16 amount = pkmn->m_stats.m_maxHP / 16;
                        if( !amount ) { amount = 1; }
                        if( canUseItem( i, j ) && pkmn->getItem( ) == I_BIG_ROOT ) {
                            amount = ( amount * 13 ) / 10;
                        }
                        healPokemon( p_ui, i, j, amount );
                    }
                }
                if( volst & VS_CURSE ) {
#ifdef DESQUID_MORE
                    p_ui->log( std::to_string( volst ) );
#endif
                    auto fmt = std::string( GET_STRING( IO::STR_UI_BATTLE_HARMED_BY_CURSE ) );
                    snprintf( buffer, TMP_BUFFER_SIZE, fmt.c_str( ),
                              p_ui->getPkmnName( getPkmnOrDisguise( i, j ), i ).c_str( ) );
                    p_ui->log( buffer );
                    u16 amount = pkmn->m_stats.m_maxHP / 4;
                    if( !amount ) { amount = 1; }
                    damagePokemon( p_ui, i, j, amount );
                }

                if( !getPkmn( i, j )->canBattle( ) ) {
                    // pkmn fainted
                    faintPokemon( p_ui, i, j );
                }
            }
        }

        // status conditions

        for( u8 i = 0; i < 2; ++i ) {
            for( u8 j = 0; j < getBattlingPKMNCount( _mode ); ++j ) {
                auto pkmn = getPkmn( i, j );
                if( pkmn == nullptr ) { continue; }

                auto volst = getVolatileStatus( i, j );

                if( hasStatusCondition( i, j, SLEEP ) && !supprA ) {
                    for( u8 i2 = 0; i2 < 2; ++i2 ) {
                        for( u8 j2 = 0; j2 < getBattlingPKMNCount( _mode ); ++j2 ) {
                            if( i2 == i && j2 == j ) { continue; }
                            auto p2 = getPkmn( i2, j2 );
                            if( p2 == nullptr ) { continue; }

                            if( p2->getAbility( ) == A_BAD_DREAMS ) {
                                p_ui->logAbility( getPkmnOrDisguise( i2, j2 ), i2 );
                                damagePokemon( p_ui, i, j, pkmn->m_stats.m_maxHP / 8 );
                            }
                        }
                    }
                }

                if( hasStatusCondition( i, j, POISON ) ) {
                    p_ui->animateStatusCondition( getPkmnOrDisguise( i, j ), i, j, POISON );
                    if( supprA || pkmn->getAbility( ) != A_POISON_HEAL ) {
                        damagePokemon( p_ui, i, j, pkmn->m_stats.m_maxHP / 8 );
                    } else if( !( volst & VS_HEALBLOCK ) ) {
                        p_ui->logAbility( getPkmnOrDisguise( i, j ), i );
                        healPokemon( p_ui, i, j, pkmn->m_stats.m_maxHP / 8 );
                    }
                }
                if( hasStatusCondition( i, j, TOXIC ) ) {
                    p_ui->animateStatusCondition( getPkmnOrDisguise( i, j ), i, j, TOXIC );
                    u8 turns = getAndIncreaseToxicCount( i, j );
                    if( turns > 15 ) { turns = 15; }
                    if( supprA || pkmn->getAbility( ) != A_POISON_HEAL ) {
                        damagePokemon( p_ui, i, j, pkmn->m_stats.m_maxHP * turns / 16 );
                    } else if( !( volst & VS_HEALBLOCK ) ) {
                        p_ui->logAbility( getPkmnOrDisguise( i, j ), i );
                        healPokemon( p_ui, i, j, pkmn->m_stats.m_maxHP / 8 );
                    }
                }
                if( hasStatusCondition( i, j, BURN ) ) {
                    p_ui->animateStatusCondition( getPkmnOrDisguise( i, j ), i, j, BURN );
                    damagePokemon( p_ui, i, j, pkmn->m_stats.m_maxHP / 8 );
                }

                if( !getPkmn( i, j )->canBattle( ) ) {
                    // pkmn fainted
                    faintPokemon( p_ui, i, j );
                }
            }
        }

        // items TODO: add messages
        for( u8 i = 0; i < 2; ++i ) {
            for( u8 j = 0; j < getBattlingPKMNCount( _mode ); ++j ) {
                auto pkmn = getPkmn( i, j );
                if( pkmn == nullptr ) { continue; }
                if( !canUseItem( i, j ) ) { continue; }
                auto volst = getVolatileStatus( i, j );

                if( pkmn->getItem( ) == I_LEFTOVERS && !( volst & VS_HEALBLOCK ) ) {
                    healPokemon( p_ui, i, j, pkmn->m_stats.m_maxHP / 16 );
                }
                if( pkmn->getItem( ) == I_BLACK_SLUDGE ) {
                    if( hasType( i, j, TYPE_POISON ) ) {
                        if( !( volst & VS_HEALBLOCK ) ) {
                            healPokemon( p_ui, i, j, pkmn->m_stats.m_maxHP / 16 );
                        }
                    } else {
                        if( suppressesAbilities( ) || pkmn->getAbility( ) != A_MAGIC_GUARD ) {
                            damagePokemon( p_ui, i, j, pkmn->m_stats.m_maxHP / 8 );
                        }
                    }
                }
                if( pkmn->getItem( ) == I_STICKY_BARB ) {
                    if( suppressesAbilities( ) || pkmn->getAbility( ) != A_MAGIC_GUARD ) {
                        damagePokemon( p_ui, i, j, pkmn->m_stats.m_maxHP / 8 );
                    }
                }
                if( pkmn->getItem( ) == I_FLAME_ORB ) {
                    if( setStatusCondition( i, j, BURN ) ) {
                        p_ui->animateGetStatusCondition( getPkmnOrDisguise( i, j ), i, j, BURN );
                        p_ui->updatePkmnStats( i, j, getPkmnOrDisguise( i, j ), true );
                    }
                }
                if( pkmn->getItem( ) == I_TOXIC_ORB ) {
                    if( setStatusCondition( i, j, TOXIC ) ) {
                        p_ui->animateGetStatusCondition( getPkmnOrDisguise( i, j ), i, j, TOXIC );
                        p_ui->updatePkmnStats( i, j, getPkmnOrDisguise( i, j ), true );
                    }
                }
                if( !getPkmn( i, j )->canBattle( ) ) {
                    // pkmn fainted
                    faintPokemon( p_ui, i, j );
                }
            }
        }
    }

    bool field::setWeather( battleUI* p_ui, weather p_newWeather, bool p_extended ) {
        if( p_newWeather == _weather ) {
            p_ui->log( GET_STRING( IO::STR_UI_BATTLE_IT_FAILED ) );
            return false;
        }

        if( _weather == WE_HEAVY_RAIN || _weather == WE_HEAVY_SUNSHINE
            || _weather == WE_HEAVY_WINDS ) [[unlikely]] {
            // weather can be replaced only with a similar weather
            if( p_newWeather != WE_HEAVY_RAIN && p_newWeather != WE_HEAVY_SUNSHINE
                && p_newWeather != WE_HEAVY_WINDS ) {
                p_ui->log( GET_STRING( IO::STR_UI_BATTLE_IT_FAILED ) );
                return false;
            }
        }

        _weather      = p_newWeather;
        _weatherTimer = p_extended ? EXTENDED_DURATION : NORMAL_DURATION;

        p_ui->setNewWeather( p_newWeather );

        // Check for castform

        if( !suppressesAbilities( ) ) {
            for( u8 i = 0; i < 2; ++i ) {
                for( u8 j = 0; j < getBattlingPKMNCount( _mode ); ++j ) {
                    auto pkmn = getPkmn( i, j );
                    if( pkmn == nullptr ) { continue; }

                    if( pkmn->getSpecies( ) == PKMN_CASTFORM
                        && pkmn->getAbility( ) == A_FORECAST ) {
                        if( !suppressesWeather( ) ) {
                            if( getWeather( ) == WE_RAIN || getWeather( ) == WE_HEAVY_RAIN ) {
                                pkmn->setBattleForme( 2 );
                                p_ui->updatePkmn( i, j, pkmn );
                                continue;
                            }
                            if( getWeather( ) == WE_SUN || getWeather( ) == WE_HEAVY_SUNSHINE ) {
                                pkmn->setBattleForme( 1 );
                                p_ui->updatePkmn( i, j, pkmn );
                                continue;
                            }
                            if( getWeather( ) == WE_HAIL ) {
                                pkmn->setBattleForme( 3 );
                                p_ui->updatePkmn( i, j, pkmn );
                                continue;
                            }
                        }
                        pkmn->setBattleForme( 0 );
                        p_ui->updatePkmn( i, j, pkmn );
                    }

                    if( pkmn->getSpecies( ) == PKMN_CHERRIM
                        && pkmn->getAbility( ) == A_FLOWER_GIFT ) {
                        if( !suppressesWeather( ) ) {
                            if( getWeather( ) == WE_SUN || getWeather( ) == WE_HEAVY_SUNSHINE ) {
                                pkmn->setBattleForme( 1 );
                                p_ui->updatePkmn( i, j, pkmn );
                                continue;
                            }
                        }
                        pkmn->setBattleForme( 0 );
                        p_ui->updatePkmn( i, j, pkmn );
                    }
                }
            }
        }

        return true;
    }

    void field::continueWeather( battleUI* p_ui ) {
        p_ui->continueWeather( );

        auto weather = getWeather( );
        if( weather == WE_NONE || suppressesWeather( ) ) {
            // nothing to do
            return;
        }

        bool supprA = suppressesAbilities( );

        for( u8 i = 0; i < 2; ++i ) {
            for( u8 j = 0; j < getBattlingPKMNCount( _mode ); ++j ) {
                auto pkmn = getPkmn( i, j );
                if( pkmn == nullptr ) { continue; }
                auto volst = getVolatileStatus( i, j );

                if( weather == WE_RAIN || weather == WE_HEAVY_RAIN ) {
                    if( !supprA && pkmn->getAbility( ) == A_HYDRATION ) {
                        if( pkmn->m_statusint ) {
                            if( removeStatusCondition( i, j ) ) {
                                p_ui->logAbility( getPkmnOrDisguise( i, j ), i );
                                p_ui->updatePkmnStats( i, j, getPkmnOrDisguise( i, j ), true );
                            }
                        }
                    }
                    if( !supprA && pkmn->getAbility( ) == A_DRY_SKIN
                        && !( volst & VS_HEALBLOCK ) ) {
                        p_ui->logAbility( getPkmnOrDisguise( i, j ), i );
                        healPokemon( p_ui, i, j, pkmn->m_stats.m_maxHP / 8 );
                    }
                    if( !supprA && pkmn->getAbility( ) == A_RAIN_DISH
                        && !( volst & VS_HEALBLOCK ) ) {
                        p_ui->logAbility( getPkmnOrDisguise( i, j ), i );
                        healPokemon( p_ui, i, j, pkmn->m_stats.m_maxHP / 16 );
                    }
                }
                if( weather == WE_SUN || weather == WE_HEAVY_SUNSHINE ) {
                    if( !supprA
                        && ( pkmn->getAbility( ) == A_DRY_SKIN
                             || pkmn->getAbility( ) == A_SOLAR_POWER ) ) {
                        p_ui->logAbility( getPkmnOrDisguise( i, j ), i );
                        damagePokemon( p_ui, i, j, pkmn->m_stats.m_maxHP / 8 );
                    }
                }
                if( weather == WE_HAIL ) {
                    if( !supprA && pkmn->getAbility( ) == A_ICE_BODY ) {
                        if( !( volst & VS_HEALBLOCK ) ) {
                            p_ui->logAbility( getPkmnOrDisguise( i, j ), i );
                            healPokemon( p_ui, i, j, pkmn->m_stats.m_maxHP / 16 );
                        }
                    } else if( !supprA
                               && ( pkmn->getAbility( ) == A_SNOW_CLOAK
                                    || pkmn->getAbility( ) == A_MAGIC_GUARD
                                    || pkmn->getAbility( ) == A_OVERCOAT ) ) {
                    } else if( canUseItem( i, j ) && pkmn->getItem( ) == I_SAFETY_GOGGLES ) {
                    } else if( hasType( i, j, TYPE_ICE ) ) {
                    } else {
                        damagePokemon( p_ui, i, j, pkmn->m_stats.m_maxHP / 16 );
                    }
                }

                if( weather == WE_SANDSTORM ) {
                    if( hasType( i, j, TYPE_GROUND ) || hasType( i, j, TYPE_ROCK )
                        || hasType( i, j, TYPE_STEEL ) ) {
                    } else if( !supprA
                               && ( pkmn->getAbility( ) == A_SAND_FORCE
                                    || pkmn->getAbility( ) == A_SAND_RUSH
                                    || pkmn->getAbility( ) == A_SAND_VEIL
                                    || pkmn->getAbility( ) == A_MAGIC_GUARD
                                    || pkmn->getAbility( ) == A_OVERCOAT ) ) {
                    } else if( canUseItem( i, j ) && pkmn->getItem( ) == I_SAFETY_GOGGLES ) {
                    } else {
                        damagePokemon( p_ui, i, j, pkmn->m_stats.m_maxHP / 16 );
                    }
                }

                if( !getPkmn( i, j )->canBattle( ) ) {
                    // pkmn fainted
                    faintPokemon( p_ui, i, j );
                }
            }
        }
    }

    bool field::removeWeather( battleUI* p_ui ) {
        if( _weather != WE_NONE ) {
            p_ui->setNewWeather( WE_NONE );
        } else {
            return false;
        }
        _weather      = WE_NONE;
        _weatherTimer = 0;
        return true;
    }

    bool field::setPseudoWeather( battleUI* p_ui, pseudoWeather p_newPseudoWeather,
                                  bool p_extended ) {
        bool change = false;
        for( u8 i = 0; i < MAX_PSEUDO_WEATHER; ++i ) {
            if( ( p_newPseudoWeather & ( 1LLU << i ) ) && _pseudoWeatherTimer[ i ]
                && ( ( 1LLU << i ) == PW_MAGICROOM || ( 1LLU << i ) == PW_WONDERROOM
                     || ( 1LLU << i ) == PW_TRICKROOM ) ) {
                removePseudoWeather( p_ui, pseudoWeather( 1LLU << i ) );
                change = true;
                continue;
            }
            if( ( p_newPseudoWeather & ( 1LLU << i ) ) && !_pseudoWeatherTimer[ i ] ) {
                change                   = true;
                _pseudoWeatherTimer[ i ] = p_extended ? EXTENDED_DURATION : NORMAL_DURATION;
                if( ( 1LLU << i ) == PW_FAIRYLOCK ) { _pseudoWeatherTimer[ i ] = 2; }
                p_ui->addPseudoWeather( i );
            }
        }
        if( !change ) { p_ui->log( GET_STRING( IO::STR_UI_BATTLE_IT_FAILED ) ); }
        return change;
    }

    bool field::removePseudoWeather( battleUI* p_ui, pseudoWeather p_pseudoWeather ) {
        bool change = false;
        for( u8 i = 0; i < MAX_PSEUDO_WEATHER; ++i ) {
            if( ( p_pseudoWeather & ( 1LLU << i ) ) && _pseudoWeatherTimer[ i ] ) {
                change                   = true;
                _pseudoWeatherTimer[ i ] = 0;
                p_ui->removePseudoWeather( i );
            }
        }
        return change;
    }

    bool field::setTerrain( battleUI* p_ui, terrain p_newTerrain, bool p_extended ) {
        if( _terrain == p_newTerrain ) {
            p_ui->log( GET_STRING( IO::STR_UI_BATTLE_TERRAIN_NO_CHANGE ) );
            return false;
        }
        p_ui->setNewTerrain( p_newTerrain );
        _terrain      = p_newTerrain;
        _terrainTimer = p_extended ? EXTENDED_DURATION : NORMAL_DURATION;
        return true;
    }

    bool field::removeTerrain( battleUI* p_ui ) {
        if( _terrain == TR_NONE ) { return false; }
        p_ui->setNewTerrain( _terrain = TR_NONE );
        _terrainTimer = 0;
        return true;
    }

    void field::megaEvolve( battleUI* p_ui, bool p_opponent, u8 p_slot ) {
        auto pkmn = getPkmn( p_opponent, p_slot );
        if( pkmn == nullptr ) [[unlikely]] { return; }

        constexpr u8 TMP_BUFFER_SIZE = 100;
        char         buffer[ TMP_BUFFER_SIZE + 10 ];
        auto         fmt = std::string( GET_STRING( IO::STR_UI_BATTLE_MEGA_EVOLVE_WISH ) );
        snprintf( buffer, TMP_BUFFER_SIZE, fmt.c_str( ),
                  p_ui->getPkmnName( getPkmnOrDisguise( p_opponent, p_slot ), p_opponent ).c_str( ),
                  FS::getItemName( pkmn->getItem( ) ).c_str( ) );
        p_ui->log( std::string( buffer ) );

        pkmn->battleTransform( );
        pkmn = getPkmn( p_opponent, p_slot );
        if( pkmn == nullptr ) [[unlikely]] { return; }
        p_ui->updatePkmn( p_opponent, p_slot, getPkmnOrDisguise( p_opponent, p_slot ) );

        fmt = std::string( GET_STRING( IO::STR_UI_BATTLE_MEGA_EVOLVE_WISH_GRANTED ) );
        snprintf( buffer, TMP_BUFFER_SIZE, fmt.c_str( ),
                  p_ui->getPkmnName( getPkmnOrDisguise( p_opponent, p_slot ), p_opponent ).c_str( ),
                  pkmn->getItem( ) );
        p_ui->log( std::string( buffer ) );

        checkOnSendOut( p_ui, p_opponent, p_slot );
    }

    std::vector<battleMove>
    field::computeSortedBattleMoves( battleUI*                               p_ui,
                                     const std::vector<battleMoveSelection>& p_selectedMoves ) {
        std::vector<battleMove> res = std::vector<battleMove>( );

        // randomly pertube scores slightly to break ties.
        u8 pertub[ 4 ] = { 0, 1, 2, 3 };
        for( u8 i = 0; i < 4; ++i ) { std::swap( pertub[ i ], pertub[ rand( ) % 4 ] ); }

        for( u8 j = 0; j < p_selectedMoves.size( ); ++j ) {
            battleMove bm;
            auto       m = p_selectedMoves[ j ];

            bm.m_param       = m.m_param;
            bm.m_user        = m.m_user;
            bm.m_target      = std::vector<fieldPosition>( );
            bm.m_pertubation = pertub[ j ];
            bm.m_moveData    = m.m_moveData;
            bm.m_megaEvolve  = m.m_megaEvolve;

            // compute targets, priority
            switch( m.m_type ) {
            case MT_ATTACK:
            case MT_SWITCH_PURSUIT: {
                if( m.m_type == MT_SWITCH_PURSUIT ) {
                    // Double the power of pursuit, ensure it happens before the switch
                    bm.m_priority = 123;
                    bm.m_moveData.m_basePower *= 2;
                } else {
                    bm.m_priority = 3 * m.m_moveData.m_priority;
                    bool haspd    = false;

                    // Check for priority changing abilities
                    if( !suppressesAbilities( ) ) [[likely]] {
                        // Gale wings
                        if( m.m_moveData.m_type == TYPE_FLYING
                            && getPkmn( m.m_user.first, m.m_user.second )->getStat( HP )
                                   == getPkmn( m.m_user.first, m.m_user.second )->m_stats.m_maxHP
                            && getPkmn( m.m_user.first, m.m_user.second )->getAbility( )
                                   == A_GALE_WINGS ) [[unlikely]] {
                            bm.m_priority += 3;
                        }

                        // Prankster
                        if( m.m_moveData.m_category == MH_STATUS
                            && getPkmn( m.m_user.first, m.m_user.second )->getAbility( )
                                   == A_PRANKSTER ) [[unlikely]] {
                            bm.m_priority += 3;
                        }

                        // Triage
                        if( m.m_moveData.m_heal
                            && getPkmn( m.m_user.first, m.m_user.second )->getAbility( )
                                   == A_TRIAGE ) [[unlikely]] {
                            bm.m_priority += 9;
                        }

                        // Stall
                        if( getPkmn( m.m_user.first, m.m_user.second )->getAbility( ) == A_STALL ) {
                            bm.m_priority--;
                            haspd = true;
                        }
                    }

                    // Items

                    if( canUseItem( m.m_user.first, m.m_user.second ) ) {
                        if( !haspd
                            && getPkmn( m.m_user.first, m.m_user.second )->getItem( )
                                   == I_LAGGING_TAIL ) [[unlikely]] {
                            bm.m_priority--;
                        }

                        if( !haspd
                            && getPkmn( m.m_user.first, m.m_user.second )->getItem( )
                                   == I_FULL_INCENSE ) [[unlikely]] {
                            bm.m_priority--;
                        }

                        if( getPkmn( m.m_user.first, m.m_user.second )->getItem( ) == I_QUICK_CLAW )
                            [[unlikely]] {
                            if( rand( ) % 100 < 20 ) {
                                bm.m_priority++;
                                res.push_back( { MT_MESSAGE_ITEM,
                                                 I_QUICK_CLAW,
                                                 { },
                                                 { 255, 255 },
                                                 127,
                                                 0,
                                                 u8( 5 + j ),
                                                 moveData( ),
                                                 false } );
                            }
                        }

                        if( suppressesAbilities( )
                            || !_sides[ m.m_user.first ? PLAYER_SIDE : OPPONENT_SIDE ]
                                    .anyHasAbility( A_UNNERVE ) ) {
                            if( getPkmn( m.m_user.first, m.m_user.second )->getItem( )
                                    == I_CUSTAP_BERRY
                                && getPkmn( m.m_user.first, m.m_user.second )->getStat( HP ) * 4
                                       < getPkmn( m.m_user.first, m.m_user.second )
                                             ->m_stats.m_maxHP ) [[unlikely]] {
                                p_ui->logItem( getPkmnOrDisguise( m.m_user.first, m.m_user.second ),
                                               m.m_user.first );
                                bm.m_priority++;
                                res.push_back( { MT_MESSAGE_ITEM,
                                                 I_CUSTAP_BERRY,
                                                 { },
                                                 { m.m_user.first, m.m_user.second },
                                                 127,
                                                 0,
                                                 u8( 5 + j ),
                                                 moveData( ),
                                                 false } );
                                removeItem( p_ui, m.m_user.first, m.m_user.second );
                                checkOnEatBerry( p_ui, m.m_user.first, m.m_user.second,
                                                 I_CUSTAP_BERRY );
                            }
                        }
                    }

                    // moves w/ changing message

                    if( bm.m_param == M_FOCUS_PUNCH || bm.m_param == M_BEAK_BLAST
                        || bm.m_param == M_SHELL_TRAP ) {
                        res.push_back( { MT_MESSAGE_MOVE,
                                         bm.m_param,
                                         { },
                                         { m.m_user.first, m.m_user.second },
                                         125,
                                         0,
                                         u8( 5 + j ),
                                         moveData( ),
                                         false } );
                    }
                }
                bm.m_type = MT_ATTACK;

                auto tg = ( m.m_moveData.m_pressureTarget != TG_NONE )
                              ? m.m_moveData.m_pressureTarget
                              : m.m_moveData.m_target;
                switch( tg ) {
                case TG_ANY:
                case TG_RANDOM:
                case TG_ALLY_OR_SELF:
                case TG_ANY_FOE:
                    // target is already computed
                    bm.m_target.push_back( m.m_target );
                    break;
                case TG_ALL_FOES_AND_ALLY:
                    if( getPkmn( m.m_user.first, !m.m_user.second ) ) {
                        bm.m_target.push_back( fieldPosition( m.m_user.first, !m.m_user.second ) );
                    }
                    [[fallthrough]];
                case TG_ALL_FOES:
                    for( u8 i = 0; i < getBattlingPKMNCount( _mode ); ++i ) {
                        if( getPkmn( !m.m_user.first, i ) != nullptr ) {
                            bm.m_target.push_back( fieldPosition( !m.m_user.first, i ) );
                        }
                    }
                    break;
                case TG_ALL_ALLIES:
                    for( u8 i = 0; i < getBattlingPKMNCount( _mode ); ++i ) {
                        if( getPkmn( m.m_user.first, i ) != nullptr ) {
                            bm.m_target.push_back( fieldPosition( m.m_user.first, i ) );
                        }
                    }
                    break;
                case TG_SELF:
                    bm.m_target.push_back( fieldPosition( m.m_user.first, m.m_user.second ) );
                    break;
                case TG_ALLY:
                    bm.m_target.push_back( fieldPosition( m.m_user.first, !m.m_user.second ) );
                    break;
                case TG_FIELD:
                case TG_SCRIPTED:
                case TG_ALLY_SIDE:
                case TG_ALLY_TEAM:
                    bm.m_target.push_back( fieldPosition( m.m_user.first, m.m_user.second ) );
                    break;
                case TG_FOE_SIDE:
                    bm.m_target.push_back( fieldPosition( m.m_user.first, m.m_user.second ) );
                    break;
                default: break;
                }
                break;
            }
            case MT_SWITCH:
            case MT_USE_ITEM:
                bm.m_type = m.m_type;
                bm.m_target.push_back( m.m_user );
                bm.m_priority = 120;
                break;
            default:
                bm.m_type = MT_NO_OP;
                bm.m_target.push_back( m.m_user );
                bm.m_priority = -120;
                break;
            }

            if( bm.m_priority > -100 ) [[likely]] {
                if( getPseudoWeather( ) & PW_TRICKROOM ) {
                    bm.m_userSpeed = -getStat( m.m_user.first, m.m_user.second, SPEED );
                } else {
                    bm.m_userSpeed = getStat( m.m_user.first, m.m_user.second, SPEED );
                }
            } else {
                bm.m_userSpeed = 0;
            }
            res.push_back( bm );
        }

        std::sort( res.begin( ), res.end( ) );

        return res;
    }

    void field::confusionSelfDamage( battleUI* p_ui, bool p_opponent, u8 p_slot ) {
        auto pkmn = getPkmn( p_opponent, p_slot );
        if( pkmn == nullptr ) { return; }

        p_ui->animateHitPkmn( p_opponent, p_slot, 100 );
        // TODO: proper calculation(?)
        damagePokemon( p_ui, p_opponent, p_slot, pkmn->m_stats.m_maxHP / 16 );
    }

    void field::executeSelfStatusEffects( battleUI* p_ui, battleMove p_move,
                                          fieldPosition p_target ) {
        (void) p_target;
        auto user = getPkmn( p_move.m_user.first, p_move.m_user.second );
        if( user == nullptr ) [[unlikely]] { return; }

        if( p_move.m_param == M_CURSE ) {
            if( !hasType( p_move.m_user.first, p_move.m_user.second, TYPE_GHOST ) ) {
                boosts bs = boosts( );
                bs.setBoost( SPEED, -1 );
                bs.setBoost( ATK, 1 );
                bs.setBoost( DEF, 1 );
                auto res = addBoosts( p_move.m_user.first, p_move.m_user.second, bs );
                p_ui->logBoosts( getPkmnOrDisguise( p_move.m_user.first, p_move.m_user.second ),
                                 p_move.m_user.first, p_move.m_user.second, bs, res );
                return;
            }
        }

        // Boosts
        if( p_move.m_moveData.m_selfBoosts != boosts( ) ) {
            auto res = addBoosts( p_move.m_user.first, p_move.m_user.second,
                                  p_move.m_moveData.m_selfBoosts );
            p_ui->logBoosts( getPkmnOrDisguise( p_move.m_user.first, p_move.m_user.second ),
                             p_move.m_user.first, p_move.m_user.second,
                             p_move.m_moveData.m_selfBoosts, res );
        }
    }

    void field::executeStatusEffects( battleUI* p_ui, battleMove p_move, fieldPosition p_target ) {
        auto user   = getPkmn( p_move.m_user.first, p_move.m_user.second );
        auto target = getPkmn( p_target.first, p_target.second );
        if( p_move.m_moveData.m_target == TG_SELF || p_move.m_moveData.m_target == TG_FIELD
            || p_move.m_moveData.m_target == TG_FOE_SIDE
            || p_move.m_moveData.m_target == TG_ALLY_SIDE ) {
            target   = user;
            p_target = p_move.m_user;
        }
        if( user == nullptr || target == nullptr ) [[unlikely]] { return; }

        if( p_move.m_param == M_CURSE ) {
            if( hasType( p_move.m_user.first, p_move.m_user.second, TYPE_GHOST ) ) {
                damagePokemon( p_ui, p_move.m_user.first, p_move.m_user.second,
                               user->m_stats.m_maxHP / 2 );
            } else {
                return;
            }
        }

        if( p_move.m_param == M_TRANSFORM ) {
            if( transformPkmn( p_move.m_user.first, p_move.m_user.second,
                               getSlot( p_target.first, p_target.second ) ) ) {
                p_ui->updatePkmn( p_move.m_user.first, p_move.m_user.second,
                                  getPkmnOrDisguise( p_move.m_user.first, p_move.m_user.second ) );
                if( getPkmn( p_move.m_user.first, p_move.m_user.second )->getAbility( )
                    != A_IMPOSTER ) {
                    checkOnSendOut( p_ui, p_move.m_user.first, p_move.m_user.second );
                }
            } else {
                p_ui->log( GET_STRING( IO::STR_UI_BATTLE_IT_FAILED ) );
            }
            return;
        }

        if( p_move.m_param == M_REST ) {
            if( getVolatileStatus( p_target.first, p_target.second ) & VS_HEALBLOCK ) {
                p_ui->log( GET_STRING( IO::STR_UI_BATTLE_IT_FAILED ) );
            } else {
                user->m_status.m_isAsleep = 3;
                p_ui->animateGetStatusCondition(
                    getPkmnOrDisguise( p_target.first, p_target.second ), p_target.first,
                    p_target.second, p_move.m_moveData.m_status );
                p_ui->updatePkmnStats( p_target.first, p_target.second,
                                       getPkmnOrDisguise( p_target.first, p_target.second ) );
                healPokemon( p_ui, p_move.m_user.first, p_move.m_user.second,
                             user->m_stats.m_maxHP );
            }
            return;
        }

        // Boosts
        if( p_move.m_moveData.m_boosts != boosts( ) ) {
            auto res = addBoosts( p_target.first, p_target.second, p_move.m_moveData.m_boosts );
            p_ui->logBoosts( getPkmnOrDisguise( p_target.first, p_target.second ), p_target.first,
                             p_target.second, p_move.m_moveData.m_boosts, res );
        }

        // Heal
        if( p_move.m_moveData.m_heal ) {
            if( getVolatileStatus( p_target.first, p_target.second ) & VS_HEALBLOCK ) {
                p_ui->log( GET_STRING( IO::STR_UI_BATTLE_IT_FAILED ) );
            } else {
                u16 amount = target->m_stats.m_maxHP * p_move.m_moveData.m_heal / 240;
                if( !amount ) { amount = 1; }
                healPokemon( p_ui, p_target.first, p_target.second, amount );
            }
        }
        if( p_move.m_param == M_SYNTHESIS || p_move.m_param == M_MOONLIGHT
            || p_move.m_param == M_MORNING_SUN ) {
            if( getVolatileStatus( p_target.first, p_target.second ) & VS_HEALBLOCK ) {
                p_ui->log( GET_STRING( IO::STR_UI_BATTLE_IT_FAILED ) );
            } else {
                u16 amount = 1;
                switch( getWeather( ) ) {
                case WE_SUN:
                case WE_HEAVY_SUNSHINE: amount = target->m_stats.m_maxHP * 2 / 3; break;
                case WE_NONE:
                case WE_HEAVY_WINDS: amount = target->m_stats.m_maxHP / 2; break;
                default: amount = target->m_stats.m_maxHP / 4; break;
                }
                if( !amount ) { amount = 1; }
                healPokemon( p_ui, p_target.first, p_target.second, amount );
            }
        }
        if( p_move.m_param == M_SHORE_UP ) {
            if( getVolatileStatus( p_target.first, p_target.second ) & VS_HEALBLOCK ) {
                p_ui->log( GET_STRING( IO::STR_UI_BATTLE_IT_FAILED ) );
            } else {
                u16 amount = 1;
                switch( getWeather( ) ) {
                case WE_SANDSTORM: amount = target->m_stats.m_maxHP * 2 / 3; break;
                default: amount = target->m_stats.m_maxHP / 2; break;
                }
                if( !amount ) { amount = 1; }
                healPokemon( p_ui, p_target.first, p_target.second, amount );
            }
        }
        if( p_move.m_param == M_PURIFY ) {
            if( removeVolatileStatus( p_ui, p_target.first, p_target.second,
                                      volatileStatus( u64( -1 ) ) ) ) {
                if( !( getVolatileStatus( p_move.m_user.first, p_move.m_user.second )
                       & VS_HEALBLOCK ) ) {
                    u16 amount = user->m_stats.m_maxHP / 2;
                    if( !amount ) { amount = 1; }
                    healPokemon( p_ui, p_move.m_user.first, p_move.m_user.second, amount );
                }
            } else {
                p_ui->log( GET_STRING( IO::STR_UI_BATTLE_IT_FAILED ) );
            }
        }
        if( p_move.m_param == M_STRENGTH_SAP ) {
            u16 amount = getStat( p_target.first, p_target.second, ATK );
            if( canUseItem( p_move.m_user.first, p_move.m_user.second )
                && user->getItem( ) == I_BIG_ROOT ) {
                amount = amount * 13 / 10;
            }
            if( !amount ) { amount = 1; }
            if( !suppressesAbilities( ) && target->getAbility( ) == A_LIQUID_OOZE ) {
                p_ui->logAbility( getPkmnOrDisguise( p_target.first, p_target.second ),
                                  p_target.first );
                damagePokemon( p_ui, p_move.m_user.first, p_move.m_user.second, amount );
            } else if( !( getVolatileStatus( p_move.m_user.first, p_move.m_user.second )
                          & VS_HEALBLOCK ) ) {
                healPokemon( p_ui, p_move.m_user.first, p_move.m_user.second, amount );
            }
        }

        // Check for status conditions
        if( p_move.m_moveData.m_status ) {
            bool corr = false;
            if( !suppressesAbilities( ) && user->getAbility( ) == A_CORROSION ) {
                corr = ( p_move.m_moveData.m_secondaryStatus == POISON );
            }

            if( setStatusCondition( p_target.first, p_target.second, p_move.m_moveData.m_status,
                                    p_move.m_moveData.m_status == SLEEP ? ( 4 + ( rand( ) & 3 ) )
                                                                        : ( corr ? 254 : 255 ) ) ) {
                p_ui->animateGetStatusCondition(
                    getPkmnOrDisguise( p_target.first, p_target.second ), p_target.first,
                    p_target.second, p_move.m_moveData.m_status );
                p_ui->updatePkmnStats( p_target.first, p_target.second,
                                       getPkmnOrDisguise( p_target.first, p_target.second ), true );
            } else {
                p_ui->log( GET_STRING( IO::STR_UI_BATTLE_IT_FAILED ) );
            }
        }

        if( p_move.m_moveData.m_sideCondition ) {
            if( p_move.m_moveData.m_target == TG_FOE_SIDE ) {
                addSideCondition( p_ui, !p_move.m_user.first, p_move.m_moveData.m_sideCondition );
            } else {
                addSideCondition( p_ui, p_move.m_user.first, p_move.m_moveData.m_sideCondition );
            }
        }

        if( p_move.m_moveData.m_weather ) {
            bool extended = false;
            if( canUseItem( p_move.m_user.first, p_move.m_user.second ) ) {
                if( p_move.m_moveData.m_weather == WE_RAIN && user->getItem( ) == I_DAMP_ROCK ) {
                    extended = true;
                }
                if( p_move.m_moveData.m_weather == WE_SUN && user->getItem( ) == I_HEAT_ROCK ) {
                    extended = true;
                }
                if( p_move.m_moveData.m_weather == WE_SANDSTORM
                    && user->getItem( ) == I_SMOOTH_ROCK ) {
                    extended = true;
                }
                if( p_move.m_moveData.m_weather == WE_HAIL && user->getItem( ) == I_ICY_ROCK ) {
                    extended = true;
                }
            }
            setWeather( p_ui, p_move.m_moveData.m_weather, extended );
        }
        if( p_move.m_moveData.m_pseudoWeather ) {
            bool extended = false;
            if( p_move.m_moveData.m_pseudoWeather == PW_MAGICROOM
                || p_move.m_moveData.m_pseudoWeather == PW_WONDERROOM
                || p_move.m_moveData.m_pseudoWeather == PW_TRICKROOM ) {
                extended = canUseItem( p_move.m_user.first, p_move.m_user.second )
                           && user->getItem( ) == I_ROOM_SERVICE;
            }

            setPseudoWeather( p_ui, p_move.m_moveData.m_pseudoWeather, extended );
        }
        if( p_move.m_moveData.m_terrain ) {
            bool extended = canUseItem( p_move.m_user.first, p_move.m_user.second )
                            && user->getItem( ) == I_TERRAIN_EXTENDER;
            setTerrain( p_ui, p_move.m_moveData.m_terrain, extended );
        }

        if( p_move.m_moveData.m_slotCondition ) {
            addSlotCondition( p_ui, p_target.first, p_target.second,
                              p_move.m_moveData.m_slotCondition );
        }

        if( p_move.m_moveData.m_volatileStatus ) {
            u8   duration = 255;
            bool fail     = false;
            if( p_move.m_moveData.m_volatileStatus & ( VS_PROTECT | VS_ENDURE ) ) {
                duration = 1;

                auto lstmv = getLastUsedMove( p_target.first, p_target.second );
                if( lstmv.m_moveData.m_volatileStatus & ( VS_PROTECT | VS_ENDURE ) ) {
                    fail = ( rand( ) & 31 ) > 10;
                }
            }

            if( !fail
                && addVolatileStatus( p_ui, p_target.first, p_target.second,
                                      p_move.m_moveData.m_volatileStatus, duration ) ) {
                p_ui->animateGetVolatileStatusCondition(
                    getPkmnOrDisguise( p_target.first, p_target.second ), p_target.first,
                    p_target.second, p_move.m_moveData.m_volatileStatus );
            } else {
                p_ui->log( GET_STRING( IO::STR_UI_BATTLE_IT_FAILED ) );
            }
        }
    }

    void field::executeSecondaryStatus( battleUI* p_ui, battleMove p_move,
                                        fieldPosition p_target ) {
        auto user   = getPkmn( p_move.m_user.first, p_move.m_user.second );
        auto target = getPkmn( p_target.first, p_target.second );
        if( p_move.m_moveData.m_target == TG_SELF ) {
            target   = user;
            p_target = p_move.m_user;
        }
        if( user == nullptr || target == nullptr ) [[unlikely]] { return; }
        // Check for status conditions
        if( p_move.m_moveData.m_secondaryStatus ) {
            bool corr = false;
            if( !suppressesAbilities( ) && user->getAbility( ) == A_CORROSION ) {
                corr = ( p_move.m_moveData.m_secondaryStatus == POISON );
            }
            if( setStatusCondition(
                    p_target.first, p_target.second, p_move.m_moveData.m_secondaryStatus,
                    p_move.m_moveData.m_secondaryStatus == SLEEP ? ( 4 + ( rand( ) & 3 ) )
                                                                 : ( corr ? 254 : 255 ) ) ) {
                p_ui->animateGetStatusCondition(
                    getPkmnOrDisguise( p_target.first, p_target.second ), p_target.first,
                    p_target.second, p_move.m_moveData.m_secondaryStatus );
                p_ui->updatePkmnStats( p_target.first, p_target.second,
                                       getPkmnOrDisguise( p_target.first, p_target.second ), true );
            } else {
                p_ui->log( GET_STRING( IO::STR_UI_BATTLE_IT_FAILED ) );
            }
        }

        if( p_move.m_moveData.m_secondaryVolatileStatus ) {
            u8   duration = 255;
            bool fail     = false;
            if( p_move.m_moveData.m_secondaryVolatileStatus & ( VS_PROTECT | VS_ENDURE ) ) {
                duration   = 1;
                auto lstmv = getLastUsedMove( p_target.first, p_target.second );
                if( lstmv.m_moveData.m_volatileStatus & ( VS_PROTECT | VS_ENDURE ) ) {
                    fail = ( rand( ) & 31 ) > 10;
                }
            }

            if( !fail
                && addVolatileStatus( p_ui, p_target.first, p_target.second,
                                      p_move.m_moveData.m_secondaryVolatileStatus, duration ) ) {
                p_ui->animateGetVolatileStatusCondition(
                    getPkmnOrDisguise( p_target.first, p_target.second ), p_target.first,
                    p_target.second, p_move.m_moveData.m_secondaryVolatileStatus );
            } else {
                p_ui->log( GET_STRING( IO::STR_UI_BATTLE_IT_FAILED ) );
            }
        }
    }

    void field::executeSecondaryEffects( battleUI* p_ui, battleMove p_move,
                                         fieldPosition p_target ) {
        auto user   = getPkmn( p_move.m_user.first, p_move.m_user.second );
        auto target = getPkmn( p_target.first, p_target.second );
        if( p_move.m_moveData.m_target == TG_SELF ) {
            target   = user;
            p_target = p_move.m_user;
        }
        if( user == nullptr || target == nullptr ) [[unlikely]] { return; }

        // Boosts
        if( p_move.m_moveData.m_secondaryBoosts != boosts( ) ) {
            auto res
                = addBoosts( p_target.first, p_target.second, p_move.m_moveData.m_secondaryBoosts );
            p_ui->logBoosts( getPkmnOrDisguise( p_target.first, p_target.second ), p_target.first,
                             p_target.second, p_move.m_moveData.m_secondaryBoosts, res );
        }

        if( p_move.m_moveData.m_secondarySelfBoosts != boosts( ) ) {
            auto res = addBoosts( p_move.m_user.first, p_move.m_user.second,
                                  p_move.m_moveData.m_secondarySelfBoosts );
            p_ui->logBoosts( getPkmnOrDisguise( p_move.m_user.first, p_move.m_user.second ),
                             p_move.m_user.first, p_move.m_user.second,
                             p_move.m_moveData.m_secondarySelfBoosts, res );
        }
    }

    void field::executeContact( battleUI* p_ui, battleMove p_move, fieldPosition p_target ) {
        auto  user   = getPkmn( p_move.m_user.first, p_move.m_user.second );
        auto& pkmn   = user;
        auto  target = getPkmn( p_target.first, p_target.second );
        if( user == nullptr || target == nullptr ) [[unlikely]] { return; }

        if( !suppressesAbilities( ) && user->getAbility( ) == A_LONG_REACH ) { return; }
        if( canUseItem( p_move.m_user.first, p_move.m_user.second )
            && user->getItem( ) == I_PROTECTIVE_PADS ) {
            return;
        }

        auto tgvol = getVolatileStatus( p_target.first, p_target.second );

        // TODO messages

        if( tgvol & VS_SPIKYSHIELD ) {
            damagePokemon( p_ui, p_move.m_user.first, p_move.m_user.second,
                           pkmn->m_stats.m_maxHP / 8 );
        }
        if( tgvol & VS_KINGSSHIELD ) {
            boosts bs = boosts( );
            bs.setBoost( ATK, -1 );
            auto res = addBoosts( p_move.m_user.first, p_move.m_user.second, bs );
            p_ui->logBoosts( getPkmnOrDisguise( p_move.m_user.first, p_move.m_user.second ),
                             p_move.m_user.first, p_move.m_user.second, bs, res );
        }
        if( tgvol & VS_OBSTRUCT ) {
            boosts bs = boosts( );
            bs.setBoost( DEF, -2 );
            auto res = addBoosts( p_move.m_user.first, p_move.m_user.second, bs );
            p_ui->logBoosts( getPkmnOrDisguise( p_move.m_user.first, p_move.m_user.second ),
                             p_move.m_user.first, p_move.m_user.second, bs, res );
        }
        if( tgvol & VS_BANEFULBUNKER ) {
            if( setStatusCondition( p_move.m_user.first, p_move.m_user.second, POISON ) ) {
                p_ui->animateGetStatusCondition(
                    getPkmnOrDisguise( p_move.m_user.first, p_move.m_user.second ),
                    p_move.m_user.first, p_move.m_user.second, POISON );
                p_ui->updatePkmnStats(
                    p_move.m_user.first, p_move.m_user.second,
                    getPkmnOrDisguise( p_move.m_user.first, p_move.m_user.second ) );
            }
        }
        if( tgvol & VS_BEAKBLAST ) {
            if( setStatusCondition( p_move.m_user.first, p_move.m_user.second, BURN ) ) {
                p_ui->animateGetStatusCondition(
                    getPkmnOrDisguise( p_move.m_user.first, p_move.m_user.second ),
                    p_move.m_user.first, p_move.m_user.second, BURN );
                p_ui->updatePkmnStats(
                    p_move.m_user.first, p_move.m_user.second,
                    getPkmnOrDisguise( p_move.m_user.first, p_move.m_user.second ) );
            }
        }

        if( !user->canBattle( ) ) { return; }

        // items

        // TODO: messages
        if( canUseItem( p_target.first, p_target.second ) ) {
            switch( target->getItem( ) ) {
            case I_ROCKY_HELMET:
                damagePokemon( p_ui, p_move.m_user.first, p_move.m_user.second,
                               pkmn->m_stats.m_maxHP / 6 );
                break;
            case I_STICKY_BARB:
                if( !user->getItem( ) && target->getAbility( ) != A_STICKY_HOLD ) {
                    moveItem( p_ui, p_target.first, p_target.second, p_move.m_user.first,
                              p_move.m_user.second );
                }
                break;
            default: break;
            }
        }

        if( !user->canBattle( ) ) { return; }
        // abilities

        if( !suppressesAbilities( ) ) {
            switch( target->getAbility( ) ) {
                // TODO: perish body
            case A_WANDERING_SPIRIT: {
                p_ui->logAbility( getPkmnOrDisguise( p_target.first, p_target.second ),
                                  p_target.first );
                p_ui->logAbility( getPkmnOrDisguise( p_move.m_user.first, p_move.m_user.second ),
                                  p_move.m_user.first );
                target->setBattleTimeAbility( user->getAbility( ) );
                user->setBattleTimeAbility( A_WANDERING_SPIRIT );
                p_ui->logAbility( getPkmnOrDisguise( p_move.m_user.first, p_move.m_user.second ),
                                  p_move.m_user.first );
                p_ui->logAbility( getPkmnOrDisguise( p_target.first, p_target.second ),
                                  p_target.first );
                break;
            }
            case A_MUMMY: {
                p_ui->logAbility( getPkmnOrDisguise( p_target.first, p_target.second ),
                                  p_target.first );
                p_ui->logAbility( getPkmnOrDisguise( p_move.m_user.first, p_move.m_user.second ),
                                  p_move.m_user.first );
                user->setBattleTimeAbility( A_MUMMY );
                p_ui->logAbility( getPkmnOrDisguise( p_move.m_user.first, p_move.m_user.second ),
                                  p_move.m_user.first );
                break;
            }

            case A_PICKPOCKET: {
                if( user->getAbility( ) != A_STICKY_HOLD && !target->getItem( )
                    && user->getItem( ) ) {
                    p_ui->logAbility( getPkmnOrDisguise( p_target.first, p_target.second ),
                                      p_target.first );
                    moveItem( p_ui, p_move.m_user.first, p_move.m_user.second, p_target.first,
                              p_target.second );
                }
                break;
            }

            case A_GOOEY:
            case A_TANGLING_HAIR: {
                p_ui->logAbility( getPkmnOrDisguise( p_target.first, p_target.second ),
                                  p_target.first );
                boosts bs = boosts( );
                bs.setBoost( SPEED, -1 );
                auto res = addBoosts( p_move.m_user.first, p_move.m_user.second, bs );
                p_ui->logBoosts( getPkmnOrDisguise( p_move.m_user.first, p_move.m_user.second ),
                                 p_move.m_user.first, p_move.m_user.second, bs, res );
                break;
            }
            case A_ROUGH_SKIN:
            case A_IRON_BARBS: {
                p_ui->logAbility( getPkmnOrDisguise( p_target.first, p_target.second ),
                                  p_target.first );
                damagePokemon( p_ui, p_move.m_user.first, p_move.m_user.second,
                               pkmn->m_stats.m_maxHP / 8 );
                break;
            }
            case A_AFTERMATH: {
                if( !target->canBattle( ) ) {
                    p_ui->logAbility( getPkmnOrDisguise( p_target.first, p_target.second ),
                                      p_target.first );
                    damagePokemon( p_ui, p_move.m_user.first, p_move.m_user.second,
                                   pkmn->m_stats.m_maxHP / 4 );
                }
                break;
            }
            case A_CUTE_CHARM: {
                if( !( rand( ) % 3 ) ) {
                    p_ui->logAbility( getPkmnOrDisguise( p_target.first, p_target.second ),
                                      p_target.first );
                    addVolatileStatus( p_ui, p_move.m_user.first, p_move.m_user.second, VS_ATTRACT,
                                       255 );
                    p_ui->animateGetVolatileStatusCondition(
                        getPkmnOrDisguise( p_target.first, p_target.second ), p_target.first,
                        p_target.second, VS_ATTRACT );
                }
                break;
            }
            case A_FLAME_BODY: {
                if( !( rand( ) % 3 ) ) {
                    if( setStatusCondition( p_move.m_user.first, p_move.m_user.second, BURN ) ) {
                        p_ui->logAbility( getPkmnOrDisguise( p_target.first, p_target.second ),
                                          p_target.first );
                        p_ui->animateGetStatusCondition(
                            getPkmnOrDisguise( p_move.m_user.first, p_move.m_user.second ),
                            p_move.m_user.first, p_move.m_user.second, BURN );
                        p_ui->updatePkmnStats(
                            p_move.m_user.first, p_move.m_user.second,
                            getPkmnOrDisguise( p_move.m_user.first, p_move.m_user.second ) );
                    }
                }
                break;
            }
            case A_POISON_POINT: {
                if( !( rand( ) % 3 ) ) {
                    if( setStatusCondition( p_move.m_user.first, p_move.m_user.second, POISON ) ) {
                        p_ui->logAbility( getPkmnOrDisguise( p_target.first, p_target.second ),
                                          p_target.first );
                        p_ui->animateGetStatusCondition(
                            getPkmnOrDisguise( p_move.m_user.first, p_move.m_user.second ),
                            p_move.m_user.first, p_move.m_user.second, POISON );
                        p_ui->updatePkmnStats(
                            p_move.m_user.first, p_move.m_user.second,
                            getPkmnOrDisguise( p_move.m_user.first, p_move.m_user.second ) );
                    }
                }
                break;
            }
            case A_STATIC: {
                if( !( rand( ) % 3 ) ) {
                    if( setStatusCondition( p_move.m_user.first, p_move.m_user.second,
                                            PARALYSIS ) ) {
                        p_ui->logAbility( getPkmnOrDisguise( p_target.first, p_target.second ),
                                          p_target.first );
                        p_ui->animateGetStatusCondition(
                            getPkmnOrDisguise( p_move.m_user.first, p_move.m_user.second ),
                            p_move.m_user.first, p_move.m_user.second, PARALYSIS );
                        p_ui->updatePkmnStats(
                            p_move.m_user.first, p_move.m_user.second,
                            getPkmnOrDisguise( p_move.m_user.first, p_move.m_user.second ) );
                    }
                }
                break;
            }

            case A_EFFECT_SPORE: {
                if( !( rand( ) % 3 ) ) {
                    // TODO message
                    switch( rand( ) % 3 ) {
                    case 0:
                        if( setStatusCondition( p_move.m_user.first, p_move.m_user.second,
                                                BURN ) ) {
                            p_ui->logAbility( getPkmnOrDisguise( p_target.first, p_target.second ),
                                              p_target.first );
                            p_ui->animateGetStatusCondition(
                                getPkmnOrDisguise( p_move.m_user.first, p_move.m_user.second ),
                                p_move.m_user.first, p_move.m_user.second, BURN );
                            p_ui->updatePkmnStats(
                                p_move.m_user.first, p_move.m_user.second,
                                getPkmnOrDisguise( p_move.m_user.first, p_move.m_user.second ) );
                        }
                        break;
                    case 1:
                        if( setStatusCondition( p_move.m_user.first, p_move.m_user.second,
                                                POISON ) ) {
                            p_ui->logAbility( getPkmnOrDisguise( p_target.first, p_target.second ),
                                              p_target.first );
                            p_ui->animateGetStatusCondition(
                                getPkmnOrDisguise( p_move.m_user.first, p_move.m_user.second ),
                                p_move.m_user.first, p_move.m_user.second, POISON );
                            p_ui->updatePkmnStats(
                                p_move.m_user.first, p_move.m_user.second,
                                getPkmnOrDisguise( p_move.m_user.first, p_move.m_user.second ) );
                        }
                        break;
                    case 2:
                        if( setStatusCondition( p_move.m_user.first, p_move.m_user.second,
                                                PARALYSIS ) ) {
                            p_ui->logAbility( getPkmnOrDisguise( p_target.first, p_target.second ),
                                              p_target.first );
                            p_ui->animateGetStatusCondition(
                                getPkmnOrDisguise( p_move.m_user.first, p_move.m_user.second ),
                                p_move.m_user.first, p_move.m_user.second, PARALYSIS );
                            p_ui->updatePkmnStats(
                                p_move.m_user.first, p_move.m_user.second,
                                getPkmnOrDisguise( p_move.m_user.first, p_move.m_user.second ) );
                        }
                        break;
                    default: break;
                    }
                }
                break;
            }

            case A_POISON_TOUCH: {
                if( !( rand( ) % 3 ) ) {
                    if( setStatusCondition( p_target.first, p_target.second, POISON ) ) {
                        p_ui->logAbility(
                            getPkmnOrDisguise( p_move.m_user.first, p_move.m_user.second ),
                            p_move.m_user.first );
                        p_ui->animateGetStatusCondition(
                            getPkmnOrDisguise( p_move.m_user.first, p_move.m_user.second ),
                            p_target.first, p_target.second, POISON );
                        p_ui->updatePkmnStats(
                            p_target.first, p_target.second,
                            getPkmnOrDisguise( p_move.m_user.first, p_move.m_user.second ) );
                    }
                }
                break;
            }
            default: break;
            }
        }
    }

    u8 field::useMove( battleUI* p_ui, battleMove p_move ) {
        constexpr u8 TMP_BUFFER_SIZE = 100;
        char         buffer[ TMP_BUFFER_SIZE + 10 ];
        bool         opponent = p_move.m_user.first;
        u8           slot     = p_move.m_user.second;

        auto volst = getVolatileStatus( opponent, slot );

        if( p_move.m_param == M_FOCUS_PUNCH && !( volst & VS_FOCUSPUNCH ) ) [[unlikely]] {
            auto fmt = std::string( GET_STRING( 548 ) );
            snprintf( buffer, TMP_BUFFER_SIZE, fmt.c_str( ),
                      p_ui->getPkmnName( getPkmnOrDisguise( opponent, slot ), opponent ).c_str( ) );
            p_ui->log( buffer );
            return MOVE_FAIL;
        }
        if( p_move.m_param == M_SHELL_TRAP && ( volst & VS_SHELLTRAP ) ) [[unlikely]] {
            auto fmt = std::string( GET_STRING( 536 ) );
            snprintf( buffer, TMP_BUFFER_SIZE, fmt.c_str( ),
                      p_ui->getPkmnName( getPkmnOrDisguise( opponent, slot ), opponent ).c_str( ) );
            p_ui->log( buffer );
            return MOVE_FAIL;
        }

        if( volst & VS_RECHARGE ) [[unlikely]] {
            auto fmt = std::string( GET_STRING( 276 ) );
            snprintf( buffer, TMP_BUFFER_SIZE, fmt.c_str( ),
                      p_ui->getPkmnName( getPkmnOrDisguise( opponent, slot ), opponent ).c_str( ) );
            p_ui->log( buffer );

            removeVolatileStatus( p_ui, opponent, slot, VS_RECHARGE );
            removeLockedMove( opponent, slot );
            return MOVE_FAIL_NO_PP;
        }

        if( volst & VS_FLINCH ) [[unlikely]] {
            auto fmt = std::string( GET_STRING( 296 ) );
            snprintf( buffer, TMP_BUFFER_SIZE, fmt.c_str( ),
                      p_ui->getPkmnName( getPkmnOrDisguise( opponent, slot ), opponent ).c_str( ) );
            p_ui->log( buffer );
            return MOVE_FAIL_NO_PP;
        }

        if( hasStatusCondition( opponent, slot, FROZEN ) ) [[unlikely]] {
            if( p_move.m_moveData.m_type == TYPE_FIRE || ( p_move.m_moveData.m_flags & MF_DEFROST )
                || ( rand( ) % 100 < 20 ) ) {
                // user thaws
                auto fmt = std::string( GET_STRING( 298 ) );
                snprintf(
                    buffer, TMP_BUFFER_SIZE, fmt.c_str( ),
                    p_ui->getPkmnName( getPkmnOrDisguise( opponent, slot ), opponent ).c_str( ) );
                p_ui->log( buffer );
                removeStatusCondition( opponent, slot );
                p_ui->updatePkmnStats( opponent, slot, getPkmn( opponent, slot ) );
            } else {
                p_ui->animateStatusCondition( getPkmn( opponent, slot ), opponent, slot, FROZEN );
                return MOVE_FAIL_NO_PP;
            }
        }

        if( u8 slp = hasStatusCondition( opponent, slot, SLEEP ); slp ) {
            if( slp > 1 ) {
                // pkmn continues to sleep
                p_ui->animateStatusCondition( getPkmn( opponent, slot ), opponent, slot, SLEEP );
                // force the sleep counter to decrease
                setStatusCondition( opponent, slot, SLEEP, slp - 1, true );
                if( !( p_move.m_moveData.m_flags & MF_SLEEPUSABLE ) ) { return MOVE_FAIL_NO_PP; }
            } else {
                removeStatusCondition( opponent, slot );
                auto fmt = std::string( GET_STRING( 300 ) );
                snprintf(
                    buffer, TMP_BUFFER_SIZE, fmt.c_str( ),
                    p_ui->getPkmnName( getPkmnOrDisguise( opponent, slot ), opponent ).c_str( ) );
                p_ui->log( buffer );
                p_ui->updatePkmnStats( opponent, slot, getPkmn( opponent, slot ) );
            }
        } else if( p_move.m_moveData.m_flags & MF_SLEEPUSABLE ) {
            if( getPkmn( opponent, slot )->getAbility( ) != A_COMATOSE ) {
                auto fmt = std::string( GET_STRING( 10 ) );
                snprintf(
                    buffer, TMP_BUFFER_SIZE, fmt.c_str( ),
                    p_ui->getPkmnName( getPkmnOrDisguise( opponent, slot ), opponent ).c_str( ),
                    FS::getMoveName( p_move.m_param ).c_str( ) );
                p_ui->log( buffer );
                p_ui->log( GET_STRING( IO::STR_UI_BATTLE_IT_FAILED ) );
                return MOVE_FAIL;
            }
        }

        if( hasStatusCondition( opponent, slot, PARALYSIS ) ) {
            if( rand( ) % 100 < 25 ) {
                p_ui->animateStatusCondition( getPkmn( opponent, slot ), opponent, slot,
                                              PARALYSIS );
                return MOVE_FAIL_NO_PP;
            }
        }

        if( volst & VS_CONFUSION ) [[unlikely]] {
            u8 curVal = getVolatileStatusCounter( opponent, slot, VS_CONFUSION );
            if( curVal > 250 ) {
                --curVal;
                p_ui->animateVolatileStatusCondition( getPkmn( opponent, slot ), opponent, slot,
                                                      VS_CONFUSION );
                auto fmt = std::string( GET_STRING( 293 ) );
                snprintf(
                    buffer, TMP_BUFFER_SIZE, fmt.c_str( ),
                    p_ui->getPkmnName( getPkmnOrDisguise( opponent, slot ), opponent ).c_str( ) );
                p_ui->log( buffer );
                addVolatileStatus( p_ui, opponent, slot, VS_CONFUSION, curVal );

                if( rand( ) % 300 < 100 ) {
                    confusionSelfDamage( p_ui, opponent, slot );
                    p_ui->log( GET_STRING( 295 ) );
                    return MOVE_FAIL_NO_PP;
                }
            } else {
                auto fmt = std::string( GET_STRING( 294 ) );
                snprintf(
                    buffer, TMP_BUFFER_SIZE, fmt.c_str( ),
                    p_ui->getPkmnName( getPkmnOrDisguise( opponent, slot ), opponent ).c_str( ) );
                p_ui->log( buffer );
                removeVolatileStatus( p_ui, opponent, slot, VS_CONFUSION );
            }
        }

        if( ( volst & VS_HEALBLOCK ) && ( p_move.m_moveData.m_flags & MF_HEAL ) ) [[unlikely]] {
            // TODO: this may be wrong
            return MOVE_FAIL;
        }

        if( _pseudoWeatherTimer[ 4 ] && ( p_move.m_moveData.m_flags & MF_GRAVITY ) ) [[unlikely]] {
            return MOVE_FAIL;
        }

        if( volst & VS_ATTRACT ) [[unlikely]] {
            if( rand( ) % 100 < 50 ) {
                auto fmt = std::string( GET_STRING( 302 ) );
                snprintf(
                    buffer, TMP_BUFFER_SIZE, fmt.c_str( ),
                    p_ui->getPkmnName( getPkmnOrDisguise( opponent, slot ), opponent ).c_str( ) );
                p_ui->log( buffer );
                return MOVE_FAIL_NO_PP;
            }
        }

        return MOVE_SUCCESS;
    }

    bool field::moveMisses( battleUI* p_ui, battleMove p_move, fieldPosition p_target,
                            bool p_critical ) {
        auto user   = getPkmn( p_move.m_user.first, p_move.m_user.second );
        auto target = getPkmn( p_target.first, p_target.second );
        if( user == nullptr || target == nullptr ) [[unlikely]] { return false; }

        auto tgvol    = getVolatileStatus( p_target.first, p_target.second );
        auto usvol    = getVolatileStatus( p_move.m_user.first, p_move.m_user.second );
        bool supprAbs = suppressesAbilities( );

        if( !supprAbs && user->getAbility( ) == A_NO_GUARD ) { return false; }

        if( p_move.m_moveData.m_target == TG_SELF || p_move.m_moveData.m_target == TG_FIELD
            || p_move.m_moveData.m_target == TG_FOE_SIDE
            || p_move.m_moveData.m_target == TG_ALLY_SIDE ) {
            return false;
        }

        if( tgvol & VS_DIGGING ) {
            return !( p_move.m_param == M_EARTHQUAKE || p_move.m_param == M_MAGNITUDE
                      || p_move.m_param == M_FISSURE );
        }
        if( tgvol & VS_DIVING ) {
            return !( p_move.m_param == M_SURF || p_move.m_param == M_WHIRLPOOL );
        }
        if( tgvol & VS_INAIR ) {
            return !( p_move.m_param == M_GUST || p_move.m_param == M_TWISTER
                      || p_move.m_param == M_THUNDER || p_move.m_param == M_THOUSAND_ARROWS
                      || p_move.m_param == M_HURRICANE || p_move.m_param == M_SMACK_DOWN
                      || p_move.m_param == M_SKY_UPPERCUT );
        }
        if( tgvol & VS_INVISIBLE ) { return true; }

        if( p_move.m_moveData.m_flags & MF_OHKOICE ) {
            if( hasType( p_target.first, p_target.second, TYPE_ICE ) ) { return true; }
        }
        if( p_move.m_moveData.m_flags & MF_OHKO ) {
            if( target->m_level > user->m_level ) { return true; }
            return ( rand( ) % 100 ) > ( 20 + user->m_level - target->m_level );
        }

        if( usvol & VS_FORESIGHT ) { return false; }
        if( usvol & VS_MIRACLEEYE ) { return false; }

        u16 acc = p_move.m_moveData.m_accuracy;
        if( !acc || acc > 100 ) { return false; }

        if( !suppressesWeather( ) && getWeather( ) == WE_FOG ) { acc = ( 3 * acc ) / 5; }

        bool ignoreEvasion = !!( p_move.m_moveData.m_flags & MF_IGNOREEVASION ) || p_critical;

        s8 ev = s8( getStat( p_move.m_user.first, p_move.m_user.second, ACCURACY ) )
                - ( ignoreEvasion * s8( getStat( p_target.first, p_target.second, EVASION ) ) );

        if( ev < -6 ) { ev = -6; }
        if( ev > 6 ) { ev = 6; }

        if( ev < 0 ) {
            acc = ( acc * 3 ) / ( -ev + 3 );
        } else if( ev > 0 ) {
            acc = ( acc * ( ev + 3 ) ) / 3;
        }

        if( !supprAbs && !breaksAbilities( p_move.m_user.first, p_move.m_user.second ) ) {
            // Victory Star
            if( _sides[ p_move.m_user.first ].anyHasAbility( A_VICTORY_STAR ) ) {
                acc = ( acc * 11 ) / 10;
            }

            // Compound Eyes
            if( user->getAbility( ) == A_COMPOUND_EYES ) { acc = ( acc * 13 ) / 10; }

            // Hustle
            if( user->getAbility( ) == A_HUSTLE && p_move.m_moveData.m_category == MH_PHYSICAL ) {
                acc = ( acc * 8 ) / 10;
            }

            // Wonder Skin
            if( target->getAbility( ) == A_WONDER_SKIN
                && p_move.m_moveData.m_category == MH_SPECIAL ) {
                acc = 50;
            }
            // Sand Veil
            if( target->getAbility( ) == A_SAND_VEIL && getWeather( ) == WE_SANDSTORM
                && !suppressesWeather( ) ) {
                acc = ( 4 * acc ) / 5;
            }
            // Snow Cloak
            if( target->getAbility( ) == A_SNOW_CLOAK && getWeather( ) == WE_HAIL
                && !suppressesWeather( ) ) {
                acc = ( 4 * acc ) / 5;
            }
            // Tangled Feet
            if( target->getAbility( ) == A_TANGLED_FEET && ( tgvol & VS_CONFUSION ) ) { acc >>= 1; }
        }

        if( canUseItem( p_target.first, p_target.second ) ) {
            if( target->getItem( ) == I_BRIGHT_POWDER || target->getItem( ) == I_LAX_INCENSE ) {
                acc = ( 9 * acc ) / 10;
            }
        }
        if( canUseItem( p_move.m_user.first, p_move.m_user.second ) ) {
            if( user->getItem( ) == I_WIDE_LENS ) { acc = ( 11 * acc ) / 10; }
            if( user->getItem( ) == I_ZOOM_LENS ) {
                // TODO proper check
                if( getStat( p_move.m_user.first, p_move.m_user.second, SPEED )
                    < getStat( p_target.first, p_target.second, SPEED ) ) {
                    acc = ( 12 * acc ) / 10;
                }
            }
        }

#ifdef DESQUID_MORE
        p_ui->log( std::string( "Accuracy computed " ) + std::to_string( acc ) );
#else
        (void) p_ui;
#endif

        return rand( ) % 100 >= acc;
    }

    bool field::executeCriticalCheck( battleUI* p_ui, battleMove p_move, fieldPosition p_target ) {
        constexpr u8 TMP_BUFFER_SIZE = 100;
        char         buffer[ TMP_BUFFER_SIZE + 10 ];
        auto         user   = getPkmn( p_move.m_user.first, p_move.m_user.second );
        auto         target = getPkmn( p_target.first, p_target.second );
        if( user == nullptr || target == nullptr ) [[unlikely]] { return false; }

        bool opponent = p_move.m_user.first;
        u8   slot     = p_move.m_user.second;

        bool supprAbs    = suppressesAbilities( );
        auto userVolStat = getVolatileStatus( p_move.m_user.first, p_move.m_user.second );

        // Check for crit suppr.

        if( !supprAbs
            && ( target->getAbility( ) == A_BATTLE_ARMOR
                 || target->getAbility( ) == A_SHELL_ARMOR ) ) {
            return false;
        }

        if( getSideCondition( !p_target.first ) & SC_LUCKYCHANT ) { return false; }

        if( userVolStat & VS_LASERFOCUS ) { return true; }
        if( p_move.m_moveData.m_flags & MF_WILLCRIT ) { return true; }

        // Calculate critical hit

        bool critical = false;

        u8 critLevel = p_move.m_moveData.m_critRatio;

        bool item = canUseItem( p_move.m_user.first, p_move.m_user.second );

        if( item ) {
            if( user->getItem( ) == I_RAZOR_CLAW || user->getItem( ) == I_SCOPE_LENS )
                [[unlikely]] {
                critLevel++;
            } else if( user->getSpecies( ) == PKMN_FARFETCH_D
                       || user->getSpecies( ) == PKMN_SIRFETCH_D ) [[unlikely]] {
                if( user->getItem( ) == I_LEEK ) [[unlikely]] { critLevel += 2; }
            } else if( user->getSpecies( ) == PKMN_CHANSEY || user->getSpecies( ) == PKMN_BLISSEY
                       || user->getSpecies( ) == PKMN_HAPPINY ) [[unlikely]] {
                if( user->getItem( ) == I_LUCKY_PUNCH ) [[unlikely]] { critLevel += 2; }
            }
        }

        if( !supprAbs && user->getAbility( ) == A_SUPER_LUCK ) { critLevel++; }

        if( item && user->getItem( ) == I_LANSAT_BERRY ) [[unlikely]] {
            u8 factor = 4;
            if( !supprAbs && user->getAbility( ) == A_GLUTTONY ) { factor = 2; }

            if( user->m_stats.m_curHP * factor < user->m_stats.m_maxHP ) {
                if( supprAbs
                    || !_sides[ opponent ? PLAYER_SIDE : OPPONENT_SIDE ].anyHasAbility(
                        A_UNNERVE ) ) [[likely]] {
                    p_ui->logItem( getPkmnOrDisguise( opponent, slot ), opponent );
                    auto fmt = std::string( GET_STRING( 279 ) );
                    snprintf(
                        buffer, TMP_BUFFER_SIZE, fmt.c_str( ),
                        p_ui->getPkmnName( getPkmnOrDisguise( opponent, slot ), opponent ).c_str( ),
                        FS::getItemName( I_LANSAT_BERRY ).c_str( ) );
                    p_ui->log( buffer );

                    critLevel += 3;

                    removeItem( p_ui, opponent, slot );
                    checkOnEatBerry( p_ui, opponent, slot, I_LANSAT_BERRY );
                }
            }
        } else if( userVolStat & VS_FOCUSENERGY ) {
            critLevel += 2;
        }

        if( critLevel >= 3 ) [[unlikely]] {
            critical = true;
        } else if( critLevel == 2 ) [[unlikely]] {
            critical = !( rand( ) % 2 );
        } else if( critLevel == 1 ) {
            critical = !( rand( ) % 8 );
        } else [[likely]] {
            critical = !( rand( ) % 24 );
        }

        return critical;
    }

    u16 field::getMovePower( battleMove p_move ) {
        u16  basepower = p_move.m_moveData.m_basePower;
        type moveType  = p_move.m_moveData.m_type;

        if( p_move.m_param == M_HIDDEN_POWER ) {
            basepower = getPkmn( p_move.m_user.first, p_move.m_user.second )->getHPPower( );
            moveType  = getPkmn( p_move.m_user.first, p_move.m_user.second )->getHPType( );
        }

        if( !suppressesAbilities( ) ) {
            switch( getPkmn( p_move.m_user.first, p_move.m_user.second )->getAbility( ) ) {
            case A_PIXILATE:
            case A_AERILATE:
            case A_REFRIGERATE:
            case A_GALVANIZE:
                if( moveType == TYPE_NORMAL ) { basepower = ( basepower * 120 ) / 100; }
                break;
            [[likely]] default:
                break;
            }
        }

        if( p_move.m_param == M_WEATHER_BALL && !suppressesWeather( ) ) {
            switch( getWeather( ) ) {
            case WE_NONE: basepower = 50; break;
            default: basepower = 100; break;
            }
        }

        if( p_move.m_param == M_TERRAIN_PULSE ) {
            switch( getTerrain( ) ) {
            case TR_NONE: basepower = 50; break;
            default: basepower = 100; break;
            }
        }

        if( p_move.m_param == M_FACADE ) {
            if( getPkmn( p_move.m_user.first, p_move.m_user.second )->m_statusint ) {
                basepower *= 2;
            }
        }

        return basepower;
    }

    type field::getMoveType( battleMove p_move ) {
        type moveType = p_move.m_moveData.m_type;
        auto pkmn     = getPkmn( p_move.m_user.first, p_move.m_user.second );
        if( pkmn == nullptr ) [[unlikely]] { return TYPE_UNKNOWN; }

        if( p_move.m_param == M_STRUGGLE ) { return TYPE_NORMAL; }

        if( p_move.m_param == M_HIDDEN_POWER ) [[unlikely]] { moveType = pkmn->getHPType( ); }

        if( p_move.m_param == M_AURA_WHEEL ) [[unlikely]] {
            if( pkmn->getForme( ) == 1 ) {
                moveType = TYPE_DARKNESS;
            } else {
                moveType = TYPE_LIGHTNING;
            }
        }

        if( !suppressesAbilities( ) ) {
            switch( pkmn->getAbility( ) ) {
            case A_NORMALIZE: moveType = TYPE_NORMAL; break;
            case A_PIXILATE:
                if( moveType == TYPE_NORMAL ) { return TYPE_FAIRY; }
                break;
            case A_AERILATE:
                if( moveType == TYPE_NORMAL ) { return TYPE_FLYING; }
                break;
            case A_GALVANIZE:
                if( moveType == TYPE_NORMAL ) { return TYPE_LIGHTNING; }
                break;
            case A_REFRIGERATE:
                if( moveType == TYPE_NORMAL ) { return TYPE_ICE; }
                break;
            case A_LIQUID_VOICE:
                if( p_move.m_moveData.m_flags & MF_SOUND ) { return TYPE_WATER; }
                break;
            [[likely]] default:
                break;
            }
        }

        if( p_move.m_param == M_JUDGMENT
            && canUseItem( p_move.m_user.first, p_move.m_user.second ) ) [[unlikely]] {
            switch( pkmn->getItem( ) ) {
            case I_FLAME_PLATE: moveType = TYPE_FIRE; break;
            case I_SPLASH_PLATE: moveType = TYPE_WATER; break;
            case I_ZAP_PLATE: moveType = TYPE_LIGHTNING; break;
            case I_MEADOW_PLATE: moveType = TYPE_GRASS; break;
            case I_ICICLE_PLATE: moveType = TYPE_ICE; break;
            case I_FIST_PLATE: moveType = TYPE_FIGHTING; break;
            case I_TOXIC_PLATE: moveType = TYPE_POISON; break;
            case I_EARTH_PLATE: moveType = TYPE_GROUND; break;
            case I_SKY_PLATE: moveType = TYPE_FLYING; break;
            case I_MIND_PLATE: moveType = TYPE_PSYCHIC; break;
            case I_INSECT_PLATE: moveType = TYPE_BUG; break;
            case I_STONE_PLATE: moveType = TYPE_ROCK; break;
            case I_SPOOKY_PLATE: moveType = TYPE_GHOST; break;
            case I_DRACO_PLATE: moveType = TYPE_DRAGON; break;
            case I_DREAD_PLATE: moveType = TYPE_DARKNESS; break;
            case I_IRON_PLATE: moveType = TYPE_STEEL; break;
            case I_PIXIE_PLATE: moveType = TYPE_FAIRY; break;
            case I_NULL_PLATE: moveType = TYPE_UNKNOWN; break;
            default: moveType = TYPE_NORMAL; break;
            }
        }

        if( p_move.m_param == M_MULTI_ATTACK
            && canUseItem( p_move.m_user.first, p_move.m_user.second ) ) [[unlikely]] {
            switch( pkmn->getItem( ) ) {
            case I_FIGHTING_MEMORY: moveType = TYPE_FIGHTING; break;
            case I_FLYING_MEMORY: moveType = TYPE_FLYING; break;
            case I_POISON_MEMORY: moveType = TYPE_POISON; break;
            case I_GROUND_MEMORY: moveType = TYPE_GROUND; break;
            case I_ROCK_MEMORY: moveType = TYPE_ROCK; break;
            case I_BUG_MEMORY: moveType = TYPE_BUG; break;
            case I_GHOST_MEMORY: moveType = TYPE_GHOST; break;
            case I_STEEL_MEMORY: moveType = TYPE_STEEL; break;
            case I_FIRE_MEMORY: moveType = TYPE_FIRE; break;
            case I_WATER_MEMORY: moveType = TYPE_WATER; break;
            case I_GRASS_MEMORY: moveType = TYPE_GRASS; break;
            case I_ELECTRIC_MEMORY: moveType = TYPE_LIGHTNING; break;
            case I_PSYCHIC_MEMORY: moveType = TYPE_PSYCHIC; break;
            case I_ICE_MEMORY: moveType = TYPE_ICE; break;
            case I_DRAGON_MEMORY: moveType = TYPE_DRAGON; break;
            case I_DARK_MEMORY: moveType = TYPE_DARKNESS; break;
            case I_FAIRY_MEMORY: moveType = TYPE_FAIRY; break;
            default: moveType = TYPE_NORMAL; break;
            }
        }

        if( p_move.m_param == M_TECHNO_BLAST
            && canUseItem( p_move.m_user.first, p_move.m_user.second ) ) [[unlikely]] {
            switch( pkmn->getItem( ) ) {
            case I_BURN_DRIVE: moveType = TYPE_FIRE; break;
            case I_DOUSE_DRIVE: moveType = TYPE_WATER; break;
            case I_CHILL_DRIVE: moveType = TYPE_ICE; break;
            case I_SHOCK_DRIVE: moveType = TYPE_LIGHTNING; break;
            default: moveType = TYPE_NORMAL; break;
            }
        }

        if( p_move.m_param == M_REVELATION_DANCE ) {
            auto types = getTypes( p_move.m_user.first, p_move.m_user.second );
            if( types.size( ) >= 1 ) {
                moveType = types[ 0 ];
            } else {
                moveType = TYPE_NORMAL;
            }
        }

        if( p_move.m_param == M_WEATHER_BALL && !suppressesWeather( ) ) {
            switch( getWeather( ) ) {
            case WE_RAIN:
            case WE_HEAVY_RAIN: moveType = TYPE_WATER; break;
            case WE_SUN:
            case WE_HEAVY_SUNSHINE: moveType = TYPE_FIRE; break;
            case WE_HAIL: moveType = TYPE_ICE; break;
            case WE_SANDSTORM: moveType = TYPE_ROCK; break;
            case WE_FOG: moveType = TYPE_FLYING; break;
            default: moveType = TYPE_NORMAL; break;
            }
        }

        if( p_move.m_param == M_TERRAIN_PULSE ) {
            switch( getTerrain( ) ) {
            case TR_PSYCHICTERRAIN: moveType = TYPE_PSYCHIC; break;
            case TR_ELECTRICTERRAIN: moveType = TYPE_LIGHTNING; break;
            case TR_MISTYTERRAIN: moveType = TYPE_FAIRY; break;
            case TR_GRASSYTERRAIN: moveType = TYPE_GRASS; break;
            default: moveType = TYPE_NORMAL; break;
            }
        }

        if( getVolatileStatus( p_move.m_user.first, p_move.m_user.second ) & VS_ELECTRIFY ) {
            moveType = TYPE_LIGHTNING;
        }

        if( ( getPseudoWeather( ) & PW_IONDELUGE ) && moveType == TYPE_NORMAL ) {
            moveType = TYPE_LIGHTNING;
        }

        return moveType;
    }

    u16 field::getEffectiveness( battleMove p_move, fieldPosition p_target ) {
        u16 res = 100;

        auto target = getPkmn( p_target.first, p_target.second );
        if( target == nullptr ) [[unlikely]] { return 0; }

        bool items = canUseItem( p_target.first, p_target.second );

        bool abilities
            = !suppressesAbilities( ) && !( p_move.m_moveData.m_flags & MF_IGNOREABILITY );

        bool wonderguard = abilities && target->getAbility( ) == A_WONDER_GUARD;

        type moveType = getMoveType( p_move );

        // Heavy weather

        if( getWeather( ) == WE_HEAVY_SUNSHINE && moveType == TYPE_WATER
            && !suppressesWeather( ) ) {
            return 0;
        }
        if( getWeather( ) == WE_HEAVY_RAIN && moveType == TYPE_FIRE && !suppressesWeather( ) ) {
            return 0;
        }

        if( ( p_move.m_moveData.m_flags & MF_SOUND )
            && ( target->getAbility( ) == A_SOUNDPROOF || target->getAbility( ) == A_CACOPHONY )
            && abilities ) [[unlikely]] {
            return 0;
        }

        // Thousand arrows
        if( p_move.m_param == M_THOUSAND_ARROWS
            || ( p_move.m_moveData.m_flags & MF_IGNOREIMMUNITYGROUND ) ) [[unlikely]] {
            if( !isGrounded( p_target.first, p_target.second )
                && hasType( p_target.first, p_target.second, TYPE_FLYING ) ) {
                return res;
            }
        }

        bool targetIsGrounded
            = isGrounded( p_target.first, p_target.second, p_move.m_param != M_SUNSTEEL_STRIKE );
        auto user = getPkmn( p_move.m_user.first, p_move.m_user.second );
        if( user == nullptr ) [[unlikely]] { return 0; }

        for( type t : getTypes( p_target.first, p_target.second ) ) {
            u16 curval = getTypeEffectiveness( moveType, t );

            if( !targetIsGrounded && moveType == TYPE_GROUND ) [[unlikely]] {
                curval = 0;
            } else if( targetIsGrounded && moveType == TYPE_GROUND && t == TYPE_FLYING ) {
                curval = 100;
            }

            if( p_move.m_param == M_FLYING_PRESS ) {
                curval = ( curval * getTypeEffectiveness( TYPE_FLYING, t ) ) / 100;
            }

            if( !curval && ( p_move.m_moveData.m_flags & MF_IGNOREIMMUNITY ) ) { curval = 100; }

            if( t == TYPE_GHOST && ( moveType == TYPE_NORMAL || moveType == TYPE_FIGHTING ) )
                [[unlikely]] {
                if( abilities && user->getAbility( ) == A_SCRAPPY ) { continue; }
                if( getVolatileStatus( p_target.first, p_target.second ) & VS_FORESIGHT )
                    [[unlikely]] {
                    continue;
                }
            }

            if( t == TYPE_DARKNESS && moveType == TYPE_PSYCHIC
                && ( getVolatileStatus( p_target.first, p_target.second ) & VS_MIRACLEEYE ) )
                [[unlikely]] {
                continue;
            }

            if( t == TYPE_WATER && p_move.m_param == M_FREEZE_DRY ) { curval = 200; }

            if( t == TYPE_FLYING ) {
                if( !suppressesWeather( ) && _weather == WE_HEAVY_WINDS ) [[unlikely]] { continue; }
            }

            if( !curval && items && target->getItem( ) == I_RING_TARGET ) [[unlikely]] { continue; }

            res = ( res * curval / 100 );
        }

        if( wonderguard && res <= 100 ) { return 0; }

        return res;
    }

    bool field::executeDamagingMove( battleUI* p_ui, battleMove p_move, fieldPosition p_target,
                                     bool p_critical, u8 p_damageModifier ) {
        constexpr u8 TMP_BUFFER_SIZE = 100;
        char         buffer[ TMP_BUFFER_SIZE + 10 ];

        auto user   = getPkmn( p_move.m_user.first, p_move.m_user.second );
        auto target = getPkmn( p_target.first, p_target.second );
        if( user == nullptr || target == nullptr ) [[unlikely]] { return false; }

        bool supprAbs    = suppressesAbilities( );
        auto userVolStat = getVolatileStatus( p_move.m_user.first, p_move.m_user.second );

        u16 effectiveness = getEffectiveness( p_move, p_target );

        if( effectiveness == 0 ) {
            auto fmt = std::string( GET_STRING( 284 ) );
            snprintf( buffer, TMP_BUFFER_SIZE, fmt.c_str( ),
                      p_ui->getPkmnName( getPkmnOrDisguise( p_target.first, p_target.second ),
                                         p_target.first, false )
                          .c_str( ) );
            p_ui->log( buffer );
            return false;
        }

        // Calculate damage
        u32 damage = 0;

        if( p_move.m_moveData.m_fixedDamage ) {
            damage = p_move.m_moveData.m_fixedDamage;
        } else {
            u16 atk = getStat( p_move.m_user.first, p_move.m_user.second,
                               p_move.m_moveData.m_category == MH_SPECIAL ? SATK : ATK,
                               !supprAbs && p_move.m_param != M_SUNSTEEL_STRIKE, p_critical );
            u16 def
                = getStat( p_target.first, p_target.second,
                           p_move.m_moveData.m_defensiveCategory == MH_SPECIAL ? SDEF : DEF,
                           !supprAbs && p_move.m_param != M_SUNSTEEL_STRIKE, false, p_critical );
            if( p_move.m_moveData.m_flags & MF_DEFASOFF ) {
                atk = getStat( p_move.m_user.first, p_move.m_user.second,
                               p_move.m_moveData.m_category == MH_SPECIAL ? SDEF : DEF,
                               !supprAbs && p_move.m_param != M_SUNSTEEL_STRIKE, p_critical );
            }

            damage = ( user->m_level * 2 ) / 5 + 2;

            // base power modifying stuff
            u16 movePower = getMovePower( p_move );

            // speed-based
            if( p_move.m_param == M_GYRO_BALL ) [[unlikely]] {
                auto atkspd = getStat( p_move.m_user.first, p_move.m_user.second, SPEED );
                auto defspd = getStat( p_target.first, p_move.m_user.second, SPEED );

                movePower = atkspd ? 25 * defspd / atkspd : 150;
                if( movePower > 150 ) { movePower = 150; }
                if( movePower == 0 ) { movePower = 1; }
            }

            if( p_move.m_param == M_ELECTRO_BALL ) [[unlikely]] {
                auto atkspd = getStat( p_move.m_user.first, p_move.m_user.second, SPEED );
                auto defspd = getStat( p_target.first, p_move.m_user.second, SPEED );

                if( !defspd ) {
                    movePower = 150;
                } else {
                    switch( atkspd / defspd ) {
                    case 0: movePower = 40; break;
                    case 1: movePower = 60; break;
                    case 2: movePower = 80; break;
                    case 3: movePower = 120; break;
                    default: movePower = 150; break;
                    }
                }
            }

            // weight-based
            auto tgweight   = getWeight( p_target.first, p_target.second, !supprAbs );
            auto userweight = getWeight( p_move.m_user.first, p_move.m_user.second, !supprAbs );

            if( p_move.m_param == M_LOW_KICK || p_move.m_param == M_GRASS_KNOT ) {
                if( tgweight < TMP_BUFFER_SIZE ) {
                    movePower = 20;
                } else if( tgweight < 249 ) {
                    movePower = 40;
                } else if( tgweight < 499 ) {
                    movePower = 60;
                } else if( tgweight < 999 ) {
                    movePower = 80;
                } else if( tgweight < 1999 ) {
                    movePower = 100;
                } else {
                    movePower = 120;
                }
            }

            if( p_move.m_param == M_HEAVY_SLAM || p_move.m_param == M_HEAT_CRASH ) {
                if( tgweight * 2 > userweight ) {
                    movePower = 40;
                } else if( tgweight * 3 > userweight ) {
                    movePower = 60;
                } else if( tgweight * 4 > userweight ) {
                    movePower = 80;
                } else if( tgweight * 5 > userweight ) {
                    movePower = 100;
                } else {
                    movePower = 120;
                }
            }

            // HP-based
            if( p_move.m_param == M_ERUPTION || p_move.m_param == M_WATER_SPOUT
                || p_move.m_param == M_DRAGON_ENERGY ) {
                movePower = 150 * user->m_stats.m_curHP / user->m_stats.m_maxHP;
            }
            if( p_move.m_param == M_FLAIL || p_move.m_param == M_REVERSAL ) {
                auto relhp = 48 * user->m_stats.m_curHP / user->m_stats.m_maxHP;
                if( relhp > 32 ) {
                    movePower = 20;
                } else if( relhp > 16 ) {
                    movePower = 40;
                } else if( relhp > 9 ) {
                    movePower = 60;
                } else if( relhp > 4 ) {
                    movePower = 100;
                } else if( relhp > 1 ) {
                    movePower = 150;
                } else {
                    movePower = 200;
                }
            }
            if( p_move.m_param == M_CRUSH_GRIP || p_move.m_param == M_WRING_OUT ) {
                movePower = 120 * target->m_stats.m_curHP / target->m_stats.m_maxHP;
            }

            // happiness-based

            if( p_move.m_param == M_RETURN ) { movePower = 1 + user->m_boxdata.m_steps * 10 / 25; }
            if( p_move.m_param == M_FRUSTRATION ) {
                movePower = 1 + ( 255 - user->m_boxdata.m_steps ) * 10 / 25;
            }

            // repetition-based

            if( p_move.m_param == M_FURY_CUTTER ) {
                u8 cnt = getConsecutiveMoveCount( p_move.m_user.first, p_move.m_user.second );

                if( cnt == 0
                    || getLastUsedMove( p_move.m_user.first, p_move.m_user.second ).m_param
                           != M_FURY_CUTTER ) {
                    movePower = 40;
                } else if( cnt == 1 ) {
                    movePower = 80;
                } else {
                    movePower = 160;
                }
            }
            if( p_move.m_param == M_ROLLOUT || p_move.m_param == M_ICE_BALL ) {
                u8 cnt = getConsecutiveMoveCount( p_move.m_user.first, p_move.m_user.second );

                if( cnt == 0
                    || getLastUsedMove( p_move.m_user.first, p_move.m_user.second ).m_param
                           != p_move.m_param ) {
                    movePower = 30;
                } else if( cnt == 1 ) {
                    movePower = 60;
                } else if( cnt == 2 ) {
                    movePower = 120;
                } else if( cnt == 3 ) {
                    movePower = 240;
                } else {
                    movePower = 480;
                }

                if( getVolatileStatus( p_move.m_user.first, p_move.m_user.second )
                    & VS_DEFENSECURL ) {
                    movePower <<= 1;
                }
            }
            if( p_move.m_param == M_SPIT_UP ) {
                u8 stkpile = getVolatileStatusCounter( p_move.m_user.first, p_move.m_user.second,
                                                       VS_STOCKPILE );
                if( stkpile > 3 ) { stkpile = 3; }

                movePower = 100 * stkpile;

                removeVolatileStatus( p_ui, p_move.m_user.first, p_move.m_user.second,
                                      VS_STOCKPILE );
                boosts bt = boosts( );
                bt.setBoost( DEF, -stkpile );
                bt.setBoost( SDEF, -stkpile );
                auto res = addBoosts( p_move.m_user.first, p_move.m_user.second, bt );
                p_ui->logBoosts( getPkmnOrDisguise( p_move.m_user.first, p_move.m_user.second ),
                                 p_move.m_user.first, p_move.m_user.second, bt, res );
            }

            // TODO
            // Stored power, power trip: 20 + 20 * atk stat boost
            // punishment: 60 + 20 * def stat boost

            // acrobatics
            // assurance
            // avalanche / revenge
            // grass / fire / water pledge
            // hex
            // payback
            // round
            // smelling salts
            // stomping tantrum
            // wake-up slap
            //
            // fling, natural gift
            //
            // beat up
            // echoed voice
            // magnitude
            // present
            // triple kick
            // trump card

            damage *= movePower;

            damage = ( damage * atk ) / def;
            damage = ( damage / 50 ) + 2;

            if( p_move.m_target.size( ) > 1 ) { damage = ( damage * 75 ) / 100; }

            if( !suppressesWeather( ) ) [[likely]] {
                if( p_move.m_moveData.m_type == TYPE_WATER
                    && ( _weather == WE_RAIN || _weather == WE_HEAVY_RAIN ) ) [[unlikely]] {
                    damage = ( damage * 3 ) >> 1;
                }
                if( p_move.m_moveData.m_type == TYPE_FIRE
                    && ( _weather == WE_RAIN || _weather == WE_HEAVY_RAIN ) ) [[unlikely]] {
                    damage = ( damage >> 1 );
                }
                if( p_move.m_moveData.m_type == TYPE_FIRE
                    && ( _weather == WE_SUN || _weather == WE_HEAVY_SUNSHINE ) ) [[unlikely]] {
                    damage = ( damage * 3 ) >> 1;
                }
                if( p_move.m_moveData.m_type == TYPE_WATER
                    && ( _weather == WE_SUN || _weather == WE_HEAVY_SUNSHINE ) ) [[unlikely]] {
                    damage = ( damage >> 1 );
                }
            }

            if( p_critical ) { damage = ( damage * 3 ) >> 1; }

            u8 rnd = rand( ) & 15;
            damage = ( damage * ( 85 + rnd ) / 100 );

            // stab
            type moveType = getMoveType( p_move );

            if( !supprAbs
                && ( user->getAbility( ) == A_PROTEAN || user->getAbility( ) == A_LIBERO ) ) {
                p_ui->logAbility( getPkmnOrDisguise( p_move.m_user.first, p_move.m_user.second ),
                                  p_move.m_user.first );
                setType( p_ui, p_move.m_user.first, p_move.m_user.second, moveType );
                // TODO log
            }

            if( hasType( p_move.m_user.first, p_move.m_user.second, moveType ) ) {
                if( !supprAbs && user->getAbility( ) == A_ADAPTABILITY ) {
                    damage <<= 1;
                } else {
                    damage = ( damage * 3 ) >> 1;
                }
            }

            // effectiveness
            damage = ( damage * effectiveness ) / 100;

            // burn

            if( user->m_status.m_isBurned && p_move.m_moveData.m_category == MH_PHYSICAL
                && ( supprAbs || user->getAbility( ) != A_GUTS ) && p_move.m_param != M_FACADE ) {
                damage >>= 1;
            }

            damage = damage * p_damageModifier / 100;

            auto sidec = getSideCondition( !p_move.m_user.first );

            if( supprAbs || !anyHasAbility( A_SCREEN_CLEANER ) ) {
                if( ( sidec & SC_AURORAVEIL )
                    || ( ( sidec & SC_REFLECT )
                         && p_move.m_moveData.m_defensiveCategory == MH_PHYSICAL )
                    || ( ( sidec & SC_LIGHTSCREEN )
                         && p_move.m_moveData.m_defensiveCategory == MH_SPECIAL ) ) {
                    // nerf intentional!
                    damage = ( damage * 66 ) / 100;
                }
            }

            if( p_move.m_param == M_BODY_SLAM || p_move.m_param == M_DRAGON_RUSH
                || p_move.m_param == M_FLYING_PRESS || p_move.m_param == M_HEAT_CRASH
                || p_move.m_param == M_HEAVY_SLAM || p_move.m_param == M_PHANTOM_FORCE
                || p_move.m_param == M_SHADOW_FORCE || p_move.m_param == M_STOMP ) {
                if( getVolatileStatus( p_target.first, p_target.second ) & VS_MINIMIZE )
                    [[unlikely]] {
                    damage <<= 1;
                }
            }

            if( p_move.m_param == M_SURF || p_move.m_param == M_WHIRLPOOL ) {
                if( getVolatileStatus( p_target.first, p_target.second ) & VS_DIVING ) {
                    damage <<= 1;
                }
            }

            if( p_move.m_param == M_EARTHQUAKE || p_move.m_param == M_MAGNITUDE ) {
                if( getVolatileStatus( p_target.first, p_target.second ) & VS_DIGGING ) {
                    damage <<= 1;
                }
            }

            if( p_move.m_param == M_GUST || p_move.m_param == M_TWISTER ) {
                if( getVolatileStatus( p_target.first, p_target.second ) & VS_INAIR ) {
                    damage <<= 1;
                }
            }

            // abilities

            // TODO
            // aura break
            // iron fist, reckless
            // battery
            // sheer force, sand force, analytic, tough claws
            // fairy aura, dark aura
            // technician
            // flare boost
            // toxic boost
            // strong jaw
            // mega launcher
            // heatproof
            // dry skin
            //
            // Muscle band
            // wise glasses
            //
            // plates, gems, incenses
            // adamant orb, lustrous orb, griseous orb, soul dew
            // normal gem
            // solar beam/blade for non-sunny
            // me first
            // knock off
            // helping hand
            // charge
            // brine
            // venoshock
            // retaliate
            // fusion bolt/flare
            // terrains
            // mud sport / water sport

            // foul play
            //
            // water bubble/stakeout
            // thick fat/water bubble (def)
            //
            // chip away/ sacred sword
            // wonder room
            //
            //  . . .

            if( !supprAbs ) {
                switch( target->getAbility( ) ) {
                case A_FLUFFY:
                    if( p_move.m_moveData.m_flags & MF_CONTACT ) {
                        if( moveType == TYPE_FIRE ) {
                            damage <<= 1;
                        } else {
                            damage >>= 1;
                        }
                    }
                    break;
                case A_FILTER:
                case A_PRISM_ARMOR:
                case A_SOLID_ROCK:
                    if( effectiveness > 100 ) { damage = ( damage * 75 ) / 100; }
                    break;
                case A_SHADOW_SHIELD:
                case A_MULTISCALE:
                    if( target->m_stats.m_curHP == target->m_stats.m_maxHP ) { damage >>= 1; }
                    break;
                case A_ICE_SCALES:
                    if( p_move.m_moveData.m_defensiveCategory == MH_SPECIAL ) { damage >>= 1; }
                    break;
                case A_PUNK_ROCK:
                    if( p_move.m_moveData.m_flags & MF_SOUND ) { damage >>= 1; }
                    break;
                [[likely]] default:
                    break;
                }

                if( getPkmn( p_target.first, !p_target.second ) != nullptr
                    && getPkmn( p_target.first, !p_target.second )->getAbility( )
                           == A_FRIEND_GUARD ) [[unlikely]] {
                    damage = ( damage * 75 ) / 100;
                }

                switch( user->getAbility( ) ) {
                case A_TORRENT:
                    if( moveType == TYPE_WATER
                        && user->m_stats.m_curHP * 3 < user->m_stats.m_maxHP ) {
                        damage = ( 3 * damage ) >> 1;
                    }
                    break;
                case A_OVERGROW:
                    if( moveType == TYPE_GRASS
                        && user->m_stats.m_curHP * 3 < user->m_stats.m_maxHP ) {
                        damage = ( 3 * damage ) >> 1;
                    }
                    break;
                case A_BLAZE:
                    if( moveType == TYPE_FIRE
                        && user->m_stats.m_curHP * 3 < user->m_stats.m_maxHP ) {
                        damage = ( 3 * damage ) >> 1;
                    }
                    break;
                case A_SWARM:
                    if( moveType == TYPE_BUG
                        && user->m_stats.m_curHP * 3 < user->m_stats.m_maxHP ) {
                        damage = ( 3 * damage ) >> 1;
                    }
                    break;
                case A_STEELWORKER:
                    if( moveType == TYPE_STEEL ) { damage = ( 3 * damage ) >> 1; }
                    break;

                case A_RIVALRY:
                    if( user->gender( ) * target->gender( ) < 0 ) {
                        damage = ( damage * 75 ) / 100;
                    } else if( user->gender( ) * target->gender( ) > 0 ) {
                        damage = ( damage * 125 ) / 100;
                    }
                    break;
                case A_NEUROFORCE:
                    if( effectiveness > 100 ) { damage = ( damage * 125 ) / 100; }
                    break;
                case A_TINTED_LENS:
                    if( effectiveness < 100 ) { damage <<= 1; }
                    break;
                case A_SNIPER:
                    if( p_critical ) { damage = ( 3 * damage ) >> 1; }
                    break;

                [[likely]] default:
                    break;
                }
            }

            if( canUseItem( p_move.m_user.first, p_move.m_user.second, !supprAbs ) ) {
                switch( user->getItem( ) ) {
                case I_EXPERT_BELT:
                    if( effectiveness > 100 ) { damage = ( damage * 120 ) / 100; }
                    break;
                case I_LIFE_ORB:
                    if( effectiveness > 100 ) { damage = ( damage * 130 ) / 100; }
                    break;
                case I_METRONOME: {
                    auto cons = 100
                                + 20
                                      * std::min( u8( 5 ),
                                                  getConsecutiveMoveCount( p_move.m_user.first,
                                                                           p_move.m_user.second ) );
                    damage = ( damage * cons / 100 );
                    break;
                }
                default: break;
                }
            }

            if( canUseItem( p_target.first, p_target.second, !supprAbs )
                && ( !supprAbs
                     || !_sides[ p_target.first ? PLAYER_SIDE : OPPONENT_SIDE ].anyHasAbility(
                         A_UNNERVE ) ) ) {
                bool eatitem = false;
                switch( target->getItem( ) ) {
                case I_CHILAN_BERRY:
                    if( moveType == TYPE_NORMAL ) { eatitem = true; }
                    break;
                case I_BABIRI_BERRY:
                    if( moveType == TYPE_STEEL && effectiveness > 100 ) { eatitem = true; }
                    break;
                case I_CHARTI_BERRY:
                    if( moveType == TYPE_ROCK && effectiveness > 100 ) { eatitem = true; }
                    break;
                case I_CHOPLE_BERRY:
                    if( moveType == TYPE_FIGHTING && effectiveness > 100 ) { eatitem = true; }
                    break;
                case I_COBA_BERRY:
                    if( moveType == TYPE_FLYING && effectiveness > 100 ) { eatitem = true; }
                    break;
                case I_COLBUR_BERRY:
                    if( moveType == TYPE_DARKNESS && effectiveness > 100 ) { eatitem = true; }
                    break;
                case I_HABAN_BERRY:
                    if( moveType == TYPE_DRAGON && effectiveness > 100 ) { eatitem = true; }
                    break;
                case I_KASIB_BERRY:
                    if( moveType == TYPE_GHOST && effectiveness > 100 ) { eatitem = true; }
                    break;
                case I_KEBIA_BERRY:
                    if( moveType == TYPE_POISON && effectiveness > 100 ) { eatitem = true; }
                    break;
                case I_OCCA_BERRY:
                    if( moveType == TYPE_FIRE && effectiveness > 100 ) { eatitem = true; }
                    break;
                case I_PASSHO_BERRY:
                    if( moveType == TYPE_WATER && effectiveness > 100 ) { eatitem = true; }
                    break;
                case I_PAYAPA_BERRY:
                    if( moveType == TYPE_PSYCHIC && effectiveness > 100 ) { eatitem = true; }
                    break;
                case I_RINDO_BERRY:
                    if( moveType == TYPE_GRASS && effectiveness > 100 ) { eatitem = true; }
                    break;
                case I_ROSELI_BERRY:
                    if( moveType == TYPE_FAIRY && effectiveness > 100 ) { eatitem = true; }
                    break;
                case I_SHUCA_BERRY:
                    if( moveType == TYPE_GROUND && effectiveness > 100 ) { eatitem = true; }
                    break;
                case I_TANGA_BERRY:
                    if( moveType == TYPE_BUG && effectiveness > 100 ) { eatitem = true; }
                    break;
                case I_WACAN_BERRY:
                    if( moveType == TYPE_LIGHTNING && effectiveness > 100 ) { eatitem = true; }
                    break;
                case I_YACHE_BERRY:
                    if( moveType == TYPE_ICE && effectiveness > 100 ) { eatitem = true; }
                    break;
                default: break;
                }
                if( eatitem ) {
                    p_ui->logItem( target, p_target.first );
                    if( target->getAbility( ) != A_RIPEN || supprAbs ) {
                        damage >>= 1;
                    } else {
                        damage >>= 2;
                    }
                    auto fmt = std::string( GET_STRING( 279 ) );
                    snprintf( buffer, TMP_BUFFER_SIZE, fmt.c_str( ),
                              p_ui->getPkmnName( getPkmnOrDisguise( p_move.m_user.first,
                                                                    p_move.m_user.second ),
                                                 p_move.m_user.first )
                                  .c_str( ),
                              FS::getItemName( target->getItem( ) ).c_str( ) );
                    p_ui->log( buffer );

                    removeItem( p_ui, p_target.first, p_target.second );
                    checkOnEatBerry( p_ui, p_target.first, p_target.second, target->getItem( ) );
                }
            }

            if( !damage ) { damage = 1; }
            if( damage > 9000 ) { damage = 9000; }
        }

        p_ui->animateHitPkmn( p_target.first, p_target.second, effectiveness );

        if( damage >= target->m_stats.m_curHP ) { damage = target->m_stats.m_curHP; }

        if( p_move.m_moveData.m_flags & MF_NOFAINT ) {
            if( damage >= target->m_stats.m_curHP ) { damage = target->m_stats.m_curHP - 1; }
        }

        // Check for focus sash etc
        // focus band: 10% to survive fatal hit
        // focus sash
        if( damage >= target->m_stats.m_curHP ) {
            if( target->m_stats.m_curHP == target->m_stats.m_maxHP ) {
                if( canUseItem( p_target.first, p_target.second, !supprAbs )
                    && target->getItem( ) == I_FOCUS_SASH ) {
                    damage = target->m_stats.m_curHP - 1;
                    p_ui->logItem( getPkmnOrDisguise( p_target.first, p_target.second ),
                                   p_target.first );
                    // consume item
                    removeItem( p_ui, p_target.first, p_target.second );
                }
                if( canUseItem( p_target.first, p_target.second, !supprAbs )
                    && target->getItem( ) == I_FOCUS_BAND && rand( ) % 100 < 10 ) {
                    damage = target->m_stats.m_curHP - 1;
                    p_ui->logItem( getPkmnOrDisguise( p_target.first, p_target.second ),
                                   p_target.first );
                }

                if( !supprAbs && target->getAbility( ) == A_STURDY ) {
                    damage = target->m_stats.m_curHP - 1;
                    p_ui->logAbility( getPkmnOrDisguise( p_target.first, p_target.second ),
                                      p_target.first );
                }
            }
        }

        // check for substitute (TODO) and A_DISGUISE
        if( !supprAbs && target->getAbility( ) == A_DISGUISE && !target->getForme( ) ) {
            p_ui->logAbility( getPkmnOrDisguise( p_target.first, p_target.second ),
                              p_target.first );
            target->setBattleForme( 1 );
            p_ui->updatePkmn( p_target.first, p_target.second,
                              getPkmnOrDisguise( p_target.first, p_target.second ) );
        } else {
            damagePokemon( p_ui, p_target.first, p_target.second, damage );

            if( effectiveness > 100 ) {
                auto fmt = std::string( GET_STRING( 285 ) );
                snprintf( buffer, TMP_BUFFER_SIZE, fmt.c_str( ),
                          p_ui->getPkmnName( getPkmnOrDisguise( p_target.first, p_target.second ),
                                             p_target.first, false )
                              .c_str( ) );
                p_ui->log( buffer );
            } else if( effectiveness < 100 ) {
                auto fmt = std::string( GET_STRING( 286 ) );
                snprintf( buffer, TMP_BUFFER_SIZE, fmt.c_str( ),
                          p_ui->getPkmnName( getPkmnOrDisguise( p_target.first, p_target.second ),
                                             p_target.first, false )
                              .c_str( ) );
                p_ui->log( buffer );
            }

            if( p_critical ) { p_ui->log( GET_STRING( 291 ) ); }

            // Calculate recoil / drain

            if( p_move.m_moveData.m_drain ) {
                u32 amount = damage * p_move.m_moveData.m_drain / 240;
                if( canUseItem( p_move.m_user.first, p_move.m_user.second )
                    && user->getItem( ) == I_BIG_ROOT ) {
                    amount = 13 * amount / 10;
                }
                if( !amount ) { amount = 1; }

                if( !suppressesAbilities( ) && target->getAbility( ) == A_LIQUID_OOZE ) {
                    p_ui->logAbility( getPkmnOrDisguise( p_target.first, p_target.second ),
                                      p_target.first );
                    damagePokemon( p_ui, p_move.m_user.first, p_move.m_user.second, amount );
                } else if( !( userVolStat & VS_HEALBLOCK ) ) {
                    if( user->m_stats.m_curHP < user->m_stats.m_maxHP ) {
                        healPokemon( p_ui, p_move.m_user.first, p_move.m_user.second, amount );

                        auto fmt = std::string( GET_STRING( 288 ) );
                        snprintf( buffer, TMP_BUFFER_SIZE, fmt.c_str( ),
                                  p_ui->getPkmnName( getPkmnOrDisguise( p_move.m_user.first,
                                                                        p_move.m_user.second ),
                                                     p_move.m_user.first )
                                      .c_str( ) );
                        p_ui->log( buffer );
                    }
                }
            }

            if( p_move.m_moveData.m_recoil ) {
                u32 amount = damage * p_move.m_moveData.m_recoil / 240;
                if( !amount ) { amount = 1; }

                damagePokemon( p_ui, p_move.m_user.first, p_move.m_user.second, amount );

                auto fmt = std::string( GET_STRING( 287 ) );
                snprintf( buffer, TMP_BUFFER_SIZE, fmt.c_str( ),
                          p_ui->getPkmnName(
                                  getPkmnOrDisguise( p_move.m_user.first, p_move.m_user.second ),
                                  p_move.m_user.first )
                              .c_str( ) );
                p_ui->log( buffer );
            }

            // Shell bell
            if( user->canBattle( ) && canUseItem( p_move.m_user.first, p_move.m_user.second )
                && user->getItem( ) == I_SHELL_BELL ) {
                if( !( userVolStat & VS_HEALBLOCK ) ) {
                    u32 amount = damage / 8;
                    if( !amount ) { amount = 1; }

                    healPokemon( p_ui, p_move.m_user.first, p_move.m_user.second, amount );

                    auto fmt = std::string( GET_STRING( 533 ) );
                    snprintf( buffer, TMP_BUFFER_SIZE, fmt.c_str( ),
                              p_ui->getPkmnName( getPkmnOrDisguise( p_move.m_user.first,
                                                                    p_move.m_user.second ),
                                                 p_move.m_user.first )
                                  .c_str( ) );
                    p_ui->log( buffer );
                }
            }

            // Life orb recoil
            if( user->canBattle( ) && canUseItem( p_move.m_user.first, p_move.m_user.second )
                && user->getItem( ) == I_LIFE_ORB ) {

                damagePokemon( p_ui, p_move.m_user.first, p_move.m_user.second,
                               user->m_stats.m_maxHP / 16 );

                auto fmt = std::string( GET_STRING( 306 ) );
                snprintf( buffer, TMP_BUFFER_SIZE, fmt.c_str( ),
                          p_ui->getPkmnName(
                                  getPkmnOrDisguise( p_move.m_user.first, p_move.m_user.second ),
                                  p_move.m_user.first )
                              .c_str( ) );
                p_ui->log( buffer );
            }

            // check for things that trigger when causing damage
            checkOnTakeDamage( p_ui, p_move, p_target, damage, effectiveness );
        }

        if( !getPkmn( p_target.first, p_target.second )->canBattle( ) ) {
            // pkmn fainted
            faintPokemon( p_ui, p_target.first, p_target.second );
        }

        // Check if user fainted
        if( !user->canBattle( ) ) {
            faintPokemon( p_ui, p_move.m_user.first, p_move.m_user.second );
        }

        return true;
    }

    void field::executeBattleMove( battleUI* p_ui, battleMove p_move,
                                   const std::vector<battleMove>& p_targetsMoves,
                                   const std::vector<battleMove>& p_tergetedMoves ) {

        (void) p_targetsMoves;
        (void) p_tergetedMoves;

        constexpr u8 TMP_BUFFER_SIZE = 100;
        char         buffer[ TMP_BUFFER_SIZE + 10 ];
        bool         opponent = p_move.m_user.first;
        u8           slot     = p_move.m_user.second;

        auto slotc = getVolatileStatus( opponent, slot );

        if( getSlotStatus( opponent, slot ) != slot::status::NORMAL ) { return; }

        // Check if the move fails
        if( u8 err = useMove( p_ui, p_move ); err != MOVE_SUCCESS ) {
            // Move failed (e.g. due to confusion, sleep etc.
            if( err != MOVE_FAIL_NO_PP ) {
                // don't reduce pp for asleep or frozen pkmn
                deducePP( opponent, slot, p_move.m_param );
            }

            if( slotc & VS_MOVECHARGE ) { // Move is already charged
                removeVolatileStatus( p_ui, opponent, slot,
                                      volatileStatus( VS_MOVECHARGE | VS_DIGGING | VS_DIVING
                                                      | VS_INAIR | VS_INVISIBLE ) );
                removeLockedMove( opponent, slot );
                p_ui->showPkmn( opponent, slot );
            }

            setLastUsedMove( opponent, slot, battleMove( ) );
            WAIT( HALF_SEC );
            return;
        }

        if( p_move.m_moveData.m_flags & MF_CHARGE ) {
            if( slotc & VS_MOVECHARGE ) { // Move is already charged
                removeVolatileStatus( p_ui, opponent, slot,
                                      volatileStatus( VS_MOVECHARGE | VS_DIGGING | VS_DIVING
                                                      | VS_INAIR | VS_INVISIBLE ) );
                removeLockedMove( opponent, slot );
                p_ui->showPkmn( opponent, slot );
            } else {
                auto tg = getPkmn( opponent, slot );
                if( tg == nullptr ) [[unlikely]] { return; }

                p_ui->prepareMove( getPkmnOrDisguise( opponent, slot ), opponent, slot, p_move );
                WAIT( HALF_SEC );

                if( ( p_move.m_param == M_SOLAR_BLADE || p_move.m_param == M_SOLAR_BEAM )
                    && !suppressesWeather( )
                    && ( _weather == WE_SUN || _weather == WE_HEAVY_SUNSHINE ) ) [[unlikely]] {
                    // empty!
                } else if( canUseItem( opponent, slot ) && tg->getItem( ) == I_POWER_HERB )
                    [[unlikely]] {
                    p_ui->logItem( getPkmnOrDisguise( opponent, slot ), opponent );
                    auto fmt = std::string( GET_STRING( 305 ) );
                    snprintf( buffer, TMP_BUFFER_SIZE, fmt.c_str( ),
                              p_ui->getPkmnName( getPkmnOrDisguise( opponent, slot ), opponent )
                                  .c_str( ) );
                    p_ui->log( buffer );

                    removeItem( p_ui, opponent, slot );
                } else {
                    addVolatileStatus( p_ui, opponent, slot, VS_MOVECHARGE, 255 );
                    switch( p_move.m_param ) {
                    case M_DIVE: addVolatileStatus( p_ui, opponent, slot, VS_DIVING, 255 ); break;
                    case M_DIG: addVolatileStatus( p_ui, opponent, slot, VS_DIGGING, 255 ); break;
                    case M_FLY:
                    case M_BOUNCE:
                    case M_SKY_DROP:
                        addVolatileStatus( p_ui, opponent, slot, VS_INAIR, 255 );
                        break;
                    case M_PHANTOM_FORCE:
                    case M_SHADOW_FORCE:
                        addVolatileStatus( p_ui, opponent, slot, VS_INVISIBLE, 255 );
                        break;

                    default: break;
                    }

                    battleMoveSelection bms = NO_OP_SELECTION;
                    bms.m_type              = MT_ATTACK;
                    bms.m_param             = p_move.m_param;
                    bms.m_moveData          = p_move.m_moveData;
                    if( p_move.m_target.size( ) != 1 ) [[unlikely]] {
#ifdef DESQUID
                        p_ui->log( "[error] Multi-turn attack has multiple targets." );
#endif
                    } else {
                        bms.m_target = p_move.m_target[ 0 ];
                    }
                    bms.m_user = p_move.m_user;

                    addLockedMove( opponent, slot, bms );
                    WAIT( HALF_SEC );
                    return;
                }
            }
        }

        if( !getLockedMoveCount( opponent, slot ) ) { deducePP( opponent, slot, p_move.m_param ); }
        auto fmt = std::string( GET_STRING( 10 ) );
        snprintf( buffer, TMP_BUFFER_SIZE, fmt.c_str( ),
                  p_ui->getPkmnName( getPkmnOrDisguise( opponent, slot ), opponent ).c_str( ),
                  FS::getMoveName( p_move.m_param ).c_str( ) );
        p_ui->log( buffer );

        bool moveHadTarget = _mode == BM_SINGLE;
        for( u8 i = 0; i < p_move.m_target.size( ); ++i ) {
            // Check if the move needs to be redirected

            auto curTg = getPkmn( p_move.m_target[ i ].first, p_move.m_target[ i ].second );
            if( curTg == nullptr ) {
                if( p_move.m_target.size( ) == 1 && _mode == BM_DOUBLE
                    && p_move.m_target[ i ].first != opponent ) {
                    // in double battles, moves targeting a single opponent get redirected
                    // to an opponent that actually exists.
                    // If both opponents fainted, the move will fail
                    p_move.m_target[ i ].second ^= 1;
                    curTg = getPkmn( p_move.m_target[ i ].first, p_move.m_target[ i ].second );
                    if( curTg == nullptr ) {
                        // both opposing pkmn fainted
                        break;
                    }
                } else {
                    continue;
                }
            }
            moveHadTarget = true;

            // Check for multi-hit moves
            u8 numHits = 1, strengthMod = 100;
            if( p_move.m_moveData.getMultiHitMax( ) > 1 ) [[unlikely]] {
                numHits = p_move.m_moveData.getMultiHitMin( );
                numHits += rand( )
                           % ( p_move.m_moveData.getMultiHitMax( )
                               - p_move.m_moveData.getMultiHitMin( ) );

                if( !suppressesAbilities( )
                    && getPkmn( opponent, slot )->getAbility( ) == A_SKILL_LINK ) [[unlikely]] {
                    numHits = p_move.m_moveData.getMultiHitMax( );
                }
            } else if( !suppressesAbilities( )
                       && getPkmn( opponent, slot )->getAbility( ) == A_PARENTAL_BOND
                       && p_move.m_moveData.m_category != MH_STATUS ) [[unlikely]] {
                numHits     = 2;
                strengthMod = 25;
            }
            bool multihit = numHits > 1;

            u8 hits = 0;
            for( u8 j = 0; j < numHits; ++j ) {
                auto tgsc
                    = getVolatileStatus( p_move.m_target[ i ].first, p_move.m_target[ i ].second );

                // Check if the move is protected against
                bool protect = false;
                if( ( tgsc & VS_PROTECT ) || ( tgsc & VS_OBSTRUCT ) || ( tgsc & VS_SPIKYSHIELD )
                    || ( p_move.m_moveData.m_category != MH_STATUS && ( tgsc & VS_KINGSSHIELD ) ) )
                    [[unlikely]] {
                    protect = true;
                    if( ( p_move.m_moveData.m_flags & MF_PROTECT ) ) {
                        fmt = std::string( GET_STRING( 674 ) );
                        snprintf(
                            buffer, TMP_BUFFER_SIZE, fmt.c_str( ),
                            p_ui->getPkmnName( getPkmnOrDisguise( p_move.m_target[ i ].first,
                                                                  p_move.m_target[ i ].second ),
                                               p_move.m_target[ i ].first )
                                .c_str( ) );
                        p_ui->log( buffer );
                    }
                }

                if( protect && ( p_move.m_moveData.m_flags & MF_BREAKSPROTECT ) ) {
                    // remove protect
                    removeVolatileStatus( p_ui, p_move.m_target[ i ].first,
                                          p_move.m_target[ i ].second, VS_PROTECT );
                    tgsc = getVolatileStatus( p_move.m_target[ i ].first,
                                              p_move.m_target[ i ].second );
                    fmt  = std::string( GET_STRING( 675 ) );
                    snprintf( buffer, TMP_BUFFER_SIZE, fmt.c_str( ),
                              p_ui->getPkmnName( getPkmnOrDisguise( p_move.m_target[ i ].first,
                                                                    p_move.m_target[ i ].second ),
                                                 p_move.m_target[ i ].first )
                                  .c_str( ) );
                    p_ui->log( buffer );
                    protect = false;
                }

                if( ( p_move.m_moveData.m_flags & MF_PROTECT ) && !protect ) [[likely]] {
                    bool critical = executeCriticalCheck( p_ui, p_move, p_move.m_target[ i ] );

                    // Check if the move misses
                    if( moveMisses( p_ui, p_move, p_move.m_target[ i ], critical ) ) {
                        fmt = std::string( GET_STRING( 280 ) );
                        snprintf(
                            buffer, TMP_BUFFER_SIZE, fmt.c_str( ),
                            p_ui->getPkmnName( getPkmnOrDisguise( p_move.m_target[ i ].first,
                                                                  p_move.m_target[ i ].second ),
                                               p_move.m_target[ i ].first )
                                .c_str( ) );
                        p_ui->log( buffer );
                        // check for crash damage

                        if( p_move.m_moveData.m_flags & MF_CRASHDAMAGE ) {
                            u16 maxHP = getPkmn( opponent, slot )->m_stats.m_maxHP;
                            damagePokemon( p_ui, opponent, slot, maxHP / 2 );
                            fmt = std::string( GET_STRING( 546 ) );
                            snprintf(
                                buffer, TMP_BUFFER_SIZE, fmt.c_str( ),
                                p_ui->getPkmnName( getPkmnOrDisguise( opponent, slot ), opponent )
                                    .c_str( ) );
                            p_ui->log( buffer );

                            if( !getPkmn( opponent, slot )->canBattle( ) ) {
                                faintPokemon( p_ui, opponent, slot );
                            }
                        }

                        continue;
                    } else if( p_move.m_moveData.m_flags & MF_OHKO ) [[unlikely]] {
                        faintPokemon( p_ui, p_move.m_target[ i ].first,
                                      p_move.m_target[ i ].second );
                        p_ui->log( GET_STRING( 547 ) );
                    } else if( p_move.m_moveData.m_category != MH_STATUS
                               && !executeDamagingMove(
                                   p_ui, p_move, p_move.m_target[ i ], critical,
                                   j == 1 && strengthMod < 100 ? strengthMod : 100 ) ) {
                        break;
                    }
                    hits++;
                }

                // Pkmn fainted
                if( getSlotStatus( opponent, slot ) == slot::status::FAINTED ) { break; }

                executeSelfStatusEffects( p_ui, p_move, p_move.m_target[ i ] );

                // Target fainted
                if( getSlotStatus( p_move.m_target[ i ].first, p_move.m_target[ i ].second )
                    == slot::status::FAINTED ) {
                    break;
                }

                if( !protect ) [[likely]] {
                    executeStatusEffects( p_ui, p_move, p_move.m_target[ i ] );

                    // Check for secondary status effects
                    if( p_move.m_moveData.m_secondaryStatus
                        && ( rand( ) % 100 < p_move.m_moveData.m_secondaryChance ) ) {
                        executeSecondaryStatus( p_ui, p_move, p_move.m_target[ i ] );
                    }
                    // Other secondary effects
                    if( rand( ) % 100 < p_move.m_moveData.m_secondaryChance ) {
                        executeSecondaryEffects( p_ui, p_move, p_move.m_target[ i ] );
                    }
                }

                // Pkmn fainted
                if( getSlotStatus( opponent, slot ) == slot::status::FAINTED ) { break; }
                // Target fainted
                if( getSlotStatus( p_move.m_target[ i ].first, p_move.m_target[ i ].second )
                    == slot::status::FAINTED ) {
                    break;
                }

                if( p_move.m_moveData.m_flags & MF_CONTACT ) {
                    // move makes contact
                    executeContact( p_ui, p_move, p_move.m_target[ i ] );
                }

                // Pkmn fainted
                if( getSlotStatus( opponent, slot ) == slot::status::FAINTED ) { break; }
                // Target fainted
                if( getSlotStatus( p_move.m_target[ i ].first, p_move.m_target[ i ].second )
                    == slot::status::FAINTED ) {
                    break;
                }

                if( p_move.m_moveData.m_flags & MF_FORCESWITCH ) {
                    // move forces switch out
                    if( _isWildBattle && p_move.m_target[ i ].first != opponent ) {
                        // recall opposing pkmn to end battle
                        recallPokemon( p_ui, OPPONENT_SIDE, 0 );
                        break;
                    }

                    // TODO: should only work if opponent has > 1 pkmn that can battle
                    recallPokemon( p_ui, p_move.m_target[ i ].first, p_move.m_target[ i ].second );
                    break;
                }

                if( p_move.m_moveData.m_flags & MF_DEFROSTTARGET ) {
                    if( hasStatusCondition( p_move.m_target[ i ].first, p_move.m_target[ i ].second,
                                            FROZEN ) ) {
                        fmt = std::string( GET_STRING( 298 ) );
                        snprintf(
                            buffer, TMP_BUFFER_SIZE, fmt.c_str( ),
                            p_ui->getPkmnName( getPkmnOrDisguise( p_move.m_target[ i ].first,
                                                                  p_move.m_target[ i ].second ),
                                               p_move.m_target[ i ].first )
                                .c_str( ) );
                        p_ui->log( buffer );
                        removeStatusCondition( p_move.m_target[ i ].first,
                                               p_move.m_target[ i ].second );
                        p_ui->updatePkmnStats( p_move.m_target[ i ].first,
                                               p_move.m_target[ i ].second,
                                               getPkmnOrDisguise( p_move.m_target[ i ].first,
                                                                  p_move.m_target[ i ].second ) );
                        WAIT( HALF_SEC );
                    }
                }

                // check for held item
                checkItemAfterAttack( p_ui, p_move.m_target[ i ].first,
                                      p_move.m_target[ i ].second );
                checkItemAfterAttack( p_ui, opponent, slot );
            }

            if( multihit && hits >= 1 ) {
                snprintf( buffer, TMP_BUFFER_SIZE, GET_STRING( 400 ), hits );
                p_ui->log( buffer );
            }

            if( getSlotStatus( opponent, slot ) == slot::status::FAINTED ) { continue; }
        }

        if( !moveHadTarget ) {
            p_ui->log( GET_STRING( IO::STR_UI_BATTLE_IT_FAILED ) ); // "It failed."
            return;
        }

        // Check for flags and stuff

        if( p_move.m_moveData.m_flags & MF_DANCE ) {
            // Check if any pkmn has the "dancer" ability
            // TODO
        }

        if( p_move.m_moveData.m_flags & MF_MINDBLOWNRECOIL ) {
            // pkmn loses half of its max hp
            u16 maxHP = getPkmn( opponent, slot )->m_stats.m_maxHP;
            damagePokemon( p_ui, opponent, slot, maxHP / 2 );

            if( !getPkmn( opponent, slot )->canBattle( ) ) {
                faintPokemon( p_ui, opponent, slot );
                return;
            }
        }

        if( p_move.m_moveData.m_flags & MF_RECHARGE ) {
            battleMoveSelection bms = NO_OP_SELECTION;
            bms.m_type              = MT_ATTACK;
            addLockedMove( opponent, slot, bms, 0 );
            addVolatileStatus( p_ui, opponent, slot, VS_RECHARGE, 255 );
        }

        if( p_move.m_moveData.m_flags & MF_LOCKEDMOVE ) {
            auto                lmcnt = getLockedMoveCount( opponent, slot );
            battleMoveSelection bms   = NO_OP_SELECTION;
            bms.m_type                = MT_ATTACK;
            bms.m_param               = p_move.m_param;
            bms.m_moveData            = p_move.m_moveData;
            bms.m_target              = { 255, 255 };
            bms.m_user                = p_move.m_user;
            if( !lmcnt ) {
                addLockedMove( opponent, slot, bms, 1 + rand( ) % 2 );
            } else if( --lmcnt ) {
                addLockedMove( opponent, slot, bms, lmcnt );
            } else {
                removeLockedMove( opponent, slot );
                if( suppressesAbilities( )
                    || getPkmn( opponent, slot )->getAbility( ) != A_OWN_TEMPO
                    || ( getTerrain( ) == TR_MISTYTERRAIN && isGrounded( opponent, slot ) )
                    || ( getVolatileStatus( opponent, slot ) & VS_SUBSTITUTE ) ) {
                    fmt = std::string( GET_STRING( 277 ) );
                    snprintf( buffer, TMP_BUFFER_SIZE, fmt.c_str( ),
                              p_ui->getPkmnName( getPkmnOrDisguise( opponent, slot ), opponent )
                                  .c_str( ) );
                    p_ui->log( buffer );

                    if( getVolatileStatus( opponent, slot ) & VS_CONFUSION ) {
                        // empty!
                    } else {
                        addVolatileStatus( p_ui, opponent, slot, VS_CONFUSION,
                                           250 + 2 + ( rand( ) % 4 ) );
                    }
                }
            }
        }

        if( p_move.m_moveData.m_flags & MF_ROOST ) {
            addVolatileStatus( p_ui, opponent, slot, VS_ROOST, 2 );
        }

        if( p_move.m_param == M_BURN_UP ) {
            addVolatileStatus( p_ui, opponent, slot, VS_BURNUP, 255 );
        }

        if( p_move.m_moveData.m_flags & MF_SELFSWITCH ) { recallPokemon( p_ui, opponent, slot ); }

        if( p_move.m_moveData.m_flags & MF_BATONPASS ) {
            recallPokemon( p_ui, opponent, slot, true );
        }

        if( p_move.m_moveData.m_flags & MF_SELFDESTRUCT ) { faintPokemon( p_ui, opponent, slot ); }

        // check for held item
        checkItemAfterAttack( p_ui, opponent, slot );

        setLastUsedMove( opponent, slot, p_move );
    }

} // namespace BATTLE
