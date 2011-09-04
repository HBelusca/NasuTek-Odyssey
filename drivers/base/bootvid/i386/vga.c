#include "precomp.h"

/* GLOBALS *******************************************************************/

ULONG ScrollRegion[4] =
{
    0,
    0,
    640 - 1,
    480 - 1
};
UCHAR lMaskTable[8] =
{
    (1 << 8) - (1 << 0),
    (1 << 7) - (1 << 0),
    (1 << 6) - (1 << 0),
    (1 << 5) - (1 << 0),
    (1 << 4) - (1 << 0),
    (1 << 3) - (1 << 0),
    (1 << 2) - (1 << 0),
    (1 << 1) - (1 << 0)
};
UCHAR rMaskTable[8] =
{
    (1 << 7),
    (1 << 7)+ (1 << 6),
    (1 << 7)+ (1 << 6) + (1 << 5),
    (1 << 7)+ (1 << 6) + (1 << 5) + (1 << 4),
    (1 << 7)+ (1 << 6) + (1 << 5) + (1 << 4) + (1 << 3),
    (1 << 7)+ (1 << 6) + (1 << 5) + (1 << 4) + (1 << 3) + (1 << 2),
    (1 << 7)+ (1 << 6) + (1 << 5) + (1 << 4) + (1 << 3) + (1 << 2) + (1 << 1),
    (1 << 7)+ (1 << 6) + (1 << 5) + (1 << 4) + (1 << 3) + (1 << 2) + (1 << 1) +
    (1 << 0),
};
UCHAR PixelMask[8] =
{
    (1 << 7),
    (1 << 6),
    (1 << 5),
    (1 << 4),
    (1 << 3),
    (1 << 2),
    (1 << 1),
    (1 << 0),
};
ULONG lookup[16] =
{
    0x0000,
    0x0100,
    0x1000,
    0x1100,
    0x0001,
    0x0101,
    0x1001,
    0x1101,
    0x0010,
    0x0110,
    0x1010,
    0x1110,
    0x0011,
    0x0111,
    0x1011,
    0x1111,
};

ULONG TextColor = 0xF;
ULONG curr_x = 0;
ULONG curr_y = 0;
BOOLEAN CarriageReturn = FALSE;
ULONG_PTR VgaRegisterBase = 0;
ULONG_PTR VgaBase = 0;

/* PRIVATE FUNCTIONS *********************************************************/

VOID
NTAPI
ReadWriteMode(UCHAR Mode)
{
    UCHAR Value;

    /* Switch to graphics mode register */
    WRITE_PORT_UCHAR((PUCHAR)VgaRegisterBase + 0x3CE, 5);

    /* Get the current register value, minus the current mode */
    Value = READ_PORT_UCHAR((PUCHAR)VgaRegisterBase + 0x3CF) & 0xF4;

    /* Set the new mode */
    WRITE_PORT_UCHAR((PUCHAR)VgaRegisterBase + 0x3CF, Mode | Value);
}

VOID
NTAPI
__outpb(IN ULONG Port,
        IN ULONG Value)
{
    /* Write to the VGA Register */
    WRITE_PORT_UCHAR((PUCHAR)VgaRegisterBase + Port, (UCHAR)Value);
}

VOID
NTAPI
__outpw(IN ULONG Port,
        IN ULONG Value)
{
    /* Write to the VGA Register */
    WRITE_PORT_USHORT((PUSHORT)(VgaRegisterBase + Port), (USHORT)Value);
}

VOID
NTAPI
SetPixel(IN ULONG Left,
         IN ULONG Top,
         IN UCHAR Color)
{
    PUCHAR PixelPosition;

    /* Calculate the pixel position. */
    PixelPosition = (PUCHAR)VgaBase + (Left >> 3) + (Top * 80);

    /* Switch to mode 10 */
    ReadWriteMode(10);

    /* Clear the 4 planes (we're already in unchained mode here) */
    __outpw(0x3C4, 0xF02);

    /* Select the color don't care register */
    __outpw(0x3CE, 7);

    /* Select the bitmask register and write the mask */
    __outpw(0x3CE, (PixelMask[Left & 7] << 8) | 8);

    /* Read the current pixel value and add our color */
    WRITE_REGISTER_UCHAR(PixelPosition,
                         READ_REGISTER_UCHAR(PixelPosition) & Color);
}

