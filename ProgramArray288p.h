#include <stddef.h>
#include <stdint.h>

/* CHANGES SUMMARY
 *
 * Misc. data:
 * S0_41:  69 >   5 Display clock = 54MHz (instead of 27)
 * S0_44:   1 >  37 Enable "min output bypass" for R and G DACs
 * S0_45:   0 >   1 ... and B DAC
 * S0_48:  63 >   0 Disable all V(RGB)_ test output and input
 * S0_4F:   0 >  48 Enable H/V sync, enable CLKOUT
 *
 * Input format:
 * S1_1C:   4 > 111 Vertical blanking start position
 * S1_1E:   8 >   4 Vertical blanking stop position
 * S1_20:  24 >  56 Line signal start position
 * S1_22: 114 > 146 Line signal stop position
 * S1_24: 123 > 155 Horizontal blank start position
 * S1_26: 127 > 159 Horizontal blank stop position
 *
 * Deinterlacer:
 * S2_00:   2 > 255 Bypass all filters
 *
 * Video processor:
 * S3_00 >  34 Disable interlace, sync in free run mode
 * S3_01 >  96 HTotal = 863 (TODO increase this to 1024 but we need to change
 *             clocks and adjust all of the following)
 * S3_02 >  19 ^ v (extra bits for neighbor registers)
 * S3_03 > 167 VTotal = 624 (2*312) (TODO try 623 or 622 for ZX)
 * S3_04 >   8 HBlank start
 * S3_05 > 195 HBlank start/stop
 * S3_06 =  53 HBlank stop
 * S3_07 = 108 VBlank start = line 620
 * S3_08 =  10
 * S3_09 = 129 VBlank stop = line 16
 * S3_0A =  80 HSync start = 80
 * S3_0B =   0
 * S3_0C =   0 Hsync stop = 0
 * S3_0D =   2 VSync start = 2
 * S3_0E =   8
 * S3_0F = 128 VSync end = 0
 * S3_10 =  77 Horizontal blanking (hide border mess) at pixel 845
 * S3_11 = 115
 * S3_12 =   8 Horizontal blanking end at pixel 135
 * S3_13 = 108 Vertical blanking start at line 620
 * S3_14 = 106
 * S3_15 = 130 Vertical blanking end at line 42
 * S3_16 = 151 H.Scaling ratio = 0.8974609375 (can we use 1? depends on clock?)
 * S3_17 =  51
 * S3_18 =  64 VDS_VSCALE = 3 (makes no sense)
 * S3_19 =   2 Lock every 3 frames for VSD_PROC
 * S3_1A =   0 Frame lock disabled, free running (TODO change to avoid flicker?)
 * S3_1B,1C,1D,1E =  0 (frame select bits)
 * S3_1F =   0 Disable frame repeat
 * S3_20..23 = 0 Secondary frame size (allows to use different frame sizes)
 * S3_24 =  96 YUV filters stuff + write delays
 * S3_25 =   3 Line buffer write position
 * S3_26 =   0 Line buffer enabled, auto luminosity disabled
 * S3_27 = 207 Black level control
 * S3_28 =  38 More black level control
 * S3_29 =  32 ' ' '
 * S3_2A = 220 Black level ; U/V delay control
 * S3_2B =  17 UV step filter
 * S3_2C = 224 U Skin color correction
 * S3_2D =  47 V ' ' '
 * S3_2E =  32 Y low ' ' '
 * S3_2F = 240 Y high ' ' '
 * S3_30 =  64 Skin color range
 * S3_31 =  26 Skin color is bypassed, so all registers above not used
 * S3_32 to 34 = 0 SVM ???
 * S3_35 = 125 Y dynamic gain
 * S3_36 =  31 Ucos gain
 * S3_37 =  44 Vcos gain
 * S3_38 =   0 Usin gain
 * S3_39 =   0 Vsin gain
 * S3_3A =   0 Y offset
 * S3_3B =   0 U offset
 * S3_3C =   0 V offset
 * S3_3D =   0 Sync level Y offset
 * S3_3E = 144 Bypass dynamic range expansion, convert YUV to RGB
 * S3_3F =   0 amplitude of UV blanking interval
 * S3_40 =   3 bypass 2nd stage interpolators
 * S3_41 =   3 line buffer write position
 * S3_42 =   0 line buffer enabled
 * S3_43 =   0 tap5 low-pass filter
 * S3_44..4D = low/band/high pass filters control
 * S3_4E = 142 bypass vertical peaking filters
 * S3_4F =  30 gain control UV vertical high pass
 *
 * S3_50 to 6F
 48,   0,  56,   8,  36,  10,  11, 234,  26,   0,   0,  26,   0, 196,  63,   4,
  4, 155, 128,   9, 233, 239, 127,  64, 210,  13, 216, 223,  63,   0,   0,   0,
 *
 * PIP:
 * S3_70 to 7F
 *
 * Memory controller:
 * S4_00:  16 > 130 Enable initial cycle, end of reset, software idle control
 * S4_13: ??? >   1 Invert memory clock
 * S4_14: ??? >  50 16bit RAM, no read to write conversion, enable refresh
 * S4_15: ??? >   5 RAM FIFO setup
 *
 * Capture/Playback
 * S4_20 to 5F
 *
 * ADC
 * S5_0B: 255 > 127 ; B channel gain control (set same as R/G)
 * S5_11:  16 > 144 ; PLLAD control bit
 * S5_18:   5 > 133 ; Trigger PA_ADC_CNTRL
 * S5_19:   2 > 130 ; Trigger PA_PLLAD_CTRL
 *
 * Sync processor
 * S5_38:   8 >   4 ; "Set the coast will valid before verical sync (line number)"
 * S5_39:  20 >  15 ; "When line cnt reach this calue coast goes down"
 * S5_41:  52 >  39 ; SOG clamp start pos LSB
 * S5_42:   0 >   6 ; SOG clamp start pos MSB
 * S5_43:  70 > 126 ; SOG clamp stop pos LSB
 * S5_44:   0 >   6 ;                    MSB
 * S5_47:   0 >   5 ; SOG HS stop position
 * S5_49:   5 >   4 ; Retiming HS start position
 */

