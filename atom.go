package izatom

/*
Resources:
  http://www.acornatom.nl/atom_handleidingen/aw123/acorn_roms.htm

*/

import (
	"embed"
	"fmt"
	"image"

	"github.com/ivanizag/iz6502"
)

const (
	romStart  = 0xa000
	ppiaStart = 0xb000
	ppiaEnd   = 0xb7ff
	viaStart  = 0xb800
	viaEnd    = 0xbfff
)

type Atom struct {
	cpu      *iz6502.State
	vdu      *mc6847
	ppia     *ins8255
	keyboard *keyboard

	ram [0xa000]uint8
	rom [0x10000 - romStart]uint8

	traceCPU bool
}

func NewAtom() *Atom {
	var a Atom
	a.cpu = iz6502.NewNMOS6502(&a)
	a.vdu = NewMC6847(&a)
	a.ppia = NewINS8255(&a)
	a.keyboard = newKeyboard()

	a.loadRom("akernel.rom", 0xf000)
	a.loadRom("dosrom.rom", 0xe000)
	a.loadRom("afloat.rom", 0xd000)
	a.loadRom("abasic.rom", 0xc000)

	//a.traceCPU = true
	a.cpu.SetTrace(a.traceCPU)
	return &a
}

func (a *Atom) Run() {
	a.cpu.Reset()
	for {
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
		} else if pc == 0xfe93 {
			//if a.cpu.GetTrace() {
			_, regX, _, _ := a.cpu.GetAXYP()
			if regX != 0 {
				//fmt.Printf("[KEYBOARD] Key detected: %v\n", regX)
			}
			//}
		}

		a.traceOS()
		a.cpu.ExecuteInstruction()

		a.keyboard.processKeys()
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
	if address < romStart {
		return a.ram[address]
	} else if address >= ppiaStart && address <= ppiaEnd {
		port := uint8(address - ppiaStart)
		value := a.ppia.read(port)
		if a.cpu.GetTrace() {
			fmt.Printf("[PPIA] Read: %04x, PPIA port%c = 0x%02x\n", address, 'A'+port, value)
		}
		return value
	} else if address >= viaStart && address <= viaEnd {
		port := uint8(address - viaStart)
		if a.cpu.GetTrace() {
			fmt.Printf("[VIA] Read: %04x, VIA port%c\n", address, 'A'+port)
		}
		return 0x00
	} else {
		return a.rom[address-romStart]
	}
}

func (a *Atom) PeekCode(address uint16) uint8 {
	return a.Peek(address)
}

func (a *Atom) Poke(address uint16, value uint8) {
	if address < romStart {
		a.ram[address] = value
	} else if address >= ppiaStart && address <= ppiaEnd {
		port := uint8(address - ppiaStart)
		if a.cpu.GetTrace() {
			fmt.Printf("[PPIA] Write: %04x, PPIA port%c = 0x%02x - %08b\n", address, 'A'+port, value, value)
		}
		a.ppia.write(port, value)
	} else if address >= viaStart && address <= viaEnd {
		port := uint8(address - viaStart)
		if a.cpu.GetTrace() {
			fmt.Printf("[VIA] Write: %04x, VIA port%c = 0x%02x - %08b\n", address, 'A'+port, value, value)
		}
	} else {
		// Do nothing
	}
}

func (a *Atom) SendKey(key int, released bool) {
	a.keyboard.sendKey(key, released)
}

func (a *Atom) Snapshot() *image.RGBA {
	return a.vdu.snapshot()
}