VOID
NTAPI
DisplayCharacter(CHAR Character,
                 ULONG Left,
                 ULONG Top,
                 ULONG TextColor,
                 ULONG BackTextColor)
{
    PUCHAR FontChar;
    ULONG i, j, XOffset;

    /* Get the font line for this character */
    FontChar = &FontData[Character * BOOTCHAR_HEIGHT];

    /* Loop each pixel height */
    i = BOOTCHAR_HEIGHT;
    do
    {
        /* Loop each pixel width */
        j = 128;
        XOffset = Left;
        do
        {
            /* Check if we should draw this pixel */
#ifdef CHAR_GEN_UPSIDE_DOWN
            if (FontChar[i] & (UCHAR)j)
#else
            /* Normal character generator (top of char is first element) */
            if (FontChar[BOOTCHAR_HEIGHT - i] & (UCHAR)j)
#endif
            {
                /* We do, use the given Text Color */
                SetPixel(XOffset, Top, (UCHAR)TextColor);
            }
            else if (BackTextColor < 16)
            {
                /* This is a background pixel. */
                /* We're drawing it unless it's transparent. */
                SetPixel(XOffset, Top, (UCHAR)BackTextColor);
            }

            /* Increase X Offset */
            XOffset++;
        } while (j >>= 1);

        /* Move to the next Y ordinate */
        Top++;
    } while (--i);
}

VOID
NTAPI
DisplayStringXY(PUCHAR String,
                ULONG Left,
                ULONG Top,
                ULONG TextColor,
                ULONG BackColor)
{
    /* Loop every character */
    while (*String)
    {
        /* Display a character */
        DisplayCharacter(*String, Left, Top, TextColor, BackColor);

        /* Move to next character and next position */
        String++;
        Left += 8;
    }
}

VOID
NTAPI
SetPaletteEntryRGB(IN ULONG Id,
                   IN ULONG Rgb)
{
    PCHAR Colors = (PCHAR)&Rgb;

    /* Set the palette index */
    __outpb(0x3C8, (UCHAR)Id);

    /* Set RGB colors */
    __outpb(0x3C9, Colors[2] >> 2);
    __outpb(0x3C9, Colors[1] >> 2);
    __outpb(0x3C9, Colors[0] >> 2);
}

VOID
NTAPI
InitPaletteWithTable(IN PULONG Table,
                     IN ULONG Count)
{
    ULONG i;
    PULONG Entry = Table;

    /* Loop every entry */
    for (i = 0; i < Count; i++, Entry++)
    {
        /* Set the entry */
        SetPaletteEntryRGB(i, *Entry);
    }
}

VOID
NTAPI
SetPaletteEntry(IN ULONG Id,
                IN ULONG PaletteEntry)
{
    /* Set the palette index */
    __outpb(0x3C8, (UCHAR)Id);

    /* Set RGB colors */
    __outpb(0x3C9, PaletteEntry & 0xFF);
    __outpb(0x3C9, (PaletteEntry >>= 8) & 0xFF);
    __outpb(0x3C9, (PaletteEntry >> 8) & 0xFF);
}

VOID
NTAPI
InitializePalette(VOID)
{
    ULONG PaletteEntry[16] = {0,
                              0x20,
                              0x2000,
                              0x2020,
                              0x200000,
                              0x200020,
                              0x202000,
                              0x202020,
                              0x303030,
                              0x3F,
                              0x3F00,
                              0x3F3F,
                              0x3F0000,
                              0x3F003F,
                              0x3F3F00,
                              0x3F3F3F};
    ULONG i;

    /* Loop all the entries and set their palettes */
    for (i = 0; i < 16; i++) SetPaletteEntry(i, PaletteEntry[i]);
}

