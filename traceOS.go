package izatom

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
		if regA != 0 {
			//fmt.Printf("[kernel] OSRDCH_RET: 0x%02x %c\n", regA, regA)
		}
	}
}
