#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"

//#include "itkImageRegistrationMethod.h"
#include "itkMultiResolutionImageRegistrationMethod.h"
#include "itkAffineTransform.h"
#include "itkMeanSquaresImageToImageMetric.h"
#include "itkRegularStepGradientDescentOptimizer.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkResampleImageFilter.h"

#include <itkMattesMutualInformationImageToImageMetric.h>

#include "itkCommand.h"

const unsigned int nDims = 3;
typedef itk::Image < int, nDims > ImageType;

class OptimizerIterationCallback : public itk::Command
{
public:
    // itk set up such that things like pointers and new() are available for this class
    //OptimizerIterationCallback::Pointer myCallback = OptimizerIterationCallback::New() ;
    // standard things we need in every itk-inheriting class
    typedef OptimizerIterationCallback Self;
    typedef itk::Command Superclass;
    typedef itk::SmartPointer<OptimizerIterationCallback> Pointer;
    itkNewMacro(OptimizerIterationCallback);

    // specific typedefs we need for our observer
    typedef itk::RegularStepGradientDescentOptimizer OptimizerType;
    typedef const OptimizerType* OptimizerPointerType;

    // if i want to change things in my caller
    void
        Execute(itk::Object* caller, const itk::EventObject& event)
    {
        Execute((const itk::Object*)caller, event);
    }

    // if i am just observing the caller (no changes)
    void
        Execute(const itk::Object* caller, const itk::EventObject& event)
    {
        // somehow get my hands on the optimizer 
        // caller is the optimizer, but it's of the wrong type, but i know it is an optimizer so i can cast it to the optimizer type
        OptimizerPointerType optimizer = dynamic_cast <OptimizerPointerType> (caller);
        std::cout << optimizer->GetCurrentIteration() << " " << optimizer->GetValue() << std::endl;
    }
};

class RegistrationIterationCallback : public itk::Command
{
public:
    // itk set up such that things like pointers and new() are available for this class
    //OptimizerIterationCallback::Pointer myCallback = OptimizerIterationCallback::New() ;
    // standard things we need in every itk-inheriting class
    typedef RegistrationIterationCallback Self;
    typedef itk::Command Superclass;
    typedef itk::SmartPointer<RegistrationIterationCallback> Pointer;
    itkNewMacro(RegistrationIterationCallback);

    // specific typedefs we need for our observer
    typedef itk::RegularStepGradientDescentOptimizer OptimizerType;
    typedef OptimizerType* OptimizerPointerType;

    typedef itk::MultiResolutionImageRegistrationMethod < ImageType, ImageType > RegistrationMethodType;
    typedef RegistrationMethodType* RegistrationPointerType;

    // if i want to change things in my caller
    void
        Execute(itk::Object* caller, const itk::EventObject& event)
    {
        // somehow get my hands on the registration method 
        // caller is the registration method
        RegistrationPointerType registration = dynamic_cast <RegistrationPointerType> (caller);
        std::cout << "Level: " << registration->GetCurrentLevel() << std::endl;

        OptimizerPointerType optimizer = dynamic_cast <OptimizerPointerType> (registration->GetModifiableOptimizer());

        optimizer->SetMaximumStepLength(optimizer->GetMaximumStepLength() * 0.5);

    }

    // if i am just observing the caller (no changes)
    void
        Execute(const itk::Object* caller, const itk::EventObject& event)
    {
        // nothing
    }
};