VOID
NTAPI
VgaScroll(ULONG Scroll)
{
    ULONG Top, RowSize, i;
    PUCHAR OldPosition, NewPosition;

    /* Clear the 4 planes */
    __outpw(0x3C4, 0xF02);

    /* Set the bitmask to 0xFF for all 4 planes */
    __outpw(0x3CE, 0xFF08);

    /* Set Mode 1 */
    ReadWriteMode(1);
    
    RowSize = (ScrollRegion[2] - ScrollRegion[0] + 1) / 8;

    /* Start loop */
    for(Top = ScrollRegion[1]; Top <= ScrollRegion[3]; ++Top)
    {
        /* Calculate the position in memory for the row */
        OldPosition = (PUCHAR)VgaBase + (Top + Scroll) * 80 + ScrollRegion[0] / 8;
        NewPosition = (PUCHAR)VgaBase + Top * 80 + ScrollRegion[0] / 8;
        
        /* Scroll the row */
        for(i = 0; i < RowSize; ++i)
            WRITE_REGISTER_UCHAR(NewPosition + i, READ_REGISTER_UCHAR(OldPosition + i));
    }
}

VOID
NTAPI
PreserveRow(IN ULONG CurrentTop,
            IN ULONG TopDelta,
            IN BOOLEAN Direction)
{
    PUCHAR Position1, Position2;
    ULONG Count;

    /* Clear the 4 planes */
    __outpw(0x3C4, 0xF02);

    /* Set the bitmask to 0xFF for all 4 planes */
    __outpw(0x3CE, 0xFF08);

    /* Set Mode 1 */
    ReadWriteMode(1);

    /* Check which way we're preserving */
    if (Direction)
    {
        /* Calculate the position in memory for the row */
        Position1 = (PUCHAR)VgaBase + CurrentTop * 80;
        Position2 = (PUCHAR)VgaBase + 0x9600;
    }
    else
    {
        /* Calculate the position in memory for the row */
        Position1 = (PUCHAR)VgaBase + 0x9600;
        Position2 = (PUCHAR)VgaBase + CurrentTop * 80;
    }

    /* Set the count and make sure it's above 0 */
    Count = TopDelta * 80;
    if (Count)
    {
        /* Loop every pixel */
        do
        {
            /* Write the data back on the other position */
            WRITE_REGISTER_UCHAR(Position1, READ_REGISTER_UCHAR(Position2));

            /* Increase both positions */
            Position2++;
            Position1++;
        } while (--Count);
    }
}

VOID
NTAPI
BitBlt(IN ULONG Left,
       IN ULONG Top,
       IN ULONG Width,
       IN ULONG Height,
       IN PUCHAR Buffer,
       IN ULONG BitsPerPixel,
       IN ULONG Delta)
{
    ULONG sx, dx, dy;
    UCHAR color;
    ULONG offset = 0;
    const ULONG Bottom = Top + Height;
    const ULONG Right = Left + Width;

    /* Check if the buffer isn't 4bpp */
    if (BitsPerPixel != 4)
    {
        /* FIXME: TODO */
        DbgPrint("Unhandled BitBlt\n"
                 "%lux%lu @ (%lu|%lu)\n"
                 "Bits Per Pixel %lu\n"
                 "Buffer: %p. Delta: %lu\n",
                 Width,
                 Height,
                 Left,
                 Top,
                 BitsPerPixel,
                 Buffer,
                 Delta);
        return;
    }

    /* 4bpp blitting */
    dy = Top;
    do
    {
        sx = 0;
        do
        {
            /* Extract color */
            color = Buffer[offset + sx];

            /* Calc destination x */
            dx = Left + (sx << 1);

            /* Set two pixels */
            SetPixel(dx, dy, color >> 4);
            SetPixel(dx + 1, dy, color & 0x0F);

            sx++;
        } while (dx < Right);
        offset += Delta;
        dy++;
    } while (dy < Bottom);
}

