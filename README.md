# Haptic Feedback Remote
This "remote" is a design concept that utilizes a touchpad as opposed to a D-pad with the intention to control a tv (although this concept doesn't include any of the hardware required for communication). Based on inputs, the remote will vibrate in specific patterns (haptic feedback) in order to simulate certain feelings like a click or a scroll. 

# Features
Click: When the touchpad is pressed and released, the voice coil will give a kick accompanied by a little chirp

Scroll: When the touchpad is pressed and moved across, the voice coil gives a very light kick for every movement in the X or Y direction

Simulated Scroll: When the touchpad is being scrolled across, and the user lets go, the micro calculates how often a scroll click should happen given the velocity of the movement before it was released. The voice coil reacts as though it's scrolling from an actual interaction
