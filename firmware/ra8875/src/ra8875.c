#include "ra8875.h"

#define RA8875_BUSY_TIMEOUT_MS   100
#define RA8875_CLEAR_TIMEOUT_MS  1000

/*
 * Panel timing of ER-TFT050-6-6428 (5", 800x480). The values come from the
 * vendor's Arduino example (buydisplay.com, built on Adafruit_RA8875, BSD
 * license); Adafruit uses the same set for its 800x480 panels. Another panel
 * on the same controller only needs a different struct here.
 */
static const struct
{
    uint8_t pixclk;          /* PCSR value */
    uint8_t hsync_nondisp;   /* horizontal values in pixels */
    uint8_t hsync_start;
    uint8_t hsync_pw;
    uint8_t hsync_finetune;
    uint16_t vsync_nondisp;  /* vertical values in lines */
    uint16_t vsync_start;
    uint8_t vsync_pw;
} panel = {
    .pixclk = RA8875_PCSR_PDATL | RA8875_PCSR_2CLK,
    .hsync_nondisp = 26,
    .hsync_start = 32,
    .hsync_pw = 96,
    .hsync_finetune = 0,
    .vsync_nondisp = 32,
    .vsync_start = 23,
    .vsync_pw = 2,
};

static void write_reg16(uint8_t reg_low, uint16_t value)
{
    ra8875_write_reg(reg_low, (uint8_t)value);
    ra8875_write_reg(reg_low + 1, (uint8_t)(value >> 8));
}

static int wait_reg_clear(uint8_t reg, uint8_t mask, uint32_t timeout_ms)
{
    uint32_t start = HAL_GetTick();

    while (ra8875_read_reg(reg) & mask)
    {
        if (HAL_GetTick() - start > timeout_ms)
        {
            return -1;
        }
    }

    return 0;
}

static int wait_memory_idle(void)
{
    uint32_t start = HAL_GetTick();

    while (ra8875_port_read_status() & RA8875_STATUS_MEM_BUSY)
    {
        if (HAL_GetTick() - start > RA8875_BUSY_TIMEOUT_MS)
        {
            return -1;
        }
    }

    return 0;
}

/* reg_r is RA8875_FGCR0 or RA8875_BGCR0: R, G and B go to three registers */
static void set_color(uint8_t reg_r, uint16_t color)
{
    ra8875_write_reg(reg_r, (color >> 11) & 0x1F);
    ra8875_write_reg(reg_r + 1, (color >> 5) & 0x3F);
    ra8875_write_reg(reg_r + 2, color & 0x1F);
}

static int display_init(void)
{
    /* System clock: 20 MHz crystal * 11 / 4 = 55 MHz */
    ra8875_write_reg(RA8875_PLLC1, RA8875_PLLC1_PLLDIV1 + 10);
    HAL_Delay(1);
    ra8875_write_reg(RA8875_PLLC2, RA8875_PLLC2_DIV4);
    HAL_Delay(1);

    ra8875_write_reg(RA8875_SYSR, RA8875_SYSR_16BPP | RA8875_SYSR_MCU8);
    ra8875_write_reg(RA8875_PCSR, panel.pixclk);
    HAL_Delay(1);

    /* Register encodings as in the RA8875 datasheet */
    ra8875_write_reg(RA8875_HDWR, RA8875_WIDTH / 8 - 1);
    ra8875_write_reg(RA8875_HNDFTR, RA8875_HNDFTR_DE_HIGH | panel.hsync_finetune);
    ra8875_write_reg(RA8875_HNDR, (panel.hsync_nondisp - panel.hsync_finetune - 2) / 8);
    ra8875_write_reg(RA8875_HSTR, panel.hsync_start / 8 - 1);
    ra8875_write_reg(RA8875_HPWR, RA8875_HPWR_LOW | (panel.hsync_pw / 8 - 1));

    write_reg16(RA8875_VDHR0, RA8875_HEIGHT - 1);
    write_reg16(RA8875_VNDR0, panel.vsync_nondisp - 1);
    write_reg16(RA8875_VSTR0, panel.vsync_start - 1);
    ra8875_write_reg(RA8875_VPWR, RA8875_VPWR_LOW | (panel.vsync_pw - 1));

    /* Active window: the whole screen */
    write_reg16(RA8875_HSAW0, 0);
    write_reg16(RA8875_VSAW0, 0);
    write_reg16(RA8875_HEAW0, RA8875_WIDTH - 1);
    write_reg16(RA8875_VEAW0, RA8875_HEIGHT - 1);

    ra8875_write_reg(RA8875_MCLR, RA8875_MCLR_START | RA8875_MCLR_FULL);
    return wait_reg_clear(RA8875_MCLR, RA8875_MCLR_START, RA8875_CLEAR_TIMEOUT_MS);
}

int ra8875_init(void)
{
    if (ra8875_port_init() != 0)
    {
        return -1;
    }

    /* The module has its own RC reset: give it time to come out of reset */
    HAL_Delay(100);

    if (ra8875_read_reg(RA8875_REG_ID) != RA8875_CHIP_ID)
    {
        return -1;
    }

    if (display_init() != 0)
    {
        return -1;
    }

    ra8875_port_set_fast();
    ra8875_display_on(1);

    /* GPIOX enables the panel on Adafruit-style boards; the vendor example sets it too */
    ra8875_write_reg(RA8875_GPIOX, 1);
    ra8875_write_reg(RA8875_P1CR, RA8875_P1CR_ENABLE | RA8875_PWM_CLK_DIV1024);
    ra8875_backlight(255);

    return 0;
}