VOID
NTAPI
RleBitBlt(IN ULONG Left,
          IN ULONG Top,
          IN ULONG Width,
          IN ULONG Height,
          IN PUCHAR Buffer)
{
    ULONG YDelta;
    ULONG x;
    ULONG RleValue, NewRleValue;
    ULONG Color, Color2;
    ULONG i, j;
    ULONG Code;

    /* Set Y height and current X value and start loop */
    YDelta = Top + Height - 1;
    x = Left;
    for (;;)
    {
        /* Get the current value and advance in the buffer */
        RleValue = *Buffer;
        Buffer++;
        if (RleValue)
        {
            /* Check if we've gone past the edge */
            if ((x + RleValue) > (Width + Left))
            {
                /* Fixeup the pixel value */
                RleValue = Left - x + Width;
            }

            /* Get the new value */
            NewRleValue = *Buffer;

            /* Get the two colors */
            Color = NewRleValue >> 4;
            Color2 = NewRleValue & 0xF;

            /* Increase buffer positition */
            Buffer++;

            /* Check if we need to do a fill */
            if (Color == Color2)
            {
                /* Do a fill and continue the loop */
                RleValue += x;
                VidSolidColorFill(x, YDelta, RleValue - 1, YDelta, (UCHAR)Color);
                x = RleValue;
                continue;
            }

            /* Check if the pixel value is 1 or below */
            if (RleValue > 1)
            {
                /* Set loop variables */
                i = (RleValue - 2) / 2 + 1;
                do
                {
                    /* Set the pixels */
                    SetPixel(x, YDelta, (UCHAR)Color);
                    x++;
                    SetPixel(x, YDelta, (UCHAR)Color2);
                    x++;

                    /* Decrease pixel value */
                    RleValue -= 2;
                } while (--i);
            }

            /* Check if there is any value at all */
            if (RleValue)
            {
                /* Set the pixel and increase posititon */
                SetPixel(x, YDelta, (UCHAR)Color);
                x++;
            }

            /* Start over */
            continue;
        }

        /* Get the current pixel value */
        RleValue = *Buffer;
        Code = RleValue;
        switch (Code)
        {
            /* Case 0 */
            case 0:

                /* Set new x value, decrease distance and restart */
                x = Left;
                YDelta--;
                Buffer++;
                continue;

            /* Case 1 */
            case 1:

                /* Done */
                return;

            /* Case 2 */
            case 2:

                /* Set new x value, decrease distance and restart */
                Buffer++;
                x += *Buffer;
                Buffer++;
                YDelta -= *Buffer;
                Buffer++;
                continue;

            /* Other values */
            default:

                Buffer++;
                break;
        }

        /* Check if we've gone past the edge */
        if ((x + RleValue) > (Width + Left))
        {
            /* Set fixed up loop count */
            i = RleValue - Left - Width + x;

            /* Fixup pixel value */
            RleValue -= i;
        }
        else
        {
            /* Clear loop count */
            i = 0;
        }

        /* Check the value now */
        if (RleValue > 1)
        {
            /* Set loop variables */
            j = (RleValue - 2) / 2 + 1;
            do
            {
                /* Get the new value */
                NewRleValue = *Buffer;

                /* Get the two colors */
                Color = NewRleValue >> 4;
                Color2 = NewRleValue & 0xF;

                /* Increase buffer position */
                Buffer++;

                /* Set the pixels */
                SetPixel(x, YDelta, (UCHAR)Color);
                x++;
                SetPixel(x, YDelta, (UCHAR)Color2);
                x++;

                /* Decrease pixel value */
                RleValue -= 2;
            } while (--j);
        }

        /* Check if there is any value at all */
        if (RleValue)
        {
            /* Set the pixel and increase position */
            Color = *Buffer >> 4;
            Buffer++;
            SetPixel(x, YDelta, (UCHAR)Color);
            x++;
            i--;
        }

        /* Check loop count now */
        if ((LONG)i > 0)
        {
            /* Decrease it */
            i--;

            /* Set new position */
            Buffer = Buffer + (i / 2) + 1;
        }

        /* Check if we need to increase the buffer */
        if ((ULONG_PTR)Buffer & 1) Buffer++;
    }
}

/* PUBLIC FUNCTIONS **********************************************************/

/*
 * @implemented
 */
ULONG
NTAPI
VidSetTextColor(ULONG Color)
{
    ULONG OldColor;

    /* Save the old color and set the new one */
    OldColor = TextColor;
    TextColor = Color;
    return OldColor;
}

/*
 * @implemented
 */
