package izatom

import "fmt"

const (
	OSRDCH     = 0xffe3
	OSWRCH     = 0xfff4
	OSRDCH_RET = 0xfeca
)

func (a *Atom) traceOS() {
	pc, _ := a.cpu.GetPCAndSP()
	regA, _, _, _ := a.cpu.GetAXYP()
	switch pc {
	case OSWRCH:
		//fmt.Printf("[kernel] OSWRCH: %c\n", regA)
	case OSRDCH_RET:
		//if regA != 0 {
		//fmt.Printf("[kernel] OSRDCH_RET: 0x%02x %c\n", regA, regA)
		//}
	case 0xe230:
		fmt.Printf("[kernel] Spin if busy\n")
	case 0xe75b:
		fmt.Printf("[kernel] Start disk motor\n")
	case 0xe7ed:
		fmt.Printf("[kernel] R/W Command 0x%02x\n", regA)
	case 0xe816:
		sectors := a.Peek(0xf1)
		sectors_left := a.Peek(0xcb)
		fmt.Printf("[kernel] Calc: sectors %d, sectors left %d\n", sectors, sectors_left)
	}

}
