# COMP3015 CW2 By Tobey Foxcroft
#### https://github.com/tfoxcroftply/COMP3015_CW2

## Development environment
This project was developed in Visual Studio 2022 Community Edition on Windows 11. GitHub was heavily relied on for this project not only for version control, but for the seamless continuation of work between multiple devices.

## How does it work? (Features)
These features were loosely based on http://learnopengl.com tutorials. Each feature's code was written entirely by me, and have multiple differences/adaptations.

Within scenebasic_uniform.h, there are boolean debugging variables which allow for these features to display important features, such as bloom threshold generations, forced gaussian blur, depth map mode, and more.
### Gaussian blur
The Gaussian blur method I implemented is based on the "ping-pong" method. It relies on sampling values around a pixel either vertically or horizontally. To create a simple light Gaussian blur, the main scene framebuffer is blurred horizontally and then vertically, to create a Gaussian blur outcome. 
The main scene framebuffer is what the scene renders to at first, and is later referenced and modified by other post-processing effects such as this. A square is drawn over scene which allows for framebuffers to be rendered as if it was being rendered normally. 
The sampling intensity for each pixel is determined by the distance of the pixel being sampled from the origin pixel against a set of gaussian weights. These gaussian values were derived from an interactive gaussian graph, which at first, they seemed to be too intense for my scene, so I added a scale factor to reduce them. In order to increase blur intensity, the number of passes can increase from the 2 standard/horizontal passes, though it does not allow for smooth transitioning between two blur settings.

![blur](/Images/blur.png)

### Bloom
I implemented bloom on my scene to make light-coloured surfaces appear to "shine" more.
To begin with, the bloom shader extracts and filters the colours from the main rendered scene, so only light coloured things beyond a certain threshold are shown.
![bloompreblur](/Images/bloompreblur.png)
As shown, a feature I added to my bloom implementation was to gradually fade out colours that are starting to not meet the threshold. This meant that sharp changes in bloom visibility are not present, as it looks rather unnatural to have a sudden shift in light. An exposure factor is also applied to this extracted light, which also prevents harsh or otherwise unnatural lighting.

Before this new data is set to be mixed, it requires blurring in order to create the actual bloom effect. This relies on the Gaussian blur function from before, and it uses a total of 6 passes.
![bloompremix](/Images/bloompremix.png)

After this, a uniform variable is flipped in the shader, and it applies this newly generated image on top of the existing rendered scene. This means that black colours will not change anything, but the white light will overlay.

The full outcome of this is as shown.
![bloom](/Images/bloom.png)

### Shadowmap (PCF)
This was one of the more complicated features, and is different from the other features as it is not considered a post-processing feature. It also required a restructure of my main render loop to allow for multiple renders within a single render loop.

In order to capture depth of objects within the scene, I had to utilise "depth-maps". A depth map in this scenario allows for my shaders to determine if certain fragments are being covered from the light source. In order to capture a depth map, I had to choose a source position of a light, and then render the entire scene from that point of view using a depth map buffer. The image below shows the depth map capturing the depth of the boat, and a nearby node.
![depth](/Images/depthmap.png)

On the final render of the scene, this depth map is applied to each draw call as a Sampler2D uniform, separate from the main texture. This allows for the shader to determine whether the final textures need to have a darkening effect to them. The light position is also attached as a uniform so the depth map position source can be determined. It is also converted into world-position by the vertex shader before passing to the fragment shader.

Shadows are linked to the Ambient setting of the Phong shader I used. When generating Phong values, the ambient is affected by a "shadow factor", which is a number between 0 and 1 returned by the shadow calculation function.

In the shadow function after fragment normalisation, the depth of each fragment is sampled against the included depth map. If the fragment being rendered is beyond the closest depth recorded by the depth map, then it must be in cover, and therefore registers as a shadow. A "shadow bias" setting has also been included to reduce an affect called "shadow acne", which occurs here when the depth map is not perpendicular to a surface normal.