VOID
NTAPI
VidDisplayStringXY(PUCHAR String,
                   ULONG Left,
                   ULONG Top,
                   BOOLEAN Transparent)
{
    ULONG BackColor;

    /* If the caller wanted transparent, then send the special value (16), else */
    /* use our default and call the helper routine. */
    BackColor = (Transparent) ? 16 : 14;
    DisplayStringXY(String, Left, Top, 12, BackColor);
}

/*
 * @implemented
 */
VOID
NTAPI
VidSetScrollRegion(ULONG x1,
                   ULONG y1,
                   ULONG x2,
                   ULONG y2)
{
    /* Assert alignment */
    ASSERT((x1 & 0x7) == 0);
    ASSERT((x2 & 0x7) == 7);

    /* Set Scroll Region */
    ScrollRegion[0] = x1;
    ScrollRegion[1] = y1;
    ScrollRegion[2] = x2;
    ScrollRegion[3] = y2;

    /* Set current X and Y */
    curr_x = x1;
    curr_y = y1;
}

/*
 * @implemented
 */
VOID
NTAPI
VidCleanUp(VOID)
{
    /* Select bit mask register */
    WRITE_PORT_UCHAR((PUCHAR)VgaRegisterBase + 0x3CE, 8);

    /* Clear it */
    WRITE_PORT_UCHAR((PUCHAR)VgaRegisterBase + 0x3CF, 255);
}

/*
 * @implemented
 */
VOID
NTAPI
VidBufferToScreenBlt(IN PUCHAR Buffer,
                     IN ULONG Left,
                     IN ULONG Top,
                     IN ULONG Width,
                     IN ULONG Height,
                     IN ULONG Delta)
{
    /* Make sure we have a width and height */
    if (!(Width) || !(Height)) return;

    /* Call the helper function */
    BitBlt(Left, Top, Width, Height, Buffer, 4, Delta);
}

/*
 * @implemented
 */
VOID
NTAPI
VidDisplayString(PUCHAR String)
{
    ULONG TopDelta = BOOTCHAR_HEIGHT + 1;

    /* Start looping the string */
    while (*String)
    {
        /* Treat new-line separately */
        if (*String == '\n')
        {
            /* Modify Y position */
            curr_y += TopDelta;
            if (curr_y + TopDelta >= ScrollRegion[3])
            {
                /* Scroll the view */
                VgaScroll(TopDelta);
                curr_y -= TopDelta;
            }
            else
            {
                /* Preserve row */
                PreserveRow(curr_y, TopDelta, FALSE);
            }

            /* Update current X */
            curr_x = ScrollRegion[0];

            /* Do not clear line if "\r\n" is given */
            CarriageReturn = FALSE;
        }
        else if (*String == '\r')
        {
            /* Update current X */
            curr_x = ScrollRegion[0];
            
            /* Check if we're being followed by a new line */
            CarriageReturn = TRUE;
        }
        else
        {
            /* check if we had a '\r' last time */
            if (CarriageReturn)
            {
                /* We did, clear the current row */
                PreserveRow(curr_y, TopDelta, TRUE);
                CarriageReturn = FALSE;
            }

            /* Display this character */
            DisplayCharacter(*String, curr_x, curr_y, TextColor, 16);
            curr_x += 8;

            /* Check if we should scroll */
            if (curr_x + 8 > ScrollRegion[2])
            {
                /* Update Y position and check if we should scroll it */
                curr_y += TopDelta;
                if (curr_y + TopDelta > ScrollRegion[3])
                {
                    /* Do the scroll */
                    VgaScroll(TopDelta);
                    curr_y -= TopDelta;
                }
                else
                {
                    /* Preserve the current row */
                    PreserveRow(curr_y, TopDelta, FALSE);
                }

                /* Update X */
                curr_x = ScrollRegion[0];
            }
        }

        /* Get the next character */
        String++;
    }
}

/*
 * @implemented
 */
