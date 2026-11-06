/* SPDX-License-Identifier: GPL-3.0-only
 * VG-1 FM1: original diagnostic sound designs for reFX Vanguard 1 architecture.
 * These are not copied from reFX's factory sounds.
 * The firmware and offline audio renderer use exactly the SAME patch definitions.
 */
#ifndef VG1_FACTORY_H
#define VG1_FACTORY_H
#include "v1_core.h"
#define V1_PRESETS 8u
#define O(w,l,p,d,s,f) {w,l,p,d,s,f}
#define ADS(a,d,s,r) {a,d,s,r}
static const v1_patch_t V1_FACTORY[V1_PRESETS] = {
    { .name="HEAVEN LEAD",
      .vg_osc={.retrig=0,.fat=49,.vcs=6,.age=7}, .vg_lfo={{.speed=5,.detune=6},{.speed=5,.detune=-4},{.speed=3,.cutoff=4}},
      .vg_env1={.cutoff=4,.detune=2},.vg_env2={.cutoff=15},.vg_keytrack={.keytrk=25,.veltrk=0},
      .vg_amp={.drive=12,.level=105,.pan=64,.spread=75},
      .vg_gate={.left=0xffff,.right=0xffff,.mix=127,.contour=96,.speed={16,0}},
      .vg_delay={.on=1,.mix=43,.type=VG1_DELAY_STEREO,.feedback=45,.damp=23,.speed={8,VG1_RATE_DOTTED}},
      .vg_reverb={.on=1,.mix=22,.predelay=21,.room_size=66,.damp=38,.width=104},.bend_range=2,.vib_speed=8,.vib_depth=25,
      .osc={O(VG1_W_AJX_SAW3,123,64,80,0,0),O(VG1_W_SAWTOOTH,88,64,0,0,7),O(VG1_W_SAWTOOTH,75,64,0,-12,-3)},
      .amp=ADS(1,25,116,40),.filter_env=ADS(1,35,85,35),
      .cutoff=99,.resonance=18,.filter_type=V1_LP12,
      .master=91
    },
    { .name="FLOOR SHAKER",
      .vg_osc={.retrig=1,.fat=0,.vcs=2,.age=0},
      .vg_env1={.cutoff=4},.vg_env2={.cutoff=49},.vg_keytrack={.keytrk=28,.veltrk=12},
      .vg_amp={.drive=56,.level=105,.pan=64,.spread=0},
      .vg_gate={.left=0xffff,.right=0xffff,.mix=127,.contour=88,.speed={16,0}},
      .bend_range=2,.vib_speed=4,.vib_depth=3,
      .osc={O(VG1_W_SAWTOOTH,125,64,0,0,0),O(VG1_W_PULSE_PWM,90,58,0,0,0),O(VG1_W_SINE,106,64,0,-12,0)},
      .amp=ADS(0,18,91,15),.filter_env=ADS(0,18,0,15),
      .cutoff=42,.resonance=40,.filter_type=V1_LP24,
      .master=93
    },
    { .name="CRYSTAL PLUCK",
      .vg_osc={.retrig=1,.fat=13,.vcs=2,.age=1},
      .vg_env1={.cutoff=20},.vg_env2={.cutoff=54},.vg_keytrack={.keytrk=36,.veltrk=8},
      .vg_amp={.drive=4,.level=100,.pan=64,.spread=27},
      .vg_gate={.left=0xffff,.right=0xffff,.mix=127,.contour=88,.speed={16,0}},
      .vg_delay={.on=1,.mix=78,.type=VG1_DELAY_CROSS,.feedback=46,.damp=34,.speed={8,VG1_RATE_DOTTED}},
      .vg_reverb={.on=1,.mix=35,.predelay=18,.room_size=54,.damp=42,.width=110},.bend_range=2,.vib_speed=8,.vib_depth=6,
      .osc={O(VG1_W_PULSE_PWM,112,52,0,0,0),O(VG1_W_SAWTOOTH,75,64,0,0,7),O(VG1_W_SINE,85,64,0,12,0)},
      .amp=ADS(0,29,0,25),.filter_env=ADS(0,27,0,20),
      .cutoff=57,.resonance=35,.filter_type=V1_LP12,
      .master=97
    },
    { .name="GATED MOTION",
      .vg_osc={.retrig=0,.fat=47,.vcs=4,.age=12},
      .vg_lfo={{.speed=3,.detune=2},{.speed=4,.cutoff=7},{.speed=2,.pwm=12}},
      .vg_env1={.cutoff=10},.vg_env2={.cutoff=20},.vg_keytrack={.keytrk=23,.veltrk=0},
      .vg_amp={.drive=0,.level=107,.pan=64,.spread=73},
      .vg_gate={.enabled=1,.stereo=1,.left=0xbaba,.right=0xeaea,.mix=127,.contour=96,.speed={16,0}},
      .vg_delay={.on=1,.mix=42,.type=VG1_DELAY_CROSS,.feedback=43,.damp=45,.speed={8,0}},
      .vg_reverb={.on=1,.mix=52,.predelay=12,.room_size=88,.damp=60,.width=116},.bend_range=2,.vib_speed=5,.vib_depth=10,
      .osc={O(VG1_W_AJX_SAW3,70,64,65,0,0),O(VG1_W_PULSE_PWM,45,74,0,0,4),O(VG1_W_SAWTOOTH,41,64,0,-12,0)},
      .amp=ADS(28,65,110,70),.filter_env=ADS(22,56,70,68),
      .cutoff=63,.resonance=18,.filter_type=V1_LP12,
      .master=55
    },
    { .name="ACID RUNNER",
      .vg_osc={.retrig=1,.fat=0,.vcs=2,.age=0},
      .vg_env1={.cutoff=0},.vg_env2={.cutoff=45,.resonance=15},.vg_keytrack={.keytrk=16,.veltrk=15},
      .vg_amp={.drive=48,.level=115,.pan=64,.spread=0},
      .vg_gate={.left=0xffff,.right=0xffff,.mix=127,.contour=127,.speed={16,0}},
      .vg_delay={.on=1,.mix=16,.type=VG1_DELAY_MONO,.feedback=27,.damp=34,.speed={8,0}},
      .vg_reverb={.on=1,.mix=6,.predelay=0,.room_size=30,.damp=66,.width=30},.bend_range=2,.vib_speed=3,.vib_depth=0,
      .osc={O(VG1_W_SAWTOOTH,125,64,0,0,0),O(VG1_W_PULSE_PWM,72,26,0,0,0),O(VG1_W_OFF,0,64,0,0,0)},
      .amp=ADS(0,20,75,11),.filter_env=ADS(0,19,0,13),
      .cutoff=47,.resonance=96,.filter_type=V1_LP24,
      .master=96
    },
    { .name="SATURN PAD",
      .vg_osc={.retrig=0,.fat=39,.vcs=6,.age=24},
      .vg_lfo={{.speed=1,.detune=3},{.speed=2,.pwm=18,.cutoff=8},{.speed=1,.detune=-3}},
      .vg_env1={.cutoff=13,.detune=3},.vg_env2={.cutoff=22},.vg_keytrack={.keytrk=19,.veltrk=0},
      .vg_amp={.drive=0,.level=107,.pan=64,.spread=97},
      .vg_gate={.left=0xffff,.right=0xffff,.mix=127,.contour=48,.speed={16,0}},
      .vg_delay={.on=1,.mix=12,.type=VG1_DELAY_WIDEN,.feedback=26,.damp=65,.widen_ms=56,.speed={4,0}},
      .vg_reverb={.on=1,.mix=85,.predelay=37,.room_size=115,.damp=54,.width=127},.bend_range=2,.vib_speed=3,.vib_depth=23,
      .osc={O(VG1_W_AJX_SAW3,97,64,49,0,0),O(VG1_W_TRI_SAW_MORPH,74,64,0,0,0),O(VG1_W_SINE,47,64,0,-12,0)},
      .amp=ADS(73,89,102,100),.filter_env=ADS(85,90,78,80),
      .cutoff=54,.resonance=13,.filter_type=V1_LP12,
      .master=70
    },
    { .name="NIGHT ARPEG",
      .vg_osc={.retrig=1,.fat=19,.vcs=3,.age=4},
      .vg_lfo={{.speed=7,.detune=0},{.speed=12,.cutoff=10},{.speed=6,.pwm=5}},
      .vg_env1={.cutoff=6},.vg_env2={.cutoff=43},.vg_keytrack={.keytrk=24,.veltrk=10},
      .vg_amp={.drive=11,.level=110,.pan=64,.spread=50},
      .vg_gate={.left=0xffff,.right=0xffff,.mix=127,.contour=100,.speed={16,0}},
      .vg_delay={.on=1,.mix=66,.type=VG1_DELAY_CROSS,.feedback=53,.damp=45,.speed={8,VG1_RATE_DOTTED}},
      .vg_reverb={.on=1,.mix=34,.predelay=18,.room_size=58,.damp=43,.width=99},
      .vg_arp={.on=1,.mode=VG1_ARP_UP,.gate=67,.octaves=2,.speed={16,0},.retrig=1},
      .bend_range=2,.vib_speed=7,.vib_depth=15,
      .osc={O(VG1_W_SAWTOOTH,117,64,0,0,0),O(VG1_W_SINE,64,64,0,12,0),O(VG1_W_PULSE_PWM,82,48,0,0,7)},
      .amp=ADS(0,25,0,25),.filter_env=ADS(0,36,0,30),
      .cutoff=67,.resonance=32,.filter_type=V1_LP12,
      .master=72
    },
    { .name="LASER SHOT",
      .vg_osc={.retrig=1,.fat=9,.vcs=2,.age=0},
      .vg_env1={.cutoff=-28},.vg_env2={.cutoff=-35,.pitch=17},.vg_keytrack={.keytrk=0,.veltrk=0},
      .vg_amp={.drive=75,.level=99,.pan=64,.spread=17},
      .vg_gate={.left=0xffff,.right=0xffff,.mix=127,.contour=127,.speed={16,0}},
      .vg_delay={.on=1,.mix=36,.type=VG1_DELAY_MONO,.feedback=34,.damp=56,.speed={16,0}},
      .vg_reverb={.on=1,.mix=40,.predelay=25,.room_size=78,.damp=63,.width=84},
      .bend_range=12,.vib_speed=15,.vib_depth=60,
      .osc={O(VG1_W_PULSE_PWM,120,82,0,0,0),O(VG1_W_SAWTOOTH,90,64,0,12,0),O(VG1_W_WHITE_NOISE,35,64,0,0,0)},
      .amp=ADS(0,21,0,21),.filter_env=ADS(0,18,0,16),
      .cutoff=96,.resonance=49,.filter_type=V1_HP12,
      .master=88
    },
};
#undef O
#undef ADS


#endif
