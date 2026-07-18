# Printing ChronoBloom

Covers the 8" build. The 15" is experimental and its notes stay in the repo history until the guide is solid.

Files: [docs/publish/ChronoBloom_3D_Files/](publish/ChronoBloom_3D_Files/). The `(ALL)` 3MF has every part on one plate with profiles already set; the individual STLs are there if you want to lay them out yourself.

## The frame

- **Material:** any PLA works. UV glow-in-the-dark PLA is a nice touch. It charges off the LEDs and glows for a while after the display sleeps at night.
- **Layer height:** 0.2mm is fine for the frame.
- **Supports:** <!-- TODO: confirm per part before treating as final --> the core and ring seats are designed to print flat with no supports. Verify in your slicer's preview before committing a long print.
- **Bed:** the 8" parts fit a 256mm bed (Bambu P1S class).

## The diffuser (this is the part that makes the bloom)

Print it, do not skip it. A bare ring is just dots; the diffuser is what turns them into soft light.

- **Thickness:** 0.6mm, printed flat on a smooth plate.
- **Material:** white PLA (PLA+ tested). Natural or lightly-tinted PLA is worth trying if you want more glow and less haze.
- **Fill pattern:** set the top and bottom surface fill to **Monotonic**, not Concentric. On a thin circular part some slicers quietly fall back to Concentric even when you pick something else, which shows up as swirl artifacts in the light. Check the sliced preview, not just the settings panel, every time.
- **Solid all the way through:** set the top and bottom shell layer counts so together they cover the full thickness, with no sparse infill gap in the middle. A gap in a translucent part reads as a bright streak or a refraction mismatch.
- **Speed:** cap it around 100mm/s, first layer near 44mm/s. Slower layers fuse better, which means fewer tiny voids and more even light. Do not rely on the volumetric-speed cap alone at these thin layer heights.

## Ring fit

The three rings drop into printed seats on the core: 60 LEDs outer, 24 middle, 12 inner, plus one loose pixel for the center. They should seat without forcing. A little cleanup of stringing around the seats is normal.

<!-- TODO: ring outer-diameter dimensions are not caliper-verified against a printed part yet
(outer 60-LED ~172-175mm, middle 24-LED ~88-92mm, inner 12-LED ~50-52mm, from the kit listing).
Confirm with calipers before anyone treats these as exact or before a Printables upload. -->

## Assembly hardware

M3 heat-set inserts and M3x10 bolts hold the frame together (roughly 6 of each). Sink the inserts with a soldering iron on a low temp, square to the hole.

## Credit

The frame geometry is a remix of Steve Manley's original NeoPixel Ring Clock design. See [NOTICE](../NOTICE). STL and print files are CC BY 4.0.