VOID
NTAPI
VidBitBlt(PUCHAR Buffer,
          ULONG Left,
          ULONG Top)
{
    PBITMAPINFOHEADER BitmapInfoHeader;
    LONG Delta;
    PUCHAR BitmapOffset;

    /* Get the Bitmap Header */
    BitmapInfoHeader = (PBITMAPINFOHEADER)Buffer;

    /* Initialize the palette */
    InitPaletteWithTable((PULONG)(Buffer + BitmapInfoHeader->biSize),
                         (BitmapInfoHeader->biClrUsed) ?
                         BitmapInfoHeader->biClrUsed : 16);

    /* Make sure we can support this bitmap */
    ASSERT((BitmapInfoHeader->biBitCount * BitmapInfoHeader->biPlanes) <= 4);

    /* Calculate the delta and align it on 32-bytes, then calculate the actual */
    /* start of the bitmap data. */
    Delta = (BitmapInfoHeader->biBitCount * BitmapInfoHeader->biWidth) + 31;
    Delta >>= 3;
    Delta &= ~3;
    BitmapOffset = Buffer + sizeof(BITMAPINFOHEADER) + 16 * sizeof(ULONG);

    /* Check the compression of the bitmap */
    if (BitmapInfoHeader->biCompression == 2)
    {
        /* Make sure we have a width and a height */
        if ((BitmapInfoHeader->biWidth) && (BitmapInfoHeader->biHeight))
        {
            /* We can use RLE Bit Blt */
            RleBitBlt(Left,
                      Top,
                      BitmapInfoHeader->biWidth,
                      BitmapInfoHeader->biHeight,
                      BitmapOffset);
        }
    }
    else
    {
        /* Check if the height is negative */
        if (BitmapInfoHeader->biHeight < 0)
        {
            /* Make it positive in the header */
            BitmapInfoHeader->biHeight *= -1;
        }
        else
        {
            /* Update buffer offset */
            BitmapOffset += ((BitmapInfoHeader->biHeight -1) * Delta);
            Delta *= -1;
        }

        /* Make sure we have a width and a height */
        if ((BitmapInfoHeader->biWidth) && (BitmapInfoHeader->biHeight))
        {
            /* Do the BitBlt */
            BitBlt(Left,
                   Top,
                   BitmapInfoHeader->biWidth,
                   BitmapInfoHeader->biHeight,
                   BitmapOffset,
                   BitmapInfoHeader->biBitCount,
                   Delta);
        }
    }
}

/*
 * @implemented
 */
VOID
NTAPI
VidScreenToBufferBlt(PUCHAR Buffer,
                     ULONG Left,
                     ULONG Top,
                     ULONG Width,
                     ULONG Height,
                     ULONG Delta)
{
    ULONG Plane;
    ULONG XDistance;
    ULONG LeftDelta, RightDelta;
    ULONG PixelOffset;
    PUCHAR PixelPosition;
    PUCHAR k, i;
    PULONG m;
    UCHAR Value, Value2;
    UCHAR a;
    ULONG b;
    ULONG x, y;

    /* Calculate total distance to copy on X */
    XDistance = Left + Width - 1;

    /* Start at plane 0 */
    Plane = 0;

    /* Calculate the 8-byte left and right deltas */
    LeftDelta = Left & 7;
    RightDelta = 8 - LeftDelta;

    /* Clear the destination buffer */
    RtlZeroMemory(Buffer, Delta * Height);

    /* Calculate the pixel offset and convert the X distance into byte form */
    PixelOffset = Top * 80 + (Left >> 3);
    XDistance >>= 3;

    /* Loop the 4 planes */
    do
    {
        /* Set the current pixel position and reset buffer loop variable */
        PixelPosition = (PUCHAR)VgaBase + PixelOffset;
        i = Buffer;

        /* Set Mode 0 */
        ReadWriteMode(0);

        /* Set the current plane */
        __outpw(0x3CE, (Plane << 8) | 4);

        /* Make sure we have a height */
        if (Height > 0)
        {
            /* Start the outer Y loop */
            y = Height;
            do
            {
                /* Read the current value */
                m = (PULONG)i;
                Value = READ_REGISTER_UCHAR(PixelPosition);

                /* Set Pixel Position loop variable */
                k = PixelPosition + 1;

                /* Check if we're still within bounds */
                if (Left <= XDistance)
                {
                    /* Start X Inner loop */
                    x = (XDistance - Left) + 1;
                    do
                    {
                        /* Read the current value */
                        Value2 = READ_REGISTER_UCHAR(k);

                        /* Increase pixel position */
                        k++;

                        /* Do the blt */
                        a = Value2 >> (UCHAR)RightDelta;
                        a |= Value << (UCHAR)LeftDelta;
                        b = lookup[a & 0xF];
                        a >>= 4;
                        b <<= 16;
                        b |= lookup[a];

                        /* Save new value to buffer */
                        *m |= (b << Plane);

                        /* Move to next destination location */
                        m++;

                        /* Write new value */
                        Value = Value2;
                    } while (--x);
                }

                /* Update pixel position */
                PixelPosition += 80;
                i += Delta;
            } while (--y);
        }
   } while (++Plane < 4);
}

