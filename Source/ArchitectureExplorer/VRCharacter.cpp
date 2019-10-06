// Fill out your copyright notice in the Description page of Project Settings.


#include "VRCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/GameEngine.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"
#include "Components/CapsuleComponent.h"
#include "OculusFunctionLibrary.h"
#include "Math/Color.h"
#include "NavigationSystem.h"
#include "Components/PostProcessComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Curves/CurveFloat.h"
#include "Components/WidgetComponent.h"
#include "Components/StereoLayerComponent.h"
#include "MotionControllerComponent.h"

// Sets default values
AVRCharacter::AVRCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Oculus = CreateDefaultSubobject<UOculusFunctionLibrary>(TEXT("Oculus"));
	Transparent = FLinearColor(1, 1, 1, 1);
	Opaque = FLinearColor(0, 0, 0, 0);

	VRRoot = CreateDefaultSubobject<USceneComponent>(TEXT("VRRoot"));
	VRRoot->SetupAttachment(GetRootComponent());

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(VRRoot);

	LeftController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("LeftController"));
	LeftController->SetupAttachment(VRRoot);
	LeftController->SetTrackingSource(EControllerHand::Left);

	RightController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("RightController"));
	RightController->SetupAttachment(VRRoot);
	RightController->SetTrackingSource(EControllerHand::Right);

	DestinationMarker = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DestinationMarker"));
	DestinationMarker->SetupAttachment(GetRootComponent());

	PostProcessComponent = CreateDefaultSubobject<UPostProcessComponent>(TEXT("PostProcessComponent"));
	PostProcessComponent->SetupAttachment(GetRootComponent());

	StereoLayer = CreateDefaultSubobject<UStereoLayerComponent>(TEXT("StereoLayer"));
	StereoLayer->SetupAttachment(Camera);
	StereoLayer->bSupportsDepth = true;
	StereoLayer->bLiveTexture = true;
	StereoLayer->SetQuadSize(FVector2D(1000, 1000));
	FVector stereoLayerOffset(10, 0, 0);
	StereoLayer->AddWorldOffset(stereoLayerOffset);

	BlinkerWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("BlinkerWidget"));
	BlinkerWidget->SetupAttachment(StereoLayer);
}

// Called when the game starts or when spawned
void AVRCharacter::BeginPlay()
{
	Super::BeginPlay();

	DestinationMarker->SetVisibility(false);

	if (init)
	{
		if (BlinkerMaterialBase)
		{
			UE_LOG(LogTemp, Warning, TEXT("*** Found blinker material base! Creating instance.."));
			BlinkerMaterialInstance = UMaterialInstanceDynamic::Create(BlinkerMaterialBase, this);
			PostProcessComponent->AddOrUpdateBlendable(BlinkerMaterialInstance);
			BlinkerWidget->SetVisibility(false);
			/*BlinkerWidget->SetRelativeRotation(FRotator(0, 180.0f, 0));
			BlinkerWidget->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			BlinkerWidget->SetMaterial(0, BlinkerMaterialInstance);*/

			init = false;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("*** Blinker material base NOT FOUND!!!"));
		}
	}
}

// Called every frame
void AVRCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	/*FVector newCameraOffset = Camera->GetComponentLocation() - GetActorLocation();
	float diffHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	newCameraOffset.Z = 0; // do not move up/down, Z is up
	AddActorWorldOffset(newCameraOffset);
	VRRoot->AddWorldOffset(-newCameraOffset);*/

	UpdateDestinationMarker();
	UpdateBlinkers();

	if (fadeState != EFade::OFF)
	{
		ContinueFade(DeltaTime);
	}
}

// Called to bind functionality to input
void AVRCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAxis(TEXT("Forward"), this, &AVRCharacter::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("Right"), this, &AVRCharacter::MoveRight);
	PlayerInputComponent->BindAction(TEXT("Teleport"), IE_Released, this, &AVRCharacter::BeginTeleport);
	PlayerInputComponent->BindAxis(TEXT("MouseLookX"), this, &AVRCharacter::MouseLookX);
	PlayerInputComponent->BindAxis(TEXT("MouseLookY"), this, &AVRCharacter::MouseLookY);
	bUseControllerRotationPitch = true;
}

bool AVRCharacter::GetMarkerPos(const FVector& start, FVector& end, FVector& newPos)
{
	FHitResult hit;
	bool isHit = GetWorld()->LineTraceSingleByChannel(hit, start, end, ECollisionChannel::ECC_Visibility);
	if (!isHit) { return false; }
	FNavLocation projHit;
	isHit &= GetProjectedHitLocation(hit.Location, projHit);
	newPos = projHit.Location;
	return isHit;
}

bool AVRCharacter::GetProjectedHitLocation(const FVector& hitLoc, FNavLocation& projHit)
{
	UNavigationSystemV1* navSys = UNavigationSystemV1::GetCurrent(GetWorld());
	return navSys->ProjectPointToNavigation(hitLoc, projHit, teleportProjectionExtent);
}

void AVRCharacter::UpdateDestinationMarker()
{
	if (fadeState != EFade::OFF)
	{
		DestinationMarker->SetVisibility(false);
		return;
	}

	FVector start;
	FVector end;
	if (LeftController->IsActive())
	{
		FVector look = LeftController->GetForwardVector();
		look = look.RotateAngleAxis(50, LeftController->GetRightVector());
		start = LeftController->GetComponentLocation() + look * 5;
		end = look * maxTeleportDistance + start;
	}
	else // fallback to camera
	{
		start = Camera->GetComponentLocation();
		end = Camera->GetForwardVector() * maxTeleportDistance + start;
	}

	DrawDebugLine(GetWorld(), start, end, FColor::Green, false, 1, 0, 1);

	FVector newPos;
	if (!GetMarkerPos(start, end, newPos))
	{
		DestinationMarker->SetVisibility(false);
		return;
	}

	DestinationMarker->SetVisibility(true);
	float diffHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	//newPos.Z -= diffHeight;
	DestinationMarker->SetWorldLocation(newPos);
}

