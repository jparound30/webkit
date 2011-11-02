README for WebCL/WebKit

---------------
Tested platform
---------------
- OS: Mac OSX
- GPU: Nvidia GPU with OpenCL 1.0 support
- GPU: AMD GPU with OpenCL 1.0 support

---------------------
Tested WebKit version
---------------------
r92365

------------
How to build
------------
1. Download a zipped file from Downloads menu to your work directory 
   (let's say, ~/work).
2. tar -zvxf webcl-20110825.tar 
3. cd webkit
4. Tools/Scripts/build-webkit

------------------
How to run samples
------------------
1. Enable WebGL
  For older Safari < 5,
	$ defaults write com.apple.Safari WebKitWebGLEnabled -bool YES
  For newer Safari 5.1, you need to enable 'Developer' menu from 'Preferences' and check 'Enable WebGL' under the menu.
  The screenshot of this process is available at http://fairerplatform.com/2011/05/new-in-os-x-lion-safari-5-1-brings-webgl-do-not-track-and-more/.
  
2. Hello example:
	$ Tools/Scripts/run-safari Examples/WebCL/Hello/index.html
3. N-body example:
	$ Tools/Scripts/run-safari Examples/WebCL/Nbody/index.html
4. Deform example:
	$ Tools/Scripts/run-safari Examples/WebCL/Deform/index.html
   
-----------------------------------
How to update source code using svn
-----------------------------------
1. cd ~/work
2. svn checkout https://webcl.googlecode.com/svn/trunk/ webcl --usename <you_email_address>
3. cp -Ri webcl/trunk/* webkit
