#include <nds.h>
#include "font_data.h"

namespace font{
    u8 font_widths[NUM_CHARS] = {
        // 0-31
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    
        // 32-63
        7,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,5,2,5,5,5,5,5,5,5,5,0,0,0,0,0,0,
    
        // 64-95
        0,5,5,5,5,5,5,5,5,1,4,5,4,7,5,5,5,6,5,5,5,5,5,7,5,5,5,0,0,0,0,0,
    
        // 96-127
        0,5,5,5,5,5,5,5,5,1,4,5,4,7,5,5,5,6,5,5,5,5,5,7,5,5,5,4,0,0,0,0
    };

    u8 font_data[NUM_CHARS * FONT_WIDTH * FONT_HEIGHT] = {
        // 0
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
    
        // 1
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
    
        // 2
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
    
        // 3
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
    
        // 4
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
    
        // 5
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
    
        // 6
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
    
        // 7
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
    
        // 8
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
    
        // 9
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
    
        // 10
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
    
        // 11
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
    
        // 12
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
    
        // 13
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
    
        // 14
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
    
        // 15
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
    
        // 16
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
    
        // 17
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
    
        // 18
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
    
        // 19
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
    
        // 20
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
    
        // 21
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
    
        // 22
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
    
        // 23
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
    
        // 24
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
    
        // 25
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
    
        // 26
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
    
        // 27
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
    
        // 28
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
    
        // 29
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
    
        // 30
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
    
        // 31
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
    
        // 32
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
    
        // 33
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
    
        // 34
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
    
        // 35
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
    
        // 36
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
    
        // 37
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
    
        // 38
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
    
        // 39
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
    
        // 40
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
    
        // 41
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
    
        // 42
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
    
        // 43
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
    
        // 44
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
    
        // 45
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        1,1,1,1,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
    
        // 46
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
    
        // 47
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
    
        // 48
        0,1,1,1,0,0,0,
        1,0,0,0,1,0,0,
        1,0,0,0,1,0,0,
        1,0,0,0,1,0,0,
        0,1,1,1,0,0,0,
        
        // 49
        1,1,0,0,0,0,0,
        0,1,0,0,0,0,0,
        0,1,0,0,0,0,0,
        0,1,0,0,0,0,0,
        0,1,0,0,0,0,0,
        
        // 50
        1,1,1,1,0,0,0,
        0,0,0,0,1,0,0,
        0,1,1,1,1,0,0,
        1,0,0,0,0,0,0,
        1,1,1,1,1,0,0,
    
        // 51
        1,1,1,1,0,0,0,
        0,0,0,0,1,0,0,
        0,1,1,1,1,0,0,
        0,0,0,0,1,0,0,
        1,1,1,1,0,0,0,
    
        // 52
        1,0,0,0,1,0,0,
        1,0,0,0,1,0,0,
        1,1,1,1,1,0,0,
        0,0,0,0,1,0,0,
        0,0,0,0,1,0,0,
    
        // 53
        1,1,1,1,1,0,0,
        1,0,0,0,0,0,0,
        1,1,1,1,0,0,0,
        0,0,0,0,1,0,0,
        1,1,1,1,0,0,0,
    
        // 54
        0,1,1,1,0,0,0,
        1,0,0,0,0,0,0,
        1,1,1,1,0,0,0,
        1,0,0,0,1,0,0,
        0,1,1,1,0,0,0,
    
        // 55
        1,1,1,1,0,0,0,
        0,0,0,1,0,0,0,
        0,0,0,1,0,0,0,
        0,0,1,0,0,0,0,
        0,0,1,0,0,0,0,
    
        // 56
        0,1,1,1,0,0,0,
        1,0,0,0,1,0,0,
        0,1,1,1,0,0,0,
        1,0,0,0,1,0,0,
        0,1,1,1,0,0,0,
    
        // 57
        0,1,1,1,0,0,0,
        1,0,0,0,1,0,0,
        0,1,1,1,1,0,0,
        0,0,0,0,1,0,0,
        0,1,1,1,0,0,0,
    
        // 58
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
    
        // 59
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
    
        // 60
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
    
        // 61
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
    
        // 62
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
    
        // 63
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
    
        // 64
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
    
        // 65
        0,1,1,1,0,0,0,
        1,0,0,0,1,0,0,
        1,1,1,1,1,0,0,
        1,0,0,0,1,0,0,
        1,0,0,0,1,0,0,
    
        // 66
        1,1,1,1,0,0,0,
        1,0,0,0,1,0,0,
        1,1,1,1,0,0,0,
        1,0,0,0,1,0,0,
        1,1,1,1,0,0,0,
    
        // 67
        0,1,1,1,1,0,0,
        1,0,0,0,0,0,0,
        1,0,0,0,0,0,0,
        1,0,0,0,0,0,0,
        0,1,1,1,1,0,0,
    
        // 68
        1,1,1,1,0,0,0,
        1,0,0,0,1,0,0,
        1,0,0,0,1,0,0,
        1,0,0,0,1,0,0,
        1,1,1,1,0,0,0,
    
        // 69
        1,1,1,1,1,0,0,
        1,0,0,0,0,0,0,
        1,1,1,1,0,0,0,
        1,0,0,0,0,0,0,
        1,1,1,1,1,0,0,
    
        // 70
        1,1,1,1,1,0,0,
        1,0,0,0,0,0,0,
        1,1,1,1,0,0,0,
        1,0,0,0,0,0,0,
        1,0,0,0,0,0,0,
    
        // 71
        0,1,1,1,1,0,0,
        1,0,0,0,0,0,0,
        1,0,1,1,1,0,0,
        1,0,0,0,1,0,0,
        0,1,1,1,1,0,0,
    
        // 72
        1,0,0,0,1,0,0,
        1,0,0,0,1,0,0,
        1,1,1,1,1,0,0,
        1,0,0,0,1,0,0,
        1,0,0,0,1,0,0,
    
        // 73
        1,0,0,0,0,0,0,
        1,0,0,0,0,0,0,
        1,0,0,0,0,0,0,
        1,0,0,0,0,0,0,
        1,0,0,0,0,0,0,
    
        // 74
        0,0,0,1,0,0,0,
        0,0,0,1,0,0,0,
        0,0,0,1,0,0,0,
        0,0,0,1,0,0,0,
        1,1,1,0,0,0,0,
    
        // 75
        1,0,0,0,1,0,0,
        1,0,0,1,0,0,0,
        1,1,1,0,0,0,0,
        1,0,0,1,0,0,0,
        1,0,0,0,1,0,0,
    
        // 76
        1,0,0,0,0,0,0,
        1,0,0,0,0,0,0,
        1,0,0,0,0,0,0,
        1,0,0,0,0,0,0,
        1,1,1,1,0,0,0,
    
        // 77
        1,1,1,0,1,1,0,
        1,0,0,1,0,0,1,
        1,0,0,1,0,0,1,
        1,0,0,1,0,0,1,
        1,0,0,1,0,0,1,
    
        // 78
        1,1,1,1,0,0,0,
        1,0,0,0,1,0,0,
        1,0,0,0,1,0,0,
        1,0,0,0,1,0,0,
        1,0,0,0,1,0,0,
    
        // 79
        0,1,1,1,0,0,0,
        1,0,0,0,1,0,0,
        1,0,0,0,1,0,0,
        1,0,0,0,1,0,0,
        0,1,1,1,0,0,0,
    
        // 80
        1,1,1,1,0,0,0,
        1,0,0,0,1,0,0,
        1,1,1,1,0,0,0,
        1,0,0,0,0,0,0,
        1,0,0,0,0,0,0,
    
        // 81
        0,1,1,1,0,0,0,
        1,0,0,0,1,0,0,
        1,0,0,0,1,0,0,
        1,0,0,0,1,0,0,
        0,1,1,1,0,1,0,
    
        // 82
        1,1,1,1,0,0,0,
        1,0,0,0,1,0,0,
        1,1,1,1,0,0,0,
        1,0,0,0,1,0,0,
        1,0,0,0,1,0,0,
    
        // 83
        0,1,1,1,1,0,0,
        1,0,0,0,0,0,0,
        0,1,1,1,0,0,0,
        0,0,0,0,1,0,0,
        1,1,1,1,0,0,0,
    
        // 84
        1,1,1,1,1,0,0,
        0,0,1,0,0,0,0,
        0,0,1,0,0,0,0,
        0,0,1,0,0,0,0,
        0,0,1,0,0,0,0,
    
        // 85
        1,0,0,0,1,0,0,
        1,0,0,0,1,0,0,
        1,0,0,0,1,0,0,
        1,0,0,0,1,0,0,
        0,1,1,1,0,0,0,
    
        // 86
        1,0,0,0,1,0,0,
        1,0,0,0,1,0,0,
        1,0,0,0,1,0,0,
        0,1,0,1,0,0,0,
        0,0,1,0,0,0,0,
    
        // 87
        1,0,0,1,0,0,1,
        1,0,0,1,0,0,1,
        1,0,0,1,0,0,1,
        1,0,0,1,0,0,1,
        0,1,1,0,1,1,0,
    
        // 88
        1,0,0,0,1,0,0,
        0,1,0,1,0,0,0,
        0,0,1,0,0,0,0,
        0,1,0,1,0,0,0,
        1,0,0,0,1,0,0,
    
        // 89
        1,0,0,0,1,0,0,
        0,1,0,1,0,0,0,
        0,0,1,0,0,0,0,
        0,0,1,0,0,0,0,
        0,0,1,0,0,0,0,
    
        // 90
        1,1,1,1,1,0,0,
        0,0,0,1,0,0,0,
        0,0,1,0,0,0,0,
        0,1,0,0,0,0,0,
        1,1,1,1,1,0,0,
    
        // 91
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
    
        // 92
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
    
        // 93
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
    
        // 94
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
    
        // 95
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
    
        // 96
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
    
        // 97
        0,1,1,1,0,0,0,
        1,0,0,0,1,0,0,
        1,1,1,1,1,0,0,
        1,0,0,0,1,0,0,
        1,0,0,0,1,0,0,
    
        // 98
        1,1,1,1,0,0,0,
        1,0,0,0,1,0,0,
        1,1,1,1,0,0,0,
        1,0,0,0,1,0,0,
        1,1,1,1,0,0,0,
    
        // 99
        0,1,1,1,1,0,0,
        1,0,0,0,0,0,0,
        1,0,0,0,0,0,0,
        1,0,0,0,0,0,0,
        0,1,1,1,1,0,0,
    
        // 100
        1,1,1,1,0,0,0,
        1,0,0,0,1,0,0,
        1,0,0,0,1,0,0,
        1,0,0,0,1,0,0,
        1,1,1,1,0,0,0,
    
        // 101
        1,1,1,1,1,0,0,
        1,0,0,0,0,0,0,
        1,1,1,1,0,0,0,
        1,0,0,0,0,0,0,
        1,1,1,1,1,0,0,
    
        // 102
        1,1,1,1,1,0,0,
        1,0,0,0,0,0,0,
        1,1,1,1,0,0,0,
        1,0,0,0,0,0,0,
        1,0,0,0,0,0,0,
    
        // 103
        0,1,1,1,1,0,0,
        1,0,0,0,0,0,0,
        1,0,1,1,1,0,0,
        1,0,0,0,1,0,0,
        0,1,1,1,1,0,0,
    
        // 104
        1,0,0,0,1,0,0,
        1,0,0,0,1,0,0,
        1,1,1,1,1,0,0,
        1,0,0,0,1,0,0,
        1,0,0,0,1,0,0,
    
        // 105
        1,0,0,0,0,0,0,
        1,0,0,0,0,0,0,
        1,0,0,0,0,0,0,
        1,0,0,0,0,0,0,
        1,0,0,0,0,0,0,
    
        // 106
        0,0,0,1,0,0,0,
        0,0,0,1,0,0,0,
        0,0,0,1,0,0,0,
        0,0,0,1,0,0,0,
        1,1,1,0,0,0,0,
    
        // 107
        1,0,0,0,1,0,0,
        1,0,0,1,0,0,0,
        1,1,1,0,0,0,0,
        1,0,0,1,0,0,0,
        1,0,0,0,1,0,0,
    
        // 108
        1,0,0,0,0,0,0,
        1,0,0,0,0,0,0,
        1,0,0,0,0,0,0,
        1,0,0,0,0,0,0,
        1,1,1,1,0,0,0,
    
        // 109
        1,1,1,0,1,1,0,
        1,0,0,1,0,0,1,
        1,0,0,1,0,0,1,
        1,0,0,1,0,0,1,
        1,0,0,1,0,0,1,
    
        // 110
        1,1,1,1,0,0,0,
        1,0,0,0,1,0,0,
        1,0,0,0,1,0,0,
        1,0,0,0,1,0,0,
        1,0,0,0,1,0,0,
    
        // 111
        0,1,1,1,0,0,0,
        1,0,0,0,1,0,0,
        1,0,0,0,1,0,0,
        1,0,0,0,1,0,0,
        0,1,1,1,0,0,0,
    
        // 112
        1,1,1,1,0,0,0,
        1,0,0,0,1,0,0,
        1,1,1,1,0,0,0,
        1,0,0,0,0,0,0,
        1,0,0,0,0,0,0,
    
        // 113
        0,1,1,1,0,0,0,
        1,0,0,0,1,0,0,
        1,0,0,0,1,0,0,
        1,0,0,0,1,0,0,
        0,1,1,1,0,1,0,
    
        // 114
        1,1,1,1,0,0,0,
        1,0,0,0,1,0,0,
        1,1,1,1,0,0,0,
        1,0,0,0,1,0,0,
        1,0,0,0,1,0,0,
    
        // 115
        0,1,1,1,1,0,0,
        1,0,0,0,0,0,0,
        0,1,1,1,0,0,0,
        0,0,0,0,1,0,0,
        1,1,1,1,0,0,0,
    
        // 116
        1,1,1,1,1,0,0,
        0,0,1,0,0,0,0,
        0,0,1,0,0,0,0,
        0,0,1,0,0,0,0,
        0,0,1,0,0,0,0,
    
        // 117
        1,0,0,0,1,0,0,
        1,0,0,0,1,0,0,
        1,0,0,0,1,0,0,
        1,0,0,0,1,0,0,
        0,1,1,1,0,0,0,
    
        // 118
        1,0,0,0,1,0,0,
        1,0,0,0,1,0,0,
        1,0,0,0,1,0,0,
        0,1,0,1,0,0,0,
        0,0,1,0,0,0,0,
    
        // 119
        1,0,0,1,0,0,1,
        1,0,0,1,0,0,1,
        1,0,0,1,0,0,1,
        1,0,0,1,0,0,1,
        0,1,1,0,1,1,0,
    
        // 120
        1,0,0,0,1,0,0,
        0,1,0,1,0,0,0,
        0,0,1,0,0,0,0,
        0,1,0,1,0,0,0,
        1,0,0,0,1,0,0,
    
        // 121
        1,0,0,0,1,0,0,
        0,1,0,1,0,0,0,
        0,0,1,0,0,0,0,
        0,0,1,0,0,0,0,
        0,0,1,0,0,0,0,
    
        // 122
        1,1,1,1,1,0,0,
        0,0,0,1,0,0,0,
        0,0,1,0,0,0,0,
        0,1,0,0,0,0,0,
        1,1,1,1,1,0,0,
    
        // 123
        0,0,1,0,0,0,0,
        0,1,1,0,0,0,0,
        1,0,0,1,0,0,0,
        1,0,1,0,0,0,0,
        0,1,1,1,0,0,0,
    
        // 124
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
    
        // 125
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
    
        // 126
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
    
        // 127
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,
        0,0,0,0,0,0,0
    };
}