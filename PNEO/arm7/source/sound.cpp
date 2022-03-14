// Based on SSEQ Player by "RocketRobz" (https://github.com/RocketRobz/SSEQPlayer)

#include <cstdio>
#include <cstdlib>
#include <nds.h>

#include "sseq.h"

#ifndef NO_SOUND
#ifndef MMOD
namespace SOUND::SSEQ {
    static void soundTimer( );
    static void soundSysMessageHandler( int, void* );
    static void adsrTickChannel( u8 );
    static void adsrTick( );

    volatile s32 SEQ_STATUS = STATUS_STOPPED;
    volatile s32 CUR_BPM;
    volatile s32 CUR_VOL;

    void installSoundSys( ) {
        // Install timer
        timerStart( 1, ClockDivider_64, -2728, soundTimer );

        // Install FIFO
        fifoSetDatamsgHandler( FIFO_SNDSYS, soundSysMessageHandler, 0 );

        // Clear track-channel assignments
        for( u8 i = 0; i < NUM_CHANNEL; ++i ) { ADSR_CHANNEL[ i ].m_track = -1; }
    }

    static void soundTimer( ) {
        static volatile s32 v = 0;

        adsrTick( );

        while( v > 240 ) v -= 240, seq_tick( );
        v += SEQ_BPM;
    }

    adsrState ADSR_CHANNEL[ NUM_CHANNEL ];

    volatile s32 ADSR_MASTER_VOLUME = 127;

    static inline void adsrTick( ) {
        for( u8 i = 0; i < NUM_CHANNEL; ++i ) { adsrTickChannel( i ); }
    }

    static void adsrTickChannel( u8 p_channel ) {
        adsrState* chstat = ADSR_CHANNEL + p_channel;
        switch( chstat->m_state ) {
        default:
        case adsrState::ADSR_NONE: return;
        case adsrState::ADSR_SUSTAIN:
            if( !SCHANNEL_ACTIVE( p_channel ) ) {
                chstat->m_state = adsrState::ADSR_NONE;
                chstat->m_count = 0;
                chstat->m_track = -1;
                return;
            }
            break;
        case adsrState::ADSR_START:
            SCHANNEL_CR( p_channel )           = 0;
            SCHANNEL_SOURCE( p_channel )       = chstat->m_reg.m_source;
            SCHANNEL_TIMER( p_channel )        = -chstat->m_reg.m_timer;
            SCHANNEL_REPEAT_POINT( p_channel ) = chstat->m_reg.m_repeatPoint;
            SCHANNEL_LENGTH( p_channel )       = chstat->m_reg.m_length;
            SCHANNEL_CR( p_channel )           = chstat->m_reg.m_cr;
            chstat->m_ampl                     = -ADSR_THRESHOLD;
            chstat->m_state                    = adsrState::ADSR_ATTACK;
        case adsrState::ADSR_ATTACK:
            chstat->m_ampl = ( chstat->m_attackRate * chstat->m_ampl ) / 255;
            if( chstat->m_ampl == 0 ) { chstat->m_state = adsrState::ADSR_DECAY; }
            break;
        case adsrState::ADSR_DECAY:
            chstat->m_ampl -= chstat->m_decayRate;
            if( chstat->m_ampl <= chstat->m_sustainRate ) {
                chstat->m_ampl  = chstat->m_sustainRate;
                chstat->m_state = adsrState::ADSR_SUSTAIN;
            }
            break;
        case adsrState::ADSR_RELEASE:
            chstat->m_ampl -= chstat->m_releaseRate;
            if( s32( chstat->m_ampl ) <= -s32( ADSR_THRESHOLD ) ) {
                //__adsr_release:
                chstat->m_state = adsrState::ADSR_NONE;
                // chstat->m_reg.CR = 0;
                chstat->m_count          = 0;
                chstat->m_track          = -1;
                SCHANNEL_CR( p_channel ) = 0;
                return;
            }
            break;
        }

        // Update the modulation params
        s32 modParam = 0;
        s32 modType  = chstat->m_modType;

        if( chstat->m_modDelayCnt < chstat->m_modDelay ) {
            chstat->m_modDelayCnt++;
        } else {
            u16 speed   = (u16) chstat->m_modSpeed << 6;
            u16 counter = ( chstat->m_modCounter + speed ) >> 8;

            while( counter >= 0x80 ) { counter -= 0x80; }

            chstat->m_modCounter += speed;
            chstat->m_modCounter &= 0xFF;
            chstat->m_modCounter |= counter << 8;

            modParam = getSoundSine( chstat->m_modCounter >> 8 ) * chstat->m_modRange
                       * chstat->m_modDepth;
        }

        modParam >>= 8;
#define CONV_VOL( p_a )     ( convertSustain( p_a ) >> 7 )
#define SOUND_VOLDIV( p_n ) ( ( p_n ) << 8 )

        s32 totalvol = CONV_VOL( ADSR_MASTER_VOLUME );
        totalvol += CONV_VOL( chstat->m_vol );
        totalvol += CONV_VOL( chstat->m_expr );
        totalvol += CONV_VOL( chstat->m_vel );
        totalvol += chstat->m_ampl >> 7;
        if( modType == 1 ) {
            totalvol += modParam;
            if( totalvol > 0 ) { totalvol = 0; }
        }
        totalvol += 723;
        if( totalvol < 0 ) { totalvol = 0; }

        u32 res = swiGetVolumeTable( totalvol );

        s32 pan = (s32) chstat->m_pan + (s32) chstat->m_pan2 - 64;
        if( modType == 2 ) { pan += modParam; }
        if( pan < 0 ) { pan = 0; }
        if( pan > 127 ) { pan = 127; }

        u32 cr = SCHANNEL_CR( p_channel )
                 & ~( SOUND_VOL( 0x7F ) | SOUND_VOLDIV( 3 ) | SOUND_PAN( 0x7F ) );
        cr |= SOUND_VOL( res ) | SOUND_PAN( pan );
        if( totalvol < ( -240 + 723 ) ) {
            cr |= SOUND_VOLDIV( 3 );
        } else if( totalvol < ( -120 + 723 ) ) {
            cr |= SOUND_VOLDIV( 2 );
        } else if( totalvol < ( -60 + 723 ) ) {
            cr |= SOUND_VOLDIV( 1 );
        }

        SCHANNEL_CR( p_channel ) = cr;
        u16 timer                = chstat->m_reg.m_timer;
        if( modType == 0 ) { timer = adjustFreq( timer, modParam ); }
        SCHANNEL_TIMER( p_channel ) = -timer;
    }

