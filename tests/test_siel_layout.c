/* SPDX-License-Identifier: GPL-3.0-only */
#include <assert.h>
#include <stdint.h>
#include "../firmware/src/siel_factory_layout.h"
int main(void) {
    for (unsigned p = 0; p < SIEL_FACTORY_PROGRAMS; p++) {
        for (unsigned i = 0; i < SIEL_FACTORY_DIGITAL_REGS; i++)
            assert(siel_factory_digital((uint8_t)p, (uint8_t)i) == siel_factory_packed[p][i]);
        for (unsigned i = 0; i < SIEL_FACTORY_ANALOG_SLOTS; i++)
            assert(siel_factory_analog((uint8_t)p, (uint8_t)i) == siel_factory_packed[p][19u-i]);
        for (unsigned i = 0; i < SIEL_FACTORY_BYTES_PER_PROGRAM; i++) {
            uint8_t lo = siel_factory_nibble((uint8_t)p, (uint8_t)(i * 2u));
            uint8_t hi = siel_factory_nibble((uint8_t)p, (uint8_t)(i * 2u+1u));
            assert((uint8_t)(lo | (hi << 4)) == siel_factory_packed[p][i]);
        }
    }
    assert(siel_factory_digital(0, 0) == 0x20);
    assert(siel_factory_digital(0, 3) == 0x77);
    assert(siel_factory_analog(0, 15) == 0x1e);
    assert(siel_factory_analog(0, 0) == 0x00);
    return 0;
}
