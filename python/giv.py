# Some routines for accessing giv from python.
#
# Dov Grobgeld <dov.grobgeld@gmail.com>
# 2017-08-06 Sun

import tempfile
import os

GivFilename = tempfile.mktemp(suffix=".giv")

GivSearchPathNt = ['c:/devtools/',
                   'd:/devtools/',
                   'c:/Program Files/',
                   'c:/Program Files (x86)/',
                   # Add additional potential search paths for giv
                   ]

def RunGiv(Filename):
  """Cross platform execution of giv command"""
  if os.name in ['nt']:
    # Search for giv in likely places
    for d in GivSearchPathNt:
      GivPath = d+"Giv/bin/giv.exe"
      if os.path.exists(GivPath):
        os.spawnl(os.P_NOWAIT, GivPath, '"'+GivPath+'"', Filename)
        return
    raise Exception("Failed finding giv executable!")
  else:
    # On Linux, just rely on giv being in the path
    os.system("giv " + Filename + "&")

def RunGxgraph(Filename):
  """Cross platform execution of giv command"""
  if os.name in ['nt']:
    # Search for giv in likely places
    for d in GivSearchPathNt:
      GivPath = d+"GxGraph/bin/gxgraph.exe"
      if os.path.exists(GivPath):
        os.spawnl(os.P_NOWAIT, GivPath, '"'+GivPath+'"', Filename)
        return
    raise Exception("Failed finding gxgraph executable!")
  else:
    # On Linux, just rely on giv being in the path
    os.system("gxgraph " + Filename + "&")

def Transform(Points, Aff):
  import euclid
  if Aff is None:
    return Points
  TPoints = []
  for P in Points:
    P = Aff * euclid.Point2(*P)
    
    TPoints.append([P[0],P[1]])
  return TPoints

# The following routines are used for generating some geometric
# shapes.
def GetRectanglePoints(X0=0,Y0=0,Width=156, Height=156, Aff=None):
  giv_epsilon = 1e-5  # giv workaround!

  Points = []

  w,h = Width,Height
  Points = [ [ X0, Y0],
             [ X0+w, Y0],
             [ X0+w, Y0+h],
             [ X0, Y0+h],
             [ X0, Y0+giv_epsilon ] ]

  return Transform(Points,Aff)

def PolyToString(Points,
                 Color = 'red',
                 OutlineColor=None,
                 Marks = None,
                 Fill = False,
                 Balloon = None,
                 Path = None,
                 LineWidth = None):
  GivString = '$color ' + Color + '\n'
  if Marks is not None:
    GivString += '$marks ' + Marks
  if Fill:
    GivString += '$polygon\n'
    if OutlineColor is not None:
      GivString += "$outline_color " + OutlineColor + "\n"
  if Balloon is not None:
    GivString += '$balloon ' + Balloon.replace('\n','\n$balloon ') + '\n'
  if Path is not None:
    GivString += '$path ' + Path +'\n'
  if LineWidth is not None:
    GivString += '$lw ' + str(LineWidth) +'\n'
  for p in Points:
    GivString += '%f %f\n'%(p[0], p[1])
  return GivString + '\n'

def ViewPoints(Points,
               NoLine=True,
               Color="red"):
  """Utility command for seeing a point set"""
  fh = open(GivFilename, "w")
  fh.write("$marks fcircle\n"
           "$color %s\n"%Color)
  if NoLine:
    fh.write("$noline\n")
  for p in Points:
    fh.write("%f %f\n"%(p[0], p[1]))
  fh.close()
  RunGiv(GivFilename)

def ViewGivString(GivString):
  open(GivFilename, "w").write(GivString)
  RunGiv(GivFilename)
  
def ViewPointSets(PointSets,
                  NoLine=True,
                  Colors=None):
  """Utility function for easily seing a list of points"""
  if Colors is None:
    Colors = ["red","green","blue"]

  fh = open(GivFilename, "w")
  for i,Points in enumerate(PointSets):
    fh.write("$marks fcircle\n"
             "$color %s\n"%Colors[i%len(Colors)])
    if NoLine:
      fh.write("$noline\n")
    for p in Points:
      fh.write("%f %f\n"%(p[0], p[1]))
    fh.write("\n")
  fh.close()
  RunGiv(GivFilename)

def StringToBalloon(BalloonString):
  """Convert a string to a multi-line Giv Balloon string"""
  GivString = ''
  for b in BalloonString.split("\n"):
    GivString += "$balloon " + b + "\n"
  return GivString

def GivRectString(X,Y,Width,Height,
                  Color=None,
                  OutlineColor=None,
                  Balloon=None,
                  Path=None,
                  Fill=False,
                  LineWidth=None):
  """Create a giv string of a rectangle"""
  GivString = ""
  if Color is not None:
    GivString += "$color " + Color + "\n"
  if Balloon is not None:
    GivString += StringToBalloon(Balloon)
  if Path is not None:
    GivString += "$path " + Path + "\n"
  if OutlineColor is not None:
    GivString += "$outline_color " + OutlineColor + "\n"
    GivString += "$color " + OutlineColor + "\n"
  if Fill:
    GivString += "$polygon\n"
  if LineWidth is not None:
    GivString += "$lw %f\n"%LineWidth

  GivString += ("%f %f\n"
                "%f %f\n"
                "%f %f\n"
                "%f %f\n"
                "%f %f\n\n"%(X,Y,
                             X+Width, Y,
                             X+Width, Y+Height,
                             X, Y+Height,
                             X,Y))
  return GivString

def GivPolygonString(Points,Color=None,OutlineColor=None,Balloon=None,Path=None,Fill=False,
                     LineWidth=None):
  """Create a giv string of a polygon"""
  GivString = ""
  if Color is not None:
    GivString += "$color " + Color + "\n"
  if Balloon is not None:
    GivString += StringToBalloon(Balloon)
  if Path is not None:
    GivString += "$path " + Path + "\n"
  if OutlineColor is not None:
    GivString += "$outline_color " + OutlineColor + "\n"
    GivString += "$color " + OutlineColor + "\n"
  if Fill:
    GivString += "$polygon\n"
  if LineWidth is not None:
    GivString += "$lw %f\n"%LineWidth

  GivString += '\n'.join(["%f %f"%(x,y) for x,y in Points]) + '\n\n'

  return GivString

def GivMultiPolygonString(MultiPoints,Color=None,OutlineColor=None,Balloon=None,Path=None,Fill=False,
                          LineWidth=None):
  """Create a giv string of a polygon"""
  GivString = ""
  if Color is not None:
    GivString += "$color " + Color + "\n"
  if Balloon is not None:
    GivString += StringToBalloon(Balloon)
  if Path is not None:
    GivString += "$path " + Path + "\n"
  if OutlineColor is not None:
    GivString += "$outline_color " + OutlineColor + "\n"
    GivString += "$color " + OutlineColor + "\n"
  if Fill:
    GivString += "$polygon\n"
  if LineWidth is not None:
    GivString += "$lw %f\n"%LineWidth

  for Points in MultiPoints:
    GivString += 'm ' + '\n '.join(["%f %f"%(x,y) for x,y in Points]) + '\n'
  GivString += '\n'

  return GivString

if __name__ == "__main__":
  s = GivRectString(10,20,15,15, Balloon="A red rectangle")
  s += GivRectString(15,22,15,15,Color="green", Balloon="A multi\nline green\nRectangle")
  fh = open("/tmp/foo.giv","w")
  fh.write(s)
  fh.close()
  RunGiv("/tmp/foo.giv")
  RunGxgraph("/tmp/foo.giv")
