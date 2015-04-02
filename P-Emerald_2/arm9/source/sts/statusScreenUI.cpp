#include "statusScreenUI.h"
#include "../defines.h"
#include "../buffer.h"
#include "../ds/move.h"
#include "../io/uio.h"
#include "../io/sprite.h"
#include "../fs/fs.h"
#include "../fs/saveGame.h"

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

    statusScreenUI::statusScreenUI( std::vector<pokemon>& p_pokemon, u8 p_pageMax )
        : _pokemon( p_pokemon ) {
        m_pagemax = p_pageMax;
    }

    //OAMTop indices
#define ICON_IDX(a) (a)
#define PKMN_SPRITE_START 6
#define PAGE_ICON_IDX  10
#define ITEM_ICON_IDX  11
#define TYPE_IDX 12
#define ATK_DMGTYPE_IDX(a) (16 + (u8(a)))
#define RIBBON_IDX 19

#define ICON_PAL(a) (a)
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


    u8 fieldCnt = 0;
    void regStsScreenUI::initSub( u16 p_pkmIdx ) {
        consoleSelect( &IO::Bottom );
        consoleSetWindow( &IO::Bottom, 16, 0, 16, 16 );
        consoleClear( );
        IO::initOAMTable( true );
        u16 nextAvailableTileIdx = 16;

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
                                                   96, 68 + (i)* 32, 64, 32, Choice_1Pal,
                                                   Choice_2Tiles, Choice_2TilesLen, false, false, true, OBJPRIORITY_2, true );
        }

        if( p_pkmIdx >= _pokemon.size( ) || _pokemon[ p_pkmIdx ].m_boxdata.m_individualValues.m_isEgg ) {
            consoleSelect( &IO::Top );
            return;
        }
        u8 u = 0;
        for( u8 i = 0; i < 4; ++i ) {
            if( _pokemon[ p_pkmIdx ].m_boxdata.m_moves[ i ] < MAXATTACK &&
                AttackList[ _pokemon[ p_pkmIdx ].m_boxdata.m_moves[ i ] ]->m_isFieldAttack ) {
                IO::Oam->oamBuffer[ CHOICE_ID + 2 * ( u ) ].isHidden = false;
                IO::Oam->oamBuffer[ CHOICE_ID + 1 + 2 * ( u ) ].isHidden = false;
                IO::Oam->oamBuffer[ CHOICE_ID + 2 * ( u ) ].y = -7 + 24 * u;
                IO::Oam->oamBuffer[ CHOICE_ID + 1 + 2 * ( u ) ].y = -7 + 24 * u;

                IO::Oam->oamBuffer[ CHOICE_ID + 2 * ( u ) ].x = 152;
                IO::Oam->oamBuffer[ CHOICE_ID + 1 + 2 * ( u ) ].x = 192 + 24;

                consoleSetWindow( &IO::Bottom, 16, 3 * u + 1, 16, 16 );
                printf( "    %s", AttackList[ _pokemon[ p_pkmIdx ].m_boxdata.m_moves[ i ] ]->m_moveName.c_str( ) );
                ++u;
            }
        }
        if( _pokemon[ p_pkmIdx ].m_boxdata.m_holdItem ) {
            IO::Oam->oamBuffer[ CHOICE_ID + 2 * ( u ) ].isHidden = false;
            IO::Oam->oamBuffer[ CHOICE_ID + 1 + 2 * ( u ) ].isHidden = false;
            IO::Oam->oamBuffer[ CHOICE_ID + 2 * ( u ) ].y = -7 + 24 * u;
            IO::Oam->oamBuffer[ CHOICE_ID + 1 + 2 * ( u ) ].y = -7 + 24 * u;

            IO::Oam->oamBuffer[ CHOICE_ID + 2 * ( u ) ].x = 152;
            IO::Oam->oamBuffer[ CHOICE_ID + 1 + 2 * ( u ) ].x = 192 + 24;

            consoleSetWindow( &IO::Bottom, 16, 3 * u + 1, 16, 16 );
            printf( "    Item nehmen" );
        }
        u16 s = u + _pokemon[ p_pkmIdx ].m_boxdata.m_holdItem;
        IO::Oam->oamBuffer[ CHOICE_ID + 2 * ( s ) ].isHidden = false;
        IO::Oam->oamBuffer[ CHOICE_ID + 1 + 2 * ( s ) ].isHidden = false;
        IO::Oam->oamBuffer[ CHOICE_ID + 2 * ( s ) ].y = -7 + 24 * u;
        IO::Oam->oamBuffer[ CHOICE_ID + 1 + 2 * ( s ) ].y = -7 + 24 * u;

        IO::Oam->oamBuffer[ CHOICE_ID + 2 * ( s ) ].x = 152;
        IO::Oam->oamBuffer[ CHOICE_ID + 1 + 2 * ( s ) ].x = 192 + 24;
        IO::updateOAM( true );

        consoleSetWindow( &IO::Bottom, 16, 3 * s + 1, 16, 16 );
        printf( "    Dexeintrag" );
        consoleSelect( &IO::Top );

        fieldCnt = u;
    }

    void regStsScreenUI::initTop( ) {
        consoleSelect( &IO::Top );
        printf( "\x1b[39m" );
        u16 tileCnt = 0;
        IO::regularFont->setColor( 0, 0 );
        IO::regularFont->setColor( 251, 1 );
        IO::regularFont->setColor( 252, 2 );

        BG_PALETTE[ 250 ] = RGB15( 31, 31, 31 );
        BG_PALETTE[ 252 ] = RGB15( 15, 15, 15 );
        BG_PALETTE[ 251 ] = RGB15( 3, 3, 3 );

        for( size_t i = 0; i < _pokemon.size( ); i++ ) {
            if( !_pokemon[ i ].m_boxdata.m_individualValues.m_isEgg ) {
                if( i % 2 == 0 ) {
                    tileCnt = IO::loadPKMNIcon( _pokemon[ i ].m_boxdata.m_speciesId,
                                                u16( borders[ i ][ 0 ] * 8 - 28 ), u16( borders[ i ][ 1 ] * 8 ),
                                                ICON_IDX( i ), ICON_PAL( i ), tileCnt, false );
                    IO::displayHP( 100, 101, borders[ i ][ 0 ] * 8 - 13, borders[ i ][ 1 ] * 8 + 8 - ( i != 2 ? 4 : 0 ), 142 + 2 * i, 143 + 2 * i, false, true );
                    IO::displayHP( 100, 100 - _pokemon[ i ].m_stats.m_acHP * 100 / _pokemon[ i ].m_stats.m_maxHP, borders[ i ][ 0 ] * 8 - 13,
                                   borders[ i ][ 1 ] * 8 + 8 - ( i != 2 ? 4 : 0 ), 142 + 2 * i, 143 + 2 * i, false, true );
                } else {
                    tileCnt = IO::loadPKMNIcon( _pokemon[ i ].m_boxdata.m_speciesId,
                                                u16( borders[ i ][ 0 ] * 8 + 76 ), u16( borders[ i ][ 1 ] * 8 ),
                                                ICON_IDX( i ), ICON_PAL( i ), tileCnt, false );
                    IO::displayHP( 100, 101, borders[ i ][ 0 ] * 8 + 63, borders[ i ][ 1 ] * 8 + 8 - ( i != 3 ? 4 : 0 ), 142 + 2 * i, 143 + 2 * i, false, true );
                    IO::displayHP( 100, 100 - _pokemon[ i ].m_stats.m_acHP * 100 / _pokemon[ i ].m_stats.m_maxHP, borders[ i ][ 0 ] * 8 + 63,
                                   borders[ i ][ 1 ] * 8 + 8 - ( i != 3 ? 4 : 0 ), 142 + 2 * i, 143 + 2 * i, false, true );
                }
                char buffer[ 100 ];

                u8 mval = 1 + ( ( i / 2 == 1 ) ? 4 : 8 );

                sprintf( buffer, "%ls", _pokemon[ i ].m_boxdata.m_name );
                IO::regularFont->printString( buffer, borders[ i ][ 0 ] * 8, borders[ i ][ 1 ] * 8 - mval, false );
                sprintf( buffer, "%s", getDisplayName( _pokemon[ i ].m_boxdata.m_speciesId ) );
                IO::regularFont->printString( buffer, borders[ i ][ 0 ] * 8, borders[ i ][ 1 ] * 8 + 14 - mval, false );

                sprintf( buffer, "%hi/%hi KP", _pokemon[ i ].m_stats.m_acHP, _pokemon[ i ].m_stats.m_maxHP );
                IO::regularFont->printString( buffer, borders[ i ][ 0 ] * 8, borders[ i ][ 1 ] * 8 + 28 - mval, false );

                sprintf( buffer, "%s", ItemList[ _pokemon[ i ].m_boxdata.getItem( ) ]->getDisplayName( ).c_str( ) );
                IO::regularFont->printString( buffer, borders[ i ][ 0 ] * 8, borders[ i ][ 1 ] * 8 + 42 - mval, false );

            } else {
                consoleSetWindow( &IO::Top, borders[ i ][ 0 ], borders[ i ][ 1 ], 12, 6 );

                char buffer[ 100 ];
                int mval = 1 + ( ( i / 2 == 1 ) ? 4 : 8 );
                sprintf( buffer, "Ei" );
                IO::regularFont->printString( buffer, borders[ i ][ 0 ] * 8, borders[ i ][ 1 ] * 8 - mval, false );
                sprintf( buffer, "Ei" );
                IO::regularFont->printString( buffer, borders[ i ][ 0 ] * 8, borders[ i ][ 1 ] * 8 + 14 - mval, false );

                sprintf( buffer, "%s", ItemList[ _pokemon[ i ].m_boxdata.getItem( ) ]->getDisplayName( ).c_str( ) );
                IO::regularFont->printString( buffer, borders[ i ][ 0 ] * 8, borders[ i ][ 1 ] * 8 + 42 - mval, false );

                if( i % 2 == 0 ) {
                    tileCnt = IO::loadEggIcon( u16( borders[ i ][ 0 ] * 8 - 28 ), u16( borders[ i ][ 1 ] * 8 ), ICON_IDX( i ), ICON_PAL( i ), tileCnt, false );
                } else {
                    tileCnt = IO::loadEggIcon( u16( borders[ i ][ 0 ] * 8 + 76 ), u16( borders[ i ][ 1 ] * 8 ), ICON_IDX( i ), ICON_PAL( i ), tileCnt, false );
                }
            }
        }

        //Preload the page specific sprites
        auto currPkmn = _pokemon[ _current ];

        if( !( tileCnt = IO::loadPKMNSprite( "nitro:/PICS/SPRITES/PKMN/", currPkmn.m_boxdata.m_speciesId, 16, 48,
            PKMN_SPRITE_START, PKMN_SPRITE_PAL, tileCnt, false,
            currPkmn.m_boxdata.isShiny( ), currPkmn.m_boxdata.m_isFemale, true ) ) ) {
            tileCnt = IO::loadPKMNSprite( "nitro:/PICS/SPRITES/PKMN/", currPkmn.m_boxdata.m_speciesId, 16, 48,
                                          PKMN_SPRITE_START, PKMN_SPRITE_PAL, IO::OamTop->oamBuffer[ PKMN_SPRITE_START ].gfxIndex, false,
                                          currPkmn.m_boxdata.isShiny( ), !currPkmn.m_boxdata.m_isFemale, true );
        }


        tileCnt = IO::loadSprite( PAGE_ICON_IDX, PAGE_ICON_PAL, tileCnt,
                                  0, 0, 32, 32, memoPal, memoTiles, memoTilesLen, false, false, false, OBJPRIORITY_0, false );
        tileCnt = IO::loadItemIcon( ItemList[ currPkmn.m_boxdata.getItem( ) ]->m_itemName, 2, 152,
                                    ITEM_ICON_IDX, ITEM_ICON_PAL, tileCnt, false );

        for( u8 i = 0; i < 4; ++i ) {
            Type t = AttackList[ currPkmn.m_boxdata.m_moves[ i ] ]->m_moveType;
            tileCnt = IO::loadTypeIcon( t, 126, 43 + 32 * i, TYPE_IDX + i, TYPE_PAL( i ), tileCnt, false );
        }
        for( u8 i = 0; i < 4; ++i )
            tileCnt = IO::loadDamageCategoryIcon( ( move::moveHitTypes )( i % 3 ), 126, 43 + 32 * i,
            ATK_DMGTYPE_IDX( i ), DMG_TYPE_PAL( i % 3 ), tileCnt, false );

        for( u8 i = PKMN_SPRITE_START; i < RIBBON_IDX; ++i )
            IO::OamTop->oamBuffer[ i ].isHidden = true;
        IO::updateOAM( false );
    }

    void regStsScreenUI::init( u8 p_current ) {
        _current = p_current;
        u8 max = _pokemon.size( );

        IO::vramSetup( );
        videoSetMode( MODE_5_2D/* | DISPLAY_BG2_ACTIVE*/ | DISPLAY_BG3_ACTIVE | DISPLAY_SPR_ACTIVE | DISPLAY_SPR_1D );
        swiWaitForVBlank( );
        IO::Top = *consoleInit( &IO::Top, 0, BgType_Text4bpp, BgSize_T_256x256, 2, 0, true, true );
        consoleSetFont( &IO::Top, IO::consoleFont );

        IO::Bottom = *consoleInit( &IO::Bottom, 0, BgType_Text4bpp, BgSize_T_256x256, 2, 0, false, true );
        consoleSetFont( &IO::Bottom, IO::consoleFont );

        FS::readPictureData( bgGetGfxPtr( IO::bg3 ), "nitro:/PICS/", "PKMNScreen" );
        consoleSelect( &IO::Top );
        consoleClear( );
        consoleSetWindow( &IO::Bottom, 0, 0, 32, 24 );
        consoleSelect( &IO::Bottom );
        consoleClear( );

        bgUpdate( );


        IO::drawSub( );
        initTop( );
        initSub( _current );

        consoleSetWindow( &IO::Top, positions[ _current ][ 0 ], positions[ _current ][ 1 ], 2, 2 );
        if( _current & 1 )
            printf( ">" );
        else
            printf( "<" );


    }

    pokemonData data;
    void regStsScreenUI::draw( u8 p_current, u8 p_page, bool p_newpok ) {
        for( u8 i = 0; i < 6; ++i )
            IO::OamTop->oamBuffer[ ICON_IDX( i ) ].isHidden = true;

        IO::setDefaultConsoleTextColors( BG_PALETTE, 6 );
        FS::readPictureData( bgGetGfxPtr( IO::bg3 ), "nitro:/PICS/", "PKMNInfoScreen", 32 );

        IO::regularFont->setColor( 0, 0 );
        IO::regularFont->setColor( 251, 1 );
        IO::regularFont->setColor( 252, 2 );
        IO::boldFont->setColor( 0, 0 );
        IO::boldFont->setColor( 253, 1 );
        IO::boldFont->setColor( 254, 2 );

        BG_PALETTE[ 250 ] = RGB15( 31, 31, 31 );
        BG_PALETTE[ 251 ] = RGB15( 20, 20, 20 );
        BG_PALETTE[ 252 ] = RGB15( 3, 3, 3 );
        if( p_newpok ) {
            BG_PALETTE_SUB[ 253 ] = RGB15( 10, 10, 10 );
            BG_PALETTE_SUB[ 254 ] = RGB15( 31, 31, 31 );
        }
        consoleSelect( &IO::Top );
        consoleSetWindow( &IO::Top, 0, 0, 32, 24 );
        consoleClear( );
        consoleSelect( &IO::Bottom );
        consoleClear( );

        auto currPkmn = _pokemon[ p_current ];
        u8 oamIdx = 0, palCnt = 0;
        u16 tileCnt = 0;

        if( !currPkmn.m_boxdata.m_individualValues.m_isEgg ) {
            BG_PALETTE[ 254 ] = RGB15( 31, 0, 0 );
            BG_PALETTE[ 255 ] = RGB15( 0, 0, 31 );
            BG_PALETTE[ 253 ] = RGB15( 31, 31, 31 );
            IO::regularFont->setColor( 253, 1 );

            consoleSetWindow( &IO::Top, 20, 1, 13, 2 );

            IO::regularFont->printString( currPkmn.m_boxdata.m_name, 150, 2, false );
            s8 G = currPkmn.m_boxdata.gender( );

            IO::regularFont->printChar( '/', 234, 2, false );
            if( currPkmn.m_boxdata.m_speciesId != 29 && currPkmn.m_boxdata.m_speciesId != 32 ) {
                if( G == 1 ) {
                    IO::regularFont->setColor( 255, 1 );
                    IO::regularFont->printChar( 136, 246, 8, false );
                } else {
                    IO::regularFont->setColor( 254, 1 );
                    IO::regularFont->printChar( 137, 246, 8, false );
                }
            }
            IO::regularFont->setColor( 253, 1 );

            getAll( currPkmn.m_boxdata.m_speciesId, data );
            IO::regularFont->printString( getDisplayName( currPkmn.m_boxdata.m_speciesId ), 160, 16, false );

            if( currPkmn.m_boxdata.getItem( ) ) {
                IO::regularFont->printString( "Item", 2, 176, false );
                IO::regularFont->setColor( 252, 1 );
                IO::regularFont->setColor( 0, 2 );
                char buffer[ 200 ];
                sprintf( buffer, "%s: %s", ItemList[ currPkmn.m_boxdata.getItem( ) ]->getDisplayName( true ).c_str( ),
                         ItemList[ currPkmn.m_boxdata.getItem( ) ]->getShortDescription( true ).c_str( ) );
                IO::regularFont->printString( buffer, 50, 159, false );
                if( p_newpok ) {
                    IO::loadItemIcon( ItemList[ currPkmn.m_boxdata.getItem( ) ]->m_itemName, 2, 152,
                                      ITEM_ICON_IDX, ITEM_ICON_PAL, IO::OamTop->oamBuffer[ ITEM_ICON_IDX ].gfxIndex, false );
                }
            } else {
                IO::regularFont->setColor( 252, 1 );
                IO::regularFont->setColor( 0, 2 );
                IO::regularFont->printString( ItemList[ currPkmn.m_boxdata.getItem( ) ]->getDisplayName( ).c_str( ), 56, 168, false );
                IO::OamTop->oamBuffer[ ITEM_ICON_IDX ].isHidden = true;
            }
            IO::regularFont->setColor( 251, 1 );
            IO::regularFont->setColor( 252, 2 );

            if( p_newpok )
                if( !IO::loadPKMNSprite( "nitro:/PICS/SPRITES/PKMN/", currPkmn.m_boxdata.m_speciesId, 16, 48,
                    PKMN_SPRITE_START, PKMN_SPRITE_PAL, IO::OamTop->oamBuffer[ PKMN_SPRITE_START ].gfxIndex, false,
                    currPkmn.m_boxdata.isShiny( ), currPkmn.m_boxdata.m_isFemale, true ) ) {
                    IO::loadPKMNSprite( "nitro:/PICS/SPRITES/PKMN/", currPkmn.m_boxdata.m_speciesId, 16, 48,
                                        PKMN_SPRITE_START, PKMN_SPRITE_PAL, IO::OamTop->oamBuffer[ PKMN_SPRITE_START ].gfxIndex, false,
                                        currPkmn.m_boxdata.isShiny( ), !currPkmn.m_boxdata.m_isFemale, true );
                }

        } else {
            p_page = 0;
            BG_PALETTE[ 253 ] = RGB15( 31, 31, 31 );
            IO::regularFont->setColor( 253, 1 );

            IO::regularFont->printString( "Ei", 150, 2, false );
            IO::regularFont->printChar( '/', 234, 2, false );
            IO::regularFont->printString( "Ei", 160, 18, false );
            IO::regularFont->setColor( 251, 1 );

            for( u8 i = 0; i < 4; ++i )
                IO::OamTop->oamBuffer[ PKMN_SPRITE_START + i ].isHidden = true;
        }

        switch( p_page ) {
            case 0:
                IO::loadSprite( PAGE_ICON_IDX, PAGE_ICON_PAL, IO::OamTop->oamBuffer[ PAGE_ICON_IDX ].gfxIndex,
                                0, 0, 32, 32, memoPal, memoTiles, memoTilesLen, false, false, false, OBJPRIORITY_0, false );

                IO::regularFont->printString( "Pok�mon-Info", 36, 4, false );
                if( !( currPkmn.m_boxdata.m_individualValues.m_isEgg ) ) {
                    consoleSetWindow( &IO::Top, 16, 4, 32, 24 );
                    printf( "     Lv.%3i", currPkmn.m_Level );

                    BG_PALETTE[ 254 ] = RGB15( 31, 0, 0 );
                    BG_PALETTE[ 255 ] = RGB15( 0, 0, 31 );

                    char buffer[ 50 ];
                    sprintf( buffer, "KP                     %3i", currPkmn.m_stats.m_maxHP );
                    IO::regularFont->printString( buffer, 130, 44, false );

                    if( NatMod[ currPkmn.m_boxdata.getNature( ) ][ 0 ] == 1.1 )
                        IO::regularFont->setColor( 254, 1 );
                    else if( NatMod[ currPkmn.m_boxdata.getNature( ) ][ 0 ] == 0.9 )
                        IO::regularFont->setColor( 255, 1 );
                    else
                        IO::regularFont->setColor( 251, 1 );
                    sprintf( buffer, "ANG                   %3i", currPkmn.m_stats.m_Atk );
                    IO::regularFont->printString( buffer, 130, 69, false );

                    if( NatMod[ currPkmn.m_boxdata.getNature( ) ][ 1 ] == 1.1 )
                        IO::regularFont->setColor( 254, 1 );
                    else if( NatMod[ currPkmn.m_boxdata.getNature( ) ][ 1 ] == 0.9 )
                        IO::regularFont->setColor( 255, 1 );
                    else
                        IO::regularFont->setColor( 251, 1 );
                    sprintf( buffer, "VER                   %3i", currPkmn.m_stats.m_Def );
                    IO::regularFont->printString( buffer, 130, 86, false );

                    if( NatMod[ currPkmn.m_boxdata.getNature( ) ][ 3 ] == 1.1 )
                        IO::regularFont->setColor( 254, 1 );
                    else if( NatMod[ currPkmn.m_boxdata.getNature( ) ][ 3 ] == 0.9 )
                        IO::regularFont->setColor( 255, 1 );
                    else
                        IO::regularFont->setColor( 251, 1 );
                    sprintf( buffer, "SAN                   %3i", currPkmn.m_stats.m_SAtk );
                    IO::regularFont->printString( buffer, 130, 103, false );

                    if( NatMod[ currPkmn.m_boxdata.getNature( ) ][ 4 ] == 1.1 )
                        IO::regularFont->setColor( 254, 1 );
                    else if( NatMod[ currPkmn.m_boxdata.getNature( ) ][ 4 ] == 0.9 )
                        IO::regularFont->setColor( 255, 1 );
                    else
                        IO::regularFont->setColor( 251, 1 );
                    sprintf( buffer, "SVE                   %3i", currPkmn.m_stats.m_SDef );
                    IO::regularFont->printString( buffer, 130, 120, false );

                    if( NatMod[ currPkmn.m_boxdata.getNature( ) ][ 2 ] == 1.1 )
                        IO::regularFont->setColor( 254, 1 );
                    else if( NatMod[ currPkmn.m_boxdata.getNature( ) ][ 2 ] == 0.9 )
                        IO::regularFont->setColor( 255, 1 );
                    else
                        IO::regularFont->setColor( 251, 1 );
                    sprintf( buffer, "INI                   \xC3\xC3""%3i", currPkmn.m_stats.m_Spd );
                    IO::regularFont->printString( buffer, 130, 137, false );

                    IO::printRectangle( (u8)158, (u8)46, u8( 158 + 68 ), u8( 46 + 12 ), false, false, (u8)251 );

                    IO::printRectangle( (u8)158, (u8)46,
                                        u8( 158 + ( 68.0 * currPkmn.m_boxdata.IVget( 0 ) / 31 ) ),
                                        u8( 46 + 6 ), false, false, u8( 7 * 16 - 1 ) );
                    IO::printRectangle( (u8)158, u8( 46 + 6 ),
                                        u8( 158 + ( 68.0*currPkmn.m_boxdata.m_effortValues[ 0 ] / 252 ) ),
                                        u8( 46 + 12 ), false, false, u8( 7 * 16 - 1 ) );

                    for( int i = 1; i < 6; ++i ) {
                        IO::printRectangle( (u8)158, u8( 54 + ( 17 * i ) ),
                                            u8( 158 + 68 ), u8( 54 + 12 + ( 17 * i ) ), false, false, (u8)251 );
                        IO::printRectangle( (u8)158, u8( 54 + ( 17 * i ) ),
                                            u8( 158 + ( 68.0*currPkmn.m_boxdata.IVget( i ) / 31 ) ),
                                            u8( 54 + 6 + ( 17 * i ) ), false, false, u8( ( 7 + i ) * 16 - 1 ) );
                        IO::printRectangle( (u8)158, u8( 54 + 6 + ( 17 * i ) ),
                                            u8( 158 + ( 68.0*currPkmn.m_boxdata.m_effortValues[ i ] / 252 ) ),
                                            u8( 54 + 12 + ( 17 * i ) ), false, false, u8( ( 7 + i ) * 16 - 1 ) );
                    }
                } else {
                    if( currPkmn.m_boxdata.m_steps > 10 ) {
                        IO::regularFont->printString( "Was da wohl", 16 * 8, 50, false );
                        IO::regularFont->printString( "schl�pfen wird?", 16 * 8, 70, false );
                        IO::regularFont->printString( "Es dauert wohl", 16 * 8, 100, false );
                        IO::regularFont->printString( "noch lange.", 16 * 8, 120, false );
                    } else if( currPkmn.m_boxdata.m_steps > 5 ) {
                        IO::regularFont->printString( "Hat es sich", 16 * 8, 50, false );
                        IO::regularFont->printString( "gerade bewegt?", 16 * 8, 70, false );
                        IO::regularFont->printString( "Da tut sich", 16 * 8, 100, false );
                        IO::regularFont->printString( "wohl bald was.", 16 * 8, 120, false );
                    } else {
                        IO::regularFont->printString( "Jetzt macht es", 16 * 8, 50, false );
                        IO::regularFont->printString( "schon Ger�usche!", 16 * 8, 70, false );
                        IO::regularFont->printString( "Bald ist es", 16 * 8, 100, false );
                        IO::regularFont->printString( "wohl soweit.", 16 * 8, 120, false );
                    }
                }
                break;
            case 1:
            {
                IO::regularFont->printString( "Attacken", 36, 4, false );
                IO::loadSprite( PAGE_ICON_IDX, PAGE_ICON_PAL, IO::OamTop->oamBuffer[ PAGE_ICON_IDX ].gfxIndex,
                                0, 0, 32, 32, atksPal, atksTiles, atksTilesLen, false, false, false, OBJPRIORITY_0, false );

                consoleSetWindow( &IO::Top, 16, 5, 32, 24 );
                for( int i = 0; i < 4; i++ ) {
                    if( currPkmn.m_boxdata.m_moves[ i ] == 0 )
                        continue;
                    Type t = AttackList[ currPkmn.m_boxdata.m_moves[ i ] ]->m_moveType;
                    IO::loadTypeIcon( t, 126, 43 + 32 * i, TYPE_IDX + i, TYPE_PAL( i ),
                                      IO::OamTop->oamBuffer[ TYPE_IDX + i ].gfxIndex, false );

                    /* if(t == data.Types[0] || t == data.Types[1])
                    printf("\x1b[32m");*/

                    printf( "    %s\n    AP %2hhu""/""%2hhu ",
                            ( AttackList[ currPkmn.m_boxdata.m_moves[ i ] ]->m_moveName.c_str( ) ), currPkmn.m_boxdata.m_acPP[ i ],
                            AttackList[ currPkmn.m_boxdata.m_moves[ i ] ]->m_movePP* ( ( 5 + ( ( currPkmn.m_boxdata.m_pPUps >> ( 2 * i ) ) % 4 ) ) / 5 ) );

                    switch( AttackList[ currPkmn.m_boxdata.m_moves[ i ] ]->m_moveHitType ) {
                        case move::PHYS:
                            printf( "PHS" );
                            break;
                        case move::SPEC:
                            printf( "SPC" );
                            break;
                        case move::STAT:
                            printf( "STS" );
                            break;
                    }
                    printf( "\n    S " );
                    if( AttackList[ currPkmn.m_boxdata.m_moves[ i ] ]->m_moveBasePower )
                        printf( "%3i", AttackList[ currPkmn.m_boxdata.m_moves[ i ] ]->m_moveBasePower );
                    else
                        printf( "---" );
                    printf( " G " );
                    if( AttackList[ currPkmn.m_boxdata.m_moves[ i ] ]->m_moveAccuracy )
                        printf( "%3i", AttackList[ currPkmn.m_boxdata.m_moves[ i ] ]->m_moveAccuracy );
                    else
                        printf( "---" );

                    printf( "\n\n" );
                    printf( "\x1b[39m" );
                }
                break;
            }
            case 2:
                IO::loadSprite( PAGE_ICON_IDX, PAGE_ICON_PAL, IO::OamTop->oamBuffer[ PAGE_ICON_IDX ].gfxIndex,
                                0, 0, 32, 32, ContestPal, ContestTiles, ContestTilesLen, false, false, false, OBJPRIORITY_0, false );
                IO::regularFont->printString( "B�nder", 36, 4, false );

                break;
            default:
                return;
        }

        if( currPkmn.m_boxdata.m_individualValues.m_isEgg ) {
            IO::Oam->oamBuffer[ SUB_PAGE_ICON_IDX( 0 ) ].isHidden = false;
            IO::Oam->oamBuffer[ SUB_PAGE_ICON_IDX( 0 ) ].y = -4;
            IO::Oam->oamBuffer[ SUB_PAGE_ICON_IDX( 0 ) ].x = -8;
            IO::Oam->oamBuffer[ SUB_PAGE_ICON_IDX( 0 ) ].priority = OBJPRIORITY_1;
            IO::Oam->oamBuffer[ SUB_PAGE_ICON_IDX( 1 ) ].isHidden = false;
            IO::Oam->oamBuffer[ SUB_PAGE_ICON_IDX( 1 ) ].y = 18;
            IO::Oam->oamBuffer[ SUB_PAGE_ICON_IDX( 1 ) ].x = -8;
            IO::Oam->oamBuffer[ SUB_PAGE_ICON_IDX( 1 ) ].priority = OBJPRIORITY_1;
            IO::Oam->oamBuffer[ SUB_PAGE_ICON_IDX( 2 ) ].isHidden = false;
            IO::Oam->oamBuffer[ SUB_PAGE_ICON_IDX( 2 ) ].y = -8;
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
                IO::Oam->oamBuffer[ CHOICE_ID + 1 ].x = 256 - 32;
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
            IO::regularFont->setColor( 254, 1 );
            IO::regularFont->setColor( 255, 2 );
            BG_PALETTE_SUB[ 253 ] = currPkmn.m_boxdata.m_oTisFemale ? RGB15( 31, 15, 0 ) : RGB15( 0, 15, 31 );
            BG_PALETTE_SUB[ 255 ] = RGB15( 0, 0, 0 );
            BG_PALETTE_SUB[ 254 ] = RGB15( 31, 31, 31 );

            IO::regularFont->printString( "OT:", 28, 22, true );
            IO::regularFont->setColor( 253, 2 );
            IO::regularFont->printString( currPkmn.m_boxdata.m_oT, 56, 16, true );
            IO::regularFont->setColor( 255, 2 );

            if( !currPkmn.m_boxdata.m_individualValues.m_isEgg ) {
                auto acAbility = ability( currPkmn.m_boxdata.m_ability );

                BG_PALETTE_SUB[ 250 ] = RGB15( 31, 31, 31 );
                IO::printRectangle( u8( 0 ), u8( 138 ), u8( 255 ), u8( 192 ), true, false, 250 );
                IO::regularFont->setColor( 253, 2 );
                u8 nlCnt = 0;
                auto nStr = FS::breakString( acAbility.m_flavourText, IO::regularFont, 220 );
                for( auto c : nStr )
                    if( c == '\n' )
                        nlCnt++;
                IO::regularFont->printString( nStr.c_str( ), 0, 138, true, u8( 16 - 2 * nlCnt ) );
                IO::regularFont->printString( acAbility.m_abilityName.c_str( ), 5, 176, true );
                IO::regularFont->setColor( 254, 1 );
                IO::regularFont->setColor( 255, 2 );
            }

            sprintf( buffer, "(%05i/%05i)", currPkmn.m_boxdata.m_oTId, currPkmn.m_boxdata.m_oTSid );
            IO::regularFont->printString( buffer, 50, 30, true );

            if( currPkmn.m_boxdata.m_oTId == FS::SAV->m_Id && currPkmn.m_boxdata.m_oTSid == FS::SAV->m_Sid ) //Trainer is OT
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
                        sprintf( buffer, "Off. gef. am %02i.%02i.%02i mit Lv. %i.",
                                 currPkmn.m_boxdata.m_hatchDate[ 0 ],
                                 currPkmn.m_boxdata.m_hatchDate[ 1 ],
                                 currPkmn.m_boxdata.m_hatchDate[ 2 ],
                                 currPkmn.m_boxdata.m_gotLevel );
                    } else
                        sprintf( buffer, "Offenbar gefangen mit Lv. %i.", currPkmn.m_boxdata.m_gotLevel );
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
        if( !currPkmn.m_boxdata.m_individualValues.m_isEgg ) {
            consoleSelect( &IO::Top );
            consoleSetWindow( &IO::Top, 4, 5, 12, 2 );

            u16 exptype = data.m_expType;

            printf( "EP(%3i%%)\nKP(%3i%%)", ( currPkmn.m_boxdata.m_experienceGained - EXP[ currPkmn.m_Level - 1 ][ exptype ] ) * 100 /
                    ( EXP[ currPkmn.m_Level ][ exptype ] - EXP[ currPkmn.m_Level - 1 ][ exptype ] ),
                    currPkmn.m_stats.m_acHP * 100 / currPkmn.m_stats.m_maxHP );
            IO::displayHP( 100, 101, 46, 80, 97, 98, false, 50, 56 );
            IO::displayHP( 100, 100 - currPkmn.m_stats.m_acHP * 100 / currPkmn.m_stats.m_maxHP, 46, 80, 97, 98, false, 50, 56 );

            IO::displayEP( 100, 101, 46, 80, 99, 100, false, 59, 62 );
            IO::displayEP( 0, ( currPkmn.m_boxdata.m_experienceGained - EXP[ currPkmn.m_Level - 1 ][ exptype ] ) * 100 /
                           ( EXP[ currPkmn.m_Level ][ exptype ] - EXP[ currPkmn.m_Level - 1 ][ exptype ] ), 46, 80, 99, 100, false, 59, 62 );

        }

        IO::updateOAM( true );
        IO::updateOAM( false );
    }

    void regStsScreenUI::draw( u8 p_current ) {
        consoleSetWindow( &IO::Top, positions[ _current ][ 0 ], positions[ _current ][ 1 ], 2, 2 );
        consoleClear( );

        _current = p_current;

        initSub( _current );

        consoleSetWindow( &IO::Top, positions[ _current ][ 0 ], positions[ _current ][ 1 ], 2, 2 );
        if( _current & 1 )
            printf( ">" );
        else
            printf( "<" );
    }
}