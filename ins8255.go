package izatom

const (
	INS8255_PORT_A = 0
	INS8255_PORT_B = 1
	INS8255_PORT_C = 2
)

type ins8255 struct {
	a       *Atom
	ports   [3]uint8
	control uint8
}

func NewINS8255(a *Atom) *ins8255 {
	return &ins8255{
		a: a,
	}
}

func (i *ins8255) reset() {
	// TODO
}

func (i *ins8255) write(port uint8, value uint8) {
	switch port {
	case 0:
		i.ports[port] = value
	case 1:
		i.ports[port] = value
	case 2:
		//panic("TODO: write in port C")
		i.ports[port] = value
	case 3:
		i.control = value
	default:
		panic("invalid port")
	}
}

func (i *ins8255) read(port uint8) uint8 {
	switch port {
	case 0:
		return i.ports[port]
	case 1:
		return i.readPortB()
	case 2:
		return i.readPortC()
	case 3:
		return i.control
	default:
		panic("invalid address")
	}
}

func (i *ins8255) readPortB() uint8 {
	pb := i.a.keyboard.getPB(i.ports[INS8255_PORT_A])
	i.ports[INS8255_PORT_B] = pb
	return pb
}

func (i *ins8255) readPortC() uint8 {
	value := i.ports[2]

	// PC7 is !FS from the MC6847
	if !i.a.vdu.fs() {
		value &= 0x7f
	} else {
		value |= 0x80 // Pull-up resistor
	}

	// PC6 is the repeat key
	if i.a.keyboard.getRept() {
		value &= 0xbf
	} else {
		value |= 0x40 // Pull-up resistor
	}

	return value
}

// On reset:
// control register is set to 0x8a 1000_1010
//     Port A is set to output
//     Port C upper is set to input
//     Port B is set to input
//     Port C lower is set to output
//     Port A mode is set to mode 0
//     Port B mode is set to mode 0
//     Mode set flag is set to 1 = not active
// Port C to 0000_0111
//     Bit flag set to 0 = active
//     Bit select set to 011 = 3
//     Bit set/reset set to 1 = set
//     => PC3 (CSS of VDU) is set to 1
