// Copyright Epic Games, Inc. All Rights Reserved.


#include "AlignmentBlueprintLibrary.h"

namespace AlignmentBlueprintLibraryNamespace
{
	struct FSquareMatrix3
	{
		float M[3][3];

		FSquareMatrix3() = default;

		FORCEINLINE void SetColumn(int32 Index, FVector Axis)
		{
			M[Index][0] = Axis.X;
			M[Index][1] = Axis.Y;
			M[Index][2] = Axis.Z;
		};

		FORCEINLINE FVector GetColumn(int32 Index) const
		{
			return FVector(M[Index][0], M[Index][1], M[Index][2]);
		};

		FORCEINLINE FSquareMatrix3 operator*(float Scale) const
		{
			FSquareMatrix3 ResultMat;

			for (int32 X = 0; X < 3; X++)
			{
				for (int32 Y = 0; Y < 3; Y++)
				{
					ResultMat.M[X][Y] = M[X][Y] * Scale;
				}
			}

			return ResultMat;
		}
	};


	FSquareMatrix3 Invert(const FSquareMatrix3& InMatrix)
	{
		FSquareMatrix3 inverse;
		bool invertible;
		auto& M = InMatrix.M;
		float c00 = M[1][1] * M[2][2] - M[1][2] * M[2][1];
		float c10 = M[1][2] * M[2][0] - M[1][0] * M[2][2];
		float c20 = M[1][0] * M[2][1] - M[1][1] * M[2][0];
		float det = M[0][0] * c00 + M[0][1] * c10 + M[0][2] * c20;
		if (FMath::Abs(det) > 0.000001f)
		{
			float invDet = 1.0f / det;
			inverse.SetColumn(0, { c00 * invDet, (M[0][2] * M[2][1] - M[0][1] * M[2][2]) * invDet, (M[0][1] * M[1][2] - M[0][2] * M[1][1]) * invDet });
			inverse.SetColumn(1, { c10 * invDet, (M[0][0] * M[2][2] - M[0][2] * M[2][0]) * invDet, (M[0][2] * M[1][0] - M[0][0] * M[1][2]) * invDet });
			inverse.SetColumn(2, { c20 * invDet, (M[0][1] * M[2][0] - M[0][0] * M[2][1]) * invDet, (M[0][0] * M[1][1] - M[0][1] * M[1][0]) * invDet });
			invertible = true;
		}
		else
		{
			inverse.SetColumn(0, { 1,0,0 });
			inverse.SetColumn(1, { 0,1,0 });
			inverse.SetColumn(2, { 0,0,1 });
			invertible = false;
		}
		return inverse;
	}

	FVector operator* (const FSquareMatrix3& InMatrix, const FVector& InVector)
	{
		auto& M = InMatrix.M;
		return {
			M[0][0] * InVector.X + M[1][0] * InVector.Y + M[2][0] * InVector.Z,
			M[0][1] * InVector.X + M[1][1] * InVector.Y + M[2][1] * InVector.Z,
			M[0][2] * InVector.X + M[1][2] * InVector.Y + M[2][2] * InVector.Z
		};
	}
}

void UAlignmentBlueprintLibrary::GenerateTestLines(FVector TargetPoint, float Length, int NumPoints, float Noise, TArray<FVector>& StartPoints, TArray<FVector>& EndPoints)
{
	StartPoints.SetNumUninitialized(NumPoints);
	EndPoints.SetNumUninitialized(NumPoints);

	for (int PointIdx = 0; PointIdx < NumPoints; ++PointIdx)
	{
		float x = FMath::RandRange(-Length, Length);
		float y = FMath::RandRange(-Length, Length);
		float z = FMath::RandRange(-Length, Length);
		StartPoints[PointIdx] = FVector(x, y, z);
		EndPoints[PointIdx] = -StartPoints[PointIdx] + FVector(FMath::RandRange(-Noise, Noise), FMath::RandRange(-Noise, Noise), FMath::RandRange(-Noise, Noise));

		StartPoints[PointIdx] += TargetPoint;
		EndPoints[PointIdx] += TargetPoint;
	}
}

void UAlignmentBlueprintLibrary::CalculateClosestIntersection(const TArray<FVector>& StartPoints, const TArray<FVector>& EndPoints, FVector& ClosestIntersection)
{
	checkf(StartPoints.Num() == EndPoints.Num(), TEXT("The StartPoints and Endpoints arrays must have the same size."));

	float SXX = 0.0f;
	float SYY = 0.0f;
	float SZZ = 0.0f;
	float SXY = 0.0f;
	float SXZ = 0.0f;
	float SYZ = 0.0f;

	float CX = 0.0f;
	float CY = 0.0f;
	float CZ = 0.0f;

	for (int PointIdx = 0; PointIdx < StartPoints.Num(); ++PointIdx)
	{
		FVector NormView = (EndPoints[PointIdx] - StartPoints[PointIdx]).GetSafeNormal();

		float XX = (NormView.X * NormView.X) - 1.0;
		float YY = (NormView.Y * NormView.Y) - 1.0;
		float ZZ = (NormView.Z * NormView.Z) - 1.0;
		float XY = (NormView.X * NormView.Y);
		float XZ = (NormView.X * NormView.Z);
		float YZ = (NormView.Y * NormView.Z);

		SXX += XX;
		SYY += YY;
		SZZ += ZZ;
		SXY += XY;
		SXZ += XZ;
		SYZ += YZ;

		CX +=	(StartPoints[PointIdx].X * XX) +
				(StartPoints[PointIdx].Y * XY) +
				(StartPoints[PointIdx].Z * XZ);

		CY +=	(StartPoints[PointIdx].X * XY) +
				(StartPoints[PointIdx].Y * YY) +
				(StartPoints[PointIdx].Z * YZ);

		CZ +=	(StartPoints[PointIdx].X * XZ) +
				(StartPoints[PointIdx].Y * YZ) +
				(StartPoints[PointIdx].Z * ZZ);
	}
	AlignmentBlueprintLibraryNamespace::FSquareMatrix3 S;
	S.SetColumn(0, { SXX, SXY, SXZ });
	S.SetColumn(1, { SXY, SYY, SYZ });
	S.SetColumn(2, { SXZ, SYZ, SZZ });

	FVector C(CX, CY, CZ);

	ClosestIntersection = AlignmentBlueprintLibraryNamespace::Invert(S) * C;
}

void UAlignmentBlueprintLibrary::CalculateAlignmentTransform(const FTransform& TransformInFirstCoordinateSystem, const FTransform& TransformInSecondCoordinateSystem, FTransform& AlignmentTransform)
{
	AlignmentTransform = TransformInSecondCoordinateSystem * TransformInFirstCoordinateSystem.Inverse();
}

void UAlignmentBlueprintLibrary::RotatorToQuaternion(FRotator InRotation, float& X, float& Y, float& Z, float& W)
{
	FQuat QuaternionRotation = InRotation.Quaternion();
	X = QuaternionRotation.X;
	Y = QuaternionRotation.Y;
	Z = QuaternionRotation.Z;
	W = QuaternionRotation.W;
}
