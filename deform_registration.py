# This is a sample Python script.

# Press Shift+F10 to execute it or replace it with your code.
# Press Double Shift to search everywhere for classes, files, tool windows, actions, and settings.


def print_hi(name):
    # Use a breakpoint in the code line below to debug your script.
    print(f'Hi, {name}')  # Press Ctrl+F8 to toggle the breakpoint.


# Press the green button in the gutter to run the script.
if __name__ == '__main__':
    print_hi('PyCharm')

# See PyCharm help at https://www.jetbrains.com/help/pycharm/
import scipy
import ants
import os, sys


file_path = "/home/frankservin/Documents/ITK_ANTS_Project/KKI2009-ALL-MPRAGE/"

file_path2 = "/home/frankservin/Documents/KK12009-ALL-MPRAGE-SYNabp/"



from os import listdir
from os.path import isfile, join
onlyfiles = [f for f in listdir(file_path) if isfile(join(file_path, f))]
#reading in the fixed image
fixedImage=ants.image_read("/home/frankservin/Documents/ITK_ANTS_Project/KKI2009-ALL-MPRAGE/KKI2009-01-MPRAGE.nii.gz");
file_path_m = []
file_path2_m = [];
antsObj = []


print(onlyfiles)
for i in onlyfiles:
    name = file_path + "/" + i
    name2 = file_path2 + "/" + i
    file_path_m.append(name)
    file_path2_m.append(name2)
    antsObj.append((ants.image_read(file_path+"/"+i)))

    registeredImages = []

for i in range(len(antsObj)):
    #registering the image
    print(i)
    print(file_path2_m[i])
    #moving_image = ants.image_read(antsObj[i])
    mytx = (ants.registration(fixed=fixedImage, moving=antsObj[i], type_of_transform="SyNabp",
                              aff_metric="mattes", syn_metric="meansquares"))
    #mywarpedimage = ants.apply_ants_transform(fixed=fixedImage, moving=antsObj[i],
    #                                         transformlist=mytx['fwdtransforms'])
    print('Registered_image')

#for i in range(len(file_path2_m)):
    #filepath of the moving image
    #should decrement
    #ants.image_write(registeredImages[i]["warpedmovout"], file_path_m[i] + "deformed.nii")
    ants.image_write(mytx["warpedmovout"], file_path2_m[i], ri=True)
    print('File_written')
#complete image registration


