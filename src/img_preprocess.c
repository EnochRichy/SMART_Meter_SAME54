#include "img_preprocess.h"

static inline void rgb565_unpack(uint16_t pixel,
                                 uint8_t *r8, uint8_t *g8, uint8_t *b8)
{
    uint8_t r5 = (pixel >> 11) & 0x1F;
    uint8_t g6 = (pixel >>  5) & 0x3F;
    uint8_t b5 =  pixel        & 0x1F;
    *r8 = (uint8_t)((r5 << 3) | (r5 >> 2));
    *g8 = (uint8_t)((g6 << 2) | (g6 >> 4));
    *b8 = (uint8_t)((b5 << 3) | (b5 >> 2));
}

void rgb565_to_modelinput_vww(const uint8_t *src, signed char *dst,
                              int src_w, int src_h)
{
    const int dx_q8 = (src_w << 8) / MODEL_IN_W;
    const int dy_q8 = (src_h << 8) / MODEL_IN_H;

    for (int dst_y = 0; dst_y < MODEL_IN_H; dst_y++)
    {
        int sy_q8 = dst_y * dy_q8;
        int sy0   = sy_q8 >> 8;
        int fy    = sy_q8 & 0xFF;
        int sy1   = sy0 + 1;
        if (sy1 >= src_h) sy1 = src_h - 1;

        const uint8_t *row0 = &src[sy0 * src_w * 2];
        const uint8_t *row1 = &src[sy1 * src_w * 2];
        signed char *out_row = &dst[dst_y * MODEL_IN_W * MODEL_IN_C];

        const int wy1 = fy;
        const int wy0 = 256 - fy;

        for (int dst_x = 0; dst_x < MODEL_IN_W; dst_x++)
        {
            int sx_q8 = dst_x * dx_q8;
            int sx0   = sx_q8 >> 8;
            int fx    = sx_q8 & 0xFF;
            int sx1   = sx0 + 1;
            if (sx1 >= src_w) sx1 = src_w - 1;

            uint16_t p_tl = row0[sx0*2] | ((uint16_t)row0[sx0*2 + 1] << 8);
            uint16_t p_tr = row0[sx1*2] | ((uint16_t)row0[sx1*2 + 1] << 8);
            uint16_t p_bl = row1[sx0*2] | ((uint16_t)row1[sx0*2 + 1] << 8);
            uint16_t p_br = row1[sx1*2] | ((uint16_t)row1[sx1*2 + 1] << 8);

            uint8_t r_tl, g_tl, b_tl, r_tr, g_tr, b_tr;
            uint8_t r_bl, g_bl, b_bl, r_br, g_br, b_br;
            rgb565_unpack(p_tl, &r_tl, &g_tl, &b_tl);
            rgb565_unpack(p_tr, &r_tr, &g_tr, &b_tr);
            rgb565_unpack(p_bl, &r_bl, &g_bl, &b_bl);
            rgb565_unpack(p_br, &r_br, &g_br, &b_br);

            const int wx1 = fx;
            const int wx0 = 256 - fx;

            int r_top = r_tl * wx0 + r_tr * wx1;
            int g_top = g_tl * wx0 + g_tr * wx1;
            int b_top = b_tl * wx0 + b_tr * wx1;
            int r_bot = r_bl * wx0 + r_br * wx1;
            int g_bot = g_bl * wx0 + g_br * wx1;
            int b_bot = b_bl * wx0 + b_br * wx1;

            int r = (r_top * wy0 + r_bot * wy1) >> 16;
            int g = (g_top * wy0 + g_bot * wy1) >> 16;
            int b = (b_top * wy0 + b_bot * wy1) >> 16;

            out_row[dst_x*3 + 0] = (signed char)(r - 128);
            out_row[dst_x*3 + 1] = (signed char)(g - 128);
            out_row[dst_x*3 + 2] = (signed char)(b - 128);
        }
    }
}