Although this produced a working shadowmap, the module requires the use of PCF (Percentage Close Filtering) to have an anti-aliasing effect on the shadows as the shadow pixels are rather large. An X/Y loop is used to sample the surrounding pixels of a texel, which are then averaged in the final shadow output to produce a number between 0 and 1.
![shadow](/Images/shadow.png)


## Code structure/Classes
This project heavily relies on classes and their associated objects. In this project, I used multiple additional classes (separate from scenebasic_uniform) such as "Boat", "Level" and "Game". Each of these classes contain the neccessary variables and logic related to their name. Object creation is started within the scenebasic_uniform initialisation stage, and is sort of like a tree of objects within objects.

Scenebasic_uniform acts in somewhat the same way as CW1, in that it has an initialisation stage and a render loop. In the initialisation stage, buffers, textures, models, shaders, class objects, and more, are created for future use within the render loop. Other important things like shader settings and texture unit allocations are also performed.

The main render loop runs until interrupted by the escape key. In the render loop, user inputs and model data are sent/retrieved to allow for the new frame to be different from the last one. The render loop actually performs two full scene renders, as one is required for the depth map, and the other is the main viewing one. The second render loop takes the information from the first render loop to render the shadows on to the model textures.

After the main render is complete, post-processing is applied, such as gaussian blur and bloom. These take the framebuffers and apply changes to it, before combining it all together and outputting it into the default viewable framebuffer.

## User interaction
To interact with this project, there are two main folders, "Compiled" and "Source". The compiled folder contains a pre-compiled executable version of my project, allowing users to open it as is. **The .zip folder must be extracted in order to run correctly.**
Mouse and keyboard controls have also been implemented for this scene, allowing for response based on user input. In this scene, the mouse controls the camera direction around the boat, and the keyboard "WASD" keys control the boat itself. The escape key has also been linked to exit the program.
The aim of the scene is to collect as many diamonds within each level as fast as possible. The user must drive the boat into the green diamonds, optionally driving into the blue diamonds as well, which serve as boost powerups.
The game is also timed, allowing for users to check their completion times. The game also saves time data into a file, which this data is presented at the end of every game using an on-screen leaderboard.

## Additional information
### Complete list of features
#### CW1
- Phong lighting
- Texture sampling
- Fog
- Mixing
- Skybox
- Model animation
- Keyboard/Mouse controls
#### CW2
- Gaussian blur
- Bloom
- Shadowmap (PCF)

### Origin
This program was based off my CW1 scene, which itself is based on the provided lab solution, "scenebasic_uniform". I believe that my program is unique as it blends the select few shading techniques together in order to create an aesthetically pleasing (but simple) scene.
### Oversights
#### Gaussian method
The gaussian blur method in this program could be considered slightly inefficient. As stated before, I used the http://learnopengl.com to guide my Gaussian blur implementation, and this resulted in a blur method which is performed in passes. Upon every pass, a texture buffer is referenced and sampled against, and for more passes, this could lead to increased frame times very quickly.
#### Per-object shadowmaps
A problem I had with this project was using a single shadowmap. With a single shadowmap, I could either have a low quality shadowmap which covers objects further away, or have a smaller shadow map which would look higher quality, but would prevent far objects from having a shadow.
If I were to make a similar project in the future, perhaps I would add a per-object shadowmap, but to get it implemented in the first place was a huge learning curve.
### Future differences
This project is not as clean as I would like it to be, and for me to be fully satisfied, I would have to rewrite the entire thing. As this project was a learning curve, things were added along the way. This caused some repetitive and otherwise potentially manageable code. 
### Bias calculation improvement
While my program does have bias calculations built in for shadowmap use, it is by no means perfect. If I were to redo the project, I would want to better understand the bias calculations to perfect it.

## YouTube video
https://youtu.be/aE-pXoOr3Kk

## Credits
glText (Text rendering) - https://github.com/vallentin/glText
irrKlang (Audio engine) - https://www.ambiera.com/irrklang/

Collect sound - https://freesound.org/s/446129/
Boost sound - https://freesound.org/s/478343/
Round end sound - https://freesound.org/s/391540/

#### Derived from CW1
Skybox: https://polyhaven.com/a/qwantani_puresky
Model: https://www.thingiverse.com/thing:1752110
