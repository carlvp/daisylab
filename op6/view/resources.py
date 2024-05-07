'''Resources, which are related to the view (i.e. images)'''

import tkinter

_base64_encodings={
# 16th-note rest, 16x24, black on transparent  
'rest16.png': '''
iVBORw0KGgoAAAANSUhEUgAAABAAAAAYCAYAAADzoH0MAAAABHNCSVQICAgIfAhkiAAAAAlwSFlz
AAAOxAAADsQBlSsOGwAAABl0RVh0U29mdHdhcmUAd3d3Lmlua3NjYXBlLm9yZ5vuPBoAAAFDSURB
VDiNldS7LgVRFMbxn0vcH0A0GqVOqyORUEiIRFS8hcYbKIRCoSLRuBYKicoDUGld4lIQiUgQidtR
nBE7Y81xzpeszMxe6/vvNXv27AaxejGZXa/wWlAXahZfKGVxi+5cTR9WInN3zvwTi0nNDN7wGAEG
AnMJ+xjEVjK2GwF6CwBRTEYA2KnC/IKOIkAj5nCRFd/gEKcJYKPIXEkLCWA8KpjCEa6xjPYkV591
UsITWvPmaX/fcy3JDyXj69HsZwHgHU1Z/iAZH40AzwGghCXsJc+PaI4A2wWAfKxGZujCcRWA4byx
LrlvwRj6s+JLfGI+yz+gU3ltqtZEMnv49/2njQQwWKu5ze/XuVPe4n9UXwEw4nc3buKj1g7S9vtr
NaftX6rQaVEibX9V+ZirST/tf6GnUmG4sjjBOe6VD5JCfQNiqIqa/q/p2wAAAABJRU5ErkJggg==
''',

# cross (x, delete), 24x24, red on transparent
'x.png': '''
iVBORw0KGgoAAAANSUhEUgAAABgAAAAYCAYAAADgdz34AAAABHNCSVQICAgIfAhkiAAAAAlwSFlz
AAAOxAAADsQBlSsOGwAAABl0RVh0U29mdHdhcmUAd3d3Lmlua3NjYXBlLm9yZ5vuPBoAAAD0SURB
VEiJvZZJEoIwEEWf3sOdN3DjrOWw8lS608NaDqjsvptgpWIggaCpYtXd70GT0IC1BH0SVylDsBfk
gl0CfCt4Co5u4CCQuRpJLHjBORWBnuBiBWQS1zXgawcuw+wVCQPB2SPZRMDngsypvQqGbuLIBKKf
RDAT3J2aTDAvK/BJHj5JbXiEZBUBXwQ6+gGMBTefRDBNggckL89uyQTLWvCApB24JZmY9rjwPAbe
TbJDJ6na3H1Vix5t97+dl9xwm8ZJquBWTrOzEHOKrdx6p7kCnv6xK4FHzYSgRH8aOHsHHhw0Hok7
eL7m8u+GvpXQ+m/LG157wcNUA8hlAAAAAElFTkSuQmCC''',

# op6 icon, 64x64, black, white and transparent
'op6-64x64.png': '''
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''',    
}

# Keep a reference to the images: works-around a known tkinter-bug
# see https://github.com/ythy/blog/issues/302
_photoImageCache={}

def getPhotoImage(name):
    img=_photoImageCache.get(name)
    if img is None:
        data=_base64_encodings[name]
        img=tkinter.PhotoImage(data=data)
        _photoImageCache[name]=img
    return img
