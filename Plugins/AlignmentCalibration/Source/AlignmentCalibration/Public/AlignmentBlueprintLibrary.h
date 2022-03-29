// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AlignmentBlueprintLibrary.generated.h"

/**
 * 
 */
UCLASS()
class ALIGNMENTCALIBRATION_API UAlignmentBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, Category = Alignment)
	static void GenerateTestLines(FVector TargetPoint, float Length, int NumPoints, float Noise, TArray<FVector>& StartPoints, TArray<FVector>& EndPoints);
	
	UFUNCTION(BlueprintCallable, Category = Alignment)
	static void CalculateClosestIntersection(const TArray<FVector>& StartPoints, const TArray<FVector>& EndPoints, FVector& ClosestIntersection);

	// Computes a transform that aligns two coordinate systems. Requires the transform of the same known point in each coordinate system.
	UFUNCTION(BlueprintCallable, Category = Alignment)
	static void CalculateAlignmentTransform(const FTransform& TransformInFirstCoordinateSystem, const FTransform& TransformInSecondCoordinateSystem, 
		FTransform& AlignmentTransform);

	UFUNCTION(BlueprintPure, Category = Alignment)
	static void RotatorToQuaternion(FRotator InRotation, float& X, float& Y, float& Z, float& W);
};