/*
 * @implemented
 */
VOID
NTAPI
VidSolidColorFill(IN ULONG Left,
                  IN ULONG Top,
                  IN ULONG Right,
                  IN ULONG Bottom,
                  IN UCHAR Color)
{
    ULONG rMask, lMask;
    ULONG LeftOffset, RightOffset, Distance;
    PUCHAR Offset;
    ULONG i, j;

    /* Get the left and right masks, shifts, and delta */
    LeftOffset = Left >> 3;
    lMask = (lMaskTable[Left & 0x7] << 8) | 8;
    RightOffset = Right >> 3;
    rMask = (rMaskTable[Right & 0x7] << 8) | 8;
    Distance = RightOffset - LeftOffset;

    /* If there is no distance, then combine the right and left masks */
    if (!Distance) lMask &= rMask;

    /* Switch to mode 10 */
    ReadWriteMode(10);

    /* Clear the 4 planes (we're already in unchained mode here) */
    __outpw(0x3C4, 0xF02);

    /* Select the color don't care register */
    __outpw(0x3CE, 7);

    /* Calculate pixel position for the read */
    Offset = VgaBase + (Top * 80) + (PUCHAR)(ULONG_PTR)LeftOffset;

    /* Select the bitmask register and write the mask */
    __outpw(0x3CE, (USHORT)lMask);

    /* Check if the top coord is below the bottom one */
    if (Top <= Bottom)
    {
        /* Start looping each line */
        i = (Bottom - Top) + 1;
        do
        {
            /* Read the previous value and add our color */
            WRITE_REGISTER_UCHAR(Offset, READ_REGISTER_UCHAR(Offset) & Color);

            /* Move to the next line */
            Offset += 80;
        } while (--i);
    }

    /* Check if we have a delta */
    if (Distance)
    {
        /* Calculate new pixel position */
        Offset = VgaBase + (Top * 80) + (PUCHAR)(ULONG_PTR)RightOffset;
        Distance--;

        /* Select the bitmask register and write the mask */
        __outpw(0x3CE, (USHORT)rMask);

        /* Check if the top coord is below the bottom one */
        if (Top <= Bottom)
        {
            /* Start looping each line */
            i = (Bottom - Top) + 1;
            do
            {
                /* Read the previous value and add our color */
                WRITE_REGISTER_UCHAR(Offset,
                                     READ_REGISTER_UCHAR(Offset) & Color);

                /* Move to the next line */
                Offset += 80;
            } while (--i);
        }

        /* Check if we still have a delta */
        if (Distance)
        {
            /* Calculate new pixel position */
            Offset = VgaBase + (Top * 80) + (PUCHAR)(ULONG_PTR)(LeftOffset + 1);

            /* Set the bitmask to 0xFF for all 4 planes */
            __outpw(0x3CE, 0xFF08);

            /* Check if the top coord is below the bottom one */
            if (Top <= Bottom)
            {
                /* Start looping each line */
                i = (Bottom - Top) + 1;
                do
                {
                    /* Loop the shift delta */
                    if (Distance > 0)
                    {
                        for (j = Distance; j; Offset++, j--)
                        {
                            /* Write the color */
                            WRITE_REGISTER_UCHAR(Offset, Color);
                        }
                    }

                    /* Update position in memory */
                    Offset += (80 - Distance);
                } while (--i);
            }
        }
    }
}

