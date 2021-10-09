/*
 * Copyright 2021 QMK Community
 * Copyright 2021 Tyler Thrailkill (@snowe/@snowe2010) <tyler.b.thrailkill@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "quantum.h"
#include "luna.h"

// KEYBOARD PET START

// settings
#define BARK_FRAMES 3
#define SIT_FRAMES 2
#define WALK_FRAMES 4
#define RUN_FRAMES 2
#define SNEAK_FRAMES 4
#define MIN_WALK_SPEED 10
#define MIN_RUN_SPEED 40

// advanced settings
#define ANIM_FRAME_DURATION 375 // how long each frame lasts in ms
#define ANIM_SIZE 96 // number of bytes in array. If you change sprites, minimize for adequate firmware size. max is 1024

bool isSneaking = false;
bool isJumping = false;
bool showedJump = true;

// status variables
int current_wpm;
led_t led_usb_state = {
    .num_lock = false,
    .caps_lock = false,
    .scroll_lock = false
};

// current frame
uint8_t current_bark_frame = 0;
uint8_t current_sit_frame = 0;
uint8_t current_walk_frame = 0;
uint8_t current_run_frame = 0;
uint8_t current_sneak_frame = 0;

// timers
uint32_t anim_timer = 0;
uint32_t anim_sleep = 0;

// logic
void render_luna(int LUNA_X, int LUNA_Y) {

    // Sit
    static const char PROGMEM sit[SIT_FRAMES][ANIM_SIZE] = {
        // 'sit1', 32x22px
        {
            0x00, 0x00, 0x00, 0x00, 0x09, 0x0d, 0x4b, 0xc9, 0x40, 0x00, 0x60, 0x90, 0x90, 0x60, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0xc1, 0x30, 0x08, 0x04, 0x04, 0x02, 0x02, 0x02,
            0x01, 0x01, 0x06, 0x18, 0xe0, 0x90, 0x08, 0xb0, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x18, 0x20, 0x20, 0x50, 0x48, 0x40, 0x72,
            0x89, 0x88, 0x88, 0x44, 0x71, 0x40, 0x21, 0x1e, 0x21, 0x41, 0x46, 0x38, 0x00, 0x00, 0x00, 0x00
        },

        // 'sit2', 32x22px
        {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x48, 0x68, 0x58, 0x48, 0x00, 0x60, 0x90, 0x90, 0x60, 0x00, 0x00,
            0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x18, 0x04, 0x02, 0x02, 0x01, 0x01, 0x01,
            0x80, 0x00, 0x03, 0x0c, 0xf0, 0x88, 0x04, 0x98, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x0c, 0x10, 0x20, 0x28, 0x44, 0x40, 0x71,
            0x88, 0x88, 0x88, 0x44, 0x70, 0x40, 0x21, 0x1e, 0x21, 0x41, 0x46, 0x38, 0x00, 0x00, 0x00, 0x00
        }
    };

    // Walk
    static const char PROGMEM walk[WALK_FRAMES][ANIM_SIZE] = {
        // 'walk1', 32x24px
        {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x40, 0x20, 0x20, 0x10, 0x10, 0x10,
            0x10, 0x20, 0x20, 0x40, 0x58, 0xa4, 0x24, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3e, 0x41, 0x80, 0x9a, 0x1e, 0x00, 0x00, 0x00,
            0x1a, 0x9e, 0x80, 0x40, 0x7c, 0x83, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x12, 0x11, 0x1c, 0x22, 0x22, 0x22, 0x3c, 0x50,
            0x40, 0x4a, 0x3c, 0x04, 0x04, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        },

        // 'walk2', 32x24px
        {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x40, 0x40, 0x20, 0x20, 0x20,
            0x20, 0x40, 0x40, 0x80, 0x8c, 0x12, 0x12, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7c, 0x83, 0x00, 0x34, 0x3c, 0x00, 0x00, 0x00,
            0x34, 0x3c, 0x00, 0x80, 0x78, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x12, 0x72, 0x9d, 0x91, 0x80, 0xa0, 0x60, 0xa0,
            0x80, 0x91, 0x9d, 0x72, 0x12, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        },

        // 'walk3', 32x24px
        {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x40, 0x20, 0x20, 0x10, 0x10, 0x10,
            0x10, 0x20, 0x20, 0x40, 0x58, 0xa4, 0x24, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x7e, 0x41, 0x80, 0x9a, 0x1e, 0x00, 0x00, 0x00,
            0x1a, 0x9e, 0x80, 0x40, 0x3c, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x04, 0x04, 0x3c, 0x4a, 0x40, 0x50, 0x3c, 0x22,
            0x22, 0x22, 0x1c, 0x11, 0x12, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        },

        // 'walk4', 32x24px
        {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x40, 0x40, 0x20, 0x20, 0x20,
            0x20, 0x40, 0x40, 0x80, 0x8c, 0x12, 0x12, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7c, 0x83, 0x00, 0x34, 0x3c, 0x00, 0x00, 0x00,
            0x34, 0x3c, 0x00, 0x80, 0x78, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x12, 0x72, 0x9d, 0x91, 0x80, 0xa0, 0x60, 0xa0,
            0x80, 0x91, 0x9d, 0x72, 0x12, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        }
    };

    // Run
    static const char PROGMEM run[RUN_FRAMES][ANIM_SIZE] = {
        // 'run1', 32x24px
        {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x40, 0x20, 0x20, 0x10, 0x10, 0x10,
            0x10, 0x20, 0x20, 0x40, 0x58, 0xa4, 0x24, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xbe, 0x41, 0x80, 0x9a, 0x1e, 0x00, 0x00, 0x00,
            0x1a, 0x9e, 0x80, 0x40, 0xbc, 0x83, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x09, 0x09, 0x0c, 0x12, 0x14, 0x0c, 0x04, 0x0c,
            0x14, 0x12, 0x0c, 0x09, 0x09, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        },

        // 'run2', 32x24px
        {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x40, 0x40, 0x20, 0x20, 0x20,
            0x20, 0x40, 0x40, 0x80, 0x8c, 0x12, 0x12, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0xc0, 0x7c, 0x83, 0x00, 0x34, 0x3c, 0x00, 0x00, 0x00,
            0x34, 0x3c, 0x00, 0x80, 0x78, 0xc7, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x13, 0x12, 0x19, 0x25, 0x28, 0x18, 0x08, 0x18,
            0x28, 0x25, 0x19, 0x12, 0x13, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        }

    };

    // Bark
    static const char PROGMEM bark[BARK_FRAMES][ANIM_SIZE] = {
        // 'bark1', 32x24px
        {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x40,
            0x40, 0x40, 0x40, 0x40, 0x80, 0x80, 0x80, 0x30, 0x48, 0x48, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x7c, 0x83, 0x00, 0x10, 0x28,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x70, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x63, 0x94, 0x9e, 0x91, 0x80, 0x60,
            0x20, 0x62, 0x8c, 0x91, 0x91, 0x91, 0x6e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        },

        // 'bark2', 32x24px
        {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x40,
            0x40, 0x40, 0x40, 0x40, 0x80, 0x80, 0x80, 0x30, 0x48, 0x48, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x7c, 0x83, 0x00, 0x10, 0x28,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x70, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x63, 0x94, 0x9e, 0x91, 0x91, 0x71,
            0x2c, 0x62, 0x80, 0x93, 0x9d, 0x94, 0x68, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        },

        // 'bark3', 32x24px
        {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x40,
            0x40, 0x40, 0x40, 0x40, 0x80, 0x80, 0x80, 0x30, 0x48, 0x48, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x7c, 0xe3, 0x10, 0x10, 0x18,
            0xe0, 0x00, 0x00, 0x00, 0x00, 0x80, 0x70, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x63, 0x94, 0x9e, 0x91, 0x81, 0x62,
            0x20, 0x61, 0x80, 0x93, 0x9d, 0x94, 0x68, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        }
    };

    // Sneak
    static const char PROGMEM sneak[SNEAK_FRAMES][ANIM_SIZE] = {
        // 'sneak1', 32x24px
        {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x80, 0x40, 0x40, 0x20, 0x20, 0x20,
            0x20, 0x40, 0x40, 0x80, 0xb0, 0x48, 0x48, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x03, 0x7c, 0x83, 0x00, 0x34, 0x3c, 0x00, 0x00, 0x00,
            0x34, 0x3c, 0x80, 0x40, 0x78, 0x47, 0x80, 0x0c, 0xca, 0xa4, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x22, 0x25, 0x25, 0x22, 0x02, 0x22, 0xa2,
            0x82, 0x81, 0x83, 0x82, 0x42, 0x42, 0x21, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        },

        // 'sneak2', 32x24px
        {
            0x00, 0x00, 0x00, 0x00, 0x10, 0x28, 0x28, 0x30, 0x00, 0x00, 0x00, 0x80, 0x80, 0x40, 0x40, 0x40,
            0x40, 0x80, 0x80, 0x00, 0x18, 0x24, 0x24, 0x18, 0x00, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x06, 0x01, 0x68, 0x78, 0x00, 0x00, 0x00,
            0x68, 0x78, 0x00, 0x01, 0xf1, 0x0e, 0x00, 0x00, 0x03, 0x02, 0x02, 0x01, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x22, 0x25, 0x45, 0x45, 0x4a, 0x42, 0x0a, 0x2a, 0x2a, 0x2a,
            0x2a, 0x22, 0x22, 0x05, 0x25, 0x05, 0x42, 0x40, 0x20, 0x20, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00
        },

        // 'sneak3', 32x24px
        {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0xa0, 0xc0, 0x00, 0x80, 0x40, 0x40, 0x20, 0x20, 0x20,
            0x20, 0x40, 0x40, 0x80, 0xb0, 0x48, 0x48, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x08, 0x14, 0x98, 0x40, 0x7c, 0x43, 0x80, 0x34, 0x3c, 0x00, 0x00, 0x00,
            0x34, 0x3c, 0x00, 0x80, 0x78, 0x07, 0x00, 0x60, 0x50, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x21, 0x42, 0x42, 0x82, 0x83, 0x81, 0x82, 0xa2, 0x22, 0x02,
            0x26, 0x25, 0x23, 0x22, 0x11, 0x10, 0x10, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        },

        // 'sneak4', 32x24px
        {
            0x00, 0x00, 0x00, 0x00, 0x10, 0x28, 0x28, 0x30, 0x00, 0x00, 0x00, 0x80, 0x80, 0x40, 0x40, 0x40,
            0x40, 0x80, 0x80, 0x00, 0x18, 0x24, 0x24, 0x18, 0x00, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x06, 0x01, 0x68, 0x78, 0x00, 0x00, 0x00,
            0x68, 0x78, 0x00, 0x01, 0xf1, 0x0e, 0x00, 0x00, 0x03, 0x02, 0x02, 0x01, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x22, 0x25, 0x45, 0x45, 0x4a, 0x42, 0x0a, 0x2a, 0x2a, 0x2a,
            0x2a, 0x22, 0x22, 0x05, 0x25, 0x05, 0x42, 0x40, 0x20, 0x20, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00
        }
    };

    // animation
    void animation_phase(void) {

        // jump
        if (isJumping || !showedJump) {

            // clear
            oled_set_cursor(LUNA_X,LUNA_Y +2);
            oled_write("     ", false);

            oled_set_cursor(LUNA_X,LUNA_Y -1);

            showedJump = true;
        } else {

            // clear
            oled_set_cursor(LUNA_X,LUNA_Y -1);
            oled_write("     ", false);

            oled_set_cursor(LUNA_X,LUNA_Y);
        }

        // current status
        if(led_usb_state.caps_lock) {
            current_bark_frame = (current_bark_frame + 1) % BARK_FRAMES;
            oled_write_raw_P(bark[abs((BARK_FRAMES-1)-current_bark_frame)], ANIM_SIZE);

        } else if(isSneaking) {
            current_sneak_frame = (current_sneak_frame + 1) % SNEAK_FRAMES;
            oled_write_raw_P(sneak[abs((SNEAK_FRAMES-1)-current_sneak_frame)], ANIM_SIZE);

        } else if(current_wpm <= MIN_WALK_SPEED) {
            current_sit_frame = (current_sit_frame + 1) % SIT_FRAMES;
            oled_write_raw_P(sit[abs((SIT_FRAMES-1)-current_sit_frame)], ANIM_SIZE);

        } else if(current_wpm <= MIN_RUN_SPEED) {
            current_walk_frame = (current_walk_frame + 1) % WALK_FRAMES;
            oled_write_raw_P(walk[abs((WALK_FRAMES-1)-current_walk_frame)], ANIM_SIZE);

        } else {
            current_run_frame = (current_run_frame + 1) % RUN_FRAMES;
            oled_write_raw_P(run[abs((RUN_FRAMES-1)-current_run_frame)], ANIM_SIZE);
        }
    }

    // animation timer
    if(timer_elapsed32(anim_timer) > ANIM_FRAME_DURATION) {
        anim_timer = timer_read32();
        current_wpm = get_current_wpm();
        animation_phase();
    }

    // this fixes the screen on and off bug
    /*if (current_wpm > 0) {
        oled_on();
        anim_sleep = timer_read32();
    } else if(timer_elapsed32(anim_sleep) > OLED_TIMEOUT) {
        oled_off();
    }*/

}

// KEYBOARD PET END