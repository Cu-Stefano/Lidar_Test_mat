#import "PoseDetectionBridge.h"
#import <Vision/Vision.h>
#import <CoreFoundation/CoreFoundation.h>
#import <CoreGraphics/CoreGraphics.h>
#import <Foundation/Foundation.h>
#import "ARTextures.h"
#if PLATFORM_IOS
#include <Vision/Vision.h>
#endif

void DetectHumanPoseFromRGBA_Debug()
{
    //DetectHumanPoseFromRGBA(IMAGE_DATA, IMAGE_WIDTH, IMAGE_HEIGHT);
}

TArray<FPoseJoint> DetectHumanPoseFromRGBA(const uint8_t* RGBAData, int Width, int Height)
{
    
    TArray<FPoseJoint> Joints;
    
    @autoreleasepool
    {
        size_t bytesPerPixel = 4;
        size_t bytesPerRow = Width * bytesPerPixel;
        

        CGDataProviderRef provider = CGDataProviderCreateWithData(
            NULL,
            RGBAData,
            bytesPerRow * Height,
            NULL
        );

        CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();

        CGImageRef cgImage = CGImageCreate(
            Width,
            Height,
            8,                      
            32,
            bytesPerRow,
            colorSpace,
            kCGBitmapByteOrderDefault | kCGImageAlphaLast,
            provider,
            NULL,
            false,
            kCGRenderingIntentDefault
        );

        CGColorSpaceRelease(colorSpace);
        CGDataProviderRelease(provider);
        

        if (!cgImage)
        {
            NSLog(@"[Vision] Failed to create CGImage");
            return TArray<FPoseJoint>();
        }

        VNDetectHumanBodyPoseRequest *request =
            [[VNDetectHumanBodyPoseRequest alloc] init];

        VNImageRequestHandler *handler =
            [[VNImageRequestHandler alloc] initWithCGImage:cgImage
                                                   orientation:kCGImagePropertyOrientationUp
                                                         options:@{}];
        NSError *error = nil;
        [handler performRequests:@[request] error:&error];
        CGImageRelease(cgImage);

        if (error)
        {
            NSLog(@"[Vision] Pose detection failed: %@", error.localizedDescription);
            return TArray<FPoseJoint>();
        }
        
        NSLog(@"[Vision] results count %lu", (unsigned long)request.results.count);

        if (request.results.count == 0)
        {
            NSLog(@"[Vision] No human detected");
            return TArray<FPoseJoint>();
        }

        for (VNHumanBodyPoseObservation *obs in request.results)
        {
            NSLog(@"[Vision DEBUG] Found VNHumanBodyPoseObservation: %@", obs);
            NSError *pointsError = nil;

            NSDictionary<NSString *, VNRecognizedPoint *> *points =
                [obs recognizedPointsForJointsGroupName:
                    VNHumanBodyPoseObservationJointsGroupNameAll
                                                      error:&pointsError];

            if (pointsError)
            {
                NSLog(@"[Vision] Points error: %@", pointsError.localizedDescription);
                continue;
            }
            
            if (!points || points.count == 0)
            {
                NSLog(@"[Vision DEBUG] Points dictionary is empty!");
                continue;
            }
            
            NSLog(@"[Vision DEBUG] Total joints detected: %lu", (unsigned long)points.count);

            NSUInteger jointIndex = 0;
            for (NSString *jointName in points)
            {
                VNRecognizedPoint *p = points[jointName];
                
                if (!p)
                {
                    NSLog(@"[Vision DEBUG] Joint %@ is nil!", jointName);
                    continue;
                }
                
                FPoseJoint joint;
                joint.X = p.x;
                joint.Y = p.y;
                joint.Confidence = p.confidence;
                
                Joints.Add(joint);
                
                /*NSLog(@"[Vision] Joint %@ -> x: %.3f y: %.3f conf: %.3f",
                      jointName,
                      p.x,
                      p.y,
                      p.confidence);*/
                
                jointIndex++;
            }
        }
    }
    
    return Joints;
}
