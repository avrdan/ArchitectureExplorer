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
#include "Components/StereoLayerComponent.h"
//#include "IXRTrackingSystem.h"
//#include "IHeadMountedDisplay.h"
//#include "LevelSequence/Public/LevelSequencePlayer.h"
//#include "LevelSequence/Public/LevelSequence.h"
//#include "LevelSequencePlayer.h"
#include "LevelSequence/Public/LevelSequence.h"
#include "LevelSequence/Public/LevelSequencePlayer.h"
#include "LevelSequence/Public/LevelSequenceActor.h"
#include "Components/WidgetComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Blueprint/UserWidget.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Animation/WidgetAnimation.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Slate/WidgetRenderer.h"
#include "OculusFunctionLibrary.h"
#include "Math/Color.h"

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

	DestinationMarker = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DestinationMarker"));
	DestinationMarker->SetupAttachment(GetRootComponent());
}

// Called when the game starts or when spawned
void AVRCharacter::BeginPlay()
{
	Super::BeginPlay();

	DestinationMarker->SetVisibility(false);
}

// Called every frame
void AVRCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	FVector newCameraOffset = Camera->GetComponentLocation() - GetActorLocation();
	newCameraOffset.Z = 0; // do not move up/down, Z is up
	AddActorWorldOffset(newCameraOffset);
	VRRoot->AddWorldOffset(-newCameraOffset);

	UpdateDestinationMarker();

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
}

void AVRCharacter::UpdateDestinationMarker()
{
	FHitResult hit;
	FVector start = Camera->GetComponentLocation();
	FVector end = Camera->GetForwardVector() * maxTeleportDistance + start;

	DrawDebugLine(GetWorld(), start, end, FColor::Green, false, 1, 0, 1);

	if (GetWorld()->LineTraceSingleByChannel(hit, start, end, ECollisionChannel::ECC_Visibility))
	{
		DestinationMarker->SetVisibility(true);
		DestinationMarker->SetWorldLocation(hit.Location);
	}
	else
	{
		DestinationMarker->SetVisibility(false);
	}
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
	markerLoc.Z -= diffHeight;
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