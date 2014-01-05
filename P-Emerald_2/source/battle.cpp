#include <nds.h>

#include "battle.h"
#include "PKMN.h"
#include "attack.h"
#include "sprite.h"
#include "item.h"
#include "scrnloader.h"
#include "mbox.h"
#include "savgm.h"
#include "bag.h"

#include "Message.h"
#include "Border_1.h"
#include "Border_2.h"
#include "Border_3.h"
#include "Border_4.h"
#include "Border_5.h"
#include "Back.h"

#include "Battle1.h"
#include "Battle2.h"

#include "BattleSub1.h"
#include "BattleSub2.h"
#include "BattleSub3.h"
#include "BattleSub4.h"
#include "BattleSub5.h"
#include "BattleSub6.h"

#include "Choice_1.h"
#include "Choice_2.h"
#include "Choice_3.h"

#include "BattleBall1.h" //Normal
#include "BattleBall2.h" //Fainted
#include "BattleBall3.h" //Statused

#include "PokeBall1.h"
#include "PokeBall2.h"
#include "PokeBall3.h"
#include "PokeBall4.h"
#include "PokeBall5.h"
#include "PokeBall6.h"
#include "PokeBall7.h"
#include "PokeBall8.h"
#include "PokeBall9.h"
#include "PokeBall10.h"
#include "PokeBall11.h"
#include "PokeBall12.h"
#include "PokeBall13.h"
#include "PokeBall14.h"
#include "PokeBall15.h"
#include "PokeBall16.h"
#include "PokeBall17.h"

#include "Shiny1.h"
#include "Shiny2.h"


extern OAMTable *oam;
extern SpriteInfo spriteInfo[SPRITE_COUNT];

extern OAMTable *oamTop;
extern SpriteInfo spriteInfoTop[SPRITE_COUNT];

extern font::Font cust_font;
extern font::Font cust_font2;
namespace BATTLE{
    const char* trainerclassnames[] = { "Pok�mon-Trainer" };

    void displayHP(int HPstart,int HP,int x,int y,int freecolor1,int freecolor2,bool delay,bool big){
        if(big)
            displayHP(HPstart,HP,x,y,freecolor1,freecolor2,delay,20,24);
        else
            displayHP(HPstart,HP,x,y,freecolor1,freecolor2,delay,8,12);
    }
    void displayHP(int HPstart,int HP,int x,int y,int freecolor1,int freecolor2,bool delay,int innerR,int outerR){
        int factor = std::max(1,outerR / 15);
        if(HP > 100 || HP < 0){
            BG_PALETTE[freecolor1] = GREEN;
            for(int i= 0; i < factor *100; ++i)
                for(int j= innerR; j <= outerR; ++j){
                    u8 nx = x + 16 + j * sin((50-i/(1.0*factor))*acos(0) / 30),ny = y + 16 + j * cos((50-i/(1.0*factor))*acos(0) / 30);
                    ((Color *)BG_BMP_RAM(1))[(nx + ny * SCREEN_WIDTH)/2] = (((u8)freecolor1)<<8) | (u8)freecolor1;
                    //printf("%i %i; ",nx,ny);
            }
        }
        else {
            BG_PALETTE[freecolor2] = NORMAL_;
            for(int i= factor*100-factor*HPstart; i < factor*HP; ++i){
                for(int j= innerR; j <= outerR; ++j){
                    u8 nx = x + 16 + j * sin((50-i/(1.0*factor))*acos(0) / 30),ny = y + 16 + j * cos((50-i/(1.0*factor))*acos(0) / 30);
                    ((Color *)BG_BMP_RAM(1))[(nx + ny * SCREEN_WIDTH)/2] = (((u8)freecolor2)<<8) | (u8)freecolor2;
                    if(i ==  factor * 50)
                        BG_PALETTE[freecolor1] = YELLOW;
                    if(i == factor * 80)
                        BG_PALETTE[freecolor1] = RED;
                }
                if(delay)
                    swiWaitForVBlank();
            }
        }
    }
    void displayEP(int EPstart,int EP,int x,int y,int freecolor1,int freecolor2,bool delay,int innerR,int outerR){
        if(EPstart >= 100 || EP > 100){
            BG_PALETTE[freecolor1] = NORMAL_;
            for(int i= 0; i < 300; ++i)
                for(int j= innerR; j <= outerR; ++j){
                    int nx = x + 16 + j * sin((50-i/3.0)*acos(0) / 30),ny = y + 16 + j * cos((50-i/3.0)*acos(0) / 30);
                    ((Color *)BG_BMP_RAM(1))[(nx + ny * SCREEN_WIDTH)/2] = (((u8)freecolor1)<<8) | (u8)freecolor1;
                    //printf("%i %i; ",nx,ny);
            }
        }
        else {
            BG_PALETTE[freecolor2] = ICE;
            for(int i= EPstart*3; i <= EP*3; ++i){
                for(int j= innerR; j <= outerR; ++j){
                    int nx = x + 16 + j * sin((50-i/3.0)*acos(0) / 30),ny = y + 16 + j * cos((50-i/3.0)*acos(0) / 30);
                    ((Color *)BG_BMP_RAM(1))[(nx + ny * SCREEN_WIDTH)/2] = (((u8)freecolor2)<<8) | (u8)freecolor2;
                }
                if(delay)
                    swiWaitForVBlank();
            }
        }
    }

    void init(){
        for(int i= 5;i <= 8; ++i) {
            oam->oamBuffer[i].isHidden = true;
            swiWaitForVBlank();
        }
        for(int i= 0;i < 4; ++i) {
            oam->oamBuffer[9+2*i].isHidden = true;
            swiWaitForVBlank();
        }
    
        updateOAMSub(oam);
    
        cust_font.set_color(0,0);
        cust_font.set_color(251,1);
        cust_font.set_color(252,2);
        cust_font2.set_color(0,0);
        cust_font2.set_color(253,1);
        cust_font2.set_color(254,2);
    
        BG_PALETTE_SUB[250] = RGB15(31,31,31);
        BG_PALETTE_SUB[251] = RGB15(15,15,15);
        BG_PALETTE_SUB[252] = RGB15(3,3,3);
        BG_PALETTE_SUB[253] = RGB15(15,15,15);
        BG_PALETTE_SUB[254] = RGB15(31,31,31);
        font::putrec(0,0,256,63,true,false,250);
    
        updateOAMSub(oam);
    }
    void clear(){
        font::putrec(0,0,256,63,true,false,250);
    }
    void dinit(){
        if(!BGs[BG_ind].load_from_rom){
            dmaCopy(BGs[BG_ind].MainMenu, bgGetGfxPtr(bg3sub), 256*256);
            dmaCopy(BGs[BG_ind].MainMenuPal, BG_PALETTE_SUB, 256*2); 
        }
        else if(!loadNavScreen(bgGetGfxPtr(bg3sub),BGs[BG_ind].Name.c_str(),BG_ind)){
            dmaCopy(BGs[0].MainMenu, bgGetGfxPtr(bg3sub), 256*256);
            dmaCopy(BGs[0].MainMenuPal, BG_PALETTE_SUB, 256*2); 
            BG_ind = 0;
        }

        for(int i= 5; i< 8; ++i)
            oam->oamBuffer[31+2*i].isHidden = false;
        for(int i= 0;i < 4; ++i) {
            oam->oamBuffer[9+2*i].isHidden = false;
            swiWaitForVBlank();
        }
        updateOAMSub(oam);
    }

    battle::battle(battle_trainer* player, battle_trainer* opponent,int max_round,int AI_level,BattleMode battlemode){
        this->max_round = max_round;
        this->AI_level = AI_level;
        this->player = player;
        this->opponent = opponent;
        this->battlemode = battlemode;
    }
    int oamIndex, palcnt,  nextAvailableTileIdx;
    int oamIndexS, palcntS,  nextAvailableTileIdxS;
    void initinitBattleScrnSprites(OAMTable* oam, SpriteInfo* spriteInfo,int ownPok,int oppPok){
        static const int BYTES_PER_16_COLOR_TILE = 32;
        static const int COLORS_PER_PALETTE = 16;
        static const int BOUNDARY_VALUE = 32; /* This is the default boundary value
                                            * (can be set in REG_DISPCNT) */
        static const int OFFSET_MULTIPLIER = BOUNDARY_VALUE / sizeof(SPRITE_GFX[0]);
        oamIndex = palcnt = nextAvailableTileIdx = 0;
        
        SpriteInfo * type1Info = &spriteInfo[oamIndex];
        SpriteEntry * type1 = &oam->oamBuffer[oamIndex];
        type1Info->oamId = oamIndex;
        type1Info->width = 16;
        type1Info->height = 16;
        type1Info->angle = 0;
        type1Info->entry = type1;
        type1->y = 0;
        type1->isRotateScale = false; 
        type1->isHidden = false;
        type1->blendMode = OBJMODE_NORMAL;
        type1->isMosaic = false;
        type1->colorMode = OBJCOLOR_16;
        type1->shape = OBJSHAPE_SQUARE;
        type1->x = 0;
        type1->size = OBJSIZE_16;
        type1->gfxIndex = nextAvailableTileIdx;
        type1->priority = OBJPRIORITY_0;
        type1->palette = palcnt;

        for(int i= 0; i< 11; ++i){
            spriteInfo[++oamIndex] = *type1Info;
            oam->oamBuffer[oamIndex] = *type1;
            oam->oamBuffer[oamIndex].x = i < 5 ? 16 + i*16 : 256 - (i-4)*16;
            oam->oamBuffer[oamIndex].isHidden = i < 5 ? oppPok < i : ownPok + 5 < i;
            oam->oamBuffer[oamIndex].y = i >= 5 ? 192-16:0;
        }
        
        dmaCopyHalfWords(SPRITE_DMA_CHANNEL, BattleBall1Pal, &SPRITE_PALETTE[(palcnt++) * COLORS_PER_PALETTE], 32);        
        dmaCopyHalfWords(SPRITE_DMA_CHANNEL, BattleBall1Tiles, &SPRITE_GFX[nextAvailableTileIdx * OFFSET_MULTIPLIER], BattleBall1TilesLen);
        nextAvailableTileIdx += BattleBall1TilesLen / BYTES_PER_16_COLOR_TILE;
        dmaCopyHalfWords(SPRITE_DMA_CHANNEL, BattleBall2Pal, &SPRITE_PALETTE[(palcnt++) * COLORS_PER_PALETTE], 32);        
        dmaCopyHalfWords(SPRITE_DMA_CHANNEL, BattleBall2Tiles, &SPRITE_GFX[nextAvailableTileIdx * OFFSET_MULTIPLIER], BattleBall2TilesLen);
        nextAvailableTileIdx += BattleBall2TilesLen / BYTES_PER_16_COLOR_TILE;
        dmaCopyHalfWords(SPRITE_DMA_CHANNEL, BattleBall3Pal, &SPRITE_PALETTE[(palcnt++) * COLORS_PER_PALETTE], 32);        
        dmaCopyHalfWords(SPRITE_DMA_CHANNEL, BattleBall3Tiles, &SPRITE_GFX[nextAvailableTileIdx * OFFSET_MULTIPLIER], BattleBall3TilesLen);
        nextAvailableTileIdx += BattleBall3TilesLen / BYTES_PER_16_COLOR_TILE;
        
        SpriteInfo * Bo4Info = &spriteInfo[++oamIndex];
        SpriteEntry * Bo4 = &oam->oamBuffer[oamIndex];
        Bo4Info->oamId = oamIndex;
        Bo4Info->width = 64;
        Bo4Info->height = 64;
        Bo4Info->angle = 0;
        Bo4Info->entry = Bo4;
        Bo4->y = 0;
        Bo4->isRotateScale = false;
        Bo4->blendMode = OBJMODE_NORMAL;
        Bo4->isMosaic = false;
        Bo4->colorMode = OBJCOLOR_16;
        Bo4->shape = OBJSHAPE_SQUARE;
        Bo4->isHidden = false;
        Bo4->x = 0;
        Bo4->size = OBJSIZE_64;
        Bo4->gfxIndex = nextAvailableTileIdx;
        Bo4->priority = OBJPRIORITY_2;
        Bo4->palette = palcnt;
        Bo4->vFlip = true;
        Bo4->hFlip = true;

        Bo4 = &oam->oamBuffer[++oamIndex];
        Bo4->y = 128;
        Bo4->isRotateScale = false;
        Bo4->blendMode = OBJMODE_NORMAL;
        Bo4->isMosaic = false;
        Bo4->colorMode = OBJCOLOR_16;
        Bo4->shape = OBJSHAPE_SQUARE;
        Bo4->isHidden = false;
        Bo4->x = 192;
        Bo4->size = OBJSIZE_64;
        Bo4->gfxIndex = nextAvailableTileIdx;
        Bo4->priority = OBJPRIORITY_2;
        Bo4->palette = palcnt;
        Bo4->vFlip = false;
        Bo4->hFlip = false;

        dmaCopyHalfWords(SPRITE_DMA_CHANNEL, Border_4Pal, &SPRITE_PALETTE[palcnt * COLORS_PER_PALETTE], 32);
        dmaCopyHalfWords(SPRITE_DMA_CHANNEL, Border_4Tiles, &SPRITE_GFX[nextAvailableTileIdx * OFFSET_MULTIPLIER], Border_4TilesLen);
        nextAvailableTileIdx += Border_4TilesLen / BYTES_PER_16_COLOR_TILE;
    
        SpriteInfo * Bo3Info = &spriteInfo[++oamIndex];
        SpriteEntry * Bo3 = &oam->oamBuffer[oamIndex];
        Bo3Info->oamId = oamIndex;
        Bo3Info->width = 64;
        Bo3Info->height = 64;
        Bo3Info->angle = 0;
        Bo3Info->entry = Bo3;
        Bo3->y = 0;
        Bo3->isRotateScale = false;
        Bo3->blendMode = OBJMODE_NORMAL;
        Bo3->isMosaic = false;
        Bo3->colorMode = OBJCOLOR_16;
        Bo3->shape = OBJSHAPE_SQUARE;
        Bo3->isHidden = false;
        Bo3->x = 64;
        Bo3->size = OBJSIZE_64;
        Bo3->gfxIndex = nextAvailableTileIdx;
        Bo3->priority = OBJPRIORITY_2;
        Bo3->palette = palcnt;
        Bo3->vFlip = true;
        Bo3->hFlip = true;

        Bo3 = &oam->oamBuffer[++oamIndex];
        Bo3->y = 128;
        Bo3->isRotateScale = false;
        Bo3->blendMode = OBJMODE_NORMAL;
        Bo3->isMosaic = false;
        Bo3->colorMode = OBJCOLOR_16;
        Bo3->shape = OBJSHAPE_SQUARE;
        Bo3->isHidden = false;
        Bo3->x = 128;
        Bo3->size = OBJSIZE_64;
        Bo3->gfxIndex = nextAvailableTileIdx;
        Bo3->priority = OBJPRIORITY_2;
        Bo3->palette = palcnt;
        Bo3->vFlip = false;
        Bo3->hFlip = false;

        dmaCopyHalfWords(SPRITE_DMA_CHANNEL,
        Border_3Tiles,
        &SPRITE_GFX[nextAvailableTileIdx * OFFSET_MULTIPLIER],
        Border_3TilesLen);
    
        nextAvailableTileIdx += Border_3TilesLen / BYTES_PER_16_COLOR_TILE;
    
        SpriteInfo * Bo2Info = &spriteInfo[++oamIndex];
        SpriteEntry * Bo2 = &oam->oamBuffer[oamIndex];
        Bo2Info->oamId = oamIndex;
        Bo2Info->width = 64;
        Bo2Info->height = 64;
        Bo2Info->angle = 0;
        Bo2Info->entry = Bo2;
        Bo2->y = 0;
        Bo2->isRotateScale = false;
        Bo2->blendMode = OBJMODE_NORMAL;
        Bo2->isMosaic = false;
        Bo2->colorMode = OBJCOLOR_16;
        Bo2->shape = OBJSHAPE_SQUARE;
        Bo2->isHidden = false;
        Bo2->x = 128;
        Bo2->size = OBJSIZE_64;
        Bo2->gfxIndex = nextAvailableTileIdx;
        Bo2->priority = OBJPRIORITY_2;
        Bo2->palette = palcnt;
        Bo2->vFlip = true;
        Bo2->hFlip = true;

        Bo2 = &oam->oamBuffer[++oamIndex];
        Bo2->y = 128;
        Bo2->isRotateScale = false;
        Bo2->blendMode = OBJMODE_NORMAL;
        Bo2->isMosaic = false;
        Bo2->colorMode = OBJCOLOR_16;
        Bo2->shape = OBJSHAPE_SQUARE;
        Bo2->isHidden = false;
        Bo2->x = 64;
        Bo2->size = OBJSIZE_64;
        Bo2->gfxIndex = nextAvailableTileIdx;
        Bo2->priority = OBJPRIORITY_2;
        Bo2->palette = palcnt;
        Bo2->vFlip = false;
        Bo2->hFlip = false;

        dmaCopyHalfWords(SPRITE_DMA_CHANNEL,
        Border_2Tiles,
        &SPRITE_GFX[nextAvailableTileIdx * OFFSET_MULTIPLIER],
        Border_2TilesLen);
        nextAvailableTileIdx += Border_2TilesLen / BYTES_PER_16_COLOR_TILE;
    
        SpriteInfo * Bo1Info = &spriteInfo[++oamIndex];
        SpriteEntry * Bo1 = &oam->oamBuffer[oamIndex];
        Bo1Info->oamId = oamIndex;
        Bo1Info->width = 64;
        Bo1Info->height = 64;
        Bo1Info->angle = 0;
        Bo1Info->entry = Bo1;
        Bo1->y = 0;
        Bo1->isRotateScale = false;
        Bo1->blendMode = OBJMODE_NORMAL;
        Bo1->isMosaic = false;
        Bo1->colorMode = OBJCOLOR_16;
        Bo1->shape = OBJSHAPE_SQUARE;
        Bo1->isHidden = false;
        Bo1->x = 192;
        Bo1->size = OBJSIZE_64;
        Bo1->gfxIndex = nextAvailableTileIdx;
        Bo1->priority = OBJPRIORITY_2;
        Bo1->palette = palcnt;
        Bo1->vFlip = true;
        Bo1->hFlip = true;

        Bo1 = &oam->oamBuffer[++oamIndex];
        Bo1->y = 128;
        Bo1->isRotateScale = false;
        Bo1->blendMode = OBJMODE_NORMAL;
        Bo1->isMosaic = false;
        Bo1->colorMode = OBJCOLOR_16;
        Bo1->shape = OBJSHAPE_SQUARE;
        Bo1->isHidden =  false;
        Bo1->x = 0;
        Bo1->size = OBJSIZE_64;
        Bo1->gfxIndex = nextAvailableTileIdx;
        Bo1->priority = OBJPRIORITY_2;
        Bo1->palette = palcnt;
        Bo1->vFlip = false;
        Bo1->hFlip = false;

        dmaCopyHalfWords(SPRITE_DMA_CHANNEL,
        Border_1Tiles,
        &SPRITE_GFX[nextAvailableTileIdx * OFFSET_MULTIPLIER],
        Border_1TilesLen);
        nextAvailableTileIdx += Border_1TilesLen / BYTES_PER_16_COLOR_TILE;
    
        SpriteInfo * Bo5Info = &spriteInfo[++oamIndex];
        SpriteEntry * Bo5 = &oam->oamBuffer[oamIndex];
        Bo5Info->oamId = oamIndex;
        Bo5Info->width = 64;
        Bo5Info->height = 64;
        Bo5Info->angle = 0;
        Bo5Info->entry = Bo5;
        Bo5->y = 64;
        Bo5->isRotateScale = false;
        Bo5->blendMode = OBJMODE_NORMAL;
        Bo5->isMosaic = false;
        Bo5->colorMode = OBJCOLOR_16;
        Bo5->shape = OBJSHAPE_SQUARE;
        Bo5->isHidden = false;
        Bo5->x = 0;
        Bo5->size = OBJSIZE_64;
        Bo5->gfxIndex = nextAvailableTileIdx;
        Bo5->priority = OBJPRIORITY_2;
        Bo5->palette = palcnt;
        Bo5->vFlip = true;
        Bo5->hFlip = true;

        Bo5 = &oam->oamBuffer[++oamIndex];
        Bo5->y = 64;
        Bo5->isRotateScale = false;
        Bo5->blendMode = OBJMODE_NORMAL;
        Bo5->isMosaic = false;
        Bo5->colorMode = OBJCOLOR_16;
        Bo5->shape = OBJSHAPE_SQUARE;
        Bo5->isHidden = false;
        Bo5->x = 192;
        Bo5->size = OBJSIZE_64;
        Bo5->gfxIndex = nextAvailableTileIdx;
        Bo5->priority = OBJPRIORITY_2;
        Bo5->palette = palcnt;
        Bo5->vFlip = false;
        Bo5->hFlip = false;

        dmaCopyHalfWords(SPRITE_DMA_CHANNEL, Border_5Tiles, &SPRITE_GFX[nextAvailableTileIdx * OFFSET_MULTIPLIER], Border_5TilesLen);
        nextAvailableTileIdx += Border_5TilesLen / BYTES_PER_16_COLOR_TILE;

        ++palcnt;
    }
    
