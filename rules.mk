BOOTLOADER = qmk-dfu

# Build Options
#   change to "no" to disable the options, or define them in the Makefile in
#   the appropriate keymap folder that will get included automatically
#
BOOTMAGIC_ENABLE = no      # Virtual DIP switch configuration
MOUSEKEY_ENABLE = no        # Mouse keys
EXTRAKEY_ENABLE = yes        # Audio control and System control
CONSOLE_ENABLE = no         # Console for debug
COMMAND_ENABLE = no         # Commands for debug and configuration
NKRO_ENABLE = yes            # Nkey Rollover - if this doesn't work, see here: https://github.com/tmk/tmk_keyboard/wiki/FAQ#nkro-doesnt-work
BACKLIGHT_ENABLE = no       # Enable keyboard backlight functionality
MIDI_ENABLE = no            # MIDI controls
AUDIO_ENABLE = no           # Audio output on port C6
UNICODE_ENABLE = no         # Unicode
BLUETOOTH_ENABLE = no       # Enable Bluetooth with the Adafruit EZ-Key HID
RGBLIGHT_ENABLE = no       # Enable WS2812 RGB underlight. 
SWAP_HANDS_ENABLE = no      # Enable one-hand typing
OLED_DRIVER_ENABLE= yes     # OLED display
WPM_ENABLE = yes
OLED_ENABLE = yes
OLED_DRIVER = SSD1306
COMBO_ENABLE = no
SPLIT_KEYBOARD = yes

# Do not enable SLEEP_LED_ENABLE. it uses the same timer as BACKLIGHT_ENABLE
SLEEP_LED_ENABLE = no    # Breathing sleep LED during USB suspend
VIA_ENABLE = yes
LTO_ENABLE = yes

OCEAN_DREAM_ENABLE = yes
LUNA_ENABLE = yes

ifeq ($(strip $(OLED_DRIVER_ENABLE)), yes)
    #... your code here...
    ifdef OCEAN_DREAM_ENABLE
        ifeq ($(strip $(OCEAN_DREAM_ENABLE)), yes)
            SRC += ocean_dream.c
            OPT_DEFS += -DOCEAN_DREAM_ENABLE
        endif
    endif
    ifndef OCEAN_DREAM_ENABLE
        SRC += ocean_dream.c
        OPT_DEFS += -DOCEAN_DREAM_ENABLE
    endif

    ifdef LUNA_ENABLE
        ifeq ($(strip $(LUNA_ENABLE)), yes)
            SRC += luna.c
            OPT_DEFS += -DLUNA_ENABLE
        endif
    endif
    ifndef LUNA_ENABLE
        SRC += luna.c
        OPT_DEFS += -DLUNA_ENABLE
    endif
endif

EXTRAFLAGS        += -flto