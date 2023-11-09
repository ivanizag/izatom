package main

import (
	"os"
	"unsafe"

	"github.com/ivanizag/izatom"
	"github.com/veandco/go-sdl2/sdl"
)

func main() {
	// Create a new atom
	a := izatom.NewAtom()
	if len(os.Args) > 1 {
		a.LoadDisk(os.Args[1])
	}

	// Run the atom
	go a.Run()

	// Prepare SDL
	window, renderer, err := sdl.CreateWindowAndRenderer(256*4, 192*4,
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
		img := a.Snapshot()

		if img != nil {
			surface, err := sdl.CreateRGBSurfaceFrom(unsafe.Pointer(&img.Pix[0]),
				int32(img.Bounds().Dx()), int32(img.Bounds().Dy()),
				32, 4*img.Bounds().Dx(),
				0x0000ff, 0x0000ff00, 0x00ff0000, 0xff000000)
			// Valid for little endian. Should we reverse for big endian?
			// 0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff)

			if err != nil {
				panic(err)
			}

			texture, err := renderer.CreateTextureFromSurface(surface)
			if err != nil {
				panic(err)
			}

			renderer.Clear()
			renderer.Copy(texture, nil, nil)
			renderer.Present()
			surface.Free()
			texture.Destroy()
		}
		sdl.Delay(1000 / 30)
	}

}