    void battle::initBattleScreen(){
        char buf[100];
        sprintf(buf,"%i.raw",this->opponent->trainer_class);
        
        loadPicture(bgGetGfxPtr(bg3sub),"nitro:/PICS/","ClearD");
        loadPicture(bgGetGfxPtr(bg3),"nitro:/PICS/","Clear");
        initinitBattleScrnSprites(oamTop,spriteInfoTop,6,6);

        for(int i= 0; i< 6; ++i)
            switch (this->acpoksts[acpokpos[i][1]][1])
            {
            case NA:
                oamTop->oamBuffer[i].isHidden = true;
                break;
            case KO:
                oamTop->oamBuffer[i].gfxIndex += BattleBall2TilesLen / 32;
                oamTop->oamBuffer[i].palette++;
                break;
            case STS:
                oamTop->oamBuffer[i].gfxIndex += BattleBall2TilesLen / 16;
                oamTop->oamBuffer[i].palette += 2;
                break;
            default:
                break;
            }
        for(int i= 6; i< 12; ++i)
            switch (this->acpoksts[acpokpos[i-6][0]][0])
            {
            case NA:
                oamTop->oamBuffer[i].isHidden = true;
                break;
            case KO:
                oamTop->oamBuffer[i].gfxIndex += BattleBall2TilesLen / 32;
                oamTop->oamBuffer[i].palette++;
                break;
            case STS:
                oamTop->oamBuffer[i].gfxIndex += BattleBall2TilesLen / 16;
                oamTop->oamBuffer[i].palette += 2;
                break;
            default:
                break;
            }
        updateOAM(oamTop);


        consoleSetWindow(&Top,0,0,32,24);
        consoleSelect(&Top);
        consoleClear();
        consoleSetWindow(&Bottom,0,0,32,24);
        consoleClear();
        consoleSetWindow(&Bottom,2,11,32,24);
        consoleSelect(&Bottom);
         
        cust_font.set_color(0,0);
        cust_font.set_color(251,1);
        cust_font.set_color(252,2);
        cust_font2.set_color(0,0);
        cust_font2.set_color(253,1);
        cust_font2.set_color(254,2);
    
        BG_PALETTE_SUB[250] = RGB15(31,31,31);
        BG_PALETTE_SUB[251] = RGB15(30,30,30);
        BG_PALETTE_SUB[252] = RGB15(15,15,15);
        BG_PALETTE_SUB[253] = RGB15(15,15,15);
        BG_PALETTE_SUB[254] = RGB15(31,31,31);
        sprintf(buf,"Eine Herausforderung von\n %s %s!",trainerclassnames[this->opponent->trainer_class],this->opponent->Name);
        cust_font.print_string(buf,16,80,true);

        loadTrainerSprite(oamTop,spriteInfoTop,"nitro:/PICS/SPRITES/TRAINER/", "n",144,16,oamIndex,palcnt,nextAvailableTileIdx,false);
        updateOAM(oamTop);
        for(int i = 0; i< 8; ++i)
            swiWaitForVBlank();
        oamIndex -= 4;
        --palcnt;
            --palcnt;
        nextAvailableTileIdx -= 144;
        loadTrainerSprite(oamTop,spriteInfoTop,"nitro:/PICS/SPRITES/TRAINER/", "n2",144,16,oamIndex,palcnt,nextAvailableTileIdx,false);
        updateOAM(oamTop);
        for(int i = 0; i< 8; ++i)
            swiWaitForVBlank();
        oamIndex -= 4;
        --palcnt;
            --palcnt;
        nextAvailableTileIdx -= 144;
        loadTrainerSprite(oamTop,spriteInfoTop,"nitro:/PICS/SPRITES/TRAINER/", "n3",144,16,oamIndex,palcnt,nextAvailableTileIdx,false);
        updateOAM(oamTop);
        oamIndex -= 4;
        --palcnt;
            --palcnt;
        nextAvailableTileIdx -= 144;
        for(int i = 0; i< 20; ++i)
            swiWaitForVBlank();

        for(int l = 0; l < 25; ++l){
            loadTrainerSprite(oamTop,spriteInfoTop,"nitro:/PICS/SPRITES/TRAINER/", "n3",144+4*l,16,oamIndex,palcnt,nextAvailableTileIdx,false);
            updateOAM(oamTop);
            oamIndex -= 4;
            --palcnt;
            --palcnt;
            nextAvailableTileIdx -= 144;
            for(int i = 0; i< 3; ++i)
                swiWaitForVBlank();
        }

        loadPicture(bgGetGfxPtr(bg3sub),"nitro:/PICS/","ClearD");
        consoleSetWindow(&Bottom,0,0,32,24);
        consoleClear();
    }
    void initBattleScreenSprites(OAMTable* oam, SpriteInfo* spriteInfo){
        static const int BYTES_PER_16_COLOR_TILE = 32;
        static const int COLORS_PER_PALETTE = 16;
        static const int BOUNDARY_VALUE = 32; /* This is the default boundary value
                                            * (can be set in REG_DISPCNT) */
        static const int OFFSET_MULTIPLIER = BOUNDARY_VALUE / sizeof(SPRITE_GFX[0]);
        oamIndex = palcnt = nextAvailableTileIdx = 0;

        SpriteInfo * type1Info = &spriteInfo[++oamIndex];
        SpriteEntry * type1 = &oam->oamBuffer[oamIndex];
        type1Info->oamId = oamIndex;
        type1Info->width = 32;
        type1Info->height = 32;
        type1Info->angle = 0;
        type1Info->entry = type1;
        type1->y = 192-32-8;
        type1->isRotateScale = false;
        type1->isHidden = false;
        type1->blendMode = OBJMODE_NORMAL;
        type1->isMosaic = false;
        type1->colorMode = OBJCOLOR_16;
        type1->shape = OBJSHAPE_SQUARE;
        type1->x = 256-36;
        type1->size = OBJSIZE_32;
        type1->gfxIndex = nextAvailableTileIdx;
        type1->priority = OBJPRIORITY_2;
        type1->palette = palcnt;
        
        spriteInfo[++oamIndex] = *type1Info;
        oam->oamBuffer[oamIndex] = *type1;
        oam->oamBuffer[oamIndex].x -= 88;
        oam->oamBuffer[oamIndex].y -= 32;
        oam->oamBuffer[oamIndex].priority = OBJPRIORITY_2;

        dmaCopyHalfWords(SPRITE_DMA_CHANNEL, Battle1Pal, &SPRITE_PALETTE[(palcnt++) * COLORS_PER_PALETTE], 32);
        dmaCopyHalfWords(SPRITE_DMA_CHANNEL, Battle1Tiles, &SPRITE_GFX[nextAvailableTileIdx * OFFSET_MULTIPLIER], Battle1TilesLen);
        nextAvailableTileIdx += Battle1TilesLen / BYTES_PER_16_COLOR_TILE;

        type1Info = &spriteInfo[++oamIndex];
        type1 = &oam->oamBuffer[oamIndex];
        type1Info->oamId = oamIndex;
        type1Info->width = 32;
        type1Info->height = 32;
        type1Info->angle = 0;
        type1Info->entry = type1;
        type1->y = 8;
        type1->isRotateScale = false;
        type1->isHidden = false;
        type1->blendMode = OBJMODE_NORMAL;
        type1->isMosaic = false;
        type1->colorMode = OBJCOLOR_16;
        type1->shape = OBJSHAPE_SQUARE;
        type1->x = 0;
        type1->size = OBJSIZE_32;
        type1->gfxIndex = nextAvailableTileIdx;
        type1->priority = OBJPRIORITY_2;
        type1->palette = palcnt;
        
        spriteInfo[++oamIndex] = *type1Info;
        oam->oamBuffer[oamIndex] = *type1;
        oam->oamBuffer[oamIndex].x += 88;
        oam->oamBuffer[oamIndex].y += 24;
        oam->oamBuffer[oamIndex].priority = OBJPRIORITY_2;

        dmaCopyHalfWords(SPRITE_DMA_CHANNEL, Battle2Pal, &SPRITE_PALETTE[(palcnt++) * COLORS_PER_PALETTE], 32);
        dmaCopyHalfWords(SPRITE_DMA_CHANNEL, Battle2Tiles, &SPRITE_GFX[nextAvailableTileIdx * OFFSET_MULTIPLIER], Battle2TilesLen);
        nextAvailableTileIdx += Battle2TilesLen / BYTES_PER_16_COLOR_TILE;
        
        
        type1Info = &spriteInfo[++oamIndex];
        type1 = &oam->oamBuffer[oamIndex];
        type1Info->oamId = oamIndex;
        type1Info->width = 16;
        type1Info->height = 16;
        type1Info->angle = 0;
        type1Info->entry = type1;
        type1->y = 0;
        type1->isRotateScale = false; 
        type1->isHidden = true;
        type1->blendMode = OBJMODE_NORMAL;
        type1->isMosaic = false;
        type1->colorMode = OBJCOLOR_16;
        type1->shape = OBJSHAPE_SQUARE;
        type1->x = 0;
        type1->size = OBJSIZE_16;
        type1->gfxIndex = nextAvailableTileIdx;
        type1->priority = OBJPRIORITY_0;
        type1->palette = palcnt;

        for(int i = 1; i < 12; ++i){
            spriteInfo[++oamIndex] = *type1Info;
            oam->oamBuffer[oamIndex] = *type1;
        }
        
        dmaCopyHalfWords(SPRITE_DMA_CHANNEL, BattleBall1Pal, &SPRITE_PALETTE[(palcnt++) * COLORS_PER_PALETTE], 32);        
        dmaCopyHalfWords(SPRITE_DMA_CHANNEL, BattleBall1Tiles, &SPRITE_GFX[nextAvailableTileIdx * OFFSET_MULTIPLIER], BattleBall1TilesLen);
        nextAvailableTileIdx += BattleBall1TilesLen / BYTES_PER_16_COLOR_TILE;
        dmaCopyHalfWords(SPRITE_DMA_CHANNEL, BattleBall2Pal, &SPRITE_PALETTE[(palcnt++) * COLORS_PER_PALETTE], 32);        
        dmaCopyHalfWords(SPRITE_DMA_CHANNEL, BattleBall2Tiles, &SPRITE_GFX[nextAvailableTileIdx * OFFSET_MULTIPLIER], BattleBall2TilesLen);
        nextAvailableTileIdx += BattleBall2TilesLen / BYTES_PER_16_COLOR_TILE;
        dmaCopyHalfWords(SPRITE_DMA_CHANNEL, BattleBall3Pal, &SPRITE_PALETTE[(palcnt++) * COLORS_PER_PALETTE], 32);        
        dmaCopyHalfWords(SPRITE_DMA_CHANNEL, BattleBall3Tiles, &SPRITE_GFX[nextAvailableTileIdx * OFFSET_MULTIPLIER], BattleBall3TilesLen);
        nextAvailableTileIdx += BattleBall3TilesLen / BYTES_PER_16_COLOR_TILE;


    }
    void initBattleSubScreenSprites(OAMTable* oam, SpriteInfo* spriteInfo,bool isWild,bool hasPNav){
        static const int BYTES_PER_16_COLOR_TILE = 32;
        static const int COLORS_PER_PALETTE = 16;
        static const int BOUNDARY_VALUE = 32; /* This is the default boundary value
                                            * (can be set in REG_DISPCNT) */
        static const int OFFSET_MULTIPLIER = BOUNDARY_VALUE / sizeof(SPRITE_GFX_SUB[0]);
        oamIndexS = palcntS = nextAvailableTileIdxS = 0;


        SpriteInfo * type1Info = &spriteInfo[oamIndexS];
        SpriteEntry * type1 = &oam->oamBuffer[oamIndexS];
        type1Info->oamId = oamIndexS;
        type1Info->width = 64;
        type1Info->height = 64;
        type1Info->angle = 0;
        type1Info->entry = type1;
        type1->y = 72;
        type1->isRotateScale = false;
        type1->isHidden = true;
        type1->blendMode = OBJMODE_NORMAL;
        type1->isMosaic = false;
        type1->colorMode = OBJCOLOR_16;
        type1->shape = OBJSHAPE_SQUARE;
        type1->x = 64;
        type1->size = OBJSIZE_64;
        type1->gfxIndex = nextAvailableTileIdxS;
        type1->priority = OBJPRIORITY_2;
        type1->palette = palcntS;

        dmaCopyHalfWords(SPRITE_DMA_CHANNEL, BattleSub1Tiles, &SPRITE_GFX_SUB[nextAvailableTileIdxS * OFFSET_MULTIPLIER], BattleSub1TilesLen);
        nextAvailableTileIdxS += BattleSub1TilesLen / BYTES_PER_16_COLOR_TILE;
        
        type1Info = &spriteInfo[++oamIndexS];
        type1 = &oam->oamBuffer[oamIndexS];
        type1Info->oamId = oamIndexS;
        type1Info->width = 64;
        type1Info->height = 64;
        type1Info->angle = 0;
        type1Info->entry = type1;
        type1->y = 72;
        type1->isRotateScale = false; 
        type1->isHidden = true;
        type1->blendMode = OBJMODE_NORMAL;
        type1->isMosaic = false;
        type1->colorMode = OBJCOLOR_16;
        type1->shape = OBJSHAPE_SQUARE;
        type1->x = 128;
        type1->size = OBJSIZE_64;
        type1->gfxIndex = nextAvailableTileIdxS;
        type1->priority = OBJPRIORITY_2;
        type1->palette = palcntS;
        
        dmaCopyHalfWords(SPRITE_DMA_CHANNEL, BattleSub2Tiles, &SPRITE_GFX_SUB[nextAvailableTileIdxS * OFFSET_MULTIPLIER], BattleSub2TilesLen);
        nextAvailableTileIdxS += BattleSub2TilesLen / BYTES_PER_16_COLOR_TILE;
        dmaCopyHalfWords(SPRITE_DMA_CHANNEL, BattleSub1Pal, &SPRITE_PALETTE_SUB[(palcntS++) * COLORS_PER_PALETTE], 32);
        
        type1Info = &spriteInfo[++oamIndexS];
        type1 = &oam->oamBuffer[oamIndexS];
        type1Info->oamId = oamIndexS;
        type1Info->width = 64;
        type1Info->height = 32;
        type1Info->angle = 0;
        type1Info->entry = type1;
        type1->y = 152;
        type1->isRotateScale = false; 
        type1->isHidden = true;
        type1->blendMode = OBJMODE_NORMAL;
        type1->isMosaic = false;
        type1->colorMode = OBJCOLOR_16;
        type1->shape = OBJSHAPE_WIDE;
        type1->x = 96;
        type1->size = OBJSIZE_64;
        type1->gfxIndex = nextAvailableTileIdxS;
        type1->priority = OBJPRIORITY_2;
        type1->palette = palcntS;

        if(isWild){
            dmaCopyHalfWords(SPRITE_DMA_CHANNEL, BattleSub3Pal, &SPRITE_PALETTE_SUB[(palcntS++) * COLORS_PER_PALETTE], 32);        
            dmaCopyHalfWords(SPRITE_DMA_CHANNEL, BattleSub3Tiles, &SPRITE_GFX_SUB[nextAvailableTileIdxS * OFFSET_MULTIPLIER], BattleSub3TilesLen);
            nextAvailableTileIdxS += BattleSub3TilesLen / BYTES_PER_16_COLOR_TILE;
        }
        else if(hasPNav){
            dmaCopyHalfWords(SPRITE_DMA_CHANNEL, BattleSub6Pal, &SPRITE_PALETTE_SUB[(palcntS++) * COLORS_PER_PALETTE], 32);        
            dmaCopyHalfWords(SPRITE_DMA_CHANNEL, BattleSub6Tiles, &SPRITE_GFX_SUB[nextAvailableTileIdxS * OFFSET_MULTIPLIER], BattleSub6TilesLen);
            nextAvailableTileIdxS += BattleSub6TilesLen / BYTES_PER_16_COLOR_TILE;
        }
        
        type1Info = &spriteInfo[++oamIndexS];
        type1 = &oam->oamBuffer[oamIndexS];
        type1Info->oamId = oamIndexS;
        type1Info->width = 64;
        type1Info->height = 32;
        type1Info->angle = 0;
        type1Info->entry = type1;
        type1->y = 144;
        type1->isRotateScale = false; 
        type1->isHidden = true;
        type1->blendMode = OBJMODE_NORMAL;
        type1->isMosaic = false;
        type1->colorMode = OBJCOLOR_16;
        type1->shape = OBJSHAPE_WIDE;
        type1->x = 16;
        type1->size = OBJSIZE_64;
        type1->gfxIndex = nextAvailableTileIdxS;
        type1->priority = OBJPRIORITY_1;
        type1->palette = palcntS;

        dmaCopyHalfWords(SPRITE_DMA_CHANNEL, BattleSub4Pal, &SPRITE_PALETTE_SUB[(palcntS++) * COLORS_PER_PALETTE], 32);        
        dmaCopyHalfWords(SPRITE_DMA_CHANNEL, BattleSub4Tiles, &SPRITE_GFX_SUB[nextAvailableTileIdxS * OFFSET_MULTIPLIER], BattleSub4TilesLen);
        nextAvailableTileIdxS += BattleSub4TilesLen / BYTES_PER_16_COLOR_TILE;
        
        type1Info = &spriteInfo[++oamIndexS];
        type1 = &oam->oamBuffer[oamIndexS];
        type1Info->oamId = oamIndexS;
        type1Info->width = 64;
        type1Info->height = 32;
        type1Info->angle = 0;
        type1Info->entry = type1;
        type1->y = 144;
        type1->isRotateScale = false; 
        type1->isHidden = true;
        type1->blendMode = OBJMODE_NORMAL;
        type1->isMosaic = false;
        type1->colorMode = OBJCOLOR_16;
        type1->shape = OBJSHAPE_WIDE;
        type1->x = 176;
        type1->size = OBJSIZE_64;
        type1->gfxIndex = nextAvailableTileIdxS;
        type1->priority = OBJPRIORITY_1;
        type1->palette = palcntS;

        dmaCopyHalfWords(SPRITE_DMA_CHANNEL, BattleSub5Pal, &SPRITE_PALETTE_SUB[(palcntS++) * COLORS_PER_PALETTE], 32);        
        dmaCopyHalfWords(SPRITE_DMA_CHANNEL, BattleSub5Tiles, &SPRITE_GFX_SUB[nextAvailableTileIdxS * OFFSET_MULTIPLIER], BattleSub5TilesLen);
        nextAvailableTileIdxS += BattleSub5TilesLen / BYTES_PER_16_COLOR_TILE;

        for(int i= 0; i < 4; ++i){
            SpriteInfo * MInfo = &spriteInfo[++oamIndexS];
            SpriteEntry * M = &oam->oamBuffer[oamIndexS];
            MInfo->oamId = oamIndexS;
            MInfo->width = 64;
            MInfo->height = 64;
            MInfo->angle = 0;
            MInfo->entry = M;
            M->y = 0;
            M->isRotateScale = false;
            M->blendMode = OBJMODE_NORMAL;
            M->isMosaic = false;
            M->colorMode = OBJCOLOR_16;
            M->shape = OBJSHAPE_SQUARE;
            M->isHidden = true;
            M->x = (i)*64;
            M->size = OBJSIZE_64;
            M->gfxIndex = nextAvailableTileIdxS;
            M->priority = OBJPRIORITY_2;
            M->palette = palcntS;    
        }
        dmaCopyHalfWords(SPRITE_DMA_CHANNEL, MessagePal, &SPRITE_PALETTE_SUB[(palcntS++) * COLORS_PER_PALETTE], 32);
        dmaCopyHalfWords(SPRITE_DMA_CHANNEL, MessageTiles, &SPRITE_GFX_SUB[nextAvailableTileIdxS * OFFSET_MULTIPLIER], MessageTilesLen);
        nextAvailableTileIdxS += MessageTilesLen / BYTES_PER_16_COLOR_TILE;

        SpriteInfo * Bo4Info = &spriteInfo[++oamIndexS];
        SpriteEntry * Bo4 = &oam->oamBuffer[oamIndexS];
        Bo4Info->oamId = oamIndexS;
        Bo4Info->width = 64;
        Bo4Info->height = 64;
        Bo4Info->angle = 0;
        Bo4Info->entry = Bo4;
        Bo4->y = 0;
        Bo4->isRotateScale = false;
        Bo4->blendMode = OBJMODE_NORMAL;
        Bo4->isMosaic = false;
        Bo4->colorMode = OBJCOLOR_16;
        Bo4->shape = OBJSHAPE_SQUARE;
        Bo4->isHidden = true;
        Bo4->x = 0;
        Bo4->size = OBJSIZE_64;
        Bo4->gfxIndex = nextAvailableTileIdxS;
        Bo4->priority = OBJPRIORITY_2;
        Bo4->palette = palcntS;
        Bo4->vFlip = true;
        Bo4->hFlip = true;

        Bo4 = &oam->oamBuffer[++oamIndexS];
        Bo4->y = 128;
        Bo4->isRotateScale = false;
        Bo4->blendMode = OBJMODE_NORMAL;
        Bo4->isMosaic = false;
        Bo4->colorMode = OBJCOLOR_16;
        Bo4->shape = OBJSHAPE_SQUARE;
        Bo4->isHidden = true;
        Bo4->x = 192;
        Bo4->size = OBJSIZE_64;
        Bo4->gfxIndex = nextAvailableTileIdxS;
        Bo4->priority = OBJPRIORITY_2;
        Bo4->palette = palcntS;
        Bo4->vFlip = false;
        Bo4->hFlip = false;

        dmaCopyHalfWords(SPRITE_DMA_CHANNEL, Border_4Pal, &SPRITE_PALETTE_SUB[palcntS * COLORS_PER_PALETTE], 32);
        dmaCopyHalfWords(SPRITE_DMA_CHANNEL, Border_4Tiles, &SPRITE_GFX_SUB[nextAvailableTileIdxS * OFFSET_MULTIPLIER], Border_4TilesLen);
        nextAvailableTileIdxS += Border_4TilesLen / BYTES_PER_16_COLOR_TILE;
    
        SpriteInfo * Bo3Info = &spriteInfo[++oamIndexS];
        SpriteEntry * Bo3 = &oam->oamBuffer[oamIndexS];
        Bo3Info->oamId = oamIndexS;
        Bo3Info->width = 64;
        Bo3Info->height = 64;
        Bo3Info->angle = 0;
        Bo3Info->entry = Bo3;
        Bo3->y = 0;
        Bo3->isRotateScale = false;
        Bo3->blendMode = OBJMODE_NORMAL;
        Bo3->isMosaic = false;
        Bo3->colorMode = OBJCOLOR_16;
        Bo3->shape = OBJSHAPE_SQUARE;
        Bo3->isHidden = true;
        Bo3->x = 64;
        Bo3->size = OBJSIZE_64;
        Bo3->gfxIndex = nextAvailableTileIdxS;
        Bo3->priority = OBJPRIORITY_2;
        Bo3->palette = palcntS;
        Bo3->vFlip = true;
        Bo3->hFlip = true;

        Bo3 = &oam->oamBuffer[++oamIndexS];
        Bo3->y = 128;
        Bo3->isRotateScale = false;
        Bo3->blendMode = OBJMODE_NORMAL;
        Bo3->isMosaic = false;
        Bo3->colorMode = OBJCOLOR_16;
        Bo3->shape = OBJSHAPE_SQUARE;
        Bo3->isHidden = true;
        Bo3->x = 128;
        Bo3->size = OBJSIZE_64;
        Bo3->gfxIndex = nextAvailableTileIdxS;
        Bo3->priority = OBJPRIORITY_2;
        Bo3->palette = palcntS;
        Bo3->vFlip = false;
        Bo3->hFlip = false;

        dmaCopyHalfWords(SPRITE_DMA_CHANNEL,
        Border_3Tiles,
        &SPRITE_GFX_SUB[nextAvailableTileIdxS * OFFSET_MULTIPLIER],
        Border_3TilesLen);
    
        nextAvailableTileIdxS += Border_3TilesLen / BYTES_PER_16_COLOR_TILE;
    
        SpriteInfo * Bo2Info = &spriteInfo[++oamIndexS];
        SpriteEntry * Bo2 = &oam->oamBuffer[oamIndexS];
        Bo2Info->oamId = oamIndexS;
        Bo2Info->width = 64;
        Bo2Info->height = 64;
        Bo2Info->angle = 0;
        Bo2Info->entry = Bo2;
        Bo2->y = 0;
        Bo2->isRotateScale = false;
        Bo2->blendMode = OBJMODE_NORMAL;
        Bo2->isMosaic = false;
        Bo2->colorMode = OBJCOLOR_16;
        Bo2->shape = OBJSHAPE_SQUARE;
        Bo2->isHidden = true;
        Bo2->x = 128;
        Bo2->size = OBJSIZE_64;
        Bo2->gfxIndex = nextAvailableTileIdxS;
        Bo2->priority = OBJPRIORITY_2;
        Bo2->palette = palcntS;
        Bo2->vFlip = true;
        Bo2->hFlip = true;

        Bo2 = &oam->oamBuffer[++oamIndexS];
        Bo2->y = 128;
        Bo2->isRotateScale = false;
        Bo2->blendMode = OBJMODE_NORMAL;
        Bo2->isMosaic = false;
        Bo2->colorMode = OBJCOLOR_16;
        Bo2->shape = OBJSHAPE_SQUARE;
        Bo2->isHidden = true;
        Bo2->x = 64;
        Bo2->size = OBJSIZE_64;
        Bo2->gfxIndex = nextAvailableTileIdxS;
        Bo2->priority = OBJPRIORITY_2;
        Bo2->palette = palcntS;
        Bo2->vFlip = false;
        Bo2->hFlip = false;

        dmaCopyHalfWords(SPRITE_DMA_CHANNEL,
        Border_2Tiles,
        &SPRITE_GFX_SUB[nextAvailableTileIdxS * OFFSET_MULTIPLIER],
        Border_2TilesLen);
        nextAvailableTileIdxS += Border_2TilesLen / BYTES_PER_16_COLOR_TILE;
    
        SpriteInfo * Bo1Info = &spriteInfo[++oamIndexS];
        SpriteEntry * Bo1 = &oam->oamBuffer[oamIndexS];
        Bo1Info->oamId = oamIndexS;
        Bo1Info->width = 64;
        Bo1Info->height = 64;
        Bo1Info->angle = 0;
        Bo1Info->entry = Bo1;
        Bo1->y = 0;
        Bo1->isRotateScale = false;
        Bo1->blendMode = OBJMODE_NORMAL;
        Bo1->isMosaic = false;
        Bo1->colorMode = OBJCOLOR_16;
        Bo1->shape = OBJSHAPE_SQUARE;
        Bo1->isHidden = true;
        Bo1->x = 192;
        Bo1->size = OBJSIZE_64;
        Bo1->gfxIndex = nextAvailableTileIdxS;
        Bo1->priority = OBJPRIORITY_2;
        Bo1->palette = palcntS;
        Bo1->vFlip = true;
        Bo1->hFlip = true;

        Bo1 = &oam->oamBuffer[++oamIndexS];
        Bo1->y = 128;
        Bo1->isRotateScale = false;
        Bo1->blendMode = OBJMODE_NORMAL;
        Bo1->isMosaic = false;
        Bo1->colorMode = OBJCOLOR_16;
        Bo1->shape = OBJSHAPE_SQUARE;
        Bo1->isHidden = true;
        Bo1->x = 0;
        Bo1->size = OBJSIZE_64;
        Bo1->gfxIndex = nextAvailableTileIdxS;
        Bo1->priority = OBJPRIORITY_2;
        Bo1->palette = palcntS;
        Bo1->vFlip = false;
        Bo1->hFlip = false;

        dmaCopyHalfWords(SPRITE_DMA_CHANNEL,
        Border_1Tiles,
        &SPRITE_GFX_SUB[nextAvailableTileIdxS * OFFSET_MULTIPLIER],
        Border_1TilesLen);
        nextAvailableTileIdxS += Border_1TilesLen / BYTES_PER_16_COLOR_TILE;
    
        SpriteInfo * Bo5Info = &spriteInfo[++oamIndexS];
        SpriteEntry * Bo5 = &oam->oamBuffer[oamIndexS];
        Bo5Info->oamId = oamIndexS;
        Bo5Info->width = 64;
        Bo5Info->height = 64;
        Bo5Info->angle = 0;
        Bo5Info->entry = Bo5;
        Bo5->y = 64;
        Bo5->isRotateScale = false;
        Bo5->blendMode = OBJMODE_NORMAL;
        Bo5->isMosaic = false;
        Bo5->colorMode = OBJCOLOR_16;
        Bo5->shape = OBJSHAPE_SQUARE;
        Bo5->isHidden = true;
        Bo5->x = 0;
        Bo5->size = OBJSIZE_64;
        Bo5->gfxIndex = nextAvailableTileIdxS;
        Bo5->priority = OBJPRIORITY_2;
        Bo5->palette = palcntS;
        Bo5->vFlip = true;
        Bo5->hFlip = true;

        Bo5 = &oam->oamBuffer[++oamIndexS];
        Bo5->y = 64;
        Bo5->isRotateScale = false;
        Bo5->blendMode = OBJMODE_NORMAL;
        Bo5->isMosaic = false;
        Bo5->colorMode = OBJCOLOR_16;
        Bo5->shape = OBJSHAPE_SQUARE;
        Bo5->isHidden = false;
        Bo5->x = 192;
        Bo5->size = OBJSIZE_64;
        Bo5->gfxIndex = nextAvailableTileIdxS;
        Bo5->priority = OBJPRIORITY_2;
        Bo5->palette = palcntS;
        Bo5->vFlip = false;
        Bo5->hFlip = false;

        dmaCopyHalfWords(SPRITE_DMA_CHANNEL, Border_5Tiles, &SPRITE_GFX_SUB[nextAvailableTileIdxS * OFFSET_MULTIPLIER], Border_5TilesLen);
        nextAvailableTileIdxS += Border_5TilesLen / BYTES_PER_16_COLOR_TILE;

        ++palcntS;
        
        Bo5 = &oam->oamBuffer[++oamIndexS];
        Bo5->y = 192;
        Bo5->isRotateScale = false;
        Bo5->blendMode = OBJMODE_NORMAL;
        Bo5->isMosaic = false;
        Bo5->colorMode = OBJCOLOR_16;
        Bo5->shape = OBJSHAPE_SQUARE;
        Bo5->isHidden = true;
        Bo5->x = 256;
        Bo5->size = OBJSIZE_64;
        Bo5->gfxIndex = nextAvailableTileIdxS;
        Bo5->priority = OBJPRIORITY_2;
        Bo5->palette = palcntS;
        Bo5->vFlip = false;
        Bo5->hFlip = false;
        SpriteInfo * backInfo = &spriteInfo[++oamIndexS];
        SpriteEntry * back = &oam->oamBuffer[oamIndexS];
        backInfo->oamId = oamIndexS;
        backInfo->width = 32;
        backInfo->height = 32;
        backInfo->angle = 0;
        backInfo->entry = back;
        back->y = SCREEN_HEIGHT - 28;
        back->isRotateScale = false;
        back->blendMode = OBJMODE_NORMAL;
        back->isMosaic = false;
        back->isHidden = true;
        back->colorMode = OBJCOLOR_16;
        back->shape = OBJSHAPE_SQUARE;
        back->x = SCREEN_WIDTH - 28;
        back->size = OBJSIZE_32;
        back->gfxIndex = nextAvailableTileIdxS;
        back->priority = OBJPRIORITY_1;
        back->palette = palcntS;

        dmaCopyHalfWords(SPRITE_DMA_CHANNEL, BackTiles, &SPRITE_GFX_SUB[nextAvailableTileIdxS * OFFSET_MULTIPLIER], BackTilesLen);
        nextAvailableTileIdxS += BackTilesLen / BYTES_PER_16_COLOR_TILE;

        dmaCopyHalfWords(SPRITE_DMA_CHANNEL, BackPal, &SPRITE_PALETTE_SUB[(palcntS++) * COLORS_PER_PALETTE], 32);
        
        ++oamIndexS;
        int nextnext = nextAvailableTileIdxS + Choice_1TilesLen / BYTES_PER_16_COLOR_TILE;
        for(int i= 0; i < 6; ++i){
            SpriteInfo * C1Info = &spriteInfo[2*i+oamIndexS];
            SpriteEntry * C1 = &oam->oamBuffer[2*i+oamIndexS];
            C1Info->oamId = oamIndexS;
            C1Info->width = 64;
            C1Info->height = 32;
            C1Info->angle = 0;
            C1Info->entry = C1;
            C1->y = 68+ (i/2)*32;
            C1->isRotateScale = false;
            C1->blendMode = OBJMODE_NORMAL;
            C1->isMosaic = false;
            C1->colorMode = OBJCOLOR_16;
            C1->shape = OBJSHAPE_WIDE;
            C1->isHidden = true;
            C1->x = ((i%2)? 32 : 128);
            C1->size = OBJSIZE_64;
            C1->gfxIndex = nextAvailableTileIdxS;
        
            C1->priority = OBJPRIORITY_2;
            C1->palette = palcntS;
        
            SpriteInfo * C3Info = &spriteInfo[2*i+oamIndexS+1];
            SpriteEntry * C3 = &oam->oamBuffer[2*i+oamIndexS+1];
            C3Info->oamId = oamIndexS;
            C3Info->width = 64;
            C3Info->height = 32;
            C3Info->angle = 0;
            C3Info->entry = C3;
            C3->y = 68+ (i/2)*32;
            C3->isRotateScale = false;
            C3->blendMode = OBJMODE_NORMAL;
            C3->isMosaic = false;
            C3->colorMode = OBJCOLOR_16;
            C3->shape = OBJSHAPE_WIDE;
            C3->isHidden = true;
            C3->x = ((i%2)? 62 : 160);
            C3->size = OBJSIZE_64;
            C3->gfxIndex = nextnext;
            C3->priority = OBJPRIORITY_2;
            C3->palette = palcntS;
        }
        dmaCopyHalfWords(SPRITE_DMA_CHANNEL, Choice_1Pal, &SPRITE_PALETTE_SUB[palcntS * COLORS_PER_PALETTE], 32);
        dmaCopyHalfWords(SPRITE_DMA_CHANNEL, Choice_1Tiles, &SPRITE_GFX_SUB[nextAvailableTileIdxS * OFFSET_MULTIPLIER], Choice_1TilesLen);
        dmaCopyHalfWords(SPRITE_DMA_CHANNEL,    Choice_3Tiles,    &SPRITE_GFX_SUB[nextnext * OFFSET_MULTIPLIER],    Choice_3TilesLen);
        nextAvailableTileIdxS = nextnext + Choice_1TilesLen / BYTES_PER_16_COLOR_TILE;

        for(int i= 0; i <3; ++i){
            SpriteInfo * C2Info = &spriteInfo[i+oamIndexS+12];
            SpriteEntry * C2 = &oam->oamBuffer[i+oamIndexS+12];
            C2Info->oamId = oamIndexS;
            C2Info->width = 64;
            C2Info->height = 32;
            C2Info->angle = 0;
            C2Info->entry = C2;
            C2->y = 68+ (i)*32;
            C2->isRotateScale = false;
            C2->blendMode = OBJMODE_NORMAL;
            C2->isMosaic = false;
            C2->colorMode = OBJCOLOR_16;
            C2->shape = OBJSHAPE_WIDE;
            C2->isHidden = true;
            C2->x = 96;
            C2->size = OBJSIZE_64;
            C2->gfxIndex = nextAvailableTileIdxS;
            C2->priority = OBJPRIORITY_2;
            C2->palette = palcntS;
        }
        dmaCopyHalfWords(SPRITE_DMA_CHANNEL, Choice_2Tiles, &SPRITE_GFX_SUB[nextAvailableTileIdxS * OFFSET_MULTIPLIER], Choice_2TilesLen);
        nextAvailableTileIdxS += Choice_2TilesLen / BYTES_PER_16_COLOR_TILE;
        ++palcntS;
        oamIndexS += 15;
    }
    

