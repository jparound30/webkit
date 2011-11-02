README for WebCL/WebKit
=======================

Tested platform
---------------

- OS: Mac OSX Lion ONLY
- GPU: Nvidia GPU with OpenCL 1.0 support

How to build
------------

1. clone repository or download a zipped file to your work directory
   (let's say, ~/work).
2. cd webkit
3. Tools/Scripts/build-webkit

How to run samples
------------------

1. Enable WebGL
        For older Safari < 5,
         $ defaults write com.apple.Safari WebKitWebGLEnabled -bool YES
        For newer Safari 5.1,
         you need to enable 'Developer' menu from 'Preferences' and check 'Enable WebGL' under the menu.
        The screenshot of this process is available at
         http://fairerplatform.com/2011/05/new-in-os-x-lion-safari-5-1-brings-webgl-do-not-track-and-more/.

2. Hello example:
        $ Tools/Scripts/run-safari Examples/WebCL/Hello/index.html
3. N-body example:
        $ Tools/Scripts/run-safari Examples/WebCL/Nbody/index.html
4. Deform example:
        $ Tools/Scripts/run-safari Examples/WebCL/Deform/index.html

