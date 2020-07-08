/*
Pokémon neo
------------------------------

file        : sound.cpp
author      : Philip Wellnitz
description :

Copyright (C) 2012 - 2020
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

#include <nds.h>

#include "fs.h"
#include "locationNames.h"
#include "pokemonNames.h"
#include "sound.h"

namespace SOUND {
    u16           currentLocation = 0;
    MAP::moveMode currentMoveMode = MAP::WALK;

    u16 LAST_CRY = -1;

    void playCry( u16 p_pokemonId, u8 p_formeId, bool ) {
#ifndef NO_SOUND
        u16 len;
        u8* cry = FS::readCry( p_pokemonId, p_formeId, len );
        if( cry == nullptr ) {
            playSoundEffect( SFX_CANCEL );
            return;
        }

        if( LAST_CRY != u16( -1 ) ) { soundKill( LAST_CRY ); }

        LAST_CRY = soundPlaySample( cry, SoundFormat_8Bit, len, 22050, 0x0f, 64, false, 0 );
#endif
    }

    void onLocationChange( u16 p_newLocation ) {
        if( currentLocation == p_newLocation ) { return; }

        currentLocation = p_newLocation;
        if( currentMoveMode == MAP::WALK ) { playBGM( BGMforLocation( currentLocation ) ); }
    }

    void onMovementTypeChange( MAP::moveMode p_newMoveMode ) {
        if( currentMoveMode == p_newMoveMode ) { return; }

        currentMoveMode = p_newMoveMode;
        playBGM( BGMforMoveMode( currentMoveMode ) );
    }

    void restartBGM( ) {
        playBGM( BGMforMoveMode( currentMoveMode ) );
    }

    u16 BGMforMoveMode( MAP::moveMode p_moveMode ) {
        switch( p_moveMode ) {
        case MAP::DIVE:
            return MOD_DIVING;
        case MAP::SURF:
            return MOD_SURFING;
        case MAP::BIKE:
        case MAP::ACRO_BIKE:
        case MAP::MACH_BIKE:
        case MAP::BIKE_JUMP:
            return MOD_CYCLING;
        case MAP::WALK:
        case MAP::ROCK_CLIMB:
        case MAP::SIT:
        default:
            return BGMforLocation( currentLocation );
        }
    }
} // namespace SOUND
