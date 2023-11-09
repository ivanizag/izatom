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
	a   *Atom
	log bool

	command  uint8
	status   uint8
	result   uint8
	register uint8
	param    uint8

	select0      bool
	select0delay uint8

	track       uint8
	sector      uint8
	sectorCount uint8

	index                int
	readEnd              int
	nextByte             uint8
	raiseNMIDelayedCycle uint64

	data []uint8
}

func NewFDC8271(a *Atom) *fdc8271 {
	var fdc fdc8271
	fdc.a = a
	return &fdc
}

func (fdc *fdc8271) logf(format string, a ...interface{}) {
	if fdc.log {
		fmt.Printf("[FDC] "+format, a...)
	}
}

func (fdc *fdc8271) tick(cycle uint64) {
	if fdc.select0delay > 0 {
		fdc.select0delay--
		if fdc.select0delay == 0 {
			fdc.logf("Drive 0 ready\n")
			fdc.select0 = true
		}
	}

	if fdc.raiseNMIDelayedCycle > 0 && cycle >= fdc.raiseNMIDelayedCycle {

		if fdc.index == fdc.readEnd {
			// We are done
			fdc.status = 0x00 /* not busy */ |
				0x10 /* Result full */ |
				0x08 /* Interrupt request */
			fdc.result = 0x00
		} else {
			fdc.nextByte = fdc.data[fdc.index]
			fdc.status = 0x80 /* busy */ |
				0x08 /* Interrupt request */ |
				0x04 /* Non DMA mode */
			fdc.index++
		}
		// fdc.logf("[FDC] Raise NMI 0x%02x %v\n", fdc.status, fdc.index)

		fdc.raiseNMIDelayedCycle = 0
		fdc.a.cpu.RaiseNMI()
	}
}

func (fdc *fdc8271) raiseNMIDelayed() {
	fdc.raiseNMIDelayedCycle = fdc.a.cpu.GetCycles() + 400
}

func (fdc *fdc8271) writeSpecialRegister(register uint8, value uint8) {
	switch register {
	case 0x17: // Mode register
		fdc.logf("Mode register 0x%02x: 0x%02x-%08b\n", register, value, value)
	case 0x23: // Drive control output port
		fdc.logf("Drive control output port register 0x%02x: 0x%02x-%08b\n", register, value, value)
		if (value & 0x40) != 0 {
			fdc.logf("Drive 0 selected\n")
			fdc.select0 = false
			fdc.select0delay = 200
		}
	default:
		fdc.logf("Unknown special register 0x%02x: 0x%02x-%08b\n", register, value, value)
	}
}

func (fdc *fdc8271) reset() {
	// TODO
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
			fdc.logf("Read data drive %v\n", drive)
		case 0x29: // SEEK
			fdc.logf("Seek drive %v\n", drive)
		case 0x2c: // READ DRIVE STATUS
			fdc.logf("Read drive status %v\n", drive)
			fdc.result = 0x80 | 0x10 /*index*/
			if fdc.select0 {
				fdc.result |= 0x04 /* ready 0 */
			}
			if fdc.track == 0 {
				fdc.result |= 0x02 /* track 0 */
			}
		case 0x35: // SPECIFY
			fdc.logf("Specify drive %v\n", drive)
		case 0x3a: // WRITE SPECIAL REGISTER
			fdc.logf("Write special register drive %v\n", drive)
		default:
			fdc.logf("Unknown command: Drive %v, Opcode 0x%02x-%06b\n", drive, fdc.command, fdc.command)
		}
	case 1:
		fdc.logf("Parameter: 0x%02x-%08b\n", value, value)
		switch fdc.command {
		case 0x13: // READ DATA
			switch fdc.param {
			case 0:
				fdc.track = value
			case 1:
				fdc.sector = value
			case 2:
				fdc.sectorCount = value & 0x1f
				fdc.logf("Multirecord parameters: track %v, sector %v, count %v, record size %v\n",
					fdc.track, fdc.sector, fdc.sectorCount, 128*(1<<(value>>5)))
				fdc.status = 0x80 /* busy */
				fdc.index = 256*10*int(fdc.track) + 256*int(fdc.sector)
				fdc.readEnd = fdc.index + 256*int(fdc.sectorCount)
				if fdc.readEnd > len(fdc.data) {
					fdc.readEnd = len(fdc.data)
					//panic("Read beyond end of disk")
				}

				fdc.logf("Read data from %v to %v\n", fdc.index, fdc.readEnd)
				fdc.raiseNMIDelayed()
			}
		case 0x29: // SEEK
			fdc.track = value
		case 0x3a: // WRITE SPECIAL REGISTER
			switch fdc.param {
			case 0:
				fdc.register = value
			case 1:
				fdc.writeSpecialRegister(fdc.register, value)
			}
		}
		fdc.param++
	case 2:
		fdc.status = 0x00
		fdc.logf("Reset: %v\n", value)
	case 3:
		fdc.logf("Do not use: %v\n", value)
	default:
		fdc.logf("Write data at %v: %v\n", port, value)

	}

}

func (fdc *fdc8271) read(port uint8) uint8 {
	switch port {
	case 0:
		//fdc.logf("Status: 0x%02x\n", fdc.status)
		return fdc.status
	case 1:
		fdc.logf("Result: 0x%02x\n", fdc.result)
		return fdc.result
	case 2:
		fdc.logf("Reset Read (Illegal)\n")
	case 3:
		fdc.logf("Do not use\n")
	default:
		if (fdc.status & 0x10) != 0 /* Result full */ {
			fdc.status = 0x00
		} else {
			fdc.status = 0x80 /* busy */
			fdc.raiseNMIDelayed()
		}

		//fdc.logf("Read data at %v\n", port)
		return fdc.nextByte
	}
	return 0
}

func (fdc *fdc8271) loadDisk(name string) {
	data, err := os.ReadFile(name)
	if err != nil {
		panic(err)
	}

	fdc.data = data
}

/*
When issuing the command *DOS, the following sequence is used:
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
