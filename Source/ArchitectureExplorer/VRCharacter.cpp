// Fill out your copyright notice in the Description page of Project Settings.


#include "VRCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/GameEngine.h"

// Sets default values
AVRCharacter::AVRCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	VRRoot = CreateDefaultSubobject<USceneComponent>(TEXT("VRRoot"));
	VRRoot->SetupAttachment(GetRootComponent());

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(VRRoot);

	// No need for setting the height on the QUEST,
	// it will be taken from the Guardian system
	//VRRoot->SetRelativeLocation(FVector(0, 0, 180));

	DestinationMarker = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DestinationMarker"));
	DestinationMarker->SetupAttachment(GetRootComponent());
}

// Called when the game starts or when spawned
void AVRCharacter::BeginPlay()
{
	Super::BeginPlay();
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
}

// Called to bind functionality to input
void AVRCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAxis(TEXT("Forward"), this, &AVRCharacter::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("Right"), this, &AVRCharacter::MoveRight);
}

void AVRCharacter::UpdateDestinationMarker()
{
	FHitResult hit;
	FVector start = Camera->GetComponentLocation();
	FVector end = Camera->GetForwardVector() * maxTeleportDistance + start;

	DrawDebugLine(GetWorld(), start, end, FColor::Green, false, 1, 0, 1);

	if (GetWorld()->LineTraceSingleByChannel(hit, start, end, ECollisionChannel::ECC_Visibility))
	{
		// Not Good for VR as the text is almost outside the view
		/*if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Ray cast HIT!!");
		}*/
		DestinationMarker->SetWorldLocation(hit.Location);
	}
}

void AVRCharacter::MoveForward(float throttle)
{
	AddMovementInput(Camera->GetForwardVector() * throttle);
}

void AVRCharacter::MoveRight(float throttle)
{
	AddMovementInput(Camera->GetRightVector() * throttle);
}