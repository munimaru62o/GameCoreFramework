// Copyright Epic Games, Inc. All Rights Reserved.
// Portions Copyright (c) 2026 munimaru62o. All rights reserved.

#include "Camera/Mode/GCFCameraMode.h"
#include "Camera/GCFCameraComponent.h"

#include "Components/CapsuleComponent.h"
#include "Engine/Canvas.h"
#include "GameFramework/Character.h"
//#include "GCFPlayerCameraManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GCFCameraMode)

#define GCF_CAMERA_DEFAULT_FOV			(80.0f)
#define GCF_CAMERA_DEFAULT_PITCH_MIN	(-89.0f)
#define GCF_CAMERA_DEFAULT_PITCH_MAX	(89.0f)

//////////////////////////////////////////////////////////////////////////
// FGCFCameraModeView
//////////////////////////////////////////////////////////////////////////
FGCFCameraModeView::FGCFCameraModeView()
	: Location(ForceInit)
	, Rotation(ForceInit)
	, ControlRotation(ForceInit)
	, FieldOfView(GCF_CAMERA_DEFAULT_FOV)
{
}

void FGCFCameraModeView::Blend(const FGCFCameraModeView& Other, float OtherWeight)
{
	if (OtherWeight <= 0.0f)
	{
		return;
	}
	else if (OtherWeight >= 1.0f)
	{
		*this = Other;
		return;
	}

	Location = FMath::Lerp(Location, Other.Location, OtherWeight);

	const FRotator DeltaRotation = (Other.Rotation - Rotation).GetNormalized();
	Rotation = Rotation + (OtherWeight * DeltaRotation);

	const FRotator DeltaControlRotation = (Other.ControlRotation - ControlRotation).GetNormalized();
	ControlRotation = ControlRotation + (OtherWeight * DeltaControlRotation);

	FieldOfView = FMath::Lerp(FieldOfView, Other.FieldOfView, OtherWeight);
}


//////////////////////////////////////////////////////////////////////////
// UGCFCameraMode
//////////////////////////////////////////////////////////////////////////
UGCFCameraMode::UGCFCameraMode()
{
	FieldOfView = GCF_CAMERA_DEFAULT_FOV;
	ViewPitchMin = GCF_CAMERA_DEFAULT_PITCH_MIN;
	ViewPitchMax = GCF_CAMERA_DEFAULT_PITCH_MAX;

	BlendTime = 0.5f;
	BlendFunction = EGCFCameraModeBlendFunction::EaseOut;
	BlendExponent = 4.0f;
	BlendAlpha = 1.0f;
	BlendWeight = 1.0f;

	// --- Default X-Axis (Distance) Curve ---
	FRichCurve& CurveX = *TargetOffsetX.GetRichCurve();
	CurveX.AddKey(-90.0f, -400.0f); // Looking Down
	CurveX.AddKey(0.0f, -400.0f);   // Horizon
	CurveX.AddKey(90.0f, -400.0f);  // Looking Up

	// --- Default Z-Axis (Height) Curve ---
	FRichCurve& CurveZ = *TargetOffsetZ.GetRichCurve();
	CurveZ.AddKey(-90.0f, 50.0f);
	CurveZ.AddKey(0.0f, 50.0f);
	CurveZ.AddKey(90.0f, 50.0f);

	// --- Default Y-Axis (Side) Curve ---
	FRichCurve& CurveY = *TargetOffsetY.GetRichCurve();
	CurveY.AddKey(0.0f, 0.0f);
}

UGCFCameraComponent* UGCFCameraMode::GetGCFCameraComponent() const
{
	return CastChecked<UGCFCameraComponent>(GetOuter());
}

UWorld* UGCFCameraMode::GetWorld() const
{
	return HasAnyFlags(RF_ClassDefaultObject) ? nullptr : GetOuter()->GetWorld();
}

AActor* UGCFCameraMode::GetTargetActor() const
{
	const UGCFCameraComponent* GCFCameraComponent = GetGCFCameraComponent();

	return GCFCameraComponent->GetTargetActor();
}

FVector UGCFCameraMode::GetPivotLocation() const
{
	const AActor* TargetActor = GetTargetActor();
	check(TargetActor);

	if (const APawn* TargetPawn = Cast<APawn>(TargetActor))
	{
		// Height adjustments for characters to account for crouching.
		if (const ACharacter* TargetCharacter = Cast<ACharacter>(TargetPawn))
		{
			const ACharacter* TargetCharacterCDO = TargetCharacter->GetClass()->GetDefaultObject<ACharacter>();
			check(TargetCharacterCDO);

			const UCapsuleComponent* CapsuleComp = TargetCharacter->GetCapsuleComponent();
			check(CapsuleComp);

			const UCapsuleComponent* CapsuleCompCDO = TargetCharacterCDO->GetCapsuleComponent();
			check(CapsuleCompCDO);

			const float DefaultHalfHeight = CapsuleCompCDO->GetUnscaledCapsuleHalfHeight();
			const float ActualHalfHeight = CapsuleComp->GetUnscaledCapsuleHalfHeight();
			const float HeightAdjustment = (DefaultHalfHeight - ActualHalfHeight) + TargetCharacterCDO->BaseEyeHeight;

			return TargetCharacter->GetActorLocation() + (FVector::UpVector * HeightAdjustment);
		}

		return TargetPawn->GetPawnViewLocation();
	}

	return TargetActor->GetActorLocation();
}

