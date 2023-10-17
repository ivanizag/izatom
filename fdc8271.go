package izatom

import (
	"fmt"
	"os"
)

/*
See the FDC 8271 datasheet for more information.

0A00-0AFF 8271 floppy disk controller
	0A00     8271 Command/Status
	0A01     8271 Parameter/Result
	0A02     8271 Reset
	0A03     8271 do not use
	0A04     8271 Data

Disks are single side and single density
256 bytes per sector, 40 tracks, 92kb per disk


TODO / BUGS:
 . When DOS loads a file that span two tracks, only the sectors from
 the first track are loaded.
*/

type fdc8271 struct {
	a       *Atom
	command uint8
	status  uint8
	result  uint8
	param   uint8

	track       uint8
	sector      uint8
	sectorCount uint8

	index   int
	readEnd int

	data []uint8
}

func NewFDC8271(a *Atom) *fdc8271 {
	var fdc fdc8271
	fdc.a = a
	fdc.loadDisk()
	return &fdc
}

func (fdc *fdc8271) reset() {
	// TODO
}

func (fdc *fdc8271) startRead() {
	fdc.index = 256*10*int(fdc.track) + 256*int(fdc.sector)
	fdc.readEnd = fdc.index + 256*int(fdc.sectorCount)

	fmt.Printf("[FDC] Read data from %v to %v\n", fdc.index, fdc.readEnd)
}

func (fdc *fdc8271) write(port uint8, value uint8) {
	// Port is CS-A1-A0
	switch port {
	case 0:
		fdc.command = value & 0x3f
		fdc.param = 0
		drive := value >> 6
		switch fdc.command {
		case 0x13: // READ DATA
			fmt.Printf("[FDC] Read data drive %v\n", drive)
		case 0x29: // SEEK
			fmt.Printf("[FDC] Seek drive %v\n", drive)
		case 0x2c: // READ DRIVE STATUS
			fmt.Printf("[FDC] Read drive status %v\n", drive)
			fdc.result = 0x80 | 0x10 /*index*/ | 0x04 /* ready 0*/
			if fdc.track == 0 {
				fdc.result |= 0x02 /* track 0 */
			}
		case 0x35: // SPECIFY
			fmt.Printf("[FDC] Specify drive %v\n", drive)
		case 0x3a: // WRITE SPECIAL REGISTER
			fmt.Printf("[FDC] Write special register drive %v\n", drive)
		default:
			fmt.Printf("[FDC] Command: Drive %v, Opcode 0x%02x-%06b\n", drive, fdc.command, fdc.command)
		}
	case 1:
		fmt.Printf("[FDC] Parameter: 0x%02x-%08b\n", value, value)
		switch fdc.command {
		case 0x13: // READ DATA
			switch fdc.param {
			case 0:
				fdc.track = value
			case 1:
				fdc.sector = value
			case 2:
				fdc.sectorCount = value & 0x1f
				fmt.Printf("[FDC] Multirecord parameters: track %v, sector %v, count %v, record size %v\n",
					fdc.track, fdc.sector, fdc.sectorCount, 128*(1<<(value>>5)))
				fdc.status = 0x80 /* busy */ |
					0x08 /* interrupt request */ |
					0x04 /* Non DMA mode */
				fdc.a.raiseNMIDelayed(40)
				fdc.startRead()
			}
		case 0x29: // SEEK
			fdc.track = value
		}
		fdc.param++
	case 2:
		fdc.status = 0x00
		fmt.Printf("[FDC] Reset: %v\n", value)
	case 3:
		fmt.Printf("[FDC] Do not use: %v\n", value)
	default:
		fmt.Printf("[FDC] Write data at %v: %v\n", port, value)

	}

}

func (fdc *fdc8271) read(port uint8) uint8 {
	switch port {
	case 0:
		// fmt.Printf("[FDC] Status\n")
		return fdc.status
	case 1:
		fmt.Printf("[FDC] Result\n")
		return fdc.result
	case 2:
		fmt.Printf("[FDC] Reset Read (Illegal)\n")
	case 3:
		fmt.Printf("[FDC] Do not use\n")
	default:
		value := fdc.data[fdc.index]
		fdc.index++

		if fdc.index >= fdc.readEnd {
			fdc.status = 0x00 + 0x04 /* Non DMA mode */
			fmt.Printf("[FDC] Read data completed\n")
		} else {
			fdc.a.raiseNMIDelayed(40)
		}

		//fmt.Printf("[FDC] Read data at %v\n", port)
		return value
	}
	return 0
}

func (fdc *fdc8271) loadDisk( /*name string*/ ) {
	data, err := os.ReadFile("../disks/mode4_graphics.40t")
	if err != nil {
		panic(err)
	}

	fdc.data = data
}

/*
When issuing the command *DOS: the following sequence is used:
[FDC] Reset: 1
[FDC] Reset: 0

[FDC] Specify drive 0
[FDC] Parameter: 0x0d-00001101 - Initialization
[FDC] Parameter: 0x14-00010100 - Step rate 20ms
[FDC] Parameter: 0x05-00000101 - Head settling time 5ms
[FDC] Parameter: 0xca-11001010 - Index count to unload 12, Head load time 40ms

[FDC] Specify drive 0
[FDC] Parameter: 0x10-00010000 - Load bad track surface 0
[FDC] Parameter: 0xff-11111111 - Bad track 1 is ff
[FDC] Parameter: 0xff-11111111 - Bad track 2 is ff
[FDC] Parameter: 0x00-00000000 - Current track is 0

[FDC] Specify drive 0
[FDC] Parameter: 0x18-00011000 - Load bad track surface 0
[FDC] Parameter: 0xff-11111111 - Bad track 1 is ff
[FDC] Parameter: 0xff-11111111 - Bad track 2 is ff
[FDC] Parameter: 0x00-00000000 - Current track is 0

[FDC] Write special register drive 0
[FDC] Parameter: 0x17-00010111 - Mode register
[FDC] Parameter: 0xc1-11000001 - Double actuator, non-DMA

*/
