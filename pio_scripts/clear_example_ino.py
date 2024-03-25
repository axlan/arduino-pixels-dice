import glob
import os

# Clear any example *.ino files.
for ino in glob.glob("src/*.ino"):
  os.remove(ino)
