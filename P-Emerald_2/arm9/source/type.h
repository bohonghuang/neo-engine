/*
    Pok�mon Emerald 2 Version
    ------------------------------

    file        : type.h
    author      : Philip Wellnitz (RedArceus)
    description : Header file. See corresponding source file for details.

    Copyright (C) 2012 - 2015
    Philip Wellnitz (RedArceus)

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

#pragma once

enum Type{
    NORMAL=0,	KAMPF=1,	FLUG=2,     GIFT=3,	    BODEN=4,
    GESTEIN=5,	KAEFER=6,	GEIST=7,	STAHL=8,	UNBEKANNT=9,
    WASSER=10,	FEUER=11,	PFLANZE=12,	ELEKTRO=13,	PSYCHO=14,
    EIS=15,     DRACHE=16,	UNLICHT=17, FEE = 18
};

float getEffectiveness( const Type& p_t1, const Type& p_t2 );