    void animatePB(int x,int y){
        static const int BYTES_PER_16_COLOR_TILE = 32;
        static const int COLORS_PER_PALETTE = 16;
        static const int BOUNDARY_VALUE = 32; /* This is the default boundary value
                                            * (can be set in REG_DISPCNT) */
        static const int OFFSET_MULTIPLIER = BOUNDARY_VALUE / sizeof(SPRITE_GFX[0]);
        x += 8; y += 8;
        SpriteInfo * type1Info = &spriteInfoTop[++oamIndex];
        SpriteEntry * type1 = &oamTop->oamBuffer[oamIndex];
        type1Info->oamId = oamIndex;
        type1Info->width = 16;
        type1Info->height = 16;
        type1Info->angle = 0;
        type1Info->entry = type1;
        type1->y = y;
        type1->isRotateScale = false;
        type1->isHidden = false;
        type1->blendMode = OBJMODE_NORMAL;
        type1->isMosaic = false;
        type1->colorMode = OBJCOLOR_16;
        type1->shape = OBJSHAPE_SQUARE;
        type1->x = x;
        type1->size = OBJSIZE_16;
        type1->gfxIndex = nextAvailableTileIdx;
        type1->priority = OBJPRIORITY_0;
        type1->palette = palcnt;
        
        dmaCopyHalfWords(SPRITE_DMA_CHANNEL, PokeBall1Pal, &SPRITE_PALETTE[(palcnt) * COLORS_PER_PALETTE], 32);
        dmaCopyHalfWords(SPRITE_DMA_CHANNEL, PokeBall1Tiles, &SPRITE_GFX[nextAvailableTileIdx * OFFSET_MULTIPLIER], PokeBall1TilesLen);
        updateOAM(oamTop);
        for(int i= 0; i< 2; ++i)
            swiWaitForVBlank();
        dmaCopyHalfWords(SPRITE_DMA_CHANNEL, PokeBall2Tiles, &SPRITE_GFX[nextAvailableTileIdx * OFFSET_MULTIPLIER], PokeBall2TilesLen);
        updateOAM(oamTop);
        for(int i= 0; i< 2; ++i)
            swiWaitForVBlank();
        dmaCopyHalfWords(SPRITE_DMA_CHANNEL, PokeBall3Tiles, &SPRITE_GFX[nextAvailableTileIdx * OFFSET_MULTIPLIER], PokeBall3TilesLen);
        updateOAM(oamTop);
        for(int i= 0; i< 2; ++i)
            swiWaitForVBlank();
        dmaCopyHalfWords(SPRITE_DMA_CHANNEL, PokeBall4Tiles, &SPRITE_GFX[nextAvailableTileIdx * OFFSET_MULTIPLIER], PokeBall4TilesLen);
        updateOAM(oamTop);
        for(int i= 0; i< 2; ++i)
            swiWaitForVBlank();
        dmaCopyHalfWords(SPRITE_DMA_CHANNEL, PokeBall5Tiles, &SPRITE_GFX[nextAvailableTileIdx * OFFSET_MULTIPLIER], PokeBall5TilesLen);
        updateOAM(oamTop);
        for(int i= 0; i< 2; ++i)
            swiWaitForVBlank();
        dmaCopyHalfWords(SPRITE_DMA_CHANNEL, PokeBall6Tiles, &SPRITE_GFX[nextAvailableTileIdx * OFFSET_MULTIPLIER], PokeBall6TilesLen);
        updateOAM(oamTop);
        for(int i= 0; i< 2; ++i)
            swiWaitForVBlank();
        dmaCopyHalfWords(SPRITE_DMA_CHANNEL, PokeBall7Tiles, &SPRITE_GFX[nextAvailableTileIdx * OFFSET_MULTIPLIER], PokeBall7TilesLen);
        updateOAM(oamTop);
        for(int i= 0; i< 2; ++i)
            swiWaitForVBlank();
        dmaCopyHalfWords(SPRITE_DMA_CHANNEL, PokeBall8Tiles, &SPRITE_GFX[nextAvailableTileIdx * OFFSET_MULTIPLIER], PokeBall8TilesLen);
        updateOAM(oamTop);
        for(int i= 0; i< 2; ++i)
            swiWaitForVBlank();
        dmaCopyHalfWords(SPRITE_DMA_CHANNEL, PokeBall9Tiles, &SPRITE_GFX[nextAvailableTileIdx * OFFSET_MULTIPLIER], PokeBall9TilesLen);
        updateOAM(oamTop);
        for(int i= 0; i< 2; ++i)
            swiWaitForVBlank();
        dmaCopyHalfWords(SPRITE_DMA_CHANNEL, PokeBall10Tiles, &SPRITE_GFX[nextAvailableTileIdx * OFFSET_MULTIPLIER], PokeBall10TilesLen);
        updateOAM(oamTop);
        for(int i= 0; i< 2; ++i)
            swiWaitForVBlank();
        dmaCopyHalfWords(SPRITE_DMA_CHANNEL, PokeBall11Tiles, &SPRITE_GFX[nextAvailableTileIdx * OFFSET_MULTIPLIER], PokeBall11TilesLen);
        updateOAM(oamTop);
        for(int i= 0; i< 3; ++i)
            swiWaitForVBlank();
        type1->isHidden = true;

        type1Info->oamId = oamIndex;
        type1Info->width = 64;
        type1Info->height = 64;
        type1Info->angle = 0;
        type1Info->entry = type1;
        type1->y = y -22;
        type1->isRotateScale = false;
        type1->isHidden = false;
        type1->blendMode = OBJMODE_NORMAL;
        type1->isMosaic = false;
        type1->colorMode = OBJCOLOR_16;
        type1->shape = OBJSHAPE_SQUARE;
        type1->x = x -22;
        type1->size = OBJSIZE_64;
        type1->gfxIndex = nextAvailableTileIdx;
        type1->priority = OBJPRIORITY_0;
        type1->palette = palcnt;
        
        dmaCopyHalfWords(SPRITE_DMA_CHANNEL, Shiny1Pal, &SPRITE_PALETTE[(palcnt) * COLORS_PER_PALETTE], 32);
        dmaCopyHalfWords(SPRITE_DMA_CHANNEL, Shiny1Tiles, &SPRITE_GFX[nextAvailableTileIdx * OFFSET_MULTIPLIER], Shiny1TilesLen);
        updateOAM(oamTop);
        for(int i= 0; i< 2; ++i)
            swiWaitForVBlank();
        dmaCopyHalfWords(SPRITE_DMA_CHANNEL, Shiny2Pal, &SPRITE_PALETTE[(palcnt) * COLORS_PER_PALETTE], 32);
        dmaCopyHalfWords(SPRITE_DMA_CHANNEL, Shiny2Tiles, &SPRITE_GFX[nextAvailableTileIdx * OFFSET_MULTIPLIER], Shiny2TilesLen);
        updateOAM(oamTop);
        for(int i= 0; i< 2; ++i)
            swiWaitForVBlank();
        type1->isHidden = true;
        updateOAM(oamTop);

        --oamIndex;
    }
    void animateShiny(int x,int y){
        static const int BYTES_PER_16_COLOR_TILE = 32;
        static const int COLORS_PER_PALETTE = 16;
        static const int BOUNDARY_VALUE = 32; /* This is the default boundary value
                                            * (can be set in REG_DISPCNT) */
        static const int OFFSET_MULTIPLIER = BOUNDARY_VALUE / sizeof(SPRITE_GFX[0]);

        SpriteInfo * type1Info = &spriteInfoTop[++oamIndex];
        SpriteEntry * type1 = &oamTop->oamBuffer[oamIndex];
        type1Info->oamId = oamIndex;
        type1Info->width = 64;
        type1Info->height = 64;
        type1Info->angle = 0;
        type1Info->entry = type1;
        type1->y = y;
        type1->isRotateScale = false;
        type1->isHidden = false;
        type1->blendMode = OBJMODE_NORMAL;
        type1->isMosaic = false;
        type1->colorMode = OBJCOLOR_16;
        type1->shape = OBJSHAPE_SQUARE;
        type1->x = x;
        type1->size = OBJSIZE_64;
        type1->gfxIndex = nextAvailableTileIdx;
        type1->priority = OBJPRIORITY_0;
        type1->palette = palcnt;
        
        dmaCopyHalfWords(SPRITE_DMA_CHANNEL, Shiny1Pal, &SPRITE_PALETTE[(palcnt) * COLORS_PER_PALETTE], 32);
        dmaCopyHalfWords(SPRITE_DMA_CHANNEL, Shiny1Tiles, &SPRITE_GFX[nextAvailableTileIdx * OFFSET_MULTIPLIER], Shiny1TilesLen);
        updateOAM(oamTop);
        for(int i= 0; i< 2; ++i)
            swiWaitForVBlank();
        dmaCopyHalfWords(SPRITE_DMA_CHANNEL, Shiny2Pal, &SPRITE_PALETTE[(palcnt) * COLORS_PER_PALETTE], 32);
        dmaCopyHalfWords(SPRITE_DMA_CHANNEL, Shiny2Tiles, &SPRITE_GFX[nextAvailableTileIdx * OFFSET_MULTIPLIER], Shiny2TilesLen);
        updateOAM(oamTop);
        for(int i= 0; i< 2; ++i)
            swiWaitForVBlank();
        dmaCopyHalfWords(SPRITE_DMA_CHANNEL, Shiny1Pal, &SPRITE_PALETTE[(palcnt) * COLORS_PER_PALETTE], 32);
        dmaCopyHalfWords(SPRITE_DMA_CHANNEL, Shiny1Tiles, &SPRITE_GFX[nextAvailableTileIdx * OFFSET_MULTIPLIER], Shiny1TilesLen);
        updateOAM(oamTop);
        for(int i= 0; i< 2; ++i)
            swiWaitForVBlank();
        dmaCopyHalfWords(SPRITE_DMA_CHANNEL, Shiny2Pal, &SPRITE_PALETTE[(palcnt) * COLORS_PER_PALETTE], 32);
        dmaCopyHalfWords(SPRITE_DMA_CHANNEL, Shiny2Tiles, &SPRITE_GFX[nextAvailableTileIdx * OFFSET_MULTIPLIER], Shiny2TilesLen);
        updateOAM(oamTop);
        for(int i= 0; i< 2; ++i)
            swiWaitForVBlank();
        dmaCopyHalfWords(SPRITE_DMA_CHANNEL, Shiny1Pal, &SPRITE_PALETTE[(palcnt) * COLORS_PER_PALETTE], 32);
        dmaCopyHalfWords(SPRITE_DMA_CHANNEL, Shiny1Tiles, &SPRITE_GFX[nextAvailableTileIdx * OFFSET_MULTIPLIER], Shiny1TilesLen);
        updateOAM(oamTop);
        for(int i= 0; i< 2; ++i)
            swiWaitForVBlank();
        dmaCopyHalfWords(SPRITE_DMA_CHANNEL, Shiny2Pal, &SPRITE_PALETTE[(palcnt) * COLORS_PER_PALETTE], 32);
        dmaCopyHalfWords(SPRITE_DMA_CHANNEL, Shiny2Tiles, &SPRITE_GFX[nextAvailableTileIdx * OFFSET_MULTIPLIER], Shiny2TilesLen);
        updateOAM(oamTop);
        for(int i= 0; i< 2; ++i)
            swiWaitForVBlank();
        type1->isHidden = true;
        updateOAM(oamTop);

        --oamIndex;
    }

#define GENDER(a) (a.boxdata.isFemale? 147 : (a.boxdata.isGenderless ? ' ' : 141))

