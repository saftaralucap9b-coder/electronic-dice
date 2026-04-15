#ifndef BSP_H
#define BSP_H

// Board Selection Logic
#if defined(BOARD_NANO)
    #include "nano.h"
#else
    #error "No Board Defined! Please define BOARD_NANO ."
#endif

#endif // BSP_H