# VLC with Stereoscopic 3D Playback

This was my Google Summer of Code 2017 Project and the following is a consice description of the same.
As part of the "Adding 3D support" project for VLC, I had various goals to attain:
## Primary Goal:
Core playback support for tagged 3D files (Side-by-side & Top-Bottom). 
## Secondary Goals:
1. Playback support for libVLC (Windows Universal Platform)
2. Playback support for Stereoscopic 3D + 360 files 
3. Side-by-side output for 2D files
4. "Cardboard" output for VR viewing of 3D files

The core playback is based on DirectX11 APIs to support stereoscopic 3D playback.
As part of the implementation, a new menu option "3D Output" is available under Video with various options.
### 3D Output Menu Options:
1. Auto-detect - If a 3D file is detected, and 3D is enabled on the system, the output is stereoscopic 3D.
2. Stereo - Forcing a 3D output.
3. Left only - Showing only the frames for the left in 2D (Left frame for Side-by-side format, Top frame for Top-Bottom format)
4. Right only - Like Left only but with right/bottom frame.
5. Side-by-side - Shows a 2D file side by side, to for example show 2D on 3D TVs. 3D files are shown as the original in 2D.
6. Cardboard - Outputs 2D and 3D files with a lens distoriton to be displayed as stereoscopic on Cardboard (VR) headsets.
Stereoscopic 3D + 360 files are displayed in 3D directly on automatic detection.

### Demo of the main options:
![VLC Stereo Demo](https://github.com/theShaan/VLC-Stereoscopic-3D-Playback/blob/FinalSubmission/VLC%20Stereo.gif)

Please refer to individual commits for a patch-by-patch breakdown of the project.
To run, either fork the project or use the individual patches.

#### The current status of the project is
- [x] Primary milestone coding and testing
- [x] All secondary milestones coding and testing
- [x] Rebased to latest version (26th August, 2017). Fully tested. 
