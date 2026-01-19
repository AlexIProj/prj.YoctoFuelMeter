#ifndef GPIO_DRIVER_H
#define GPIO_DRIVER_H

#include <gpiod.h>

struct gpiod_line_request* ConfigLineInput(const char* chip_path, int offset, const char* consumer, int detect_edge);

#endif