    // Adapted from VGMTrans

    u8 convertAttack( u8 p_attack ) {
        const u8 lookup[] = { 0x00, 0x01, 0x05, 0x0E, 0x1A, 0x26, 0x33, 0x3F, 0x49, 0x54,
                              0x5C, 0x64, 0x6D, 0x74, 0x7B, 0x7F, 0x84, 0x89, 0x8F };

        return ( p_attack >= 0x6D ) ? lookup[ 0x7F - p_attack ] : ( 0xFF - p_attack );
    }

    u16 convertFall( u8 p_fall ) {
        if( p_fall == 0x7F ) {
            return 0xFFFF;
        } else if( p_fall == 0x7E ) {
            return 0x3C00;
        } else if( p_fall < 0x32 ) {
            return ( ( p_fall << 1 ) + 1 ) & 0xFFFF;
        } else {
            return ( 0x1E00 / ( 0x7E - p_fall ) ) & 0xFFFF;
        }
    }

    u16 convertSustain( u8 p_sustain ) {
        const u16 lookup[] = {
            0xFD2D, 0xFD2E, 0xFD2F, 0xFD75, 0xFDA7, 0xFDCE, 0xFDEE, 0xFE09, 0xFE20, 0xFE34, 0xFE46,
            0xFE57, 0xFE66, 0xFE74, 0xFE81, 0xFE8D, 0xFE98, 0xFEA3, 0xFEAD, 0xFEB6, 0xFEBF, 0xFEC7,
            0xFECF, 0xFED7, 0xFEDF, 0xFEE6, 0xFEEC, 0xFEF3, 0xFEF9, 0xFEFF, 0xFF05, 0xFF0B, 0xFF11,
            0xFF16, 0xFF1B, 0xFF20, 0xFF25, 0xFF2A, 0xFF2E, 0xFF33, 0xFF37, 0xFF3C, 0xFF40, 0xFF44,
            0xFF48, 0xFF4C, 0xFF50, 0xFF53, 0xFF57, 0xFF5B, 0xFF5E, 0xFF62, 0xFF65, 0xFF68, 0xFF6B,
            0xFF6F, 0xFF72, 0xFF75, 0xFF78, 0xFF7B, 0xFF7E, 0xFF81, 0xFF83, 0xFF86, 0xFF89, 0xFF8C,
            0xFF8E, 0xFF91, 0xFF93, 0xFF96, 0xFF99, 0xFF9B, 0xFF9D, 0xFFA0, 0xFFA2, 0xFFA5, 0xFFA7,
            0xFFA9, 0xFFAB, 0xFFAE, 0xFFB0, 0xFFB2, 0xFFB4, 0xFFB6, 0xFFB8, 0xFFBA, 0xFFBC, 0xFFBE,
            0xFFC0, 0xFFC2, 0xFFC4, 0xFFC6, 0xFFC8, 0xFFCA, 0xFFCC, 0xFFCE, 0xFFCF, 0xFFD1, 0xFFD3,
            0xFFD5, 0xFFD6, 0xFFD8, 0xFFDA, 0xFFDC, 0xFFDD, 0xFFDF, 0xFFE1, 0xFFE2, 0xFFE4, 0xFFE5,
            0xFFE7, 0xFFE9, 0xFFEA, 0xFFEC, 0xFFED, 0xFFEF, 0xFFF0, 0xFFF2, 0xFFF3, 0xFFF5, 0xFFF6,
            0xFFF8, 0xFFF9, 0xFFFA, 0xFFFC, 0xFFFD, 0xFFFF, 0x0000 };

        return ( p_sustain == 0x7F ) ? 0 : -( ( 0x10000 - (s32) lookup[ p_sustain ] ) << 7 );
    }

