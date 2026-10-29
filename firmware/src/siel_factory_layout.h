/* SPDX-License-Identifier: GPL-3.0-only */
/* DK600 v10 ROM-derived storage layout. Original bytes are NOT FM1 parameters.
 * E7D5..E7E0 copies C047..C034 (descending) into program storage;
 * F6B2..F6C4 sends each byte's low nibble then high nibble.
 * E54A/E557/E565/E58D mirror four digital registers to C044..C047.
 * E1B5..E1BA mirrors sixteen ADC slots to C034..C043.
 * SysEx packed bytes 0..3 are digital register mirrors; bytes 4..19
 * are analog slots 15..0 (reverse scan order).
 * Named analog controls and individual digital bit assignments remain unknown.
 */
#ifndef SIEL_FACTORY_LAYOUT_H
#define SIEL_FACTORY_LAYOUT_H
#include "siel_factory_bank.h"
#define SIEL_FACTORY_DIGITAL_REGS 4u
#define SIEL_FACTORY_ANALOG_SLOTS 16u
/* digital_index 0..3: C047, C046, C045, C044 respectively. */
static inline uint8_t siel_factory_digital(uint8_t program, uint8_t digital_index) {
    return siel_factory_packed[program][digital_index];
}
/* analog_slot 0..15: RAM C034..C043, in ascending hardware scan order. */
static inline uint8_t siel_factory_analog(uint8_t program, uint8_t analog_slot) {
    return siel_factory_packed[program][19u - analog_slot];
}
#endif /* SIEL_FACTORY_LAYOUT_H */