FRotator UGCFCameraMode::GetPivotRotation() const
{
	const AActor* TargetActor = GetTargetActor();
	check(TargetActor);

	if (const APawn* TargetPawn = Cast<APawn>(TargetActor))
	{
		return TargetPawn->GetViewRotation();
	}

	return TargetActor->GetActorRotation();
}


float UGCFCameraMode::GetCameraZoomRatio() const
{
	if (const UGCFCameraComponent* GCFCameraComp = GetGCFCameraComponent()) {
		return GCFCameraComp->GetTargetZoomRatio();
	}
	return 1.0f;
}


FVector UGCFCameraMode::CalculateOffsetFromCurves(float Pitch) const
{
	const float OffsetX = TargetOffsetX.GetRichCurveConst()->Eval(Pitch);
	const float OffsetY = TargetOffsetY.GetRichCurveConst()->Eval(Pitch);
	const float OffsetZ = TargetOffsetZ.GetRichCurveConst()->Eval(Pitch);

	return FVector(OffsetX, OffsetY, OffsetZ);
}


void UGCFCameraMode::UpdateCameraMode(float DeltaTime)
{
	UpdateView(DeltaTime);
	UpdateBlending(DeltaTime);
}

void UGCFCameraMode::UpdateView(float DeltaTime)
{
	// [Base Implementation]
	// Simply snaps the camera to the pivot point.
	// Subclasses should override this to apply offsets, lag, and collision.

	FVector PivotLocation = GetPivotLocation();
	FRotator PivotRotation = GetPivotRotation();

	PivotRotation.Pitch = FMath::ClampAngle(PivotRotation.Pitch, ViewPitchMin, ViewPitchMax);

	View.Location = PivotLocation;
	View.Rotation = PivotRotation;
	View.ControlRotation = View.Rotation;
	View.FieldOfView = FieldOfView;
}

void UGCFCameraMode::SetBlendWeight(float Weight)
{
	BlendWeight = FMath::Clamp(Weight, 0.0f, 1.0f);

	// Since we're setting the blend weight directly, we need to calculate the blend alpha to account for the blend function.
	const float InvExponent = (BlendExponent > 0.0f) ? (1.0f / BlendExponent) : 1.0f;

	switch (BlendFunction)
	{
	case EGCFCameraModeBlendFunction::Linear:
		BlendAlpha = BlendWeight;
		break;

	case EGCFCameraModeBlendFunction::EaseIn:
		BlendAlpha = FMath::InterpEaseIn(0.0f, 1.0f, BlendWeight, InvExponent);
		break;

	case EGCFCameraModeBlendFunction::EaseOut:
		BlendAlpha = FMath::InterpEaseOut(0.0f, 1.0f, BlendWeight, InvExponent);
		break;

	case EGCFCameraModeBlendFunction::EaseInOut:
		BlendAlpha = FMath::InterpEaseInOut(0.0f, 1.0f, BlendWeight, InvExponent);
		break;

	default:
		checkf(false, TEXT("SetBlendWeight: Invalid BlendFunction [%d]\n"), (uint8)BlendFunction);
		break;
	}
}

void UGCFCameraMode::UpdateBlending(float DeltaTime)
{
	if (BlendTime > 0.0f)
	{
		BlendAlpha += (DeltaTime / BlendTime);
		BlendAlpha = FMath::Min(BlendAlpha, 1.0f);
	}
	else
	{
		BlendAlpha = 1.0f;
	}

	const float Exponent = (BlendExponent > 0.0f) ? BlendExponent : 1.0f;

	switch (BlendFunction)
	{
	case EGCFCameraModeBlendFunction::Linear:
		BlendWeight = BlendAlpha;
		break;

	case EGCFCameraModeBlendFunction::EaseIn:
		BlendWeight = FMath::InterpEaseIn(0.0f, 1.0f, BlendAlpha, Exponent);
		break;

	case EGCFCameraModeBlendFunction::EaseOut:
		BlendWeight = FMath::InterpEaseOut(0.0f, 1.0f, BlendAlpha, Exponent);
		break;

	case EGCFCameraModeBlendFunction::EaseInOut:
		BlendWeight = FMath::InterpEaseInOut(0.0f, 1.0f, BlendAlpha, Exponent);
		break;

	default:
		checkf(false, TEXT("UpdateBlending: Invalid BlendFunction [%d]\n"), (uint8)BlendFunction);
		break;
	}
}

void UGCFCameraMode::DrawDebug(UCanvas* Canvas) const
{
	check(Canvas);

	FDisplayDebugManager& DisplayDebugManager = Canvas->DisplayDebugManager;

	DisplayDebugManager.SetDrawColor(FColor::White);
	DisplayDebugManager.DrawString(FString::Printf(TEXT("      GCFCameraMode: %s (%f)"), *GetName(), BlendWeight));
}
