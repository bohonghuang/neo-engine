/*
Pok�mon Emerald 2 Version
------------------------------

file        : sprite.cpp
author      : Philip Wellnitz (RedArceus)
description : Some sprite code.

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

#include <algorithm>
#include <initializer_list>

#include "sprite.h"
#include "uio.h"
#include "../fs/fs.h"
#include "../ds/move.h"

#include "damage_0.h"
#include "damage_1.h"
#include "damage_2.h"
#include "NoItem.h"

u32 TEMP[ 12288 ] = { 0 };
u16 TEMP_PAL[ 256 ] = { 0 };

namespace IO {
    const unsigned int* TypeTiles[ 19 ] =
    {
        NormalTiles, KampfTiles, FlugTiles, GiftTiles, BodenTiles,
        GestTiles, KaeferTiles, GeistTiles, StahlTiles, UnbekTiles,
        WasserTiles, FeuerTiles, PflTiles, ElekTiles, PsychoTiles,
        EisTiles, DraTiles, UnlTiles, FeeTiles
    };
    const unsigned short* TypePals[ 19 ] =
    {
        NormalPal, KampfPal, FlugPal, GiftPal, BodenPal,
        GestPal, KaeferPal, GeistPal, StahlPal, UnbekPal,
        WasserPal, FeuerPal, PflPal, ElekPal, PsychoPal,
        EisPal, DraPal, UnlPal, FeePal
    };

    const unsigned int* HitTypeTiles[ 3 ] =
    {
        damage_0Tiles, damage_1Tiles, damage_2Tiles
    };
    const unsigned short* HitTypePals[ 3 ] =
    {
        damage_0Pal, damage_1Pal, damage_2Pal
    };


    [[ deprecated( "Use the methods of IO instead." ) ]]
    const u8 SPRITE_DMA_CHANNEL = 2;

    [[ deprecated( "Use the methods of IO instead." ) ]]
    const u16 BYTES_PER_16_COLOR_TILE = 32;
    [[ deprecated( "Use the methods of IO instead." ) ]]
    const u16 COLORS_PER_PALETTE = 16;
    [[ deprecated( "Use the methods of IO instead." ) ]]
    const u16 BOUNDARY_VALUE = 32;
    [[ deprecated( "Use the methods of IO instead." ) ]]
    const u16 OFFSET_MULTIPLIER = ( BOUNDARY_VALUE / sizeof( SPRITE_GFX[ 0 ] ) );
    [[ deprecated( "Use the methods of IO instead." ) ]]
    const u16 OFFSET_MULTIPLIER_SUB = ( BOUNDARY_VALUE / sizeof( SPRITE_GFX_SUB[ 0 ] ) );

    void updateOAM( bool p_bottom ) {
        OAMTable* oam = ( p_bottom ? Oam : OamTop );
        DC_FlushAll( );
        if( p_bottom ) {
            dmaCopyHalfWords( SPRITE_DMA_CHANNEL,
                              oam->oamBuffer,
                              OAM_SUB,
                              SPRITE_COUNT * sizeof( SpriteEntry ) );
        } else {
            dmaCopyHalfWords( SPRITE_DMA_CHANNEL,
                              oam->oamBuffer,
                              OAM,
                              SPRITE_COUNT * sizeof( SpriteEntry ) );
        }
    }
    void initOAMTable( bool p_bottom ) {
        OAMTable* oam = ( p_bottom ? Oam : OamTop );
        /*
        * For all 128 sprites on the DS, disable and clear any attributes they
        * might have. This prevents any garbage from being displayed and gives
        * us a clean slate to work with.
        */
        for( int i = 0; i < SPRITE_COUNT; i++ ) {
            oam->oamBuffer[ i ].attribute[ 0 ] = ATTR0_DISABLED;
            oam->oamBuffer[ i ].attribute[ 1 ] = 0;
            oam->oamBuffer[ i ].attribute[ 2 ] = 0;
            oam->oamBuffer[ i ].isHidden = true;
        }
        for( int i = 0; i < MATRIX_COUNT; i++ ) {
            /* If you look carefully, you'll see this is that affine trasformation
            * matrix again. We initialize it to the identity matrix, as we did
            * with backgrounds
            */
            oam->matrixBuffer[ i ].hdx = 1 << 8;
            oam->matrixBuffer[ i ].hdy = 0;
            oam->matrixBuffer[ i ].vdx = 0;
            oam->matrixBuffer[ i ].vdy = 1 << 8;
        }
        updateOAM( p_bottom );
    }
    void rotateSprite( SpriteRotation * p_spriteRotation, int p_angle ) {
        s16 s = sinLerp( p_angle ) >> 4;
        s16 c = cosLerp( p_angle ) >> 4;

        p_spriteRotation->hdx = c;
        p_spriteRotation->hdy = s;
        p_spriteRotation->vdx = -s;
        p_spriteRotation->vdy = c;
    }
    void setSpriteVisibility( SpriteEntry * p_spriteEntry, bool p_hidden, bool p_affine, bool p_doubleBound ) {
        if( p_hidden ) {
            /*
            * Make the sprite invisible.
            *
            * An affine sprite cannot be hidden. We have to turn it into a
            * non-affine sprite before we can hide it. To hide any sprite, we must
            * set bit 8 and clear bit 9. For non-affine sprites, this is a bit
            * redundant, but it is faster than a branch to just set it regardless
            * of whether or not it is already set.
            */
            p_spriteEntry->isRotateScale = false; // Bit 9 off
            p_spriteEntry->isHidden = true; // Bit 8 on
        } else {
            /* Make the sprite visible.*/
            if( p_affine ) {
                /* Again, keep in mind that affine sprites cannot be hidden, so
                * enabling affine is enough to show the sprite again. We also need
                * to allow the user to get the double bound flag in the sprite
                * attribute. If we did not, then our sprite hiding function would
                * not be able to properly hide and restore double bound sprites.
                * We enable bit 9 here because we want an affine sprite.
                */
                p_spriteEntry->isRotateScale = true;

                /* The double bound flag only acts as the double bound flag when
                * the sprite is an affine sprite. At all other times, it acts as
                * the sprite invisibility flag. We only enable bit 8 here if we want
                * a double bound sprite. */
                p_spriteEntry->isSizeDouble = p_doubleBound;
            } else {
                /* Bit 9 (the affine flag) will already be off here, so we don't
                * need to clear it. However, bit 8 (the sprite invisibility flag)
                * will need to be cleared. */
                p_spriteEntry->isHidden = false;
            }
        }
    }
    inline void setSpritePosition( SpriteEntry* p_spriteEntry, u16 p_x, u16 p_y ) {
        p_spriteEntry->x = p_x;
        p_spriteEntry->y = p_y;
    }
    inline void setSpritePriority( SpriteEntry* p_spriteEntry, ObjPriority p_priority ) {
        p_spriteEntry->priority = p_priority;
    }

    u16 loadSprite( const u8    p_oamIdx,
                    const u8    p_palIdx,
                    const u16   p_tileIdx,
                    const u16   p_posX,
                    const u16   p_posY,
                    const u8    p_width,
                    const u8    p_height,
                    const unsigned short *p_spritePal,
                    const unsigned int   *p_spriteData,
                    const u32   p_spriteDataLen,
                    bool        p_flipX,
                    bool        p_flipY,
                    bool        p_hidden,
                    ObjPriority p_priority,
                    bool        p_bottom ) {
        IO::SpriteInfo* sInfo = ( p_bottom ? spriteInfo : spriteInfoTop );
        OAMTable* oam = ( p_bottom ? Oam : OamTop );

        SpriteInfo * spriteInfo = &sInfo[ p_oamIdx ];
        SpriteEntry * spriteEntry = &oam->oamBuffer[ p_oamIdx ];

        spriteInfo->m_oamId = p_oamIdx;
        spriteInfo->m_width = p_width;
        spriteInfo->m_height = p_height;
        spriteInfo->m_angle = 0;
        spriteInfo->m_entry = spriteEntry;

        spriteEntry->palette = p_palIdx;
        spriteEntry->gfxIndex = p_tileIdx;
        spriteEntry->x = p_posX;
        spriteEntry->y = p_posY;
        spriteEntry->vFlip = p_flipX;
        spriteEntry->hFlip = p_flipY;
        spriteEntry->isHidden = p_hidden;
        spriteEntry->priority = p_priority;

        spriteEntry->isRotateScale = false;
        spriteEntry->isMosaic = false;
        spriteEntry->blendMode = OBJMODE_NORMAL;
        spriteEntry->colorMode = OBJCOLOR_16;

        spriteEntry->shape = ( ( p_width == p_height ) ? OBJSHAPE_SQUARE : ( ( p_width > p_height ) ? OBJSHAPE_WIDE : OBJSHAPE_TALL ) );

        u8 maxSize = std::max( p_width, p_height );
        spriteEntry->size = ( ( maxSize == 64 ) ? OBJSIZE_64 :
                              ( ( maxSize == 32 ) ? OBJSIZE_32 :
                              ( ( maxSize == 16 ) ? OBJSIZE_16 : OBJSIZE_8 ) ) );

        if( !p_bottom ) {
            if( p_spritePal )
                dmaCopyHalfWords( SPRITE_DMA_CHANNEL, p_spritePal, &SPRITE_PALETTE[ p_palIdx * COLORS_PER_PALETTE ], 32 );
            if( p_spriteData )
                dmaCopyHalfWords( SPRITE_DMA_CHANNEL, p_spriteData, &SPRITE_GFX[ p_tileIdx * OFFSET_MULTIPLIER ], p_spriteDataLen );
        } else {
            if( p_spritePal )
                dmaCopyHalfWords( SPRITE_DMA_CHANNEL, p_spritePal, &SPRITE_PALETTE_SUB[ p_palIdx * COLORS_PER_PALETTE ], 32 );
            if( p_spriteData )
                dmaCopyHalfWords( SPRITE_DMA_CHANNEL, p_spriteData, &SPRITE_GFX_SUB[ p_tileIdx * OFFSET_MULTIPLIER_SUB ], p_spriteDataLen );
        }
        return p_tileIdx + ( p_spriteDataLen / BYTES_PER_16_COLOR_TILE );
    }

    u16 loadPKMNSprite( const char* p_path, const u16& p_pkmnId, const s16 p_posX,
                        const s16 p_posY, u8 p_oamIndex, u8 p_palCnt, u16 p_tileCnt, bool p_bottom, bool p_shiny, bool p_female, bool p_flipx, bool p_topOnly ) {
        char buffer[ 100 ];
        if( !p_female )
            sprintf( buffer, "%d/%d.raw", p_pkmnId, p_pkmnId );
        else
            sprintf( buffer, "%d/%df.raw", p_pkmnId, p_pkmnId );
        if( !FS::readData( p_path, buffer, u16( 16 ), TEMP_PAL, u32( 96 * 96 ), TEMP ) )
            return false;

        if( p_shiny ) {
            if( !p_female )
                sprintf( buffer, "%s%d/%ds.raw", p_pkmnId, p_pkmnId );
            else
                sprintf( buffer, "%s%d/%dsf.raw", p_pkmnId, p_pkmnId );
            FS::readData( p_path, buffer, u16( 16 ), TEMP_PAL );
        }

        p_tileCnt = loadSprite( p_oamIndex++, p_palCnt, p_tileCnt, p_flipx ? 32 + p_posX : p_posX, p_posY,
                                64, 64, TEMP_PAL, TEMP, 96 * 96 / 2, p_flipx, false, false, OBJPRIORITY_0, p_bottom );
        p_tileCnt = loadSprite( p_oamIndex++, p_palCnt, p_tileCnt, p_flipx ? p_posX : 64 + p_posX, p_posY,
                                32, 64, 0, 0, 0, p_flipx, false, false, OBJPRIORITY_0, p_bottom );
        if( !p_topOnly ) {
            p_tileCnt = loadSprite( p_oamIndex++, p_palCnt, p_tileCnt, p_flipx ? 32 + p_posX : p_posX, p_posY + 64,
                                    64, 32, 0, 0, 0, p_flipx, false, false, OBJPRIORITY_0, p_bottom );
            p_tileCnt = loadSprite( p_oamIndex++, p_palCnt, p_tileCnt, p_flipx ? p_posX : 64 + p_posX, p_posY + 64,
                                    32, 32, 0, 0, 0, p_flipx, false, false, OBJPRIORITY_0, p_bottom );
        }
        updateOAM( p_bottom );
        return p_tileCnt;
    }

    u16 loadTrainerSprite( const char* p_path, const char* p_name, const s16 p_posX,
                           const s16 p_posY, u8 p_oamIndex, u8 p_palCnt, u16 p_tileCnt, bool p_bottom, bool p_flipx, bool p_topOnly ) {
        char buffer[ 100 ];
        sprintf( buffer, "Sprite_%s.raw", p_name );
        if( !FS::readData( p_path, buffer, u16( 16 ), TEMP_PAL, u32( 96 * 96 ), TEMP ) )
            return false;

        p_tileCnt = loadSprite( p_oamIndex++, p_palCnt, p_tileCnt, p_flipx ? 32 + p_posX : p_posX, p_posY,
                                64, 64, TEMP_PAL, TEMP, 96 * 96 / 2, p_flipx, false, false, OBJPRIORITY_0, p_bottom );
        p_tileCnt = loadSprite( p_oamIndex++, p_palCnt, p_tileCnt, p_flipx ? p_posX : 64 + p_posX, p_posY,
                                32, 64, 0, 0, 0, p_flipx, false, false, OBJPRIORITY_0, p_bottom );
        if( !p_topOnly ) {
            p_tileCnt = loadSprite( p_oamIndex++, p_palCnt, p_tileCnt, p_flipx ? 32 + p_posX : p_posX, p_posY + 64,
                                    64, 32, 0, 0, 0, p_flipx, false, false, OBJPRIORITY_0, p_bottom );
            p_tileCnt = loadSprite( p_oamIndex++, p_palCnt, p_tileCnt, p_flipx ? p_posX : 64 + p_posX, p_posY + 64,
                                    32, 32, 0, 0, 0, p_flipx, false, false, OBJPRIORITY_0, p_bottom );
        }
        updateOAM( p_bottom );
        return p_tileCnt;
    }

    u16 loadIcon( const char* p_path, const char* p_name, const s16 p_posX, const s16 p_posY, u8 p_oamIndex, u8 p_palCnt, u16 p_tileCnt, bool p_bottom ) {
        if( FS::readData( p_path, p_name, (u32)128, TEMP, (u16)16, TEMP_PAL ) ) {
            return loadSprite( p_oamIndex, p_palCnt, p_tileCnt, p_posX, p_posY, 32, 32, TEMP_PAL, TEMP, 512,
                               false, false, false, p_bottom ? OBJPRIORITY_1 : OBJPRIORITY_0, p_bottom );
        } else {
            return loadSprite( p_oamIndex, p_palCnt, p_tileCnt, p_posX, p_posY, 32, 32, NoItemPal, NoItemTiles, NoItemTilesLen,
                               false, false, false, p_bottom ? OBJPRIORITY_1 : OBJPRIORITY_0, p_bottom );
        }
    }

    u16 loadPKMNIcon( const u16& p_pkmnId, const u16 p_posX, const u16 p_posY, u8 p_oamIndex, u8 p_palCnt, u16 p_tileCnt, bool p_bottom ) {
        char buffer[ 100 ];
        sprintf( buffer, "%hu/Icon_%hu", p_pkmnId, p_pkmnId );
        return loadIcon( "nitro:/PICS/SPRITES/PKMN/", buffer, p_posX, p_posY, p_oamIndex, p_palCnt, p_tileCnt, p_bottom );
    }

    u16 loadEggIcon( const u16 p_posX, const u16 p_posY, u8 p_oamIndex, u8 p_palCnt, u16 p_tileCnt, bool p_bottom ) {
        return loadIcon( "nitro:/PICS/SPRITES/PKMN/", "Icon_egg", p_posX, p_posY, p_oamIndex, p_palCnt, p_tileCnt, p_bottom );
    }

    u16 loadItemIcon( const std::string& p_itemName, const u16 p_posX, const u16 p_posY, u8 p_oamIndex, u8 p_palCnt, u16 p_tileCnt, bool p_bottom ) {
        return loadIcon( "nitro:/PICS/SPRITES/ITEMS/", p_itemName.c_str( ), p_posX, p_posY, p_oamIndex, p_palCnt, p_tileCnt, p_bottom );
    }

    u16 loadTMIcon( Type p_type, bool p_hm, const u16 p_posX, const u16 p_posY,
                    u8 p_oamIndex, u8 p_palCnt, u16 p_tileCnt, bool p_bottom ) {
        std::string itemname = ( p_hm ? "VM" : "TM" ) + ( std::vector<std::string>( { "Normal", "Kampf", "Flug", "Gift", "Boden", "Gestein", "Pflanze", "Geist",
            "Stahl", "Unbekannt", "Wasser", "Feuer", "Pflanze", "Elektro", "Psycho", "Eis",
            "Drache", "Unlicht", "Fee" } )[ p_type ] );

        return loadItemIcon( itemname, p_posX, p_posY, p_oamIndex, p_palCnt, p_tileCnt, p_bottom );
    }

    u16 loadTypeIcon( Type p_type, const u16 p_posX, const u16 p_posY, u8 p_oamIndex, u8 p_palCnt, u16 p_tileCnt, bool p_bottom ) {
        return loadSprite( p_oamIndex, p_palCnt, p_tileCnt, p_posX, p_posY, 32, 16, TypePals[ p_type ], TypeTiles[ p_type ], 256,
                           false, false, false, OBJPRIORITY_0, p_bottom );
    }
    
    u16 loadDamageCategoryIcon( move::moveHitTypes p_type, const u16 p_posX, const u16 p_posY, u8 p_oamIndex, u8 p_palCnt, u16 p_tileCnt, bool p_bottom ) {
        return loadSprite( p_oamIndex, p_palCnt, p_tileCnt, p_posX, p_posY, 32, 16, HitTypePals[ p_type ], HitTypeTiles[ p_type ], 256,
                           false, false, false, OBJPRIORITY_0, p_bottom );
    }

}