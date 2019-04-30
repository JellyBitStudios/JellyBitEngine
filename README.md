# JellyBit Engine

JellyBit Engine is a 3D game engine developed by students from CITM-UPC Terrassa.

## About the engine

The code is written in C++

- GitHub repository: [JellyBit Engine](https://github.com/JellyBitStudios/JellyBitEngine)

#### Game Objects
- Drag and drop game objects in the Hierarchy.
- Create/Delete game objects by right-clicking them at Hierarchy.
- Delete selected game object at Hierarchy by pressing the Delete key.

- Create/Delete components in Inspector.
- Swap/Reorder components in Inspector by drag and drop them through the 'Move' button.
- If more than one camera is set as main, play mode cannot be entered.

#### Resource Manager 
Assets panel:
- The entire Assets folder is shown in real-time at the Assets panel.
- If any file (asset or meta) is removed/moved/renamed/changed from OS (offline mode), click the 'Refresh' button to update the affected resources.
- Dropped files will be imported.
- When importing a model, if it has a texture, the engine will search for it in the Assets folder. If it exists, it will be assigned to the model.

**IMPORTANT: after dropping any file into the engine or changing something in the Assets folder from outside the engine, push REFRESH button at Assets panel.
If the button is not clicked when one of this situations happens, the Assets panel will show the new files but resources will not be updated, so they will not work properly (invalid resource). 
Sorry for the inconvenience!**

- Drag and drop any texture to a material component.
- For models (fbx, dae and obj), open them to see its associated resources and drag and drop them to a mesh component.

- Select any texture or model and see its current import settings at Inspector. Change them and reimport the asset.

Library panel:
- Drag and drop any resource from the Library panel to an equivalent component.

- Select any resource and see its information at Inspector.

#### Scene Serialization
- Each new model in Assets generates a scene. To load the scene, drag and drop it from the Assets panel to the Hierarchy.
- Save the current scene or load a scene from Menu->File->Save Scene/Load Scene.

#### Options
- At Debug Draw panel, change the current debug draw options.
- At Edit panel, change between guizmo operations (shortcuts: W, E, R) and/or modes (shortcut: T).
- At Edit panel, enter Play mode. When entering Play mode, the scene is saved in memory.
- From Play mode, Pause or Tick the simulation.
- When leaving Play mode and entering Editor mode, the scene is loaded from memory.

#### Other
- At Settings panel, Scene section, check the quadtree structure. For debug purposes, random static game objects can be created (they will be automatically added to the quadtree).
- At Settings panel, Time Manager section, check the values of the game clock and the real time clock.


If something is not working as expected, please, revise console panel to understand what is happening.
If, after that, you assume the engine is running an error, revise issues or send a new one (we would be very grateful).

### Controls

#### Camera
- Mouse Hold Right:
	- WASD: move forward, left, backward and right
	- QE: move up and down
	- Look around
- F: look at target
- Hold Alt:
	- Mouse Hold Left: look around target
- Mouse Wheel: zoom in/out
- Mouse Hold Shift: movement speed x2, zoom speed x0.5

#### Windows
- Hold Alt:
	- I: open/close Inspector panel
	- S: open/close Settings panel
	- C: open/close Console panel
	- H: open/close Hierarchy panel
	- A: open/close Assets panel
	- D: open/close DebugDraw panel
	- E: open/close Edit panel

#### Other
- Change ImGuizmo operation/mode:
	- W: Translate operation
	- E: Rotate operation
	- R: Scale operation
	- T: World/Local mode

### Tools used to develop the engine

- IDE: Microsoft Visual Studio 2017
- External libraries: 
	- SDL 2.0.8
	- OpenGL 3.1
	- Glew 2.1
	- Assimp 4.1
	- DevIL 1.8
	- ImGui 1.66 WIP
	- ImGuiColorTextEdit 1.0
	- ImGuizmo 1.0
	- MathGeoLib 1.5
	- PCG Random Number Generator 0.94
	- Parson
	- PhysFS 3.0.1
	- MMGR
	- PhysX
