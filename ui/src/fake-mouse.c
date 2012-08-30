/*
 * fake-mouse.c: Fake user input on a mouse.
 *
 * This program uses the XTest library to fake mouse movements. It draws
 * a smiley face 240 pixels wide centered at the position of your
 * choosing. Before running this program, you should lanch a drawing
 * program (e.g. Gimp), create an empty canvas and select a pen.
 *
 * Compile:
 *      gcc -Wall -Wextra fake-mouse.c -lXtst -lX11 -lm -o fake-mouse
 *
 * Usage:
 *      ./fake-mouse center_x center_y
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>

#define DEGREE (2*M_PI/360)  /* in radians */
#define MILLISECOND 1000     /* in microseconds */

void mouse_move(Display *display, int x, int y)
{
    XTestFakeMotionEvent(display, -1, x, y, CurrentTime);
    XFlush(display);
}

void button_press(Display *display)
{
    XTestFakeButtonEvent(display, 1, True, CurrentTime);
    XFlush(display);
}

void button_release(Display *display)
{
    XTestFakeButtonEvent(display, 1, False, CurrentTime);
    XFlush(display);
}

/*
 * Draw an arc centerred at (x0, y0), of radius r,
 * from angle "from" to angle "to".
 */
void draw_arc(Display *display, int x0, int y0, int r, int from, int to)
{
    int angle;

    mouse_move(display,
            x0 + r * cos(from * DEGREE),
            y0 + r * sin(from * DEGREE));
    usleep(100 * MILLISECOND);
    button_press(display);
    usleep(100 * MILLISECOND);
    for (angle = from + 5; angle < to; angle += 5) {
        mouse_move(display,
                x0 + r * cos(angle * DEGREE),
                y0 + r * sin(angle * DEGREE));
        usleep(10 * MILLISECOND);
    }
    mouse_move(display,
            x0 + r * cos(to * DEGREE),
            y0 + r * sin(to * DEGREE));
    usleep(10 * MILLISECOND);
    button_release(display);
    usleep(100 * MILLISECOND);
}

/*
 * Draw a smiley face.
 */
int main(int argc, char *argv[])
{
    int x0 = 690, y0 = 550;

    /* Override default position by cmd line parameters. */
    if (argc > 2) {
        x0 = atoi(argv[1]);
        y0 = atoi(argv[2]);
    }

    /* Open display. */
    Display *display = XOpenDisplay(NULL);
    if (!display) {
        fprintf(stderr, "Can't open display.\n");
        return EXIT_FAILURE;
    }

    /* Face. */
    draw_arc(display, x0, y0, 120, -90, 270);

    /* Eyes. */
    draw_arc(display, x0 - 40, y0 - 40, 12, -90, 270);
    draw_arc(display, x0 + 40, y0 - 40, 12, -90, 270);

    /* Smile. */
    draw_arc(display, x0, y0, 72, 30, 150);

    /* Done. */
    mouse_move(display, x0 + 120, y0 + 120);
    XCloseDisplay(display);
    return EXIT_SUCCESS;
}
