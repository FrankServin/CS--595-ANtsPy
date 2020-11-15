#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkDivideImageFilter.h"
#include "itkNaryAddImageFilter.h"
#include  <string.h> //double bracket does not print here

int
main(int argc, char* argv[])
{
    // Setup types
    const unsigned int nDims = 3;
    typedef itk::Image< double, nDims > ImageType;


    // Create and setup readers for images:
    typedef itk::ImageFileReader< ImageType >  readerType;
    readerType::Pointer readers[21];

    for (int i = 1; i <= 21; i = i + 1)
    {
        std::string  digit = "";
        if (i < 10)
        {
            digit = "0";
        }
        readers[i - 1] = readerType::New();
        //C:/Users\Frank\Documents\1.0 Vanderbilt\CS Medical Imaging\ITK_ANTS_Project\KKI2009-ALL-MPRAGE
        //readers[i - 1]->SetFileName("../KKI2009-ALL-MPRAGE/KKI2009-" + digit + std::to_string(i) + "-MPRAGE.nii");
        readers[i - 1]->SetFileName("../KKI2009-ALL-MPRAGE/KKI2009-" + digit + std::to_string(i) + "-MPRAGE.nii");
        readers[i - 1]->Update();
    }

    //Create a filter that allows the summation of 3D images
    typedef itk::NaryAddImageFilter<ImageType, ImageType>NAddImageFilterType;
    NAddImageFilterType::Pointer nAddImage = NAddImageFilterType::New();
    for (int i = 0; i <= 20; i++)
    {
        nAddImage->SetInput(i, readers[i]->GetOutput());
    }
    nAddImage->Update();


    typedef itk::DivideImageFilter<ImageType, ImageType, ImageType>DivideImageFilterType;
    DivideImageFilterType::Pointer divider = DivideImageFilterType::New();
    divider->SetInput(nAddImage->GetOutput());
    divider->SetConstant(21.0);
    divider->Update();


    // write out the result to:
    typedef itk::ImageFileWriter < ImageType > writerType;
    writerType::Pointer writer = writerType::New();
    //writer->SetInput ( divider->GetOutput());
    writer->SetFileName("Sum21.nii");
    writer->SetInput(divider->GetOutput());
    writer->Update();


    // Done.
    return 0;
}