    void battle::switchOppPkmn(int newPok,int toSwitch){
        if((*this->opponent->pkmn_team)[acpokpos[newPok][1]].stats.acHP == 0)
            return;

        consoleSelect(&Bottom);
        consoleClear();
        char buf[100];
        if((*this->opponent->pkmn_team)[acpokpos[toSwitch][1]].stats.acHP){
            clear();
            sprintf(buf,"%ls wird von\n%s %s\nauf die Bank geschickt. ",(*this->opponent->pkmn_team)[acpokpos[toSwitch][1]].boxdata.Name,
                trainerclassnames[this->opponent->trainer_class],this->opponent->Name);
            cust_font.print_string(buf,8,8,true);
        }        
        for(int i = 1 + (this->opponent->pkmn_team->size() > 1 && acpoksts[acpokpos[1][1]][1] != KO); i < 6; ++i){
            switch(this->acpoksts[acpokpos[i][1]][1]){
            case KO:
                oamTop->oamBuffer[11+i].palette--;
                oamTop->oamBuffer[11+i].gfxIndex -= BattleBall1TilesLen / 32;
                break;
            case STS:
                oamTop->oamBuffer[11+i].palette-=2;
                oamTop->oamBuffer[11+i].gfxIndex -= BattleBall1TilesLen / 16;
                break;
            default:
                break;
            }
        }

        updateOAM(oamTop);
        
        std::swap(acpokpos[newPok][1],acpokpos[toSwitch][1]);

        for(int i= 0; i < 150; ++i)
            swiWaitForVBlank();
        
        clear();
        sprintf(buf,"%s %s\nschickt %ls in den Kampf.",trainerclassnames[this->opponent->trainer_class],this->opponent->Name,(*this->opponent->pkmn_team)[acpokpos[toSwitch][1]].boxdata.Name);
        cust_font.print_string(buf,8,8,true);

        consoleSelect(&Top);
        
        if(toSwitch == 0){
            oamIndex -= 16;
            palcnt -= 5;
            nextAvailableTileIdx -= (4)*(144);

            oamTop->oamBuffer[12].isHidden = true;
            for(int i = 1; i <= 4; ++i)
                oamTop->oamBuffer[oamIndex + i].isHidden = true;
            updateOAM(oamTop);
            
            oamIndex += 17;
            palcnt += (1);
            animatePB(206,50); 
            palcnt -= (1);
            oamIndex -= 17;

            oamTop->oamBuffer[12].isHidden = false;

            if(!loadPKMNSprite(oamTop,spriteInfoTop,"nitro:/PICS/SPRITES/PKMN/",(*this->opponent->pkmn_team)[acpokpos[0][1]].boxdata.SPEC,176,20,oamIndex,palcnt,nextAvailableTileIdx,false,
                (*this->opponent->pkmn_team)[acpokpos[0][1]].boxdata.isShiny(),(*this->opponent->pkmn_team)[acpokpos[0][1]].boxdata.isFemale))
                loadPKMNSprite(oamTop,spriteInfoTop,"nitro:/PICS/SPRITES/PKMN/",(*this->opponent->pkmn_team)[acpokpos[0][1]].boxdata.SPEC,176,20,oamIndex,palcnt,nextAvailableTileIdx,false,
                    (*this->opponent->pkmn_team)[acpokpos[0][1]].boxdata.isShiny(),!(*this->opponent->pkmn_team)[acpokpos[0][1]].boxdata.isFemale);
            
            oamIndex += (12);
            palcnt += (3);
            nextAvailableTileIdx += (3)*(144);

            updateOAM(oamTop);
            
            oamIndex += (1);
            palcnt += (1);
            if((*this->opponent->pkmn_team)[acpokpos[0][1]].boxdata.isShiny())
                animateShiny(176+16,36);
            oamIndex -= (1);
            palcnt -= (1);

            displayHP(100,101 ,88,32,42,43,false);
            displayHP(100,100-(*this->opponent->pkmn_team)[acpokpos[0][1]].stats.acHP*100/(*this->opponent->pkmn_team)[acpokpos[0][1]].stats.maxHP,88,32,42,43,false);      
            oamTop->oamBuffer[12].x = 96;
            oamTop->oamBuffer[12].y = 41;
            consoleSetWindow(&Top,0,5,20,2);
            consoleClear();
            printf("%10ls%c\n",(*this->opponent->pkmn_team)[acpokpos[0][1]].boxdata.Name,GENDER((*this->player->pkmn_team)[acpokpos[0][1]]));
            if((*this->opponent->pkmn_team)[acpokpos[0][1]].Level < 10)
                printf(" ");
            if((*this->opponent->pkmn_team)[acpokpos[0][1]].Level < 100)
                printf(" ");
            printf("Lv%d%4dKP",(*this->opponent->pkmn_team)[acpokpos[0][1]].Level,
                (*this->opponent->pkmn_team)[acpokpos[0][1]].stats.acHP);
        } 
        else
        {
            oamTop->oamBuffer[11].isHidden = true;
            updateOAM(oamTop);
            consoleSelect(&Top);
            oamIndex -= (12);
            palcnt -= (4);
            nextAvailableTileIdx -= (3)*(144);

            for(int i = 1; i <= 4; ++i)
                oamTop->oamBuffer[oamIndex + i].isHidden = true;
            updateOAM(oamTop);
            
            oamIndex += (13);
            palcnt += (5);
            animatePB(142,34); 
            oamIndex -= (13);
            palcnt -= (6);
            oamTop->oamBuffer[11].isHidden = false;
            

            if(!loadPKMNSprite(oamTop,spriteInfoTop,"nitro:/PICS/SPRITES/PKMN/",(*this->opponent->pkmn_team)[acpokpos[1][1]].boxdata.SPEC,112,4,oamIndex,palcnt,nextAvailableTileIdx,false,
                (*this->opponent->pkmn_team)[acpokpos[1][1]].boxdata.isShiny(),(*this->opponent->pkmn_team)[acpokpos[1][1]].boxdata.isFemale))
                loadPKMNSprite(oamTop,spriteInfoTop,"nitro:/PICS/SPRITES/PKMN/",(*this->opponent->pkmn_team)[acpokpos[1][1]].boxdata.SPEC,112,4,oamIndex,palcnt,nextAvailableTileIdx,false,
                (*this->opponent->pkmn_team)[acpokpos[1][1]].boxdata.isShiny(),!(*this->opponent->pkmn_team)[acpokpos[1][1]].boxdata.isFemale);
            
            oamIndex += (9);
            palcnt += (3);
            nextAvailableTileIdx += (2)*(144);
            if((*this->opponent->pkmn_team)[acpokpos[1][1]].boxdata.isShiny())
                animateShiny(112+16,20);
            oamIndex -= (9);
            palcnt -= (3);
           
            oamIndex += (8);
            palcnt += (2);
            updateOAM(oamTop);

            displayHP(100,101,0,8,45,44,false);   
            displayHP(100,100-(*this->opponent->pkmn_team)[acpokpos[1][1]].stats.acHP*100/(*this->opponent->pkmn_team)[acpokpos[1][1]].stats.maxHP,0,8,45,44,false); 
            oamTop->oamBuffer[11].x = 8;
            oamTop->oamBuffer[11].y = 17;
            consoleSetWindow(&Top,4,2,20,2);
            consoleClear();
            printf("%ls%c\nLv%d%4dKP",(*this->opponent->pkmn_team)[acpokpos[1][1]].boxdata.Name,GENDER((*this->player->pkmn_team)[acpokpos[1][1]]),
                (*this->opponent->pkmn_team)[acpokpos[1][1]].Level, (*this->opponent->pkmn_team)[acpokpos[1][1]].stats.acHP);
            }

        consoleSelect(&Bottom);
        
        for(int i = 1 + (this->opponent->pkmn_team->size() > 1 && acpoksts[acpokpos[1][1]][1] != KO); i < 6; ++i){
            oamTop->oamBuffer[11+i].x = -4+ 18*i;
            oamTop->oamBuffer[11+i].y = -4;
            oamTop->oamBuffer[11+i].isHidden = false;
            switch(this->acpoksts[acpokpos[i][1]][1]){
            case NA:
                oamTop->oamBuffer[11+i].isHidden = true;
                break;
            case KO:
                oamTop->oamBuffer[11+i].palette++;
                oamTop->oamBuffer[11+i].gfxIndex += BattleBall1TilesLen / 32;
                break;
            case STS:
                oamTop->oamBuffer[11+i].palette+=2;
                oamTop->oamBuffer[11+i].gfxIndex += BattleBall1TilesLen / 16;
                break;
            default:
                break;
            }
        }
        updateOAM(oamTop);
        for(int i= 0; i < 200; ++i)
            swiWaitForVBlank();
        
        if(abilities[(*this->opponent->pkmn_team)[acpokpos[toSwitch][1]].boxdata.ability].T & ablty::BEFORE_BATTLE){
            clear();
            sprintf(buf,"%s von\n %ls (Gegn.) wirkt!\n",
                abilities[(*this->opponent->pkmn_team)[acpokpos[toSwitch][1]].boxdata.ability].Name.c_str(),
                (*this->opponent->pkmn_team)[acpokpos[toSwitch][1]].boxdata.Name);
            cust_font.print_string(buf,8,8,true);
            //abilities[(*this->opponent->pkmn_team)[acpokpos[0][1]].boxdata.ability].run();
            for(int i= 0; i < 100; ++i)
                swiWaitForVBlank();
        }
    }
    void battle::switchOwnPkmn(int newPok,int toSwitch){
        if((*this->player->pkmn_team)[acpokpos[newPok][0]].stats.acHP == 0)
            return;
        init();
        for(int i = 1 + (this->player->pkmn_team->size() > 1 && acpoksts[acpokpos[1][0]][0] != KO); i < 6; ++i){
            oamTop->oamBuffer[5+i].x = 236 - 18*i;
            oamTop->oamBuffer[5+i].y = 196 - 16;
            oamTop->oamBuffer[5+i].isHidden = false;
            switch(this->acpoksts[acpokpos[i][0]][0]){
            case KO:
                oamTop->oamBuffer[5+i].palette--;
                oamTop->oamBuffer[5+i].gfxIndex -= BattleBall1TilesLen / 32;
                break;
            case STS:
                oamTop->oamBuffer[5+i].palette-=2;
                oamTop->oamBuffer[5+i].gfxIndex -= BattleBall1TilesLen / 16;
                break;
            default:
                break;
            }
        }
        updateOAM(oamTop);
        consoleSelect(&Bottom);
        consoleClear();
        char buf[100];
        if((*this->player->pkmn_team)[acpokpos[toSwitch][0]].stats.acHP){
            clear();
            sprintf(buf,"Auf die Bank,\n %ls!",(*this->player->pkmn_team)[acpokpos[toSwitch][0]].boxdata.Name);
            cust_font.print_string(buf,8,8,true);
        }
        std::swap(acpokpos[newPok][0],acpokpos[toSwitch][0]);

        for(int i= 0; i < 100; ++i)
            swiWaitForVBlank();

        
        consoleSelect(&Bottom);
        clear();
        sprintf(buf,"Los %ls!",(*this->player->pkmn_team)[acpokpos[toSwitch][0]].boxdata.Name);
        cust_font.print_string(buf,8,8,true);
        
        consoleSelect(&Top);
        if(toSwitch == 0){
            oamIndex -= 8;
            palcnt -= 3;
            nextAvailableTileIdx -= (2)*(144);

            oamTop->oamBuffer[6].isHidden = true;
            for(int i = 1; i <= 4; ++i)
                oamTop->oamBuffer[oamIndex + i].isHidden = true;
            updateOAM(oamTop);
            
            oamIndex += 9;
            palcnt += 4;
            animatePB(80,170);
            oamIndex -= 9;
            palcnt -= 4;            
                        
            oamTop->oamBuffer[6].isHidden = false;
            
            if(!loadPKMNSprite(oamTop,spriteInfoTop,"nitro:/PICS/SPRITES/PKMNBACK/",(*this->player->pkmn_team)[acpokpos[0][0]].boxdata.SPEC,-10,100,oamIndex,palcnt,nextAvailableTileIdx,false,
                (*this->player->pkmn_team)[acpokpos[0][0]].boxdata.isShiny(),(*this->player->pkmn_team)[acpokpos[0][0]].boxdata.isFemale))
                loadPKMNSprite(oamTop,spriteInfoTop,"nitro:/PICS/SPRITES/PKMNBACK/",(*this->player->pkmn_team)[acpokpos[0][0]].boxdata.SPEC,-10,100,oamIndex,palcnt,nextAvailableTileIdx,false,
                (*this->player->pkmn_team)[acpokpos[0][0]].boxdata.isShiny(),!(*this->player->pkmn_team)[acpokpos[0][0]].boxdata.isFemale);
        
            updateOAM(oamTop);
            
            oamIndex += (4);
            palcnt += (1);
            nextAvailableTileIdx += (144);

            if((*this->player->pkmn_team)[acpokpos[0][0]].boxdata.isShiny())
                animateShiny(6,116);
            
            displayHP(100,101,256-96-28,192-32-8-32,42,49,false);
            displayHP(100,100-(*this->player->pkmn_team)[acpokpos[0][0]].stats.acHP*100/(*this->player->pkmn_team)[acpokpos[0][0]].stats.maxHP,256-96-28,192-32-8-32,42,49,false);       
            displayEP(100,100,256-96-28,192-32-8-32,46,47,false);
            oamTop->oamBuffer[6].x = 256-88-32+4;
            oamTop->oamBuffer[6].y = 192-31-32;
            consoleSetWindow(&Top,21,16,20,4);
            consoleClear();
            printf("%ls%c\nLv%d%4dKP",(*this->player->pkmn_team)[acpokpos[0][0]].boxdata.Name,GENDER((*this->player->pkmn_team)[acpokpos[0][0]]),(*this->player->pkmn_team)[acpokpos[0][0]].Level,
                (*this->player->pkmn_team)[acpokpos[0][0]].stats.acHP);

            
            oamTop->oamBuffer[6].isHidden = oamTop->oamBuffer[2].isHidden = false;
            updateOAM(oamTop);
        } 
        else
        {
            oamTop->oamBuffer[5].isHidden = true;
            updateOAM(oamTop);
            consoleSelect(&Top);
            oamIndex -= (4);
            palcnt -= (2);
            nextAvailableTileIdx -= (1)*(144);

            for(int i = 1; i <= 4; ++i) //hide PKMN sprite
                oamTop->oamBuffer[oamIndex + i].isHidden = true;
            updateOAM(oamTop);
            
            oamIndex += (5);
            palcnt += 3;
            animatePB(142,34); 
            oamIndex -= (5);
            palcnt -= 3;
            oamTop->oamBuffer[5].isHidden = false;
            

            if(!loadPKMNSprite(oamTop,spriteInfoTop,"nitro:/PICS/SPRITES/PKMNBACK/",(*this->player->pkmn_team)[acpokpos[1][0]].boxdata.SPEC,50,120,oamIndex,palcnt,nextAvailableTileIdx,false,
                (*this->player->pkmn_team)[acpokpos[1][0]].boxdata.isShiny(),(*this->player->pkmn_team)[acpokpos[1][0]].boxdata.isFemale))
                loadPKMNSprite(oamTop,spriteInfoTop,"nitro:/PICS/SPRITES/PKMNBACK/",(*this->player->pkmn_team)[acpokpos[1][0]].boxdata.SPEC,50,120,oamIndex,palcnt,nextAvailableTileIdx,false,
                (*this->player->pkmn_team)[acpokpos[1][0]].boxdata.isShiny(),!(*this->player->pkmn_team)[acpokpos[1][0]].boxdata.isFemale);
            if((*this->player->pkmn_team)[acpokpos[1][0]].boxdata.isShiny())
                animateShiny(50+16,136);

           
            updateOAM(oamTop);
            
            displayHP(100,101,256-36,192-40,51,50,false);   
            displayHP(100,100-(*this->player->pkmn_team)[acpokpos[1][0]].stats.acHP*100/(*this->player->pkmn_team)[acpokpos[1][0]].stats.maxHP,256-36,192-40,51,50,false);        
            displayEP(100,100,256-36,192-40,46,47,false);
            oamTop->oamBuffer[5].x = 256-32+4;
            oamTop->oamBuffer[5].y = 192-31;
            consoleSetWindow(&Top,16,20,20,5);
            consoleClear();
            printf("%10ls%c\n",(*this->player->pkmn_team)[acpokpos[1][0]].boxdata.Name,GENDER((*this->player->pkmn_team)[acpokpos[1][0]]));
            if((*this->player->pkmn_team)[acpokpos[1][0]].Level < 10)
                printf(" ");
            if((*this->player->pkmn_team)[acpokpos[1][0]].Level < 100)
                printf(" ");
            printf("Lv%d%4dKP",(*this->player->pkmn_team)[acpokpos[1][0]].Level,
                (*this->player->pkmn_team)[acpokpos[1][0]].stats.acHP);
            
            oamTop->oamBuffer[5].isHidden = oamTop->oamBuffer[1].isHidden = false;
            updateOAM(oamTop);
        }
        
        for(int i = 1 + (this->player->pkmn_team->size() > 1 && acpoksts[acpokpos[1][0]][0] != KO); i < 6; ++i){
            oamTop->oamBuffer[5+i].x = 236 - 18*i;
            oamTop->oamBuffer[5+i].y = 196 - 16;
            oamTop->oamBuffer[5+i].isHidden = false;
            switch(this->acpoksts[acpokpos[i][0]][0]){
            case NA:
                oamTop->oamBuffer[5+i].isHidden = true;
                break;
            case KO:
                oamTop->oamBuffer[5+i].palette++;
                oamTop->oamBuffer[5+i].gfxIndex += BattleBall1TilesLen / 32;
                break;
            case STS:
                oamTop->oamBuffer[5+i].palette+=2;
                oamTop->oamBuffer[5+i].gfxIndex += BattleBall1TilesLen / 16;
                break;
            default:
                break;
            }
        }
        
        for(int i= 0; i < 100; ++i)
            swiWaitForVBlank();
        updateOAM(oamTop);

        if(abilities[(*this->player->pkmn_team)[acpokpos[toSwitch][0]].boxdata.ability].T & ablty::BEFORE_BATTLE){
            clear();
            sprintf(buf,"%s von\n %ls wirkt!\n",
                abilities[(*this->player->pkmn_team)[acpokpos[toSwitch][0]].boxdata.ability].Name.c_str(),
                (*this->player->pkmn_team)[acpokpos[toSwitch][0]].boxdata.Name);
            cust_font.print_string(buf,8,8,true);
            //abilities[(*this->opponent->pkmn_team)[acpokpos[0][1]].boxdata.ability].run();
            for(int i= 0; i < 100; ++i)
                swiWaitForVBlank();
        }
    }