// BANK 0 ---------------------------------------------------------------------
static const uint8_t programArray288p0[] = {
// 40-5F: misc. data
124,   5,   0,   0,  37,   1,  95,   7,   0,   0,   0,   0,   0, 42,   0,  48,
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  0,   0,   0
};

// Bank 1 ---------------------------------------------------------------------
static const uint8_t programArray288p1[] = {
// 00-2F: input format
 96, 224, 100, 255, 255, 255, 255, 255, 255, 255, 255,  79, 134,   5,  89, 203,
 18,   0,  71,   0,  44,   3,  92,   0,  87,   3, 135,   0, 111,   2,   4,   0,
 56,   0, 146,   3, 155,   6, 159,   6,   4,   0,   0,   0,   0,   0,   0,   0,

// 30-5F: HD bypass
// No changes here, can this section be removed?
202,   0, 128,   0,  63,   0, 128,  44, 204,   0,   0,   0,   0,   1, 192,   0,
  0,   1, 192,   0,   0,   1, 192,   0,   0,   1, 192,   0,   0,   1, 192,   0,
  0,   1, 192,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,

// 60-8F: Mode detect
// No changes here, can this section be removed?
208,  34,  32,  39,  65,  62, 178, 154,  78, 214, 177, 142, 124,  99, 139, 118,
112,  98, 133, 105,  83,  72,  93, 148, 178,  70, 198, 238, 140,  98, 118, 156,
  0,   0,  53,   0,   0,  12, 202,   0,   0,   0,   0,   0,   0,   0,   0,   0
};

