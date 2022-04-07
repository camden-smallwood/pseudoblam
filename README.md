# Pseudo-BLAM!

**What the heck is this?**

Great question! When I began writing this code, I wasn't entirely sure what I was even going to make. Eventually I ended up writing a WaveFront OBJ parser in C99. From this experiment, I decided that I would like to render my loaded models on screen. This required me to learn a 3D graphics API. Being lazy, I opted for OpenGL 4.1, because it requires the least amount of code to get something on screen.

Once I started to learn more OpenGL stuff, I thought it might be fun to try and remake stuff from Halo 3. I spent many years modding Halo stuff and learned a lot through research and reverse engineering, so it seemed like a natural choice when it came to apply what I was learning towards. So that's what I'm doing with this: making a bastardized, crappy version of Halo based on OpenGL instead of D3D from the perspective of an obsessive autist who never actually spent much time playing the games.

Currently I am focusing on the model animation code and rendering code, experimenting with different approaches to rendering techniques and layering render passes with deferred rendering. Once I'm happy enough with the graphical content and the code handling it, I'll move on to building up some more of the foundational game systems like physics, ui, sound, etc. If I haven't blown my brains out by then or given up completely, I'll see what's left before starting on the multiplayer engines.