    void setMainBattleVisibility(bool hidden){
        for(int i = 0; i<= 4; ++i)
            oam->oamBuffer[i].isHidden = hidden;
        oam->oamBuffer[2].isHidden |= !hidden & !SAV.activatedPNav;
        for(int i = 19; i <= 19; ++i)
            oam->oamBuffer[i].isHidden = hidden;
        updateOAMSub(oam);
    }

    void waitForTouchUp(){
        while(1)
        {
            scanKeys();
            swiWaitForVBlank();
            updateTime();
            if(keysUp() & KEY_TOUCH)
                break;
        }
    }

    void battle::initBattleScene(int battle_back,Weather weather){        
        for(int i = 0; i < 6; ++i){
            this->acpokpos[i][0] = this->acpokpos[i][1] = i;
            if(this->player->pkmn_team->size() > i){
                if((*this->player->pkmn_team)[i].stats.acHP == 0 || (*this->player->pkmn_team)[i].boxdata.IV.isEgg)
                    this->acpoksts[i][0] = KO;
                else if((*this->player->pkmn_team)[i].statusint)
                    this->acpoksts[i][0] = STS;
                else
                    this->acpoksts[i][0] = OK;
            }
            else
                this->acpoksts[i][0] = NA;
            if(this->opponent->pkmn_team->size() > i){
                if ((*this->opponent->pkmn_team)[i].stats.acHP == 0 || (*this->opponent->pkmn_team)[i].boxdata.IV.isEgg)
                    this->acpoksts[i][1] = KO;
                else if((*this->opponent->pkmn_team)[i].statusint)
                    this->acpoksts[i][1] = STS;
                else this->acpoksts[i][1] = OK;
            }
            else
                this->acpoksts[i][1] = NA;
        }
        if(acpoksts[acpokpos[0][0]][0] == KO)
            for(int i= 2; i< 6; ++i)
                if(acpoksts[acpokpos[i][0]][0] != KO)
                    std::swap(acpokpos[0][0],acpokpos[i][0]);
        if(this->battlemode == DOUBLE && acpoksts[acpokpos[1][0]][0] == KO)
            for(int i= 2; i< 6; ++i)
                if(acpoksts[acpokpos[i][0]][0] != KO)
                    std::swap(acpokpos[1][0],acpokpos[i][0]);
        
        if(acpoksts[acpokpos[0][1]][1] == KO)
            for(int i= 2; i< 6; ++i)
                if(acpoksts[acpokpos[i][1]][1] != KO)
                    std::swap(acpokpos[0][1],acpokpos[i][1]);
        if(this->battlemode == DOUBLE && acpoksts[acpokpos[1][1]][1] == KO)
            for(int i= 2; i< 6; ++i)
                if(acpoksts[acpokpos[i][1]][1] != KO)
                    std::swap(acpokpos[1][1],acpokpos[i][1]);

        vramSetup();
        videoSetMode(MODE_5_2D |DISPLAY_BG2_ACTIVE | DISPLAY_BG3_ACTIVE | DISPLAY_SPR_ACTIVE | DISPLAY_SPR_1D );		
        Top = *consoleInit(&Top, 0, BgType_Text4bpp, BgSize_T_256x256,2,0, true ,true);
        consoleSetFont(&Top,&cfont);
    
        Bottom = *consoleInit(&Bottom,  0, BgType_Text4bpp, BgSize_T_256x256, 2,0, false,true );
        consoleSetFont(&Bottom, &cfont);
    
        touchPosition t;

        initOAMTableSub(oam);
        initOAMTable(oamTop);
        this->round = this->max_round;
        this->initBattleScreen(); 


        if(!BGs[BG_ind].load_from_rom){
            dmaCopy(BGs[BG_ind].MainMenu, bgGetGfxPtr(bg3sub), 256*256);
            dmaCopy(BGs[BG_ind].MainMenuPal, BG_PALETTE_SUB, 256*2); 
        }
        else if(!loadNavScreen(bgGetGfxPtr(bg3sub),BGs[BG_ind].Name.c_str(),BG_ind)){
            dmaCopy(BGs[0].MainMenu, bgGetGfxPtr(bg3sub), 256*256);
            dmaCopy(BGs[0].MainMenuPal, BG_PALETTE_SUB, 256*2); 
            BG_ind = 0;
        }
        loadPicture(bgGetGfxPtr(bg3),"nitro:/PICS/","Clear");
        initOAMTableSub(oam);
        initOAMTable(oamTop);
        initBattleScreenSprites(oamTop,spriteInfoTop);
        
        initBattleSubScreenSprites(oam,spriteInfo,false,SAV.activatedPNav);
        for(int i = 5; i<= 19; ++i)
            oam->oamBuffer[i].isHidden = false;
        updateOAMSub(oam);

        for(int i= 1; i <= 4; ++i)
            oamTop->oamBuffer[i].isHidden = true;
        updateOAM(oamTop);
        
        init();
        //Opp's Side
        consoleSelect(&Bottom);
        consoleClear();
        char buf[100];
        if(this->battlemode == DOUBLE && this->opponent->pkmn_team->size() > 1 && acpoksts[acpokpos[1][1]][1] != KO)
            sprintf(buf,"%s %s\nschickt %ls ",trainerclassnames[this->opponent->trainer_class],this->opponent->Name,(*this->opponent->pkmn_team)[acpokpos[0][1]].boxdata.Name);
        else
            sprintf(buf,"%s %s\nschickt %ls in den Kampf.",trainerclassnames[this->opponent->trainer_class],this->opponent->Name,(*this->opponent->pkmn_team)[acpokpos[0][1]].boxdata.Name);
        cust_font.print_string(buf,8,8,true);
        consoleSelect(&Top);
            
        animatePB(206,50); 

        oamTop->oamBuffer[4].isHidden = oamTop->oamBuffer[12].isHidden = false;

        if(!loadPKMNSprite(oamTop,spriteInfoTop,"nitro:/PICS/SPRITES/PKMN/",(*this->opponent->pkmn_team)[acpokpos[0][1]].boxdata.SPEC,176,20,oamIndex,palcnt,nextAvailableTileIdx,false,
            (*this->opponent->pkmn_team)[acpokpos[0][1]].boxdata.isShiny(),(*this->opponent->pkmn_team)[acpokpos[0][1]].boxdata.isFemale))
            loadPKMNSprite(oamTop,spriteInfoTop,"nitro:/PICS/SPRITES/PKMN/",(*this->opponent->pkmn_team)[acpokpos[0][1]].boxdata.SPEC,176,20,oamIndex,palcnt,nextAvailableTileIdx,false,
                (*this->opponent->pkmn_team)[acpokpos[0][1]].boxdata.isShiny(),!(*this->opponent->pkmn_team)[acpokpos[0][1]].boxdata.isFemale);
        if((*this->opponent->pkmn_team)[acpokpos[0][1]].boxdata.isShiny())
            animateShiny(176+16,36);

        displayHP(100,101 ,88,32,44,45,false);
        displayHP(100,100-(*this->opponent->pkmn_team)[acpokpos[0][1]].stats.acHP*100/(*this->opponent->pkmn_team)[acpokpos[0][1]].stats.maxHP,88,32,44,45,false);      
        oamTop->oamBuffer[12].x = 96;
        oamTop->oamBuffer[12].y = 41;
        consoleSetWindow(&Top,0,5,20,5);

        printf("%10ls%c\n",(*this->opponent->pkmn_team)[acpokpos[0][1]].boxdata.Name,GENDER((*this->player->pkmn_team)[acpokpos[0][1]]));
        if((*this->opponent->pkmn_team)[acpokpos[0][1]].Level < 10)
            printf(" ");
        if((*this->opponent->pkmn_team)[acpokpos[0][1]].Level < 100)
            printf(" ");
        printf("Lv%d%4dKP",(*this->opponent->pkmn_team)[acpokpos[0][1]].Level,
            (*this->opponent->pkmn_team)[acpokpos[0][1]].stats.acHP);

        if(this->battlemode == DOUBLE && this->opponent->pkmn_team->size() > 1 && acpoksts[acpokpos[1][1]][1] != KO){
            oamTop->oamBuffer[3].isHidden = oamTop->oamBuffer[11].isHidden = true;
            updateOAM(oamTop);
            for(int i= 0; i < 80; ++i)
                swiWaitForVBlank();
            consoleSelect(&Bottom);
            clear();
            sprintf(buf,"und %ls in den Kampf.",(*this->opponent->pkmn_team)[acpokpos[1][1]].boxdata.Name);
            cust_font.print_string(buf,8,8,true);
            consoleSelect(&Top);

            animatePB(142,34); 
            oamTop->oamBuffer[3].isHidden = oamTop->oamBuffer[11].isHidden = false;

            if(!loadPKMNSprite(oamTop,spriteInfoTop,"nitro:/PICS/SPRITES/PKMN/",(*this->opponent->pkmn_team)[acpokpos[1][1]].boxdata.SPEC,112,4,oamIndex,palcnt,nextAvailableTileIdx,false,
                (*this->opponent->pkmn_team)[acpokpos[1][1]].boxdata.isShiny(),(*this->opponent->pkmn_team)[acpokpos[1][1]].boxdata.isFemale))
                loadPKMNSprite(oamTop,spriteInfoTop,"nitro:/PICS/SPRITES/PKMN/",(*this->opponent->pkmn_team)[acpokpos[1][1]].boxdata.SPEC,112,4,oamIndex,palcnt,nextAvailableTileIdx,false,
                (*this->opponent->pkmn_team)[acpokpos[1][1]].boxdata.isShiny(),!(*this->opponent->pkmn_team)[acpokpos[1][1]].boxdata.isFemale);
            if((*this->opponent->pkmn_team)[acpokpos[1][1]].boxdata.isShiny())
                animateShiny(112+16,20);
           

            displayHP(100,101,0,8,42,43,false);   
            displayHP(100,100-(*this->opponent->pkmn_team)[acpokpos[1][1]].stats.acHP*100/(*this->opponent->pkmn_team)[acpokpos[1][1]].stats.maxHP,0,8,42,43,false); 
            oamTop->oamBuffer[11].x = 8;
            oamTop->oamBuffer[11].y = 17;
            consoleSetWindow(&Top,4,2,20,5);
            printf("%ls%c\nLv%d%4dKP",(*this->opponent->pkmn_team)[acpokpos[1][1]].boxdata.Name,GENDER((*this->player->pkmn_team)[acpokpos[1][1]]),
                (*this->opponent->pkmn_team)[acpokpos[1][1]].Level, (*this->opponent->pkmn_team)[acpokpos[1][1]].stats.acHP);
        }
        else{
            oamIndex += 4;
            palcnt++;
            nextAvailableTileIdx += 144;
        }
        
        consoleSelect(&Bottom);
        for(int i = 1 + (this->opponent->pkmn_team->size() > 1 && acpoksts[acpokpos[1][1]][1] != KO); i < 6; ++i){
            oamTop->oamBuffer[11+i].x = -4+ 18*i;
            oamTop->oamBuffer[11+i].y = -4;
            oamTop->oamBuffer[11+i].isHidden = false;
            switch(this->acpoksts[acpokpos[i][1]][1]){
            case NA:
                oamTop->oamBuffer[11+i].isHidden = true;
                break;
            case KO:
                oamTop->oamBuffer[11+i].palette++;
                oamTop->oamBuffer[11+i].gfxIndex += BattleBall1TilesLen / 32;
                break;
            case STS:
                oamTop->oamBuffer[11+i].palette+=2;
                oamTop->oamBuffer[11+i].gfxIndex += BattleBall1TilesLen / 16;
                break;
            default:
                break;
            }
        }
        updateOAM(oamTop);
        for(int i= 0; i < 80; ++i)
            swiWaitForVBlank();

        if(abilities[(*this->opponent->pkmn_team)[acpokpos[0][1]].boxdata.ability].T & ablty::BEFORE_BATTLE){
            clear();
            sprintf(buf,"%s von\n %ls (Gegn.) wirkt!\n",
                abilities[(*this->opponent->pkmn_team)[acpokpos[0][1]].boxdata.ability].Name.c_str(),
                (*this->opponent->pkmn_team)[acpokpos[0][1]].boxdata.Name);
            cust_font.print_string(buf,8,8,true);
            //abilities[(*this->opponent->pkmn_team)[acpokpos[0][1]].boxdata.ability].run();
            for(int i= 0; i < 100; ++i)
                swiWaitForVBlank();
        }
        if(this->battlemode == DOUBLE && abilities[(*this->opponent->pkmn_team)[acpokpos[1][1]].boxdata.ability].T & ablty::BEFORE_BATTLE){
            clear();
            sprintf(buf,"%s von\n %ls (Gegn.) wirkt!\n",
                abilities[(*this->opponent->pkmn_team)[acpokpos[1][1]].boxdata.ability].Name.c_str(),
                (*this->opponent->pkmn_team)[acpokpos[1][1]].boxdata.Name);
            cust_font.print_string(buf,8,8,true);
            //abilities[(*this->opponent->pkmn_team)[acpokpos[0][1]].boxdata.ability].run();
            for(int i= 0; i < 100; ++i)
                swiWaitForVBlank();
        }


        //Own Side
        consoleSelect(&Bottom);
        clear();
        sprintf(buf,"Los %ls! ",(*this->player->pkmn_team)[acpokpos[0][0]].boxdata.Name);
        cust_font.print_string(buf,8,8,true);
        consoleSelect(&Top);

        animatePB(20,150);
        if(!loadPKMNSprite(oamTop,spriteInfoTop,"nitro:/PICS/SPRITES/PKMNBACK/",(*this->player->pkmn_team)[acpokpos[0][0]].boxdata.SPEC,-10,100,oamIndex,palcnt,nextAvailableTileIdx,false,
            (*this->player->pkmn_team)[acpokpos[0][0]].boxdata.isShiny(),(*this->player->pkmn_team)[acpokpos[0][0]].boxdata.isFemale))
            loadPKMNSprite(oamTop,spriteInfoTop,"nitro:/PICS/SPRITES/PKMNBACK/",(*this->player->pkmn_team)[acpokpos[0][0]].boxdata.SPEC,-10,100,oamIndex,palcnt,nextAvailableTileIdx,false,
            (*this->player->pkmn_team)[acpokpos[0][0]].boxdata.isShiny(),!(*this->player->pkmn_team)[acpokpos[0][0]].boxdata.isFemale);
        if((*this->player->pkmn_team)[acpokpos[0][0]].boxdata.isShiny())
            animateShiny(6,116);
        
        displayHP(100,101,256-96-28,192-32-8-32,48,49,false);
        displayHP(100,100-(*this->player->pkmn_team)[acpokpos[0][0]].stats.acHP*100/(*this->player->pkmn_team)[acpokpos[0][0]].stats.maxHP,256-96-28,192-32-8-32,48,49,false);       
        displayEP(100,100,256-96-28,192-32-8-32,46,47,false);
        oamTop->oamBuffer[6].x = 256-88-32+4;
        oamTop->oamBuffer[6].y = 192-31-32;
        consoleSetWindow(&Top,21,16,20,5);

        printf("%ls%c\nLv%d%4dKP",(*this->player->pkmn_team)[acpokpos[0][0]].boxdata.Name,GENDER((*this->player->pkmn_team)[acpokpos[0][0]]),(*this->player->pkmn_team)[acpokpos[0][0]].Level,
            (*this->player->pkmn_team)[acpokpos[0][0]].stats.acHP);

            
        oamTop->oamBuffer[6].isHidden = oamTop->oamBuffer[2].isHidden = false;
        updateOAM(oamTop);
        for(int i= 0; i < 80; ++i)
            swiWaitForVBlank();

        if(this->battlemode == DOUBLE && this->player->pkmn_team->size() > 1 && acpoksts[acpokpos[1][0]][0] != KO){
            consoleSelect(&Bottom);
            clear();
            sprintf(buf,"Auf in den Kampf %ls! ",(*this->player->pkmn_team)[acpokpos[1][0]].boxdata.Name);
            cust_font.print_string(buf,8,8,true);
            consoleSelect(&Top);
                
            animatePB(80,170);
            
            if(!loadPKMNSprite(oamTop,spriteInfoTop,"nitro:/PICS/SPRITES/PKMNBACK/",(*this->player->pkmn_team)[acpokpos[1][0]].boxdata.SPEC,50,120,oamIndex,palcnt,nextAvailableTileIdx,false,
                (*this->player->pkmn_team)[acpokpos[1][0]].boxdata.isShiny(),(*this->player->pkmn_team)[acpokpos[1][0]].boxdata.isFemale))
                loadPKMNSprite(oamTop,spriteInfoTop,"nitro:/PICS/SPRITES/PKMNBACK/",(*this->player->pkmn_team)[acpokpos[1][0]].boxdata.SPEC,50,120,oamIndex,palcnt,nextAvailableTileIdx,false,
                (*this->player->pkmn_team)[acpokpos[1][0]].boxdata.isShiny(),!(*this->player->pkmn_team)[acpokpos[1][0]].boxdata.isFemale);
            if((*this->player->pkmn_team)[acpokpos[1][0]].boxdata.isShiny())
                animateShiny(50+16,136);
            
            oamTop->oamBuffer[5].isHidden = oamTop->oamBuffer[1].isHidden = false;
            updateOAM(oamTop);

            displayHP(100,101,256-36,192-40,50,51,false);   
            displayHP(100,100-(*this->player->pkmn_team)[acpokpos[1][0]].stats.acHP*100/(*this->player->pkmn_team)[acpokpos[1][0]].stats.maxHP,256-36,192-40,50,51,false);        
            displayEP(100,100,256-36,192-40,46,47,false);
            oamTop->oamBuffer[5].x = 256-32+4;
            oamTop->oamBuffer[5].y = 192-31;
            consoleSetWindow(&Top,16,20,20,5);
            printf("%10ls%c\n",(*this->player->pkmn_team)[acpokpos[1][0]].boxdata.Name,GENDER((*this->player->pkmn_team)[acpokpos[1][0]]));
            if((*this->player->pkmn_team)[acpokpos[1][0]].Level < 10)
                printf(" ");
            if((*this->player->pkmn_team)[acpokpos[1][0]].Level < 100)
                printf(" ");
            printf("Lv%d%4dKP",(*this->player->pkmn_team)[acpokpos[1][0]].Level,
                (*this->player->pkmn_team)[acpokpos[1][0]].stats.acHP);

        }
        else{
            oamIndex += 4;
            palcnt++;
            nextAvailableTileIdx += 144;
        }

        for(int i = 1 + (this->player->pkmn_team->size() > 1 && acpoksts[acpokpos[1][0]][0] != KO); i < 6; ++i){
            oamTop->oamBuffer[5+i].x = 236 - 18*i;
            oamTop->oamBuffer[5+i].y = 196 - 16;
            oamTop->oamBuffer[5+i].isHidden = false;
            switch(this->acpoksts[acpokpos[i][0]][0]){
            case NA:
                oamTop->oamBuffer[5+i].isHidden = true;
                break;
            case KO:
                oamTop->oamBuffer[5+i].palette++;
                oamTop->oamBuffer[5+i].gfxIndex += BattleBall1TilesLen / 32;
                break;
            case STS:
                oamTop->oamBuffer[5+i].palette+=2;
                oamTop->oamBuffer[5+i].gfxIndex += BattleBall1TilesLen / 16;
                break;
            default:
                break;
            }
        }
        updateOAM(oamTop);
        for(int i= 0; i < 80; ++i)
            swiWaitForVBlank();
        
        consoleSelect(&Bottom);
        if(abilities[(*this->player->pkmn_team)[acpokpos[0][0]].boxdata.ability].T & ablty::BEFORE_BATTLE){
            clear();
            sprintf(buf,"%s von\n %ls wirkt!\n",
                abilities[(*this->player->pkmn_team)[acpokpos[0][0]].boxdata.ability].Name.c_str(),
                (*this->player->pkmn_team)[acpokpos[0][0]].boxdata.Name);
            cust_font.print_string(buf,8,8,true);
            //abilities[(*this->opponent->pkmn_team)[acpokpos[0][1]].boxdata.ability].run();
            for(int i= 0; i < 100; ++i)
                swiWaitForVBlank();
        }
        if(this->battlemode == DOUBLE && abilities[(*this->player->pkmn_team)[acpokpos[1][0]].boxdata.ability].T & ablty::BEFORE_BATTLE){
            clear();
            sprintf(buf,"%s von\n %ls wirkt!\n",
                abilities[(*this->player->pkmn_team)[acpokpos[1][0]].boxdata.ability].Name.c_str(),
                (*this->player->pkmn_team)[acpokpos[1][0]].boxdata.Name);
            cust_font.print_string(buf,8,8,true);
            //abilities[(*this->opponent->pkmn_team)[acpokpos[0][1]].boxdata.ability].run();
            for(int i= 0; i < 100; ++i)
                swiWaitForVBlank();
        }

        oamTop->oamBuffer[4].isHidden = oamTop->oamBuffer[12].isHidden = false;

        updateOAM(oamTop);
        
        for(int i = 0; i<= 4; ++i)
            oam->oamBuffer[i].isHidden = false;
        oam->oamBuffer[2].isHidden = !SAV.activatedPNav; 
        updateOAMSub(oam);
        consoleSelect(&Bottom);
        consoleClear();
    }

