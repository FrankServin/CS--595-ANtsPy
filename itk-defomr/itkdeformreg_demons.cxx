#include "itkConfigure.h"

#ifndef ITK_USE_FFTWD
#  error "This program needs FFTWD to work."
#endif


#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"

#include "itkCurvatureRegistrationFilter.h"
#include "itkDisplacementFieldTransform.h"
#include "itkFastSymmetricForcesDemonsRegistrationFunction.h"
#include "itkMultiResolutionPDEDeformableRegistration.h"

#include "itkHistogramMatchingImageFilter.h"
#include "itkCastImageFilter.h"
#include "itkLinearInterpolateImageFunction.h"

const unsigned int Dimension = 3;

//  The following section of code implements a Command observer
//  that will monitor the evolution of the registration process.
//
class CommandIterationUpdate : public itk::Command
{
public:
    typedef CommandIterationUpdate Self;
    typedef itk::Command Superclass;
    typedef itk::SmartPointer<CommandIterationUpdate> Pointer ;
    itkNewMacro(CommandIterationUpdate);

protected:
    CommandIterationUpdate() = default;

    typedef itk::Image<float, Dimension> InternalImageType;
    typedef itk::Vector<float, Dimension> VectorPixelType ;
    typedef itk::Image<VectorPixelType, Dimension> DisplacementFieldType;

    typedef tk::CurvatureRegistrationFilter<InternalImageType, InternalImageType, DisplacementFieldType,
        itk::FastSymmetricForcesDemonsRegistrationFunction< InternalImageType, InternalImageType, DisplacementFieldType>> RegistrationFilterType;

public:
    void
        Execute(itk::Object* caller, const itk::EventObject& event) override
    {
        Execute((const itk::Object*)caller, event);
    }

    void
        Execute(const itk::Object* object, const itk::EventObject& event) override
    {
        const auto* filter = static_cast<const RegistrationFilterType*>(object);
        if (!(itk::IterationEvent().CheckEvent(&event)))
        {
            return;
        }
        std::cout << filter->GetMetric() << std::endl;
    }
};


