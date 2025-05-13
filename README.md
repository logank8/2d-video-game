This is EVICTION OF THE DAMNED, a project created for CPSC 427 (Video Game Programming) at UBC Vancouver by Aayush Behl, Derrick Cheng, Wendy Greening, Luke Joe, and Logan Keener. It is made entirely in C++ and OpenGL.

<h3>Launch instructions:</h3>

Create the 'build' folder.

Use the following commands in the terminal for the build folder:

```
cd build

cmake ../ -DCMAKE_BUILD_TYPE=Release

make

./eviction
```

Doing this should launch the game in a new window.

<h3>Game premise:</h3>
You find out that your landlord has been practicing black magic, and has taken control of the apartment with his evil monsters. 
It's your job to free each tenant from the demons, and then defeat the landlord at the top floor.

<h3>Technical highlights: </h3>

<li>A* Pathfinding</li>
<li>5 different enemy types with different fighting styles</li>
<li>Animation system made from scratch for spritesheet-based animations</li>
<li>BOIDs flocking algorithm for swarm-based enemy</li>
<li>Particle system for smoke effect using instance-based rendering</li>


<h3>Credits:</h3>
- Various furniture sprites (https://penzilla.itch.io/top-down-retro-interior), (https://limezu.itch.io/moderninteriors)
- Various Icons (https://kyrise.itch.io/kyrises-free-16x16-rpg-icon-pack)
- Player sprite (https://game-endeavor.itch.io/mystic-woods)
- Beetles in swarm (https://opengameart.org/content/ambient-pixel-art-insects )
- Homing projectile ranged enemy, slowing enemy, and dashing enemy () https://pixel-boy.itch.io/ninja-adventure-asset-pack )
- Slime (https://pixel-poem.itch.io/dungeon-assetpuck )
- Skeleton (https://itch.io/profile/snoblin)
- Ranged enemy (https://elthen.itch.io/2d-pixel-art-mini-golem-sprites/ )
- Final boss (https://escape-pixel.itch.io/)
- Keyboard keys (https://dreammix.itch.io/keyboard-keys-for-ui )
- UI bars (https://byandrox.itch.io/crimson-fantasy-gui/ )
- Projectiles ( https://ho88it.itch.io/2-d-projectile-sprites-wild-west-character-pack)
- Start screen (https://ansimuz.itch.io/cyberpunk-street-environment/devlog/784831/city-skyline)
- Background music (https://davidkbd.itch.io/eternity-metal-scfi-music-pack/purchase)
- SFX (https://darkworldaudio.itch.io/sound-effects-survival-i), (https://jdwasabi.itch.io/8-bit-16-bit-sound-effects-pack/)
- Final boss (https://immortal-burrito.itch.io/blood-demons)