# This is a sample Python script.

# See PyCharm help at https://www.jetbrains.com/help/pycharm/
import scipy
import ants
import os, sys

#directory with the input files
file_path = "/home/frankservin/Documents/ITK_ANTS_Project/KKI2009-ALL-MPRAGE/"
#directory for the output files
file_path2 = "/home/frankservin/Documents/KK12009-ALL-MPRAGE-SYNabp/"

from os import listdir
from os.path import isfile, join
onlyfiles = [f for f in listdir(file_path) if isfile(join(file_path, f))]
#reading in the fixed image
fixedImage=ants.image_read("/home/frankservin/Documents/ITK_ANTS_Project/KKI2009-ALL-MPRAGE/KKI2009-01-MPRAGE.nii.gz");
file_path_m = []
file_path2_m = [];
antsObj = []

#reading in the images into an object 
print(onlyfiles)
for i in onlyfiles:
    name = file_path + "/" + i
    name2 = file_path2 + "/" + i
    file_path_m.append(name)
    file_path2_m.append(name2)
    antsObj.append((ants.image_read(file_path+"/"+i)))

    registeredImages = []
#preforming the registration. 
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
    #ants.image_write(registeredImages[i]["warpedmovout"], file_path_m[i] + "deformed.nii")
    ants.image_write(mytx["warpedmovout"], file_path2_m[i], ri=True)
    #file should be written to the output directory
    print('File_written')



