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

func (mc *mc6847) snapshotText() *image.RGBA {
	/* Two possible text modes per character:
	- ascii with internal chars
	- semigrahics6
	*/
	//isSemigraphics := false // TODO Pin A/S, from D6
	//int_ext := false // TODO, from D6
	//css := false // TODO, from PC3
	//inv := false // TODO, from D7

	size := image.Rect(0, 0, 256, 192)
	img := image.NewRGBA(size)

	/*
		chars are 8*12 pixels (2+5+1)*(3+7+2)
		32 rows, 16 lines
	*/

	// TODO: use the proper colors
	light := color.White
	dark := color.Black

	//	ch := uint8(0)
	for line := 0; line < 16; line++ {
		for charLine := 0; charLine < 12; charLine++ {
			for col := 0; col < 32; col++ {
				ch := mc.a.Peek(0x8000 + uint16(line*32+col))
				pixels := mc6847getFontLine(ch&0x3f, charLine)
				for charRow := 7; charRow >= 0; charRow-- {
					color := dark
					if pixels&1 != 0 {
						color = light
					}
					img.Set(col*8+charRow, line*12+charLine, color)
					pixels >>= 1
				}
			}
			//ch++
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