// Bank 2 ---------------------------------------------------------------------
static const uint8_t programArray288p2[] = {
// OO-3F: deinterlace
255,   3, 204,   0,   0,   0,   5,   5,   7,   0,  76,   4, 204, 152, 255,  73,
 33, 136, 142,   0,   0,   0, 124,  35, 214, 208,   0,  16,   0,   0,   0,  16,
 81,   2,   4,  15,   0,   0,  76,  12,   0,   0,   0,   0,   0,   0,   0,   0,
  0,   0,  52,   0, 136,  71,   3,  11,   4, 100,  11,   4, 143,   0,   0,   0
};

// Bank 3 ---------------------------------------------------------------------
static const uint8_t programArray288p3[] = {
// 00-6F: Video processor
// Note: not set by startup
 34,  96,  19, 167,   8, 195,  53, 108,  10, 129,  80,   0,   0,   2,   8, 128,
 77, 115,   8, 108, 106, 130, 151,  51,  64,   2,   0,   0,   0,   0,   0,   0,
  0,   0,   0,   0,  96,   3,   0, 207,  38,  32, 220,  17, 224,  47,  32, 240,
 64,  26,   0,   0,   0, 125,  31,  44,   0,   0,   0,   0,   0,   0, 144,   0,
  3,   3,   0,   0, 248,  31, 248,  31, 248,  30, 208,  32, 248,  10, 142,  30,
 48,   0,  56,   8,  36,  10,  11, 234,  26,   0,   0,  26,   0, 196,  63,   4,
  4, 155, 128,   9, 233, 239, 127,  64, 210,  13, 216, 223,  63,   0,   0,   0,

// 70-7F: PIP
// Note: not set by startup
  0,   8,   0, 180,   5,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0
};

// Bank 4 ---------------------------------------------------------------------
static const uint8_t programArray288p4[] = {
// 00-1F: memory
130,  48,   0,   0,  48,  17,  66,  48,   1, 148,  17, 127,   0, 116,   0,   6,
  0, 146,   1,   1, 150,   5,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,

// 20-5F: capture/playback + FIFOs
// Note: not set by startup
  0,  43,   3,  31, 255, 255, 207, 255, 255,  31,   0, 164,  30,   0, 128,   0,
  0,   0,   0,   8,   0,   0,  16, 180, 204, 179,   0,   2,   0,   4,   3,   0,
  4,   0, 105,   0, 255, 255,   7, 255, 255,   7,   0,  68,   0, 224,  40,  62,
192,   0,   0,   0, 104,   1, 192, 180, 204,  90, 204,  76,   0,   0,   0,   0,
};

// Bank 5 ---------------------------------------------------------------------
static const uint8_t programArray288p5[] = {
// 00-1F: ADC
216,   0,  87, 241,   0,   0,  63,  63,  63, 127, 127, 127,   0,   0,   0,   0,
  0, 144, 179, 198,   0,   0,  32, 206, 133, 130,   0,   0,   0,   0, 128,   4,

// 20-6F: Sync processor
208,  32,  15,   0,  64,   0,   5,   0,   0,   0,  15,   0,   0,   4,   0,   4,
  0,  47,   0,  40,   3,  21,   0,   4,   4,  15,  10,   0,   0,   0, 192,   3,
 11,  39,   6, 126,   6,   0, 192,   5, 192,   4, 192,  52, 192, 103, 192, 103,
192,   0, 192,   5, 192, 192,  33, 192,   5, 192,   1, 200,   6,   0,   0,   0,
  0,   0,   0,  15,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
};

struct bank {
	size_t offset;
	size_t size;
	const uint8_t* data;
};

const struct bank programArray288[] = {
	{0x40, sizeof(programArray288p0), programArray288p0 },
	{0x00, sizeof(programArray288p1), programArray288p1 },
	{0x00, sizeof(programArray288p2), programArray288p2 },
	{0x00, sizeof(programArray288p3), programArray288p3 },
	{0x00, sizeof(programArray288p4), programArray288p4 },
	{0x00, sizeof(programArray288p5), programArray288p5 }
};
