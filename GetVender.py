#downloads the vender packages needed for compiling.
#GitPython is required to be installed via Pip

from git import Repo
import os

#clones a repo into vender
def GitClone(URL, name, branch = "master"):
     #checks if Vender folder exists
    if not os.path.exists("Venders"):
        os.mkdir("Venders")

    #if it already exists, don't clone it
    if not os.path.exists("Venders/" + name):
        print("Cloning " + name + ".....")
        Repo.clone_from(URL, "Venders/" + name, branch = branch)
        print("Done cloning " + name)

#not implemented