int main()
{
    Verify command line arguments
        if (argc < 4)
        {
            std::cerr << "Usage: " << std::endl;
            std::cerr << argv[0] << " inputMovingImageFile inputFixedImageFile outputRegisteredImageFile" << std::endl;
            return -1;
        }

    // Setup types
    const unsigned int nDims = 3;
    typedef itk::Image < int, nDims > ImageType;
    typedef itk::ImageFileReader < ImageType > ImageReaderType;
    typedef itk::ImageFileWriter < ImageType > ImageWriterType;

    // read in the first image
    ImageReaderType::Pointer movingReader = ImageReaderType::New();
    movingReader->SetFileName(argv[2]);
    movingReader->Update();

    // read in the second image
    ImageReaderType::Pointer fixedReader = ImageReaderType::New();
    fixedReader->SetFileName(argv[2]);
    fixedReader->Update();

    // Registration!

    // set up typedefs (don't forget to include the header files)
    //  typedef itk::ImageRegistrationMethod < ImageType, ImageType > RegistrationMethodType ;
    typedef itk::MultiResolutionImageRegistrationMethod < ImageType, ImageType > RegistrationMethodType;
    //
    typedef itk::AffineTransform < double, 3 > AffineTransformType; // leaving at default values, could skip it
    typedef itk::MattesMutualInformationImageToImageMetric< ImageType, ImageType > MetricType;
    typedef itk::RegularStepGradientDescentOptimizer OptimizerType; // no template arguments
    typedef itk::LinearInterpolateImageFunction < ImageType > InterpolatorType;
    typedef itk::ResampleImageFilter < ImageType, ImageType > ResampleFilterType;

    // declare the variables
    RegistrationMethodType::Pointer registrationMethod = RegistrationMethodType::New();
    AffineTransformType::Pointer transform = AffineTransformType::New();
    MetricType::Pointer metric = MetricType::New();
    OptimizerType::Pointer optimizer = OptimizerType::New();
    InterpolatorType::Pointer interpolator = InterpolatorType::New();

    // connect the pipeline
    registrationMethod->SetMovingImage(movingReader->GetOutput());
    registrationMethod->SetFixedImage(fixedReader->GetOutput());
    registrationMethod->SetOptimizer(optimizer);
    registrationMethod->SetMetric(metric);
    registrationMethod->SetInterpolator(interpolator);
    registrationMethod->SetTransform(transform);

    // set up the relevant parameters
    optimizer->MinimizeOn();
    optimizer->SetNumberOfIterations(20);

    std::cout << "Min: " << optimizer->GetMinimumStepLength() << std::endl;
    std::cout << "Max: " << optimizer->GetMaximumStepLength() << std::endl;
    std::cout << "Current: " << optimizer->GetCurrentStepLength() << std::endl;

    optimizer->SetMinimumStepLength(0);
    optimizer->SetMaximumStepLength(0.05); // TODO: might want to adjust this some more
    transform->SetIdentity();
    registrationMethod->SetInitialTransformParameters(transform->GetParameters());


    // set up the callback function
    OptimizerIterationCallback::Pointer myCallback = OptimizerIterationCallback::New();
    optimizer->AddObserver(itk::IterationEvent(), myCallback);

    // set up the second callback for the registration method
    RegistrationIterationCallback::Pointer myRegCallback = RegistrationIterationCallback::New();
    registrationMethod->AddObserver(itk::IterationEvent(), myRegCallback);

    registrationMethod->SetNumberOfLevels(3);
    registrationMethod->SetFixedImageRegion(fixedReader->GetOutput()->GetLargestPossibleRegion());

    // run the registration
    // TODO: put this in a try-catch block
    registrationMethod->Update();

    // why did it stop?
    std::cout << optimizer->GetStopConditionDescription() << std::endl;

    // apply the transform we get from the registration
    ResampleFilterType::Pointer resampleFilter = ResampleFilterType::New();
    resampleFilter->SetInput(movingReader->GetOutput());
    // transform that we are applying
    resampleFilter->SetTransform(transform);
    // set the grid to where the fixed image is, just in case it moved too far
    resampleFilter->SetReferenceImage(fixedReader->GetOutput());
    resampleFilter->UseReferenceImageOn();

    // update
    resampleFilter->Update();


    // Write out the result  
    ImageWriterType::Pointer writer = ImageWriterType::New();
    writer->SetFileName("C:/Users/Frank/Documents/1.0 Vanderbilt/CS Medical Imaging/ITK_ANTS_Project/KK12009-ALL-MPRAGE-AFFINE_ITK/MPRAGE-1-affine");
    writer->SetInput(resampleFilter->GetOutput()); // need to figure out the argument here
    writer->Update();


    // Done.
    return 0;
}
