package izatom

/*
See the Motorola MC6847 datasheet for more information.
*/

type mc6847 struct {
	a *Atom
}

func NewMC6847(a *Atom) *mc6847 {
	return &mc6847{a: a}
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
