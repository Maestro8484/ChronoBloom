# Printing ChronoBloom

Covers the 8" build. The 15" files ship for the adventurous, but a guide is not written yet; the 3MF and the LightBurn SVG are the whole story for now.

Files: [docs/publish/ChronoBloom_3D_Files/ChronoBloom_8inch/V13/](publish/ChronoBloom_3D_Files/ChronoBloom_8inch/V13/). The assembly 3MF has every part with print profiles already embedded (Bambu Studio); the individual STLs are there if you want to lay them out yourself.

| Part | Qty | File |
|---|---|---|
| Everything, profiles embedded | -- | `Chronobloom-8inch-v13-All-AssemblyView.3mf` |
| Frame ring | 1 | `Chronobloom-8inch-v13-Ring.stl` |
| Front diffuser | 1 | `Chronobloom-8inch-v13-frontDiffuser.stl` |
| Reflector | 1 | `Chronobloom-8inch-v13-Reflector.stl` |
| Spacer | 1 | `Chronobloom-8inch-v13-Spacer.stl` (print it, it is not optional, see below) |
| Back plate, desk (default) | 1 | `Chronobloom-8inch-v13-backCover.stl` |
| Back plate, wall (alternative) | 1 | `Chronobloom-8inch-v13-backCover-Buttons.stl` |
| Desk stand | 1 | `Chronobloom-8inch-v13-stand.stl` |

The stand is in the assembly 3MF as well, so you get it either way.

There used to be a second project file holding the same parts arranged as a stack to look at.
The two were consolidated in August 2026, so `Chronobloom-8inch-v13-All-AssemblyView.3mf` is now
the only one and it does both jobs: it carries all seven parts with the print profiles already
set, laid out across eight plates, and you can slice it directly.

Three SVGs ship alongside for anyone cutting rather than printing the flat parts:
`ChronoBloom-FrontDiffuser.svg`, `ChronoBloom-backCover.svg` and
`ChronoBloom-backCoverButtons.svg`.

Print one back plate or the other, not both. The choice is desk or wall.

**Desk, and this is the default.** `Chronobloom-8inch-v13-backCover.stl` carries the cutout for
the stand.

**Wall.** `Chronobloom-8inch-v13-backCover-Buttons.stl`, 2 mm, with the openings for the two
momentary buttons. Both back plates are in the all-in-one assembly 3MF, so you do not have to go
looking for either.

One thing to think about before you commit to the wall version: the buttons sit on the back, so
mounted flat they end up tight against the wall and may be awkward to reach. Leave yourself a
little standoff. How much has not been measured, so treat that as a warning rather than a number.

## Two parts, two jobs, and the names matter

The **reflector** sits behind the LEDs and shapes the light into pointed hands. That's the part
descended from Steve Manley's 2015 design, and it's why the clock looks the way it does.

The **front diffuser** is a separate thin sheet over the face that softens the pixels into a
bloom. Different part, different job, and it's the fussy one.

## The frame

- **Material:** basic PLA. That's what I'd print your first one in. Glow-in-the-dark PLA on the frame is a fun thing to try if you happen to have some, since it charges off the LEDs and glows for a while after the display sleeps at night, but it isn't required and it isn't what I'd tell you to buy.
- **Layer height:** 0.2mm is fine for the frame.
- **Bed:** the largest 8" part is 209 mm square (measured from the shipped v11 STLs), so any 220 mm bed will do. A 256 mm bed (Bambu P1S class) has room to spare.
- **Supports and orientation, per part:** the assembly 3MF already carries orientation and settings, so starting from it is the safe path. Slicing the STLs yourself, here is what is known:

| Part | Orientation | Supports | Notes |
|---|---|---|---|
| Frame ring | Flat on the bed | None by design | Ring seats print flat. Unverified -- needs one slice-check |
| Front diffuser | Flat on a smooth plate | None | 0.6mm thick; settings in the diffuser section below |
| Reflector | Flat on the bed | None by design | Unverified -- needs one slice-check |
| Back cover | Flat on the bed | None by design | Unverified -- needs one slice-check |
| Stand | As laid out in the assembly 3MF | Unverified -- needs one slice-check | |

Either way, check the sliced preview before committing a long print.

## The front diffuser (this is the part that makes the bloom)

Print it, don't skip it. A bare ring is just dots; the diffuser is what turns them into soft light.
This one took a lot of failed prints to land on, so here's what's actually fixed and what you should
expect to experiment with.

