package main

import (
	"github.com/ivanizag/izatom"
	"github.com/veandco/go-sdl2/sdl"
)

func main() {
	// Create a new atom
	a := izatom.NewAtom()

	// Run the atom
	go a.Run()

	// Prepare SDL
	window, renderer, err := sdl.CreateWindowAndRenderer(500, 300,
		sdl.WINDOW_SHOWN)
	if err != nil {
		panic(err)
	}
	defer window.Destroy()
	defer renderer.Destroy()

	window.SetTitle("IzAtom")
	window.SetResizable(true)

	running := true
	for running {
		// Handle events
		for event := sdl.PollEvent(); event != nil; event = sdl.PollEvent() {
			switch e := event.(type) {
			case *sdl.QuitEvent:
				running = false
			case *sdl.KeyboardEvent:
				sendKey(a, e)
			}
		}

		// Draw
		renderer.Clear()
		renderer.Present()
		sdl.Delay(1000 / 30)
	}

}
