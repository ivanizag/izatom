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
	isGraphic := (pa & 0x10) != 0 // pin A/G, from PA4

	if isGraphic {
		return mc.snapshotGraphic()
	} else {
		return mc.snapshotText()
	}
}

// Colors taken from MAME, only the first 4 used as CSS is not connected
var palette = [8]color.RGBA{
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
					lightColor := palette[1] // Yellow
					if inverse {
						lightColor = palette[3] // Red
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
	pa := mc.a.ppia.read(INS8255_PORT_A)
	graphicMode := ((pa >> 5) & 0x07) // pins GM0-1-2 from PA5-6-7

	size := image.Rect(0, 0, 256, 192)
	img := image.NewRGBA(size)

	var columns int
	var lines int
	var colorBits int
	switch graphicMode {
	case 0:
		// 64x64, 4 colors
		columns, lines, colorBits = 64, 64, 2
	case 1:
		// 128x64, 2 colors
		columns, lines, colorBits = 128, 64, 1
	case 2:
		// 128x64, 4 colors
		columns, lines, colorBits = 128, 64, 2
	case 3:
		// 128x96, 2 colors
		columns, lines, colorBits = 128, 96, 1
	case 4:
		// 128x96, 4 colors
		columns, lines, colorBits = 128, 96, 2
	case 5:
		// 128x192, 2 colors
		columns, lines, colorBits = 128, 192, 1
	case 6:
		// 128x192, 4 colors
		columns, lines, colorBits = 128, 192, 2
	case 7:
		// 256x192, 2 colors
		columns, lines, colorBits = 256, 192, 1
	}

	pixelWidth := 256 / columns
	pixelHeight := 192 / lines
	bytesPerLine := colorBits * columns / 8
	pixelsPerByte := 8 / colorBits

	pointer := uint16(0x8000)
	x := 0
	y := 0
	var color color.RGBA
	for l := 0; l < lines; l++ {
		x = 0
		for b := 0; b < bytesPerLine; b++ {
			data := mc.a.Peek(pointer)
			pointer++
			for pixel := 0; pixel < pixelsPerByte; pixel++ {
				if colorBits == 1 {
					if data&0x80 != 0 {
						color = textColorLight
					} else {
						color = textColorDark
					}
					data <<= 1
				} else if colorBits == 2 {
					colorIndex := (data >> 6) & 0x03
					color = palette[colorIndex]
					data <<= 2
				} else {
					panic("invalid colorBits")
				}
				for i := 0; i < pixelWidth; i++ {
					for j := 0; j < pixelHeight; j++ {
						img.Set(x, y+j, color)
					}
					x++
				}
			}
		}
		y += pixelHeight
	}
	return img
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
