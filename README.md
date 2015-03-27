# SP Engine

Opengl framework for exploratory use. So not much of an engine really.

### Motivation

Just to learn OpenGL and perhaps some code organization. I'm taking the approach to build upon and grow this library, constantly refactoring and pruning and optimizing. So I guess this is like growing a garden rather than building a cathedral. It started off with a combined procedural with OOP thrown in. I'm hoping this would give me some perspective on code reuse and OOP patterns.

### Screenshots

![Ogre Scene](/screenshots/simple_with_ogre.png?raw=true "Simple scene with gouroud shading, IQM model loading, animation and cube map")

#### Dependencies

- RakNet is already provided inside the repo
- Freetype2
- SDL2
- boost.filesystem
- SDL2 image
- GLEW
- glm

#### Linux
I only tested this on kubuntu, the packages should be relatively easy to install through a package manager.

#### Mac OS
To make life a little easier, I suggest installing the dependencies through `brew`.

#### Windows
I've made it compatible with msvc. Just need to add a visual studio project file maybe.

#### Compiling
`make`

##### Running the demo
`./sp`