- **Material:** white PLA. That one is not negotiable. PLA+ is fine.
- **Thickness:** a starting point, not a spec. 0.6mm is what works for me. PLA scatters light differently brand to brand, so expect to print a couple at different thicknesses before you're happy. Print it flat on a smooth plate.
- **Fill pattern:** set the top and bottom surface fill to **Monotonic**, not Concentric. On a thin circular part some slicers quietly fall back to Concentric even when you pick something else, which shows up as swirl artifacts in the light. Check the sliced preview, not just the settings panel, every time. Worth experimenting with as well: this is the other setting that visibly changes the result.
- **Solid all the way through:** set the top and bottom shell layer counts so together they cover the full thickness, with no sparse infill gap in the middle. A gap in a translucent part reads as a bright streak or a refraction mismatch.
- **Speed:** cap it around 100mm/s, first layer near 44mm/s. Slower layers fuse better, which means fewer tiny voids and more even light. Don't rely on the volumetric-speed cap alone at these thin layer heights.

## Ring fit

The three rings drop into printed seats on the core: 60 LEDs outer, 24 middle, 12 inner, plus one loose pixel for the center. The LEDs on each ring seat exactly into the recesses, no forcing. A little cleanup of stringing around the seats is normal.

Measured off the rings themselves with calipers, not copied from the kit listing:

| Ring | LEDs | Outer diameter |
|---|---|---|
| Outer | 60 | **172 mm** |
| Middle | 24 | **92 mm** |
| Inner | 12 | **52 mm** |
| Center | 1 | **6 mm** opening for a single LED |

Any WS2812B rings in 60/24/12 counts at those diameters will fit. The seats were sized to the rings in the WESIRI 241-LED kit from the BOM, so that kit drops straight in.

> Assembly here is words only for now. Photos of a full build are coming, and my guess is
> within a week or so of 2026-07-31. If you are reading this well after that and there are
> still no pictures, chase me. Until then, read the spacer orientation note twice.

## The spacer, and which way round it goes

The spacer is easy to mistake for an optional shim. It is not. It does two jobs.

It sets the depth. The raised rings moulded into it stand exactly as tall as the WS2812B rings
do once they are seated properly in the reflector's LED slots, so the stack closes at the right
height with no guessing and nothing pressing on the LEDs.

It stops light bleeding between rings. Those same raised rings sit between the 60, the 24 and
the 12 and act as walls, so the seconds ring does not wash into the minutes ring behind the
diffuser. Without it the face reads muddier and the hands lose their edge.

**Orientation matters: the flat side faces away from the front of the clock.** The raised rings
point in toward the LEDs. Fit it backwards and you lose both the depth setting and the light
barriers.

## Assembly hardware

**M2.5 fine thread bolts, and nothing else.** No glue, no pins, no heat-set inserts, no
soldering iron for the frame. The v13 parts changed this: earlier versions leaned on filament
offcuts as alignment pins with a drop of superglue, because the old screws could not reach
through the stack. They can now, so that whole workaround is gone. If you have read an older
version of these instructions, ignore it.

The ring is the spine of the build. It is **21 mm thick**, and everything bolts into it:

- **From the front**, one bolt runs through all three front parts, the front diffuser, the
  reflector and the spacer, and threads into the ring.
- **From the back**, the back cover bolts into the same ring.

So the ring takes the thread from both directions and holds the sandwich together on its own.

| Hole | Diameter | Where | Job |
|---|---|---|---|
| Pass-through | **2.9 mm** | front diffuser, reflector, spacer, back cover | Clearance. The bolt slides through without biting |
| Threaded | **2.6 mm** | the ring only | The M2.5 fine thread cuts its own thread here |

The 2.6 mm figure is the modelled hole. Calipered on an actual printed part the crest measures
about **2.35 mm**, which is what gives the fine thread something to bite into. If your printer
runs tight and the bolt will not start, open the ring holes with a 2.6 mm bit rather than
forcing it. Do not drill the 2.9 mm pass-through holes any wider or the bolt heads lose their
seat.

Use fine thread, not coarse. Coarse M2.5 will strip a 2.6 mm printed hole on the first pass.

The electronics are a lighter touch than you might expect too: the rings arrive with 3-wire JST
connectors already on them, so soldering the chain is about a joint that survives years on a
wall, not about making the connection at all. See the wiring section in the README.

## Credit

The frame geometry is a remix of Steve Manley's original NeoPixel Ring Clock design. See [NOTICE](../NOTICE). STL and print files are CC BY 4.0.
