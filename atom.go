package izatom

/*
Resources:
  http://www.acornatom.nl/atom_handleidingen/aw123/acorn_roms.htm

*/

import (
	"embed"
	"fmt"
	"image"
	"time"

	"github.com/ivanizag/iz6502"
)

const (
	romStart  = 0xa000
	ppiaStart = 0xb000
	viaStart  = 0xb800
)

type Atom struct {
	cpu      *iz6502.State
	vdu      *mc6847
	ppia     *ins8255
	fdc      *fdc8271
	keyboard *keyboard

	ram [romStart]uint8
	rom [0x10000 - romStart]uint8

	traceCPU bool
	traceIO  bool
}

func NewAtom() *Atom {
	var a Atom
	a.cpu = iz6502.NewNMOS6502(&a)
	a.vdu = NewMC6847(&a)
	a.ppia = NewINS8255(&a)
	a.fdc = NewFDC8271(&a)
	a.keyboard = newKeyboard()

	a.loadRom("akernel.rom", 0xf000)
	a.loadRom("dosrom.rom", 0xe000)
	a.loadRom("afloat.rom", 0xd000)
	a.loadRom("abasic.rom", 0xc000)
	a.loadRom("Demo.rom", 0xa000)

	//a.traceIO = true
	//a.traceCPU = true
	a.cpu.SetTrace(a.traceCPU)
	return &a
}

func (a *Atom) LoadDisk(path string) {
	a.fdc.loadDisk(path)
}

const (
	maxWaitDuration = 100 * time.Millisecond
	cpuSpinLoops    = 100
	cycleDurationNs = 1000 // 1 MHz
)

func (a *Atom) Run() {
	a.cpu.Reset()

	isDoingReset := false
	referenceTime := time.Now()

	for {
		// Keyboard
		a.keyboard.processKeys()
		a.fdc.tick(a.cpu.GetCycles())

		// Reset
		if a.keyboard.getBreak() {
			if !isDoingReset {
				a.cpu.Reset()
				a.ppia.reset()
				a.fdc.reset()
				isDoingReset = true
			}
		} else {
			isDoingReset = false
		}

		// Traces
		pc, _ := a.cpu.GetPCAndSP()
		if pc == 0xfe66 {
			// Skip tracing at FE66_wait_for_flyback_start
			a.cpu.SetTrace(false)
		} else if pc == 0xfe6b {
			// Skip tracing at FE6B_wait_for_flyback
			a.cpu.SetTrace(false)
		} else if pc == 0xfe70 {
			// Resume tracing after the flyback wait
			a.cpu.SetTrace(a.traceCPU)
		}
		//a.traceOS()

		// Trace DOS ROM
		//a.cpu.SetTrace((pc >= 0xe000 && pc <= 0xefff) || pc < 0x100)

		// CPU
		a.cpu.ExecuteInstruction()

		// Spped control
		if a.cpu.GetCycles()%cpuSpinLoops == 0 {
			clockDuration := time.Since(referenceTime)
			simulatedDuration := time.Duration(float64(a.cpu.GetCycles()) * cycleDurationNs)
			waitDuration := simulatedDuration - clockDuration
			if waitDuration > maxWaitDuration || -waitDuration > maxWaitDuration {
				// We have to wait too long or are too much behind. Let's fast forward
				referenceTime = referenceTime.Add(-waitDuration)
				waitDuration = 0
			}
			if waitDuration > 0 {
				time.Sleep(waitDuration)
			}
		}

	}
}

// log
func (a *Atom) logf(format string, args ...interface{}) {
	if a.traceIO {
		fmt.Printf(format, args...)
	}
}

//go:embed resources
var resources embed.FS

func (a *Atom) loadRom(name string, address uint16) {
	f, err := resources.Open("resources/" + name)
	if err != nil {
		panic(err) // Should never happen
	}

	_, err = f.Read(a.rom[address-romStart:])
	if err != nil {
		panic(err) // Should never happen
	}
}

// Memory interface
func (a *Atom) Peek(address uint16) uint8 {
	if address&0xff00 == 0x0a00 {
		port := uint8(address & 0x07) // 3 bits used
		return a.fdc.read(port)
	} else if address < romStart {
		return a.ram[address]
	} else if address&0xf800 == ppiaStart {
		port := uint8(address & 0x03) // 2 bits used
		value := a.ppia.read(port)
		//a.logf("[PPIA] Read: %04x, PPIA port%c = 0x%02x\n", address, 'A'+port, value)
		return value
	} else if address&0xf800 == viaStart {
		port := uint8(address & 0x0f) // 4 bits used
		a.logf("[VIA] Read: %04x, VIA port%c\n", address, 'A'+port)
		return 0x00
	} else {
		return a.rom[address-romStart]
	}
}

func (a *Atom) PeekCode(address uint16) uint8 {
	return a.Peek(address)
}

func (a *Atom) Poke(address uint16, value uint8) {
	if address&0xff00 == 0x0a00 {
		port := uint8(address & 0x07) // 3 bits used
		a.fdc.write(port, value)
	} else if address < romStart {
		a.ram[address] = value
	} else if address&0xf800 == ppiaStart {
		port := uint8(address & 0x03) // 2 bits used
		//a.logf("[PPIA] Write: %04x, PPIA port%c = 0x%02x - %08b\n", address, 'A'+port, value, value)
		a.ppia.write(port, value)
	} else if address&0xf800 == viaStart {
		port := uint8(address & 0x0f) // 4 bits used
		a.logf("[VIA] Write: %04x, VIA port %c = 0x%02x - %08b\n", address, 'A'+port, value, value)
	}
}

func (a *Atom) SendKey(key int, released bool) {
	a.keyboard.sendKey(key, released)
}

func (a *Atom) Snapshot() *image.RGBA {
	return a.vdu.snapshot()
}
