#include <driver/rmt.h>

#ifndef IR_SETUP_H
#define IR_SETUP_H

#define IR_CHANNEL_NUM (RMT_CHANNEL_0)
#define IR_DI_IO (GPIO_NUM_26)

void init_ir();

#endif
