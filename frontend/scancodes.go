package main

import (
	"github.com/ivanizag/izatom"
	"github.com/veandco/go-sdl2/sdl"
)

func sendKey(a *izatom.Atom, e *sdl.KeyboardEvent) {
	atomkey := izatom.KEY_NONE
	switch e.Keysym.Scancode {
	case sdl.SCANCODE_ESCAPE:
		atomkey = izatom.KEY_ESC
	case sdl.SCANCODE_1:
		atomkey = izatom.KEY_1_BANG
	case sdl.SCANCODE_2:
		atomkey = izatom.KEY_2_DQUOTE
	case sdl.SCANCODE_3:
		atomkey = izatom.KEY_3_HASH
	case sdl.SCANCODE_4:
		atomkey = izatom.KEY_4_DOLLAR
	case sdl.SCANCODE_5:
		atomkey = izatom.KEY_5_PERCENT
	case sdl.SCANCODE_6:
		atomkey = izatom.KEY_6_AMP
	case sdl.SCANCODE_7:
		atomkey = izatom.KEY_7_QUOTE
	case sdl.SCANCODE_8:
		atomkey = izatom.KEY_8_LPAREN
	case sdl.SCANCODE_9:
		atomkey = izatom.KEY_9_RPAREN
	case sdl.SCANCODE_0:
		atomkey = izatom.KEY_0
	case sdl.SCANCODE_MINUS:
		atomkey = izatom.KEY_MINUS_EQUALS
	case sdl.SCANCODE_EQUALS:
		atomkey = izatom.KEY_COLON_ASTERISK
	case sdl.SCANCODE_INSERT:
		atomkey = izatom.KEY_UP
	case sdl.SCANCODE_DELETE:
		atomkey = izatom.KEY_BREAK

	case sdl.SCANCODE_LEFT:
		atomkey = izatom.KEY_LEFT_RIGHT
	case sdl.SCANCODE_TAB:
		atomkey = izatom.KEY_COPY
	case sdl.SCANCODE_Q:
		atomkey = izatom.KEY_Q
	case sdl.SCANCODE_W:
		atomkey = izatom.KEY_W
	case sdl.SCANCODE_E:
		atomkey = izatom.KEY_E
	case sdl.SCANCODE_R:
		atomkey = izatom.KEY_R
	case sdl.SCANCODE_T:
		atomkey = izatom.KEY_T
	case sdl.SCANCODE_Y:
		atomkey = izatom.KEY_Y
	case sdl.SCANCODE_U:
		atomkey = izatom.KEY_U
	case sdl.SCANCODE_I:
		atomkey = izatom.KEY_I
	case sdl.SCANCODE_O:
		atomkey = izatom.KEY_O
	case sdl.SCANCODE_P:
		atomkey = izatom.KEY_P
	case sdl.SCANCODE_LEFTBRACKET:
		atomkey = izatom.KEY_AT
	case sdl.SCANCODE_RIGHTBRACKET:
		atomkey = izatom.KEY_BACKSLASH
	case sdl.SCANCODE_BACKSPACE:
		atomkey = izatom.KEY_DELETE

	case sdl.SCANCODE_DOWN:
		atomkey = izatom.KEY_UP_DOWN
	case sdl.SCANCODE_LCTRL:
		atomkey = izatom.KEY_CTRL
	case sdl.SCANCODE_A:
		atomkey = izatom.KEY_A
	case sdl.SCANCODE_S:
		atomkey = izatom.KEY_S
	case sdl.SCANCODE_D:
		atomkey = izatom.KEY_D
	case sdl.SCANCODE_F:
		atomkey = izatom.KEY_F
	case sdl.SCANCODE_G:
		atomkey = izatom.KEY_G
	case sdl.SCANCODE_H:
		atomkey = izatom.KEY_H
	case sdl.SCANCODE_J:
		atomkey = izatom.KEY_J
	case sdl.SCANCODE_K:
		atomkey = izatom.KEY_K
	case sdl.SCANCODE_L:
		atomkey = izatom.KEY_L
	case sdl.SCANCODE_SEMICOLON:
		atomkey = izatom.KEY_SEMICOLON_PLUS
	case sdl.SCANCODE_APOSTROPHE:
		atomkey = izatom.KEY_LBRACKET
	case sdl.SCANCODE_BACKSLASH:
		atomkey = izatom.KEY_RBRACKET
	case sdl.SCANCODE_RETURN:
		atomkey = izatom.KEY_RETURN

	case sdl.SCANCODE_GRAVE:
		atomkey = izatom.KEY_LOCK
	case sdl.SCANCODE_LSHIFT:
		atomkey = izatom.KEY_LSHIFT
	case sdl.SCANCODE_Z:
		atomkey = izatom.KEY_Z
	case sdl.SCANCODE_X:
		atomkey = izatom.KEY_X
	case sdl.SCANCODE_C:
		atomkey = izatom.KEY_C
	case sdl.SCANCODE_V:
		atomkey = izatom.KEY_V
	case sdl.SCANCODE_B:
		atomkey = izatom.KEY_B
	case sdl.SCANCODE_N:
		atomkey = izatom.KEY_N
	case sdl.SCANCODE_M:
		atomkey = izatom.KEY_M
	case sdl.SCANCODE_COMMA:
		atomkey = izatom.KEY_COMMA_LESS
	case sdl.SCANCODE_PERIOD:
		atomkey = izatom.KEY_PERIOD_GREATER
	case sdl.SCANCODE_SLASH:
		atomkey = izatom.KEY_SLASH_QUESTION
	case sdl.SCANCODE_RSHIFT:
		atomkey = izatom.KEY_RSHIFT
	case sdl.SCANCODE_RCTRL:
		atomkey = izatom.KEY_REPT
	case sdl.SCANCODE_SPACE:
		atomkey = izatom.KEY_SPACE
	default:
		atomkey = izatom.KEY_NONE
	}
	if atomkey != izatom.KEY_NONE {
		a.SendKey(atomkey, e.State == sdl.RELEASED)
	}
}
