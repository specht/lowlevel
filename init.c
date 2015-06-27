unsigned char scanline_0_b[321];
unsigned char scanline_1_b[321];
unsigned char buffer[64000];
float phi = 0.0;

extern void set_mode_0x13();

inline void outportb(unsigned int port, unsigned char value)
{
    asm volatile ("outb %%al,%%dx": :"d" (port), "a" (value));
}

inline unsigned char inportb(unsigned int port)
{
    unsigned char value;
    asm volatile ("inb %%dx": "=a" (value):"d" (port));
    return value;
}

void vsync()
{
    do { } while (inportb(0x3DA) & 8);
    do { } while (!(inportb(0x3DA) & 8));
}

inline float sin(float f)
{
    float result; __asm__ __volatile__ ("fld %1; fsin; fstp %0;" : "=m" (result) : "m" (f)); return result;
}

inline float cos(float f)
{
    float result; __asm__ __volatile__ ("fld %1; fcos; fstp %0;" : "=m" (result) : "m" (f)); return result;
}

inline float fdiv(float a, float b)
{
    float result;
    __asm__ __volatile__ ("fld %2; fld %1; fdivp; fstp %0;" : "=m" (result) : "m" (a), "m" (b));
    return result;
}

void set_grayscale_palette()
{
    int i;
    outportb(0x03c6, 0xff);
    outportb(0x03c8, 0);
    for (i = 0; i < 64; i++)
    {
        outportb(0x03c9, i);
        outportb(0x03c9, i);
        outportb(0x03c9, i);
    }
}

void set_color(unsigned char i, unsigned char r, unsigned char g, unsigned char b)
{
    outportb(0x03c6, 0xff);
    outportb(0x03c8, i);
    outportb(0x03c9, r);
    outportb(0x03c9, g);
    outportb(0x03c9, b);
}

void set_rgb_palette()
{
    int r, g, b;
    outportb(0x03c6, 0xff);
    outportb(0x03c8, 0);
    for (r = 0; r < 8; ++r)
    {
        for (g = 0; g < 8; ++g)
        {
            for (b = 0; b < 4; ++b)
            {
                outportb(0x03c9, r * 63 / 7);
                outportb(0x03c9, g * 63 / 7);
                outportb(0x03c9, b * 63 / 3);
            }
        }
    }
}

int render(float x, float y)
{
    x -= 160.0;
    y -= 100.0;
    float sum = 0.0;
    int i;
    float rx, ry;
    float c, s;
    c = cos(i * 1.570796326795 + phi * (i + 1) / 2);
    s = sin(i * 1.570796326795 + phi * (i + 1) / 2);
    rx = fabs(x * c - y * s);
    ry = fabs(x * s + y * c);

    if (rx > 8.0 && rx < 10.0 && ry > 8.0 && ry < 10.0)
    {
        sum += 32.0;
    }
    if (sum > 255.0)
        sum = 255.0;

    return (int)sum;
}

void init(void)
{
    int rng = 1, i;
    set_mode_0x13();
//     set_grayscale_palette();
    for (i = 0; i <= 21; ++i)
    {
        set_color(i, i * 63 / 21, 0, 0);
        set_color(i + 21, 63, i * 63 / 21, 0);
        set_color(i + 42, 63, 63, i * 63 / 21);
    }
    for (i = 0; i < 16000; ++i)
        buffer[i] = 0;
    while (1)
    {
        int x, y;
        unsigned char* p = buffer;
        unsigned char* scanline_0 = (unsigned char*)scanline_0_b;
        unsigned char* scanline_1 = (unsigned char*)scanline_1_b;
        unsigned char* temp;
        unsigned int* source = buffer;
        unsigned int* target = 0xa0000;
        int i;

        for (i = 0; i < 60; i++)
        {
            int px, py, y, c;
            rng = (rng * 1103515245U + 12345U) & 0x7fffffffU;
            px = (rng >> 15) % 0xffff * 300 / 65535;
            rng = (rng * 1103515245U + 12345U) & 0x7fffffffU;
            c = 63 - ((rng >> 15) % 0xffff * 16 / 65535);
            py = 190;
            for (y = 0; y < 4; ++y)
                for (x = 0; x < 4; ++x)
                    buffer[(y + py) * 320 + x + px + 4] = c;
        }
        for (x = 5; x < 315; ++x)
            buffer[194 * 320 + x] = 32;

        for (i = 0; i < 2; ++i)
        {
            float x = cos((i * 5.0 * 3.14159265359f / 20.0 + phi) * 3.0) * 140.0 + 160;
            float y = sin((i * 5.0 * 3.14159265359f / 20.0 + phi) * 5.0) * 80.0 + 100;
            int sx = (int)x;
            int sy = (int)y;
            buffer[sy * 320 + sx] = 128;
            buffer[sy * 320 + sx + 1] = 64;
            buffer[sy * 320 + sx + 320] = 64;
            buffer[sy * 320 + sx + 321] = 192;
        }

        for (y = 0; y < 199; ++y)
        {
            for (x = 1; x < 319; ++x)
            {
                int p = y * 320 + x;
                int sum = buffer[p] + buffer[p + 319] + buffer[p + 320] + buffer[p + 321];
                rng = (rng * 1103515245U + 12345U) & 0x7fffffffU;
                if (((rng >> 15) % 0xffff) < 8192)
                {
                    sum -= 16;
                    if (sum < 0)
                        sum = 0;
                }
//                 sum <<= 2;

                // add noise
                rng = (rng * 1103515245U + 12345U) & 0x7fffffffU;
                sum += (rng >> 16) & 3;
                if (sum < 0)
                    sum = 0;
                if (sum > 255)
                    sum = 255;

                sum >>= 2;
                buffer[p] = sum;
            }
        }


        /*
        for (x = 0; x <= 320; ++x)
            scanline_0[x] = render(x, 0);

        for (y = 0; y < 200; y++)
        {
            for (x = 0; x <= 320; ++x)
                scanline_1[x] = render(x, y + 1);

            for (x = 0; x < 320; x++)
            {
                int a = scanline_0[x];
                int b = scanline_0[x + 1];
                int c = scanline_1[x];
                int d = scanline_1[x + 1];
                int m = (a + b + c + d) >> 2;

                int color = 0;
//                 if (abs(a - m) >= 16 || abs(b - m) >= 16 || abs(c - m) >= 16 || abs(d - m) >= 16)
//                 {
//                     int dx, dy;
//                     // recurse
//                     color = 0;
//                     for (dx = 0; dx < 2; dx++)
//                     {
//                         for (dy = 0; dy < 2; dy++)
//                         {
//                             color += render(x + dx * 0.5, y + dy * 0.5);
//                         }
//                     }
//                     color >>= 2;
//                 }
//                 else
//                     color = m;
                color = a;

                // add noise
                rng = (rng * 1103515245U + 12345U) & 0x7fffffffU;
                color += (rng >> 16) & 7;
                if (color < 0)
                    color = 0;
                if (color > 255)
                    color = 255;

                *(p++) = color >> 2;
            }

            // swap scanline pointers
            temp = scanline_0;
            scanline_0 = scanline_1;
            scanline_1 = temp;
        }
        */
        vsync();
        for (i = 0; i < 16000; ++i)
        {
            *(target) = *(source);
            target += 1;
            source += 1;
        }
        phi += 0.005;
    }
}
