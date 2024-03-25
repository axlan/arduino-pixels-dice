import os
import shutil

Import("env")

example_target = env.GetProjectOption("custom_example_target")

shutil.copy(os.path.join("examples", example_target, example_target + ".ino"), os.path.join("src"))