    s8 getSoundSine( u8 p_arg ) {
        constexpr u8 lut_size = 32;
        const s8     lookup[]
            = { 0,  6,  12,  19,  25,  31,  37,  43,  49,  54,  60,  65,  71,  76,  81,  85, 90,
                94, 98, 102, 106, 109, 112, 115, 117, 120, 122, 123, 125, 126, 126, 127, 127 };

        if( p_arg < 1 * lut_size ) { return lookup[ p_arg ]; }
        if( p_arg < 2 * lut_size ) { return lookup[ 2 * lut_size - p_arg ]; }
        if( p_arg < 3 * lut_size ) { return -lookup[ p_arg - 2 * lut_size ]; }
        return -lookup[ 4 * lut_size - p_arg ];
    }

    void soundSysMessageHandler( int p_length, void* ) {
        soundSysMessage msg;
        fifoGetDatamsg( FIFO_SNDSYS, p_length, (u8*) &msg );

        switch( msg.m_message ) {
        case SNDSYS_PAUSESEQ: {
            if( SEQ_STATUS == STATUS_PLAYING ) {
                CUR_BPM            = SEQ_BPM;
                SEQ_BPM            = 0;
                CUR_VOL            = ADSR_MASTER_VOLUME;
                ADSR_MASTER_VOLUME = 0;
                SEQ_STATUS         = STATUS_PAUSED;
            } else if( SEQ_STATUS == STATUS_PAUSED ) {
                SEQ_BPM            = CUR_BPM;
                ADSR_MASTER_VOLUME = CUR_VOL;
                SEQ_STATUS         = STATUS_PLAYING;
            }
            return;
        }

        case SNDSYS_PLAYSEQ: {
            playSequence( &msg.m_seq, &msg.m_bnk, msg.m_war );
            SEQ_STATUS = STATUS_PLAYING;
            return;
        }

        case SNDSYS_FADESEQ: {
            SEQ_STATUS = STATUS_FADING;
            return;
        }

        case SNDSYS_STOPSEQ: {
            stopSequence( );
            SEQ_STATUS = STATUS_STOPPED;
            return;
        }
        }
    }
} // namespace SOUND::SSEQ

#endif
#endif
