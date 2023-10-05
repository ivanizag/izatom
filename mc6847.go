package izatom

import (
	"image"
	"image/color"
)

/*
See the Motorola MC6847 datasheet for more information.
*/

type mc6847 struct {
	a *Atom
}

func NewMC6847(a *Atom) *mc6847 {
	return &mc6847{a: a}
}
func (mc *mc6847) snapshot() *image.RGBA {
	pa := mc.a.ppia.read(INS8255_PORT_A)
	isGraphic := (pa & 0x10) == 1 // pin A/G, from PA4

	if isGraphic {
		return mc.snapshotGraphic()
	} else {
		return mc.snapshotText()
	}
}

// Colors taken from MAME, only the first 4 used as CSS is not connected
var semigrahics6Palette = [8]color.RGBA{
	{0x30, 0xd2, 0x00, 0xff}, /* GREEN */
	{0xc1, 0xe5, 0x00, 0xff}, /* YELLOW */
	{0x4c, 0x3a, 0xb4, 0xff}, /* BLUE */
	{0x9a, 0x32, 0x36, 0xff}, /* RED */
	{0xbf, 0xc8, 0xad, 0xff}, /* BUFF */
	{0x41, 0xaf, 0x71, 0xff}, /* CYAN */
	{0xc8, 0x4e, 0xf0, 0xff}, /* MAGENTA */
	{0xd4, 0x7f, 0x00, 0xff}, /* ORANGE */
}

var textColorLight = color.RGBA{0x30, 0xd2, 0x00, 0xff}
var textColorDark = color.RGBA{0x00, 0x7c, 0x00, 0xff}

//	rgb_t(0x26, 0x30, 0x16), /* BLACK */
//	rgb_t(0x30, 0xd2, 0x00), /* GREEN */
//	rgb_t(0x26, 0x30, 0x16), /* BLACK */
//	rgb_t(0xbf, 0xc8, 0xad), /* BUFF */
//
//	rgb_t(0x00, 0x7c, 0x00), /* ALPHANUMERIC DARK GREEN */
//	rgb_t(0x30, 0xd2, 0x00), /* ALPHANUMERIC BRIGHT GREEN */
//	rgb_t(0x6b, 0x27, 0x00), /* ALPHANUMERIC DARK ORANGE */
//	rgb_t(0xff, 0xb7, 0x00)  /* ALPHANUMERIC BRIGHT ORANGE */

func (mc *mc6847) snapshotText() *image.RGBA {
	/*
		Chars are 8*12 pixels (2+5+1)*(3+7+2)
		The screen is 32 rows, 16 lines

		Two possible text modes per character:
		- ascii with internal chars
		- semigrahics6
	*/

	size := image.Rect(0, 0, 256, 192)
	img := image.NewRGBA(size)

	//	ch := uint8(0)
	for line := 0; line < 16; line++ {
		for charLine := 0; charLine < 12; charLine++ {
			for col := 0; col < 32; col++ {
				ch := mc.a.Peek(0x8000 + uint16(line*32+col))
				inverse := ch&0x80 != 0      // Bit 7
				semigraphics := ch&0x40 != 0 // Bit 6
				if semigraphics {
					// Semigraphics
					// The chip supports 8 colors, but CSS is
					// always 0 and C0 is 1
					darkColor := textColorDark
					lightColor := semigrahics6Palette[1] // Yellow
					if inverse {
						lightColor = semigrahics6Palette[3] // Red
					}
					segment := (2 - (charLine / 4)) * 2
					// First half
					pixel := (ch>>(segment+1))&0x01 != 0
					color := darkColor
					if pixel {
						color = lightColor
					}
					for dotRow := 0; dotRow < 4; dotRow++ {
						img.Set(col*8+dotRow, line*12+charLine, color)
					}
					// Second half
					pixel = (ch>>segment)&0x01 != 0
					color = darkColor
					if pixel {
						color = lightColor
					}
					for dotRow := 4; dotRow < 8; dotRow++ {
						img.Set(col*8+dotRow, line*12+charLine, color)
					}
				} else {
					// Text
					pixels := mc6847getFontLine(ch&0x3f, charLine)
					for charRow := 7; charRow >= 0; charRow-- {
						color := textColorDark
						if (pixels&1 != 0) != inverse {
							color = textColorLight
						}
						img.Set(col*8+charRow, line*12+charLine, color)
						pixels >>= 1
					}
				}
			}
		}
	}

	return img
}

func (mc *mc6847) snapshotGraphic() *image.RGBA {
	return nil
	/*
	   pa := mc.a.ppia.read(INS8255_PORT_A)
	   graphicMode := ((pa >> 5) & 0x07) // pins GM0-1-2 from PA5-6-7

	   size := image.Rect(0, 0, 256, 192)
	   img := image.NewRGBA(size)

	   	for l := 0; l < grLines; l++ {
	   		for c := 0; c < columns; c++ {
	   			img.Set(c*pixelWidth+i, l*4+r, v)

	   		}
	   	}

	   return img
	*/
}

/*
There are 262 lines per 60Hz frame, 192 of which are visible
and 70 of which are blanking.
*/
const cpuCyclesPerFrame = 1_000_000 / 60 // 1Mhz / 60Hz
const cpuCyclesPerFramBlanking = cpuCyclesPerFrame * 70 / 262

// Field Sync, true during the blanking period.
func (mc *mc6847) fs() bool {
	return mc.a.cpu.GetCycles()%cpuCyclesPerFrame < cpuCyclesPerFramBlanking
}