void AVRCharacter::MoveForward(float throttle)
{
	if (fadeState != EFade::OFF)
		return;
	AddMovementInput(Camera->GetForwardVector() * throttle);
}

void AVRCharacter::MoveRight(float throttle)
{
	if (fadeState != EFade::OFF)
		return;
	AddMovementInput(Camera->GetRightVector() * throttle);
}

void AVRCharacter::MouseLookX(float throttle)
{
	AddControllerYawInput(throttle);
}

void AVRCharacter::MouseLookY(float throttle)
{
	AddControllerPitchInput(throttle);
}

void AVRCharacter::BeginTeleport()
{
	if (fadeState != EFade::OFF)
		return;
	fadeState = EFade::START;
	FTimerHandle handle;
	GetWorldTimerManager().SetTimer(handle, this, &AVRCharacter::EndTeleport, teleportFadeTime, false);
}

void AVRCharacter::EndTeleport()
{
	SetLocationToMarker();
	FTimerHandle handle;
	GetWorldTimerManager().SetTimer(handle, this, &AVRCharacter::ReverseFade, fadeScreenTime, false);
}

void AVRCharacter::SetLocationToMarker()
{
	float diffHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	FVector markerLoc = DestinationMarker->GetComponentLocation();
	markerLoc.Z += diffHeight;
	SetActorLocation(markerLoc);
}

void AVRCharacter::ContinueFade(float DeltaTime)
{
	fadeDelta += DeltaTime;

	if (fadeState == EFade::REVERSE)
	{
		UE_LOG(LogTemp, Warning, TEXT("*** Fade IN!"));
		Fade(Opaque, Transparent);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("*** Fade OUT!"));
		Fade(Transparent, Opaque);
	}

	if (fadeDelta > teleportFadeTime)
	{
		fadeDelta = 0;
		if (fadeState == EFade::REVERSE)
		{
			UE_LOG(LogTemp, Error, TEXT("*** Fade OVER!"));
			Oculus->SetColorScaleAndOffset(FLinearColor(1, 1, 1, 1), FLinearColor(0, 0, 0, 0), true);
		}
		fadeState = EFade::OFF;
	}
}

void AVRCharacter::Fade(FLinearColor start, FLinearColor end)
{
	UE_LOG(LogTemp, Display, TEXT("*** Fade delta: %f"), fadeDelta);
	FLinearColor updatedColor = FMath::Lerp(start, end, fadeDelta);
	UE_LOG(LogTemp, Display, TEXT("*** Updated color red: %f"), updatedColor.R);
	UE_LOG(LogTemp, Display, TEXT("*** Updated color green: %f"), updatedColor.G);
	UE_LOG(LogTemp, Display, TEXT("*** Updated color blue: %f"), updatedColor.B);
	UE_LOG(LogTemp, Display, TEXT("*** Updated color alpha: %f"), updatedColor.A);
	Oculus->SetColorScaleAndOffset(updatedColor, FLinearColor(0, 0, 0, 0), true);
}

void AVRCharacter::ReverseFade()
{
	fadeState = EFade::REVERSE;
}

void AVRCharacter::UpdateBlinkers()
{
	if (!RadiusVsVelocity)
		return;

	//float velocity{ GetVelocity().Size() };
	FVector vVelocity{ GetVelocity() };
	//vVelocity.Normalize();
	float velocity{ vVelocity.Size() };
	UE_LOG(LogTemp, Warning, TEXT("*** Velocity %f"), velocity);

	float newRadius = RadiusVsVelocity->GetFloatValue(velocity);
	BlinkerMaterialInstance->SetScalarParameterValue(TEXT("Radius"), newRadius);

	if (velocity > 0.5f)
	{
		Oculus->SetTiledMultiresLevel(ETiledMultiResLevel::ETiledMultiResLevel_LMSHighTop);
	}
	else
	{
		Oculus->SetTiledMultiresLevel(ETiledMultiResLevel::ETiledMultiResLevel_Off);
	}

	FVector2D center = GetBlinkerCenter();
	BlinkerMaterialInstance->SetVectorParameterValue(TEXT("Center"), FLinearColor(center.X, center.Y, 0));
}

FVector2D AVRCharacter::GetBlinkerCenter()
{
	FVector moveDir = GetVelocity().GetSafeNormal();
	APlayerController* controller = Cast<APlayerController>(GetController());
	if (moveDir.IsNearlyZero() || !controller)
	{
		return FVector2D{ 0.5f, 0.5 };
	}

	int projSize = 1000; // 10 m
	if (FVector::DotProduct(Camera->GetForwardVector(), moveDir) < 0)
	{
		moveDir *= -1;
	}
	FVector worldLoc = Camera->GetComponentLocation() + moveDir * projSize;
	FVector2D screenLoc;
	controller->ProjectWorldLocationToScreen(worldLoc, screenLoc);
	int32 sizeX, sizeY;
	controller->GetViewportSize(sizeX, sizeY);

	return FVector2D{ screenLoc.X / sizeX, screenLoc.Y / sizeY };
}