void ra8875_display_on(int on)
{
    ra8875_write_reg(RA8875_PWRR, RA8875_PWRR_NORMAL | (on ? RA8875_PWRR_DISPON : RA8875_PWRR_DISPOFF));
}

void ra8875_backlight(uint8_t level)
{
    ra8875_write_reg(RA8875_P1DCR, level);
}

/* Line, rectangle and triangle share the start/end point registers */
static void draw_shape(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1,
                       uint16_t color, uint8_t dcr)
{
    write_reg16(RA8875_DLHSR0, x0);
    write_reg16(RA8875_DLVSR0, y0);
    write_reg16(RA8875_DLHER0, x1);
    write_reg16(RA8875_DLVER0, y1);
    set_color(RA8875_FGCR0, color);

    ra8875_write_reg(RA8875_DCR, RA8875_DCR_LINESQUTRI_START | dcr);
    wait_reg_clear(RA8875_DCR, RA8875_DCR_LINESQUTRI_STATUS, RA8875_BUSY_TIMEOUT_MS);
}

void ra8875_fill_screen(uint16_t color)
{
    ra8875_fill_rect(0, 0, RA8875_WIDTH, RA8875_HEIGHT, color);
}

void ra8875_draw_line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color)
{
    draw_shape(x0, y0, x1, y1, color, RA8875_DCR_DRAWLINE);
}

void ra8875_draw_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    if (w == 0 || h == 0)
    {
        return;
    }
    draw_shape(x, y, x + w - 1, y + h - 1, color, RA8875_DCR_DRAWSQUARE);
}

void ra8875_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    if (w == 0 || h == 0)
    {
        return;
    }
    draw_shape(x, y, x + w - 1, y + h - 1, color, RA8875_DCR_DRAWSQUARE | RA8875_DCR_FILL);
}

static void triangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1,
                     uint16_t x2, uint16_t y2, uint16_t color, uint8_t fill)
{
    write_reg16(RA8875_DTPH0, x2);
    write_reg16(RA8875_DTPV0, y2);
    draw_shape(x0, y0, x1, y1, color, RA8875_DCR_DRAWTRIANGLE | fill);
}

void ra8875_draw_triangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1,
                          uint16_t x2, uint16_t y2, uint16_t color)
{
    triangle(x0, y0, x1, y1, x2, y2, color, 0);
}

void ra8875_fill_triangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1,
                          uint16_t x2, uint16_t y2, uint16_t color)
{
    triangle(x0, y0, x1, y1, x2, y2, color, RA8875_DCR_FILL);
}

static void circle(uint16_t x, uint16_t y, uint8_t r, uint16_t color, uint8_t fill)
{
    write_reg16(RA8875_DCHR0, x);
    write_reg16(RA8875_DCVR0, y);
    ra8875_write_reg(RA8875_DCRR, r);
    set_color(RA8875_FGCR0, color);

    ra8875_write_reg(RA8875_DCR, RA8875_DCR_CIRCLE_START | fill);
    wait_reg_clear(RA8875_DCR, RA8875_DCR_CIRCLE_STATUS, RA8875_BUSY_TIMEOUT_MS);
}

void ra8875_draw_circle(uint16_t x, uint16_t y, uint8_t r, uint16_t color)
{
    circle(x, y, r, color, 0);
}

void ra8875_fill_circle(uint16_t x, uint16_t y, uint8_t r, uint16_t color)
{
    circle(x, y, r, color, RA8875_DCR_FILL);
}

void ra8875_draw_text(uint16_t x, uint16_t y, const char *text,
                      uint16_t fg, uint16_t bg, uint8_t scale)
{
    uint8_t reg;

    if (scale > 3)
    {
        scale = 3;
    }

    ra8875_write_reg(RA8875_MWCR0, ra8875_read_reg(RA8875_MWCR0) | RA8875_MWCR0_TXTMODE);

    /* Internal CGROM font */
    reg = ra8875_read_reg(RA8875_FNCR0);
    ra8875_write_reg(RA8875_FNCR0, reg & ~(RA8875_FNCR0_CGRAM | RA8875_FNCR0_EXT_FONT));

    /* Opaque background, same enlargement horizontally and vertically */
    reg = ra8875_read_reg(RA8875_FNCR1);
    reg &= ~(RA8875_FNCR1_TRANSPARENT | RA8875_FNCR1_SCALE_MASK);
    ra8875_write_reg(RA8875_FNCR1, reg | (scale << 2) | scale);

    write_reg16(RA8875_F_CURX0, x);
    write_reg16(RA8875_F_CURY0, y);
    set_color(RA8875_FGCR0, fg);
    set_color(RA8875_BGCR0, bg);

    ra8875_port_write_cmd(RA8875_MRWC);
    while (*text)
    {
        ra8875_port_write_data((uint8_t)*text++);
        wait_memory_idle();
    }

    ra8875_write_reg(RA8875_MWCR0, ra8875_read_reg(RA8875_MWCR0) & ~RA8875_MWCR0_TXTMODE);
}

void ra8875_write_pixels(uint16_t x, uint16_t y, const uint16_t *pixels, uint32_t count)
{
    write_reg16(RA8875_CURH0, x);
    write_reg16(RA8875_CURV0, y);
    ra8875_port_write_cmd(RA8875_MRWC);
    ra8875_port_write_pixels(pixels, count);
}

void ra8875_write_reg(uint8_t reg, uint8_t value)
{
    ra8875_port_write_cmd(reg);
    ra8875_port_write_data(value);
}

uint8_t ra8875_read_reg(uint8_t reg)
{
    ra8875_port_write_cmd(reg);
    return ra8875_port_read_data();
}

uint8_t ra8875_read_status(void)
{
    return ra8875_port_read_status();
}
