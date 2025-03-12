# gameframework2d (GF2D)
a collection of utlitity functions designed to facilitate creating 2D games with SDL2
This project is specifically intended to function as an educational tool for my students taking 2D Game Programming.

FOR FULL FEATURES CHECKOUT THE MASTER BRANCH
Students: Don't do that.  You are not ready.... yet.

The main branch is kept deliberately minimal to allow students to build out systems as they are learning.
Other branches have more complete system: Collisions, Windows, Armatures, Entities, Particles...

# Build Process

Before you can build the example code we are providing for you, you will need to obtain the libraries required
by the source code
 - SDL2
 - SDL2_image
 - SDL2_mixer
 - SDL2_ttf
There are additional sub modules that are needed for this project to work as well, but they can be pulled right from within the project.
Performable from the following steps from the root of the cloned git repository within a terminal. 

Make sure you fetch submodules: `git submodule update --init --recursive`
Go into each submodule's src directory and type:
`make`
`make static`

Once each submodule has been made you can go into the base project src folder anre simply type:
`make`

You should now have a `gf2d` binary within the root of your git repository. Executing this will start your game.

# video overviews and tutorials
Overview: https://www.youtube.com/watch?v=nvVQ_n6ycC4

Linux Setup: https://www.youtube.com/watch?v=0Znnv5C4mCo

Windows Setup: https://www.youtube.com/watch?v=zj_egJ4sw3I


Personal Deliverables List:
- Branching path world  (A world with branching paths. Methinks - as opposed to being one, linear path)      ✔
- Monster interaction ["Battle"] screen -- Branching Menu screen.                                            ✔  (OH WE BRANCHIN' !!!)
- 10 different dragons to interact with                                                                    (8/10)
- Stats view                                                                                                 ✔
- 5 items that can be collected.                                                                             ✔

