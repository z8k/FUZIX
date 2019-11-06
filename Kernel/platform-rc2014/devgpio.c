/*
 *	Z80 PIO based GPIO ports on the Z80 PIO card
 */

#include <kernel.h>
#include <kdata.h>
#include <gpio.h>

/* Two 8bit individually bit direction controlled ports per PIO */
/* Optional console switches card at 0xFF */
static struct gpio pioinfo[5] = {
    {
        0xFF, 0x00, GPIO_BIT_CONFIG, 0x00, 0xFF, 0x00
    },
    {
        0xFF, 0x00, GPIO_BIT_CONFIG, 0x00, 0xFF, 0x00
    },
    {
        0xFF, 0x00, GPIO_BIT_CONFIG, 0x00, 0xFF, 0x00
    },
    {
        0xFF, 0x00, GPIO_BIT_CONFIG, 0x00, 0xFF, 0x00
    },
    {
        /* Console switches module lives at 0xFF read */
        0xFF, 0x00, 0x00, 0x00, 0xFF, 0x00
    }
};

#define PIO_BASE	0x68

/* Track the actual data port as it's easier to mix types with that
   approach */
static uint8_t portmap[] = {
    0x6A, 0x6B, 0x6E, 0x6F, 0xFF
};

int gpio_ioctl(uarg_t request, char *data)
{
    struct gpio *p;
    uint8_t port;
    static struct gpioreq gr;

    /* No autodetect so always report the possible 32 */
    if (request == GPIOC_COUNT)
        return 40;

    if (uget(data, &gr, sizeof(struct gpioreq)) == -1)
        return -1;

    if (gr.pin >= 40) {
        udata.u_error = ENODEV;
        return -1;
    }

    p = &pioinfo[gr.pin >> 3];
    port = portmap[gr.pin >> 3];

    switch(request) {
    case GPIOC_SETBYTE:
        p->wdata = gr.val;
        break;
    case GPIOC_SET:
        p->wdata |= gr.val;
        break;
    case GPIOC_CLR:
        p->wdata &= ~gr.val;
        break;
    case GPIOC_GETBYTE:
        return in(port) & ~p->wmask;
    case GPIOC_GETINFO:
        return uput(p, data, sizeof(struct gpio));
    case GPIOC_SETRW:
        /* This one differs per device. The console switches are fixed */
        if (port < 32) {
            p->wmask = gr.val;
            port += 2;
            out(port, 0xFF);
            out(port, ~p->wmask);
            return 0;
        }
        /* Fall through */
    default:
        udata.u_error = ENOTTY;
        return -1;
    }
    out(port, p->wdata);
    return 0;
}