    int battle::start(int battle_back,Weather weather){
        initBattleScene(battle_back, weather);
        char buf[100];
        touchPosition t;

        while((this->round--) != 0) {
ACR:
            setMainBattleVisibility(false);
            int os2 = oamIndexS, pS2 = palcntS, ts2 = nextAvailableTileIdxS;
            drawPKMNIcon(oam,spriteInfo,(*this->player->pkmn_team)[acpokpos[0][0]].boxdata.SPEC,112,64,os2,pS2,ts2,true);
            clear();
            sprintf(buf,"Was soll %ls tun?          ",(*this->player->pkmn_team)[acpokpos[0][0]].boxdata.Name);
            cust_font.print_string(buf,8,8,true);
            updateOAMSub(oam);

            std::pair<int,int> ownAtk[2]; //AtkIdx; Target 1->opp[0]/2->opp[1]/3->both_opp/4->self/8->partner
            std::pair<int,int> oppAtk[2]; //AtkIdx; Target 1->own[0]/2->own[1]/3->both_opp/4->self/8->partner

            int switchWith[2][2] = {{0}};

            int aprest = 0;
            while(42){
                updateTime();
                swiWaitForVBlank();
                touchRead(&t);
                if(t.px > 64 && t.px < 64 + 128 && t.py > 72 && t.py < 72 + 64){ //FIGHT
                    waitForTouchUp();
                    setMainBattleVisibility(true);
                    oam->oamBuffer[oamIndexS+1].isHidden = true;
                    consoleClear();

                    aprest = 0;
                    for(int i= 0; i < 4; ++i)
                        aprest += (*this->player->pkmn_team)[acpokpos[0][0]].boxdata.AcPP[i];
                    if(aprest == 0){
                        clear();
                        printf("%ls hat keine\n restlichen Attacken...",(*this->player->pkmn_team)[acpokpos[0][0]].boxdata.Name);
                        cust_font.print_string(buf,8,8,true);
                        ownAtk[0] = std::pair<int,int>(0,1|2|4|8);
                    }
                    else {
                        (oam->oamBuffer[20]).isHidden = false;

                        for(int i = 21; i < 29; i+=2){
                            (oam->oamBuffer[i]).isHidden = false;
                            (oam->oamBuffer[i+1]).isHidden = false;
                            (oam->oamBuffer[i+1]).y += 8+16 * ((i-21)/4);
                            (oam->oamBuffer[i]).y += 8+16 * ((i-21)/4);
                            if((i/2)%2)
                                (oam->oamBuffer[i]).x -= 16;
                            else
                                (oam->oamBuffer[i+1]).x += 16;
                            updateOAMSub(oam); 
                        
                            consoleSetWindow(&Bottom,(oam->oamBuffer[i]).x/8+1,(oam->oamBuffer[i]).y/8+1,20,5);
                            drawTypeIcon(oam,spriteInfo,os2,pS2,ts2,
                                AttackList[(*this->player->pkmn_team)[acpokpos[0][0]].boxdata.Attack[(i-21)/2]]->type,
                                (oam->oamBuffer[i]).x+8,(oam->oamBuffer[i]).y-8,true);
                            printf("    %s\n    AP %2i""/""%2i ",
                                &(AttackList[(*this->player->pkmn_team)[acpokpos[0][0]].boxdata.Attack[(i-21)/2]]->Name[0]),
                                (*this->player->pkmn_team)[acpokpos[0][0]].boxdata.AcPP[i],
                                AttackList[(*this->player->pkmn_team)[acpokpos[0][0]].boxdata.Attack[(i-21)/2]]->PP *
                                ((5 +(*this->player->pkmn_team)[acpokpos[0][0]].boxdata.PPupget(i)) / 5));
                            switch (AttackList[(*this->player->pkmn_team)[acpokpos[0][0]].boxdata.Attack[(i-21)/2]]->HitType)
                            {
                            case attack::PHYS:
                                printf("PHS");
                                break;
                            case attack::SPEC:
                                printf("SPC");
                                break;
                            case attack::STAT:
                                printf("STS");
                                break;
                            }
                            printf("\n    S ");
                            if(AttackList[(*this->player->pkmn_team)[acpokpos[0][0]].boxdata.Attack[(i-21)/2]]->Base_Power)
                                printf("%3i",AttackList[(*this->player->pkmn_team)[acpokpos[0][0]].boxdata.Attack[(i-21)/2]]->Base_Power);
                            else
                                printf("---");
                            printf(" G ");
                            if(AttackList[(*this->player->pkmn_team)[acpokpos[0][0]].boxdata.Attack[(i-21)/2]]->Accuracy)
                                printf("%3i",AttackList[(*this->player->pkmn_team)[acpokpos[0][0]].boxdata.Attack[(i-21)/2]]->Accuracy);
                            else
                                printf("---");
                        }

                        updateOAMSub(oam);
                        consoleSetWindow(&Bottom,0,0,32,5);
                        clear();
                        sprintf(buf,"Welchen Angriff?");
                        cust_font.print_string(buf,8,8,true);

                        while(42){
                            updateTime();
                            swiWaitForVBlank();
                            touchRead(&t);
                            if ( t.px>224 && t.py>164){ //Back  
                                while(1) {
                                    scanKeys();
                                    swiWaitForVBlank();
                                    updateTime();
                                    if(keysUp() & KEY_TOUCH)
                                    break;
                                }
                                consoleSelect(&Bottom);
                                consoleSetWindow(&Bottom,0,0,32,24);
                                consoleClear();
                                for(int i= 20; i <= os2; ++i)
                                    (oam->oamBuffer[i]).isHidden = true;
                                setMainBattleVisibility(false);

                            
                                for(int i = 21; i < 29; i+=2){
                                    (oam->oamBuffer[i]).isHidden = true;
                                    (oam->oamBuffer[i+1]).isHidden = true;
                                    (oam->oamBuffer[i+1]).y -= 8+16 * ((i-21)/4);
                                    (oam->oamBuffer[i]).y -= 8+16 * ((i-21)/4);
                        
                                    if((i/2)%2)
                                        (oam->oamBuffer[i]).x += 16;
                                    else
                                        (oam->oamBuffer[i+1]).x -= 16;
                                    updateOAMSub(oam); 
                                }

                                goto ACR;
                            }
                            
                            for(int i = 21; i < 29; i+=2)
                                if(t.px>(oam->oamBuffer[i].x) && t.px < (oam->oamBuffer[i+1].x + 64) &&
                                    t.py>(oam->oamBuffer[i]).y && t.py < (oam->oamBuffer[i+1].y + 64)){
                                        while(1) {
                                            scanKeys();
                                            swiWaitForVBlank();
                                            updateTime();
                                            if(keysUp() & KEY_TOUCH)
                                            break;
                                        }
                                        int trg = 0;

                                        //CHOOSE TARGET
                                        printf("CHOSE ATK %i",(i-21)/2);
                                        
                                        ownAtk[0] = std::pair<int,int>((*this->player->pkmn_team)[acpokpos[0][0]].boxdata.Attack[(i-21)/2],trg);
                                        goto ATTACKCHOSEN;
                                }
                        }
                        ATTACKCHOSEN:
                        for(int i = 21; i < 29; i+=2){
                            (oam->oamBuffer[i]).isHidden = true;
                            (oam->oamBuffer[i+1]).isHidden = true;
                            (oam->oamBuffer[i+1]).y -= 8+16 * ((i-21)/4);
                            (oam->oamBuffer[i]).y -= 8+16 * ((i-21)/4);
                        
                            if((i/2)%2)
                                (oam->oamBuffer[i]).x += 16;
                            else
                                (oam->oamBuffer[i+1]).x -= 16;
                            
                            updateOAMSub(oam); 
                        }
                        for(int i= os2; i >= os2 - 3; --i)
                            (oam->oamBuffer[i]).isHidden = true;
                        updateOAMSub(oam); 
                        consoleSetWindow(&Bottom,0,0,32,24);
                        consoleClear();
                        consoleSetWindow(&Bottom,0,0,32,5);
                    }

                    if(this->battlemode == DOUBLE){
ACR2:
                        os2 = oamIndexS; pS2 = palcntS; ts2 = nextAvailableTileIdxS;
                        setMainBattleVisibility(false);
                        drawPKMNIcon(oam,spriteInfo,(*this->player->pkmn_team)[acpokpos[1][0]].boxdata.SPEC,112,64,os2,pS2,ts2,true);
                        clear();
                        sprintf(buf,"Was soll %ls tun?          ",(*this->player->pkmn_team)[acpokpos[1][0]].boxdata.Name);
                        cust_font.print_string(buf,8,8,true);
                        (oam->oamBuffer[20]).isHidden = false;
                        updateOAMSub(oam);
                        while(42){
                            updateTime();
                            swiWaitForVBlank();
                            touchRead(&t);
                            if ( t.px>224 && t.py>164)//Back
                            {  
                                while(1) {
                                    scanKeys();
                                    swiWaitForVBlank();
                                    updateTime();
                                    if(keysUp() & KEY_TOUCH)
                                    break;
                                }
                                consoleSelect(&Bottom);
                                consoleSetWindow(&Bottom,0,0,32,24);
                                consoleClear();
                                for(int i= 20; i <= os2; ++i)
                                    (oam->oamBuffer[i]).isHidden = true;
                                setMainBattleVisibility(false);

                                goto ACR;
                            }
                            else if(t.px > 64 && t.px < 64 + 128 && t.py > 72 && t.py < 72 + 64){ //FIGHT
                                waitForTouchUp();
                                setMainBattleVisibility(true);
                                (oam->oamBuffer[20]).isHidden = false;

                                updateOAMSub(oam);
                                oam->oamBuffer[oamIndexS+1].isHidden = true;
                                consoleClear();

                                aprest = 0;
                                for(int i= 0; i < 4; ++i)
                                    aprest += (*this->player->pkmn_team)[acpokpos[1][0]].boxdata.AcPP[i];
                                if(aprest == 0){
                                    clear();
                                    printf("%ls hat keine\n restlichen Attacken...",(*this->player->pkmn_team)[acpokpos[1][0]].boxdata.Name);
                                    cust_font.print_string(buf,8,8,true);
                                    ownAtk[0] = std::pair<int,int>(0,1|2|4|8);
                                }
                                else {
                                    for(int i = 21; i < 29; i+=2){
                                        (oam->oamBuffer[i]).isHidden = false;
                                        (oam->oamBuffer[i+1]).isHidden = false;
                                        (oam->oamBuffer[i+1]).y += 8+16 * ((i-21)/4);
                                        (oam->oamBuffer[i]).y += 8+16 * ((i-21)/4);
                                        if((i/2)%2)
                                            (oam->oamBuffer[i]).x -= 16;
                                        else
                                            (oam->oamBuffer[i+1]).x += 16;
                                        updateOAMSub(oam); 
                        
                                        consoleSetWindow(&Bottom,(oam->oamBuffer[i]).x/8+1,(oam->oamBuffer[i]).y/8+1,20,5);
                                        drawTypeIcon(oam,spriteInfo,os2,pS2,ts2,
                                            AttackList[(*this->player->pkmn_team)[acpokpos[1][0]].boxdata.Attack[(i-21)/2]]->type,
                                            (oam->oamBuffer[i]).x+8,(oam->oamBuffer[i]).y-8,true);
                                        printf("    %s\n    AP %2i""/""%2i ",
                                            &(AttackList[(*this->player->pkmn_team)[acpokpos[1][0]].boxdata.Attack[(i-21)/2]]->Name[0]),
                                            (*this->player->pkmn_team)[acpokpos[1][0]].boxdata.AcPP[i],
                                            AttackList[(*this->player->pkmn_team)[acpokpos[1][0]].boxdata.Attack[(i-21)/2]]->PP *
                                            ((5 +(*this->player->pkmn_team)[acpokpos[1][0]].boxdata.PPupget(i)) / 5));
                                        switch (AttackList[(*this->player->pkmn_team)[acpokpos[1][0]].boxdata.Attack[(i-21)/2]]->HitType)
                                        {
                                        case attack::PHYS:
                                            printf("PHS");
                                            break;
                                        case attack::SPEC:
                                            printf("SPC");
                                            break;
                                        case attack::STAT:
                                            printf("STS");
                                            break;
                                        }
                                        printf("\n    S ");
                                        if(AttackList[(*this->player->pkmn_team)[acpokpos[1][0]].boxdata.Attack[(i-21)/2]]->Base_Power)
                                            printf("%3i",AttackList[(*this->player->pkmn_team)[acpokpos[1][0]].boxdata.Attack[(i-21)/2]]->Base_Power);
                                        else
                                            printf("---");
                                        printf(" G ");
                                        if(AttackList[(*this->player->pkmn_team)[acpokpos[1][0]].boxdata.Attack[(i-21)/2]]->Accuracy)
                                            printf("%3i",AttackList[(*this->player->pkmn_team)[acpokpos[1][0]].boxdata.Attack[(i-21)/2]]->Accuracy);
                                        else
                                            printf("---");
                                    }

                                    updateOAMSub(oam);
                                    consoleSetWindow(&Bottom,0,0,32,5);
                                    clear();
                                    sprintf(buf,"Welchen Angriff?");
                                    cust_font.print_string(buf,8,8,true);

                                    while(42){
                                        updateTime();
                                        swiWaitForVBlank();
                                        touchRead(&t);
                                        if ( t.px>224 && t.py>164){ //Back  
                                            while(1) {
                                                scanKeys();
                                                swiWaitForVBlank();
                                                updateTime();
                                                if(keysUp() & KEY_TOUCH)
                                                break;
                                            }
                                            consoleSelect(&Bottom);
                                            consoleSetWindow(&Bottom,0,0,32,24);
                                            consoleClear();
                                            for(int i= 20; i <= os2; ++i)
                                                (oam->oamBuffer[i]).isHidden = true;
                                            setMainBattleVisibility(false);

                            
                                            for(int i = 21; i < 29; i+=2){
                                                (oam->oamBuffer[i]).isHidden = true;
                                                (oam->oamBuffer[i+1]).isHidden = true;
                                                (oam->oamBuffer[i+1]).y -= 8+16 * ((i-21)/4);
                                                (oam->oamBuffer[i]).y -= 8+16 * ((i-21)/4);
                        
                                                if((i/2)%2)
                                                    (oam->oamBuffer[i]).x += 16;
                                                else
                                                    (oam->oamBuffer[i+1]).x -= 16;
                                                updateOAMSub(oam); 
                                            }

                                            goto ACR;
                                        }
                            
                                        for(int i = 21; i < 29; i+=2)
                                            if(t.px>(oam->oamBuffer[i].x) && t.px < (oam->oamBuffer[i+1].x + 64) &&
                                                t.py>(oam->oamBuffer[i]).y && t.py < (oam->oamBuffer[i+1].y + 64)){
                                                    while(1) {
                                                        scanKeys();
                                                        swiWaitForVBlank();
                                                        updateTime();
                                                        if(keysUp() & KEY_TOUCH)
                                                        break;
                                                    }
                                                    int trg = 0;

                                                    //CHOOSE TARGET
                                                    printf("CHOSE ATK %i",(i-21)/2);
                                        
                                                    ownAtk[1] = std::pair<int,int>((*this->player->pkmn_team)[acpokpos[1][0]].boxdata.Attack[(i-21)/2],trg);
                                                    goto ATTACKCHOSEN2;
                                            }
                                    }
                                    ATTACKCHOSEN2:
                                    for(int i = 21; i < 29; i+=2){
                                        (oam->oamBuffer[i]).isHidden = true;
                                        (oam->oamBuffer[i+1]).isHidden = true;
                                        (oam->oamBuffer[i+1]).y -= 8+16 * ((i-21)/4);
                                        (oam->oamBuffer[i]).y -= 8+16 * ((i-21)/4);
                        
                                        if((i/2)%2)
                                            (oam->oamBuffer[i]).x += 16;
                                        else
                                            (oam->oamBuffer[i+1]).x -= 16;
                            
                                        updateOAMSub(oam); 
                                    }
                                    for(int i= os2; i >= os2 - 3; --i)
                                        (oam->oamBuffer[i]).isHidden = true;
                                    updateOAMSub(oam); 
                                    consoleSetWindow(&Bottom,0,0,32,24);
                                    consoleClear();
                                    consoleSetWindow(&Bottom,0,0,32,5);
                            
                                }
                                break;
                            }
                            else if(t.px > 16 && t.px < 16 + 64 && t.py > 144 && t.py < 144 + 32){ //BAG
                                waitForTouchUp();
                                //printf("PAL: %d; OAM: %d; TILE: %d\n",palcnt,oamIndex,nextAvailableTileIdx);
                                goto BATTLE_END;
                            }
                            else if(SAV.activatedPNav && t.px > 96 && t.px < 96 + 64 && t.py > 152 && t.py < 152+32){ //POKENAV
                                waitForTouchUp();
                                printf("TEST3");
                            }
                            else if(t.px > 176 && t.px < 176 + 64 && t.py > 144 && t.py < 144 + 32){ //PKMN
                                waitForTouchUp();
                                setMainBattleVisibility(true);
                                consoleClear();
                                dinit();
                                for(int i= 0; i <= 8; ++i)
                                    (oam->oamBuffer[i]).isHidden = true;
                                updateOAMSub(oam);
                                
                                os2 = oamIndexS; pS2 = palcntS; ts2 = nextAvailableTileIdxS;
                                int num = (int)this->player->pkmn_team->size();
                                consoleSelect(&Bottom);
                                for(int i = 23; i < 32; i+=(((i-21)/2)%2?-2:+6)){
                                    if((((i-21)/2)^1) >= num)
                                    break;
                                    (oam->oamBuffer[i]).isHidden = false;
                                    (oam->oamBuffer[i+1]).isHidden = false;
                                    (oam->oamBuffer[i+1]).y -= 16 * (2-((i-21)/4));
                                    (oam->oamBuffer[i]).y -= 16 * (2-((i-21)/4));
                                    updateOAMSub(oam); 
                                    consoleSetWindow(&Bottom,((oam->oamBuffer[i]).x+6)/8,((oam->oamBuffer[i]).y+6)/8,12,3);
                                    if(!(*this->player->pkmn_team)[acpokpos[((i-21)/2)^1][0]].boxdata.IV.isEgg){
                                        printf("   %3i/%3i\n ",(*this->player->pkmn_team)[acpokpos[((i-21)/2)^1][0]].stats.acHP,(*this->player->pkmn_team)[acpokpos[((i-21)/2)^1][0]].stats.maxHP);
                                        wprintf((*this->player->pkmn_team)[acpokpos[((i-21)/2)^1][0]].boxdata.Name); printf("\n");
                                        printf("%11s",ItemList[(*this->player->pkmn_team)[acpokpos[((i-21)/2)^1][0]].boxdata.Item].getDisplayName().c_str());
                                        drawPKMNIcon(oam,spriteInfo,(*this->player->pkmn_team)[acpokpos[((i-21)/2)^1][0]].boxdata.SPEC,(oam->oamBuffer[i]).x-4,(oam->oamBuffer[i]).y-20,os2,pS2,ts2,true);
                                    }
                                    else{
                                        printf("\n Ei");
                                        drawEggIcon(oam,spriteInfo,(oam->oamBuffer[i]).x-4,(oam->oamBuffer[i]).y-20,os2,pS2,ts2,true);                                    
                                    }
                                    updateOAMSub(oam); 
                                }

                                while(42){
                                    swiWaitForVBlank();
                                    updateOAMSub(oam);
                                    updateTime();
                                    touchRead(&t);

                                    if ( t.px>224 && t.py>164)
                                    {  
                                        while(1)
                                        {
                                            scanKeys();
                                            swiWaitForVBlank();
                                            updateTime();
                                            if(keysUp() & KEY_TOUCH)
                                            break;
                                        }
                                        consoleSelect(&Bottom);
                                        consoleSetWindow(&Bottom,0,0,32,24);
                                        consoleClear();
                                        for(int i= 5; i <= 8; ++i)
                                            (oam->oamBuffer[i]).isHidden = false;
                                        for(int i= 20; i <= os2; ++i)
                                            (oam->oamBuffer[i]).isHidden = true;
                                        updateOAMSub(oam);
                                        
                                        for(int i = 23; i < 32; i+=(((i-21)/2)%2?-2:+6)){
                                            (oam->oamBuffer[i+1]).y += 16 * (2-((i-21)/4));
                                            (oam->oamBuffer[i]).y += 16 * (2-((i-21)/4));
                                        }
                                        init();
                                        goto ACR2;
                                    }
                                    for(int i = 23; i < 32; i+=(((i-21)/2)%2?-2:+6)){
                                        if((((i-21)/2)^1) >= num)
                                            break;
                                        else if (t.px > oam->oamBuffer[i].x && t.py > oam->oamBuffer[i].y && t.px-64 < oam->oamBuffer[i+1].x && t.py-32 < oam->oamBuffer[i].y){  
                                            if((*this->player->pkmn_team)[acpokpos[((i-21)/2)^1][0]].boxdata.IV.isEgg || 
                                                (*this->player->pkmn_team)[acpokpos[((i-21)/2)^1][0]].stats.acHP == 0 || (((i-21)/2)^1) < 2 || 
                                                (((i-21)/2)^1) == switchWith[0][0])
                                                continue;

                                            (oam->oamBuffer[i]).isHidden = true;
                                            (oam->oamBuffer[i+1]).isHidden = true;
                                            (oam->oamBuffer[3 + (((i-21)/2)^1)]).isHidden = true;
                                            updateOAMSub(oam);
                                            
                                            waitForTouchUp();
 
                                            switchWith[1][0] = ((i-21)/2)^1;
                                            goto OUT;

                                        }
                                    }
                                }                                
OUT:
                                consoleSelect(&Bottom);
                                consoleSetWindow(&Bottom,0,0,32,24);
                                consoleClear();
                                init();
                                for(int i= 20; i <= os2; ++i)
                                    (oam->oamBuffer[i]).isHidden = true;
                                          
                                for(int i = 23; i < 32; i+=(((i-21)/2)%2?-2:+6)){
                                    (oam->oamBuffer[i+1]).y += 16 * (2-((i-21)/4));
                                    (oam->oamBuffer[i]).y += 16 * (2-((i-21)/4));
                                }
                                updateOAMSub(oam);
                                break;
                            }
                        }
                    }
                    (oam->oamBuffer[os2]).isHidden = true;
                    updateOAMSub(oam);

                    //OPP'S ACTIONS
                    switchWith[0][1] = switchWith[0][0];
                    switchWith[1][1] = switchWith[1][0];
OPPTURN:
                    
                    consoleClear();
                    for(int i = 0; i< 40; ++i)
                        swiWaitForVBlank();
                    
                    if(switchWith[0][0])
                        switchOwnPkmn(switchWith[0][0],0);
                    if(switchWith[1][0])
                        switchOwnPkmn(switchWith[1][0],1);

                    if(switchWith[0][1])
                        switchOppPkmn(switchWith[0][1],0);
                    if(switchWith[1][1])
                        switchOppPkmn(switchWith[1][1],1);


                    break;
                }
                else if(t.px > 16 && t.px < 16 + 64 && t.py > 144 && t.py < 144 + 32){ //BAG
                    waitForTouchUp();
                    //printf("PAL: %d; OAM: %d; TILE: %d\n",palcnt,oamIndex,nextAvailableTileIdx);
                    goto BATTLE_END;
                }
                else if(SAV.activatedPNav && t.px > 96 && t.px < 96 + 64 && t.py > 152 && t.py < 152+32){ //POKENAV
                    waitForTouchUp();
                    printf("TEST3");
                }
                else if(t.px > 176 && t.px < 176 + 64 && t.py > 144 && t.py < 144 + 32){ //PKMN
                    waitForTouchUp();
                    setMainBattleVisibility(true);
                    consoleClear();
                    dinit();
                    for(int i= 0; i <= 8; ++i)
                        (oam->oamBuffer[i]).isHidden = true;
                    (oam->oamBuffer[20]).isHidden = false;
                    updateOAMSub(oam);
                                
                    os2 = oamIndexS; pS2 = palcntS; ts2 = nextAvailableTileIdxS;
                    int num = (int)this->player->pkmn_team->size();
                    consoleSelect(&Bottom);

                    for(int i = 23; i < 32; i+=(((i-21)/2)%2?-2:+6)){
                        if((((i-21)/2)^1) >= num)
                        break;
                        (oam->oamBuffer[i]).isHidden = false;
                        (oam->oamBuffer[i+1]).isHidden = false;
                        (oam->oamBuffer[i+1]).y -= 16 * (2-((i-21)/4));
                        (oam->oamBuffer[i]).y -= 16 * (2-((i-21)/4));
                        updateOAMSub(oam); 
                        consoleSetWindow(&Bottom,((oam->oamBuffer[i]).x+6)/8,((oam->oamBuffer[i]).y+6)/8,12,3);
                        if(!(*this->player->pkmn_team)[acpokpos[((i-21)/2)^1][0]].boxdata.IV.isEgg){
                            printf("   %3i/%3i\n ",(*this->player->pkmn_team)[acpokpos[((i-21)/2)^1][0]].stats.acHP,(*this->player->pkmn_team)[acpokpos[((i-21)/2)^1][0]].stats.maxHP);
                            wprintf((*this->player->pkmn_team)[acpokpos[((i-21)/2)^1][0]].boxdata.Name); printf("\n");
                            printf("%11s",ItemList[(*this->player->pkmn_team)[acpokpos[((i-21)/2)^1][0]].boxdata.Item].getDisplayName().c_str());
                            drawPKMNIcon(oam,spriteInfo,(*this->player->pkmn_team)[acpokpos[((i-21)/2)^1][0]].boxdata.SPEC,(oam->oamBuffer[i]).x-4,(oam->oamBuffer[i]).y-20,os2,pS2,ts2,true);
                        }
                        else{
                            printf("\n Ei");
                            drawEggIcon(oam,spriteInfo,(oam->oamBuffer[i]).x-4,(oam->oamBuffer[i]).y-20,os2,pS2,ts2,true);                                    
                        }
                        updateOAMSub(oam); 
                    }

                    while(42){
                        swiWaitForVBlank();
                        updateOAMSub(oam);
                        updateTime();
                        touchRead(&t);

                        if ( t.px>224 && t.py>164)
                        {  
                            while(1)
                            {
                                scanKeys();
                                swiWaitForVBlank();
                                updateTime();
                                if(keysUp() & KEY_TOUCH)
                                break;
                            }
                            consoleSelect(&Bottom);
                            consoleSetWindow(&Bottom,0,0,32,24);
                            consoleClear();
                            for(int i= 5; i <= 8; ++i)
                                (oam->oamBuffer[i]).isHidden = false;
                            for(int i= 20; i <= os2; ++i)
                                (oam->oamBuffer[i]).isHidden = true;
                            updateOAMSub(oam);
                                        
                            for(int i = 23; i < 32; i+=(((i-21)/2)%2?-2:+6)){
                                (oam->oamBuffer[i+1]).y += 16 * (2-((i-21)/4));
                                (oam->oamBuffer[i]).y += 16 * (2-((i-21)/4));
                            }
                            init();
                            goto ACR;
                        }
                        for(int i = 23; i < 32; i+=(((i-21)/2)%2?-2:+6)){
                            if((((i-21)/2)^1) >= num)
                                break;
                            else if (t.px > oam->oamBuffer[i].x && t.py > oam->oamBuffer[i].y && t.px-64 < oam->oamBuffer[i+1].x && t.py-32 < oam->oamBuffer[i].y){  
                                if((*this->player->pkmn_team)[acpokpos[((i-21)/2)^1][0]].boxdata.IV.isEgg || 
                                    (*this->player->pkmn_team)[acpokpos[((i-21)/2)^1][0]].stats.acHP == 0 || (((i-21)/2)^1) < 2)
                                    continue;

                                (oam->oamBuffer[i]).isHidden = true;
                                (oam->oamBuffer[i+1]).isHidden = true;
                                (oam->oamBuffer[3 + (((i-21)/2)^1)]).isHidden = true;
                                updateOAMSub(oam);
                                            
                                waitForTouchUp();
 
                                switchWith[0][0] = ((i-21)/2)^1;
                                goto OUT2;

                            }
                        }
                    }
OUT2:
                    consoleSelect(&Bottom);
                    consoleSetWindow(&Bottom,0,0,32,24);
                    consoleClear();
                    for(int i= 5; i <= 8; ++i)
                        (oam->oamBuffer[i]).isHidden = false;
                    for(int i= 20; i <= os2; ++i)
                        (oam->oamBuffer[i]).isHidden = true;          
                    for(int i = 23; i < 32; i+=(((i-21)/2)%2?-2:+6)){
                        (oam->oamBuffer[i+1]).y += 16 * (2-((i-21)/4));
                        (oam->oamBuffer[i]).y += 16 * (2-((i-21)/4));
                    }
                    updateOAMSub(oam);
                    init();
                    goto ACR2;
                }
            }
        }
BATTLE_END:
        dinit();
        consoleSetWindow(&Bottom,0,0,32,24);
        consoleSelect(&Bottom);
        consoleClear();
        initOAMTableSub(oam);
        initMainSprites(oam,spriteInfo);
        setMainSpriteVisibility(false);
        oam->oamBuffer[8].isHidden = true;
        oam->oamBuffer[0].isHidden = true;
        oam->oamBuffer[1].isHidden = false;
        return 0;
    }
}