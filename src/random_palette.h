#ifndef RANDOM_PALETTE_H
#define RANDOM_PALETTE_H


typedef enum PAL_FUNC
{
    PF_EX_ERR = -1,
    PF_EX_RG,
    PF_EX_GB,
    PF_EX_BR,
    PF_ROT_RGB,
    PF_INV_RGB,
    PF_INV_R,
    PF_INV_G,
    PF_INV_B

} pal_func;


typedef struct
{
    double r_strength;
    double g_strength;
    double b_strength;
    double r_bands;
    double g_bands;
    double b_bands;
    int offset;
    int stripe;
    int spread;

} random_palette;


typedef struct
{
    pal_func func;
    int offset;
    int stripe;
    int spread;

} function_palette;

#endif
