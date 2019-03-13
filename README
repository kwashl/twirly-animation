* Dependencies
  - If you are using linux/OSX, the script build_deps_<linux/osx>.sh should fetch and extract the
    required dependencies.
    - Use the 4 export commands at the end to export environment variables.
  - GLM (http://glm.g-truc.net): get the source code.
  - GLFW (http://www.glfw.org): get the source code.
  - GLEW (http://glew.sourceforge.net): get the source code for macOS/Linux, binaries for Windows.
  - Set the environment variables: GLEW_INCLUDE_DIR, GLFW_DIR, GLEW_DIR
  - Alternatively, use http://courses.cs.tamu.edu/sueda/CSCE441/2018S/labs/L00/ for reference on
    setting up the environment.
    - Check the "Setting Up" section for the OS you are using.

* Build instructions
  - Make a build directory and run CMake for the project after setting the above environment variables.
  - Set environment variables as above (You can use the source command for the file env.sh)
  - For linux(similar for OSX) it looks as follows:
    ./build-deps-linux.sh
    source env.sh
    mkdir build
    cd build
    cmake ../
    make -j4

* Usage
  - ./A4 <SHADER DIR> <MESH FILE> <ATTACHMENT FILE> <SKELETON FILE> <ANIM FILE>
  - <ANIM FILE> is optional. If not provided, no animation is loaded.
  - Example:
  - ./A4 ../resources ../cheb/cheb.obj ../cheb/cheb_attachment.txt ../cheb/cheb_skeleton.txt
  - ./A4 ../resources ../cheb/cheb.obj ../cheb/cheb_attachment.txt ../cheb/cheb_skeleton.txt ../cheb/test_anim.txt

* Hierarchy
  - Animation is performed by moving and rotating the root, and rotating joints to orient child bones.
  - The mesh has 18 bones with 17 joints. These numbers are provided in cheb_skeleton.txt.

* Key press events
  |---------+-----------------------------------------------|
  | Key     | Purpose                                       |
  |---------+-----------------------------------------------|
  | 'n'     | No mode. Clear selections.                    |
  | 'f'     | Record frame for playback.                    |
  | 'g'     | Toggle GPU skinning.                          |
  | 'b'     | Bone selection mode.                          |
  | 'j'     | Joint selection mode                          |
  | 'a'     | Axis selection mode                           |
  | '.'     | Complete selection.                           |
  | 'p'     | Playback of loaded/recorded animation.        |
  | '<'/'>' | Moves body along the selected axis.           |
  | '+'/'-' | Rotate/Move selected axis                     |
  | '='     | Clear rotation/movement of joints             |
  | 'r'     | Reset all joints                              |
  | '0'-'9' | Editing numbers for joint and bone selection. |
  |---------+-----------------------------------------------|

** Joint rotation workflow
   - Focus on OpenGL window.
   - Joint selection mode with 'j'.
   - Pick number (say '12' for joint index 12).
   - Use '.' to complete selection.
   - Select axis with 'a' followed by axis number (0-2) and '.'.
   - Rotate with '+'/'-'.
   - Example: 'j2.a2.+++':  Rotate right scapula about the z-axis 3 times.

** Visualizing weights
   - Bone selection mode with 'b'.
   - Pick bone number followed by '.'.
   - Example: 'b3.': Visualize head weights.

** How to record frame?
   - Move to position and orientation that you want.
   - Record frame with 'f'.
   - stdout output is in the format accepted by loadAnim function.
     - rootQuat rootPos joint1Quat joint2Quat .... jointnQuat

* Assignment description
  Fill in the required parts of the code (marked with TODO comments) to perform the following
  operations:
  - Linear blend skinning using CPU.
  - Linear blend skinning using GPU.
  - Quaternion interpolation using Catmull-Rom curves between frames.
  - Create a simple animation.

* Acknowledgements
  - The base code for this assignment was provided by Prof. Shinjiro Sueda at Texas A&M University
    (http://faculty.cs.tamu.edu/sueda/).
  - The mesh was initially created using Cosmic Blobs software developed by Dassault Systemes
    SolidWorks Corp.
