#ifndef _PS2_SCANCODE_SET_2_H_
#define _PS2_SCANCODE_SET_2_H_

#include "types.h"

typedef struct
{
    uint64_t f9:1;
    uint64_t a0:1;
    uint64_t f5:1;
    uint64_t f3:1;
    uint64_t f1:1;
    uint64_t f2:1;
    uint64_t f12:1;
    uint64_t a1:1;
    uint64_t f10:1;
    uint64_t f8:1;
    uint64_t f6:1;
    uint64_t f4:1;
    uint64_t tab:1;
    uint64_t backtick:1;
    uint64_t a2:1;
    uint64_t a3:1;
    uint64_t l_alt:1;
    uint64_t l_shift:1;
    uint64_t a4:1;
    uint64_t l_ctrl:1;
    uint64_t q:1;
    uint64_t _1_:1;
    uint64_t a5:1;
    uint64_t a6:1;
    uint64_t a7:1;
    uint64_t z:1;
    uint64_t s:1;
    uint64_t a:1;
    uint64_t w:1;
    uint64_t _2_:1;
    uint64_t a8:1;
    uint64_t a9:1;
    uint64_t c:1;
    uint64_t x:1;
    uint64_t d:1;
    uint64_t e:1;
    uint64_t _4_:1;
    uint64_t _3_:1;
    uint64_t a10:1;
    uint64_t a11:1;
    uint64_t space:1;
    uint64_t v:1;
    uint64_t f:1;
    uint64_t t:1;
    uint64_t r:1;
    uint64_t _5_:1;
    uint64_t a12:1;
    uint64_t a13:1;
    uint64_t n:1;
    uint64_t b:1;
    uint64_t h:1;
    uint64_t g:1;
    uint64_t y:1;
    uint64_t _6_:1;
    uint64_t a14:1;
    uint64_t a15:1;
    uint64_t a16:1;
    uint64_t m:1;
    uint64_t j:1;
    uint64_t u:1;
    uint64_t _7_:1;
    uint64_t _8_:1;
    uint64_t a17:1;
    uint64_t a18:1;
    uint64_t comma:1;
    uint64_t k:1;
    uint64_t i:1;
    uint64_t o:1;
    uint64_t _0_:1;
    uint64_t _9_:1;
    uint64_t a19:1;
    uint64_t a20:1;
    uint64_t dot:1;
    uint64_t for_slash:1;
    uint64_t l:1;
    uint64_t semi_colon:1;
    uint64_t p:1;
    uint64_t sub:1;
    uint64_t a21:1;
    uint64_t a22:1;
    uint64_t a23:1;
    uint64_t s_apostrophe:1;
    uint64_t a24:1;
    uint64_t o_sq_brace:1;
    uint64_t equal:1;
    uint64_t a25:1;
    uint64_t a26:1;
    uint64_t caps:1;
    uint64_t r_shift:1;
    uint64_t enter:1;
    uint64_t c_sq_brace:1;
    uint64_t a27:1;
    uint64_t bac_slash:1;
    uint64_t a28:1;
    uint64_t a29:1;
    uint64_t a30:1;
    uint64_t a31:1;
    uint64_t bspace:1;
    uint64_t a32:1;
    uint64_t a33:1;
    uint64_t k_1:1;
    uint64_t a34:1;
    uint64_t k_4:1;
    uint64_t k_7:1;
    uint64_t a340:1;
    uint64_t a35:1;
    uint64_t a36:1;
    uint64_t k_0:1;
    uint64_t k_dot:1;
    uint64_t k_2:1;
    uint64_t k_5:1;
    uint64_t k_6:1;
    uint64_t k_8:1;
    uint64_t esc:1;
    uint64_t num_lock:1;
    uint64_t f11:1;
    uint64_t k_add:1;
    uint64_t k_3:1;
    uint64_t k_sub:1;
    uint64_t k_mul:1;
    uint64_t k_9:1;
    uint64_t scroll_lock:1;
    uint64_t a37:1;
    uint64_t a38:1;
    uint64_t a39:1;
    uint64_t a40:1;
    uint64_t f7:1;	//0x83

    uint64_t a50;
    uint64_t a51;

    //E0, 00
    uint64_t a52:15;
    uint64_t search:1;
    uint64_t r_alt:1;
    uint64_t a53:1;
    uint64_t a54:1;
    uint64_t r_ctrl:1;
    uint64_t m_prev_track:1;
    uint64_t a55:1;
    uint64_t a56:1;
    uint64_t fav:1;
    uint64_t a57:1;
    uint64_t a58:1;
    uint64_t a59:1;
    uint64_t a60:1;
    uint64_t a61:1;
    uint64_t a62:1;
    uint64_t l_gui:1;
    uint64_t refresh:1;
    uint64_t vol_down:1;
    uint64_t a63:1;
    uint64_t m_mute:1;
    uint64_t a64:1;
    uint64_t a65:1;
    uint64_t a66:1;
    uint64_t r_gui:1;
    uint64_t stop:1;
    uint64_t a67:1;
    uint64_t a68:1;
    uint64_t m_calc:1;
    uint64_t a69:1;
    uint64_t a71:1;
    uint64_t a72:1;
    uint64_t apps:1;
    uint64_t m_W_fwd:1;
    uint64_t a73:1;
    uint64_t m_vol_up:1;
    uint64_t a730:1;
    uint64_t m_play_pause:1;
    uint64_t a74:1;
    uint64_t a75:1;
    uint64_t acpi_pwr:1;
    uint64_t m_w_back:1;
    uint64_t a76:1;
    uint64_t m_W_home:1;
    uint64_t m_stop:1;
    uint64_t a77:1;
    uint64_t a78:1;
    uint64_t a79:1;
    uint64_t acpi_slp:1;
    uint64_t a80:1;
    uint64_t a81:1;
    uint64_t a82:1;
    uint64_t a83:1;
    uint64_t email:1;
    uint64_t a84:1;
    uint64_t k_fwd_slash:1;
    uint64_t a85:1;
    uint64_t a86:1;
    uint64_t m_next:1;
    uint64_t a87:1;
    uint64_t a88:1;
    uint64_t m_media_select:1;
    uint64_t a89:1;
    uint64_t a90:1;
    uint64_t a91:1;
    uint64_t a92:1;
    uint64_t a93:1;
    uint64_t k_enter:1;
    uint64_t a94:1;
    uint64_t a95:1;
    uint64_t a96:1;
    uint64_t acpi_wake:1;
    uint64_t a97:1;
    uint64_t a98:1;
    uint64_t end:1;
    uint64_t a99:1;
    uint64_t cur_left:1;
    uint64_t home:1;
    uint64_t aa0:1;
    uint64_t aa1:1;
    uint64_t aa2:1;
    uint64_t insert:1;
    uint64_t del:1;
    uint64_t cur_down:1;
    uint64_t aa3:1;
    uint64_t cur_right:1;
    uint64_t cur_up:1;
    uint64_t aa4:1;
    uint64_t aa5:1;
    uint64_t aa6:1;
    uint64_t aa7:1;
    uint64_t pg_down:1;
    uint64_t aa8:1;
    uint64_t pause:1;
    uint64_t ab0;
    uint64_t ab1;
    uint64_t ab2;
    uint64_t ab3;
    uint64_t ab4:22;
} PS2_ScanCodes_2_;


#endif