int
main(int argc, char* argv[])
{
    //Verify command line arguments
        if (argc < 4)
        {
            std::cerr << "Usage: " << std::endl;
            std::cerr << argv[0] << " inputMovingImageFile inputFixedImageFile outputRegisteredImageFile" << std::endl;
            return -1;
        }

    typedef short PixelType;
    //setting up the fixed image and the movin image typedefs
    typedef itk::Image<PixelType, Dimension> FixedImageType ;
    typedef itk::Image<PixelType, Dimension> MovingImageType ;
    //setting up the fixed 
    typedef itk::ImageFileReader<FixedImageType> FixedImageReaderType;
    typedef itk::ImageFileReader<MovingImageType> MovingImageReaderType ;
    //set up the moving image and the fixed image 
    FixedImageReaderType::Pointer fixedImageReader = FixedImageReaderType::New();
    MovingImageReaderType::Pointer movingImageReader = MovingImageReaderType::New();
    //set the fixed image as the first input to the program and the moving image as the second input
    fixedImageReader->SetFileName(argv[1]);
    movingImageReader->SetFileName(argv[2]);

    //set up the typedefs for caster for the fixed and moving image respectively
    typedef float InternalPixelType ;
    typedef itk::Image<InternalPixelType, Dimension> InternalImageType;
    typedef itk::CastImageFilter<FixedImageType, InternalImageType> FixedImageCasterType ;
    typedef itk::CastImageFilter<MovingImageType, InternalImageType> MovingImageCasterType ;

    //set up the variables for the image caseter 
    FixedImageCasterType::Pointer fixedImageCaster =  FixedImageCasterType::New();
    MovingImageCasterType::Pointer movingImageCaster =  MovingImageCasterType::New();
    //set the caster to be the inpur fo
    fixedImageCaster->SetInput(fixedImageReader->GetOutput());
    movingImageCaster->SetInput(movingImageReader->GetOutput());

    //set up the variable for the image maching filter
    typedef itk::HistogramMatchingImageFilter<InternalImageType, InternalImageType> MatchingFilterType ;
    MatchingFilterType::Pointer matcher = MatchingFilterType::New();
   
    //iniitalize the matcher (optimizer) hisogram bins, match points and set the threshold to normalize the intensity 
    matcher->SetInput(movingImageCaster->GetOutput());
    matcher->SetReferenceImage(fixedImageCaster->GetOutput());
    matcher->SetNumberOfHistogramLevels(800);
    matcher->SetNumberOfMatchPoints(5);
    matcher->ThresholdAtMeanIntensityOn();

    //This section of the code initilalizes the vector the deformation calculation
    typedef itk::Vector<float, Dimension> VectorPixelType ;
    typedef itk::Image<VectorPixelType, Dimension> DisplacementFieldType ;
    typedef itk::CurvatureRegistrationFilter<
        InternalImageType,
        InternalImageType,
        DisplacementFieldType,
        itk::FastSymmetricForcesDemonsRegistrationFunction<
        InternalImageType,
        InternalImageType,
        DisplacementFieldType>> RegistrationFilterType ;
   
    //
    RegistrationFilterType::Pointer filter = RegistrationFilterType::New();
    filter->SetTimeStep(1);
    filter->SetConstraintWeight(0.1);

    CommandIterationUpdate::Pointer observer = CommandIterationUpdate::New();
    filter->AddObserver(itk::IterationEvent(), observer);

    //set the registration to multidimension to allow for 3D image registration
    typedef itk::MultiResolutionPDEDeformableRegistration<InternalImageType, InternalImageType, DisplacementFieldType> MultiResRegistrationFilterType ;
    MultiResRegistrationFilterType::Pointer multires = MultiResRegistrationFilterType::New();
    multires->SetRegistrationFilter(filter);
    multires->SetNumberOfLevels(2);
    multires->SetFixedImage(fixedImageCaster->GetOutput());
    multires->SetMovingImage(matcher->GetOutput());

    //this is the number of iterations to run the program 
    unsigned int nIterations[4] = { 10, 20, 50, 50 };
    multires->SetNumberOfIterations(nIterations);
    multires->Update();
    //set the displacement field transform for the images. multires allows for multiple image modalitie to be registered 
    typedef itk::DisplacementFieldTransform<InternalPixelType, Dimension> DisplacementFieldTransformType ;
    auto displacementTransform = DisplacementFieldTransformType::New();
    displacementTransform->SetDisplacementField(multires->GetOutput());
    //set up the resampling filter and set the interpolator type (linearInterpolatorType) 
    typedef itk::ResampleImageFilter<MovingImageType,MovingImageType,InternalPixelType, InternalPixelType> WarperType ;
    typedef itk::LinearInterpolateImageFunction<MovingImageType, InternalPixelType> InterpolatorType;
    WarperType::Pointer       warper = WarperType::New();
    InterpolatorType::Pointer interpolator = InterpolatorType::New();
    FixedImageType::Pointer   fixedImage = fixedImageReader->GetOutput();
    //set the warper to act on the fixed image to created a warped filter output to act on the moving image 
    warper->SetInput(movingImageReader->GetOutput());
    warper->SetInterpolator(interpolator);
    warper->SetOutputSpacing(fixedImage->GetSpacing());
    warper->SetOutputOrigin(fixedImage->GetOrigin());
    warper->SetOutputDirection(fixedImage->GetDirection());
    warper->SetTransform(displacementTransform);

    // Write warped image out to file
    typedef unsigned short OutputPixelType ;
    typedef itk::Image<OutputPixelType, Dimension> OutputImageType ;
    typedef itk::CastImageFilter<MovingImageType, OutputImageType> CastFilterType;
    typedef itk::ImageFileWriter<OutputImageType> WriterType ;

    WriterType::Pointer     writer = WriterType::New();
    CastFilterType::Pointer caster = CastFilterType::New();
   //set the third argument as the 
    writer->SetFileName(argv[3]);
    //apply the warper to get the output 
    caster->SetInput(warper->GetOutput());
    writer->SetInput(caster->GetOutput());
    writer->Update();


    return EXIT_SUCCESS;
}