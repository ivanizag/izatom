package izatom

const (
	// Top row
	KEY_ESC            = 0
	KEY_1_BANG         = 1
	KEY_2_DQUOTE       = 2
	KEY_3_HASH         = 3
	KEY_4_DOLLAR       = 4
	KEY_5_PERCENT      = 5
	KEY_6_AMP          = 6
	KEY_7_QUOTE        = 7
	KEY_8_LPAREN       = 8
	KEY_9_RPAREN       = 9
	KEY_0              = 10
	KEY_MINUS_EQUALS   = 11
	KEY_COLON_ASTERISK = 12
	KEY_UP             = 13
	KEY_BREAK          = 14

	// Second row
	KEY_LEFT_RIGHT = 15
	KEY_COPY       = 16
	KEY_Q          = 17
	KEY_W          = 18
	KEY_E          = 19
	KEY_R          = 20
	KEY_T          = 21
	KEY_Y          = 22
	KEY_U          = 23
	KEY_I          = 24
	KEY_O          = 25
	KEY_P          = 26
	KEY_AT         = 27
	KEY_BACKSLASH  = 28
	KEY_DELETE     = 29

	// Third row
	KEY_UP_DOWN        = 30
	KEY_CTRL           = 31
	KEY_A              = 32
	KEY_S              = 33
	KEY_D              = 34
	KEY_F              = 35
	KEY_G              = 36
	KEY_H              = 37
	KEY_J              = 38
	KEY_K              = 39
	KEY_L              = 40
	KEY_SEMICOLON_PLUS = 41
	KEY_LBRACKET       = 42
	KEY_RBRACKET       = 43
	KEY_RETURN         = 44

	// Fourth row
	KEY_LOCK           = 45
	KEY_LSHIFT         = 46
	KEY_Z              = 47
	KEY_X              = 48
	KEY_C              = 49
	KEY_V              = 50
	KEY_B              = 51
	KEY_N              = 52
	KEY_M              = 53
	KEY_COMMA_LESS     = 54
	KEY_PERIOD_GREATER = 55
	KEY_SLASH_QUESTION = 56
	KEY_RSHIFT         = 57
	KEY_REPT           = 58

	// Fifth row
	KEY_SPACE = 59

	// Usefull constants
	KEY_NONE        = 60
	KEY_SIZE        = 61  // Number of keys
	KEY_IS_RELEASED = 128 // To be added on key releases
)

/*
The keyboard is a 10*6 matrix:

Row     Data (read)
        PB5  PB4  PB3  PB2  PB1  PB0

0       ESC  Q    G    -=   3#
1       Z    P    F    ,<   2"
2       Y    O    E    ;+   1!   up/down
3       X    N    D    :*   0    left/right
4       W    M    C    9)   del  lock
5       V    L    B    8(   copy up
6       U    K    A    7'   ret  ]
7       T    J    @    6&        \
8       S    I    /?   5%        [
9       R    H    .>   4$        space

Row is decoded from PA0-PA3
shift is data bit PB7
ctrl is data bit PB6
rept is data bit PC6
break resets the 6502, the 8255 and the 6522

*/

var keyboardMatrix = [6][10]int{
	{KEY_NONE, KEY_NONE, KEY_UP_DOWN, KEY_LEFT_RIGHT, KEY_LOCK, KEY_UP, KEY_RBRACKET, KEY_BACKSLASH, KEY_LBRACKET, KEY_SPACE},
	{KEY_3_HASH, KEY_2_DQUOTE, KEY_1_BANG, KEY_0, KEY_DELETE, KEY_COPY, KEY_RETURN, KEY_NONE, KEY_NONE, KEY_NONE},
	{KEY_MINUS_EQUALS, KEY_COMMA_LESS, KEY_SEMICOLON_PLUS, KEY_COLON_ASTERISK, KEY_9_RPAREN, KEY_8_LPAREN, KEY_7_QUOTE, KEY_6_AMP, KEY_5_PERCENT, KEY_4_DOLLAR},
	{KEY_G, KEY_F, KEY_E, KEY_D, KEY_C, KEY_B, KEY_A, KEY_AT, KEY_SLASH_QUESTION, KEY_PERIOD_GREATER},
	{KEY_Q, KEY_P, KEY_O, KEY_N, KEY_M, KEY_L, KEY_K, KEY_J, KEY_I, KEY_H},
	{KEY_ESC, KEY_Z, KEY_Y, KEY_X, KEY_W, KEY_V, KEY_U, KEY_T, KEY_S, KEY_R},
}

type keyboard struct {
	keyChannel chan int
	isPressed  [KEY_SIZE]bool
}

func newKeyboard() *keyboard {
	return &keyboard{
		keyChannel: make(chan int),
	}
}

func (k *keyboard) sendKey(key int, released bool) {
	if key < 0 || key >= KEY_SIZE {
		return // Invalid key
	}

	if released {
		key += KEY_IS_RELEASED
	}
	k.keyChannel <- key
}

func (k *keyboard) processKeys() {

	for {
		select {
		case key := <-k.keyChannel:
			if key >= KEY_IS_RELEASED {
				key -= KEY_IS_RELEASED
				k.isPressed[key] = false
				//fmt.Printf("[KEYBOARD] Key released: %d\n", key)
			} else {
				k.isPressed[key] = true
				//fmt.Printf("[KEYBOARD] Key pressed: %d\n", key)
			}
		default:
			return
		}
	}

}

func (k *keyboard) getPB(pa uint8) uint8 {
	var pb uint8 = 0xff // Pull-up resistors
	for i := 0; i < 6; i++ {
		if k.isPressed[keyboardMatrix[i][pa]] {
			pb &^= 1 << i
		}
	}
	if k.isPressed[KEY_CTRL] {
		pb &^= 1 << 6
	}
	if k.isPressed[KEY_LSHIFT] {
		pb &^= 1 << 7
	}
	return pb
}

func (k *keyboard) getReset() bool {
	return k.isPressed[KEY_BREAK]
}

func (k *keyboard) getRept() bool {
	return k.isPressed[KEY_REPT]
}
