VolRend Demo/Example
--------------------

Usage:

    volrend filename

-----------------------------------------------------------------------

Config File Parameters:

  VOLREND_filename      <str>  #Def = 3dhead.ldm

  VOLREND_METHOD        <str>  #Def = AUTO else TEX2D or TEX3D
  VOLREND_cameraType    <str>  #Def = Perspective (else Orthographic)

  # Optional displays
  VOLREND_showBBox      <bool> #Def = 0
  VOLREND_showAxes      <bool> #Def = 0
  VOLREND_showHistogram <bool> #Def = 1
  VOLREND_showColormap  <bool> #Def = 1
  VOLREND_showText      <bool> #Def = 1
  VOLREND_tileOutline   <bool> #Def = 0 (tiles in tex memory)
  VOLREND_dataOutline   <bool> #Def = 0 (tiles in main memory)

  # Select number of ortho slices to create initially
  VOLREND_numOrthoSlice <int>  #Def = 1

  # Transfer function
  VOLREND_colorMap      <int> #Def = 7 (SEISMIC) 0 means read colorMapFile
  VOLREND_colorMapFile  <str> #Def = "BlueWhiteRed.txt" (Read if colorMap == 0)
  VOLREND_sliceColorMap <int> #Def = 2 (TEMPERATURE)
  VOLREND_sliceColorMapFile  <str> #Def = "BlueWhiteRed.txt" (Read if sliceColorMap == 0)
  VOLREND_minColorMap   <int> #Def = 0
  VOLREND_maxColorMap   <int> #Def = 255
  VOLREND_minOpaqueMap  <int> #Def = 0
  VOLREND_maxOpaqueMap  <int> #Def = 255

  # Rendering options
  VOLREND_globalAlpha       <float> #Def = 0.3
  VOLREND_lighting          <bool>  #Def = 0
  VOLREND_viewAlignedSlices <bool>  #Def = 0
  VOLREND_numSlicesControl  <int>   #Def = 2  // AUTOMATIC
  VOLREND_numSlices         <int>   #Def = -1 // not used by default
  VOLREND_viewCulling       <bool>  #Def = 1
  VOLREND_viewPointRefine   <bool>  #Def = 1
  VOLREND_sliceEqualRes     <bool>  #Def = 0
  VOLREND_moveLowRes        <bool>  #Def = 0

  # LDM performance settings
  VOLREND_mainMemSize       <int>   #Def =  256 // 1 - 512 MBytes
  VOLREND_mainMemTiles      <int>   #Def = 1024 // 1 - 2048
  VOLREND_texMemSize        <int>   #Def =   48 // 1 - 128 MBytes
  VOLREND_texMemTiles       <int>   #Def =  192 // 1 - 512
  VOLREND_texLoadRate       <int>   #Def =    1 // 0 - 64
  VOLREND_sliceNumTex       <int>   #Def =  256 // 1 - 1024
  VOLREND_sliceTexLoadRate  <int>   #Def =   64 // 1 - 256

  # CAVELib only
  VOLREND_caveSceneSize <float> #Def = 10
  VOLREND_caveScenePos  <vec3f> #Def = 0 0 0
  VOLREND_cavePanIncr   <float> #Def = 0.01  // translation increment
  VOLREND_caveRotIncr   <float> #Def = 0.01  // rotation increment

-----------------------------------------------------------------------

SPACEMOUSE:

 Support and drivers for the SpaceMouse are available at: http://www.3dconnexion.com/

 NOTE: SpaceMouse events are not handled when the viewer is in "viewing"
       mode (when the "hand" cursor is visible).  You must switch to
       selection mode to use the SpaceMouse (press ESC key).

 By default, you are in rotation-only mode.

 Buttons:
 1 - Rotation-only mode
 2 - Translation-only mode
 3 - Translation and rotation mode
 4 - Color map mode
 5 - Reset to home

 Reset to home: because of the way the spaceMouse is handled by Open Inventor, the viewer's
 reset to home button of Open Inventor will not work. You will have to use button 5 instead.

 In color map mode, moving the SpaceMouse:
  - up/down applies the next color map to the volume.
  - left/right changes the value of the minimum value of the color map.
  - front/back changes the value of the maximum value of the color map.

-----------------------------------------------------------------------

CAVELib Keyboard Interface:

Translation:
  H - left
  J - down
  J - up
  L - right
  N - in
  M - out

  Space - reset
  Shift+any - faster translation

Rotation:
  U - left
  I - right

Auto-rotate:
  A - Spin (each press increases speed)
  Shift+A - Stop

Slices:
  S - on
  Shift+S - off

Volume:
  V - on
  Shift+V - off

ROI
  R - on
  Shift+R - off
