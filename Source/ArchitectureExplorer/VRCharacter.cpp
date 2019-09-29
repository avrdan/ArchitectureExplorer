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
	//TUniquePtr<FVector> Transparent = MakeUnique<FVector>(0, 0, 1);
	//TUniquePtr<FVector> Opaque = MakeUnique<FVector>(0, 0, 0);
	Transparent = FLinearColor(1, 1, 1, 1);
	Opaque = FLinearColor(0, 0, 0, 0);

	VRRoot = CreateDefaultSubobject<USceneComponent>(TEXT("VRRoot"));
	VRRoot->SetupAttachment(GetRootComponent());

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(VRRoot);

	/*StereoLayer = CreateDefaultSubobject<UStereoLayerComponent>(TEXT("StereoLayer"));
	StereoLayer->SetupAttachment(Camera);
	StereoLayer->bSupportsDepth = true;
	StereoLayer->bLiveTexture = true;
	StereoLayer->SetQuadSize(FVector2D(1000, 1000));
	FVector stereoLayerOffset(10, 0, 0);
	StereoLayer->AddWorldOffset(stereoLayerOffset);*/

	// No need for setting the height on the QUEST,
	// it will be taken from the Guardian system
	//VRRoot->SetRelativeLocation(FVector(0, 0, 180));

	DestinationMarker = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DestinationMarker"));
	DestinationMarker->SetupAttachment(GetRootComponent());

	//FStringAssetReference SequenceName("/Game/TeleportFadeSequence");
	//SequenceAsset = Cast<UMovieSceneSequence>(SequenceName.TryLoad());
	SequenceAsset = LoadObject<ULevelSequence>(NULL, TEXT("/Game/TeleportFadeSequence"), NULL, LOAD_None, NULL);

	if (IsValid(SequenceAsset))
	{
		UE_LOG(LogTemp, Warning, TEXT("Sequence asset loaded!"));

		ALevelSequenceActor* actor = nullptr;
		SequencePlayer = ULevelSequencePlayer::CreateLevelSequencePlayer(GetWorld(), SequenceAsset, FMovieSceneSequencePlaybackSettings(), actor);

		//FMovieSceneSequencePlaybackSettings settings;
		//SequencePlayer = CreateDefaultSubobject< UMovieSceneSequencePlayer>(TEXT("MovieSceneSequencePlayer"));
		//MovieActor = GetWorld()->SpawnActor<ALevelSequenceActor>();
		//MovieActor->SetSequence(SequenceAsset);

		//SequencePlayer->Initialize(SequenceAsset, settings);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Sequence asset could NOT be loaded!"));
	}

	/*SequencePlayer = ULevelSequencePlayer::CreateLevelSequencePlayer(GetWorld(), SequenceAsset, FMovieSceneSequencePlaybackSettings());
	if (SequencePlayer)
	{
		SequencePlayer->Play();
	}*/


	/*static ConstructorHelpers::FClassFinder<UUserWidget> FadeWidget(TEXT("/Game/BP_FadeWidget"));
	Widget = CreateDefaultSubobject<UWidgetComponent>(TEXT("FadeWidget"));
	//Widget->SetupAttachment(Camera);
	Widget->SetupAttachment(StereoLayer);
	//Widget->AttachTo(StereoLayer);
	if (FadeWidget.Succeeded())
	{
		Widget->SetWidgetClass(FadeWidget.Class);
	}

	// set material
	static ConstructorHelpers::FObjectFinder<UMaterialInstanceConstant> TranslucentMat(TEXT("/Engine/EngineMaterials/Widget3DPassThrough_Translucent"));
	if (TranslucentMat.Succeeded())
	{
		UE_LOG(LogTemp, Warning, TEXT("Loaded Material for Widget.."));
		WidgetMat = TranslucentMat.Object;
	}*/
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

	if (!init)
	{
		/*Widget->SetMaterial(0, WidgetMat);
		Widget->SetDrawSize(FVector2D(1000, 1000));
		Widget->SetRelativeRotation(FRotator(0, 180.0f, 0));
		Widget->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		FadeAnim = GetWidgetAnim();*/
		init = true;

		//Widget->SetVisibility(false);
		//UTextureRenderTarget2D* renderTarget = Widget->GetRenderTarget();
		/*if (renderTarget)
		{
			UE_LOG(LogTemp, Warning, TEXT("*** FOUND Render target"));
			if (renderTarget && renderTarget->IsValidLowLevel())
			{
				UE_LOG(LogTemp, Warning, TEXT("Found render target!"));
				init = true;
				StereoLayer->SetTexture(Widget->GetRenderTarget());

			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("NO RENDER TARGET!"));
			}
		}*/



	}
	else
	{
		/*FWidgetRenderer* WidgetRenderer = new FWidgetRenderer();
		UTextureRenderTarget2D* Texture = WidgetRenderer->CreateTargetFor(Widget->GetDrawSize(), TextureFilter::TF_Default, false);
		TSharedRef<SWidget> swidget = Widget->GetUserWidgetObject()->TakeWidget();
		WidgetRenderer->DrawWidget(Texture, swidget, FVector2D(1000, 1000), 0);
		//UTextureRenderTarget2D* renderTarget = WidgetRenderer->CreateTargetFor(Widget->GetDrawSize(), TextureFilter::TF_Default, false);
		StereoLayer->SetTexture(Texture);
		delete WidgetRenderer;*/
	}

	if (startFade)
	{
		StartFade(DeltaTime);
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
		// Not Good for VR as the text is almost outside the view
		/*if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Ray cast HIT!!");
		}*/
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
	AddMovementInput(Camera->GetForwardVector() * throttle);
}

void AVRCharacter::MoveRight(float throttle)
{
	AddMovementInput(Camera->GetRightVector() * throttle);
}

void AVRCharacter::BeginTeleport()
{
	//Widget->GetUserWidgetObject()->PlayAnimation(FadeAnim, 0.1, 1);
	//Widget->SetVisibility(true);

	SetStartFadeFlag(true);

	//SequencePlayer->Play();
	//Widget->GetUserWidgetObject()->PlayAnimation(FadeAnim, 0, 100);
	//if (!IsHeadMountedDisplayConnected())
	//{
		/*APlayerController* controller = Cast<APlayerController>(GetController());
		if (controller)
		{
			controller->PlayerCameraManager->StartCameraFade(0, 1, teleportFadeTime, FLinearColor::Black);
		}*/
		//SequencePlayer->Play();

	FTimerHandle handle;
	GetWorldTimerManager().SetTimer(handle, this, &AVRCharacter::EndTeleport, teleportFadeTime, false);
	/*}
	else
	{
		SetLocationToMarker();
	}*/
}

void AVRCharacter::EndTeleport()
{
	SetLocationToMarker();
	FTimerHandle handle;
	GetWorldTimerManager().SetTimer(handle, this, &AVRCharacter::ReverseFade, fadeScreenTime, false);

	//Widget->SetVisibility(false);
	/*APlayerController* controller = Cast<APlayerController>(GetController());
	if (controller)
	{
		controller->PlayerCameraManager->StartCameraFade(1, 0, teleportFadeTime, FLinearColor::Black);
	}*/
}

void AVRCharacter::SetLocationToMarker()
{
	float diffHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	FVector markerLoc = DestinationMarker->GetComponentLocation();
	markerLoc.Z -= diffHeight;
	SetActorLocation(markerLoc);
}

UWidgetAnimation* AVRCharacter::GetWidgetAnim()
{
	if (!Widget)
		return nullptr;
	UProperty* prop = Widget->GetUserWidgetObject()->GetClass()->PropertyLink;
	while (prop)
	{
		if (prop->GetClass() == UObjectProperty::StaticClass())
		{
			UObjectProperty* objProp = Cast<UObjectProperty>(prop);
			if (objProp->PropertyClass == UWidgetAnimation::StaticClass())
			{
				UObject* object = objProp->GetObjectPropertyValue_InContainer(Widget->GetUserWidgetObject());
				UWidgetAnimation* widgetAnim = Cast<UWidgetAnimation>(object);
				if (widgetAnim->GetName().Find("FadeAnim") >= 0)
				{
					UE_LOG(LogTemp, Warning, TEXT("Found anim: %s"), *widgetAnim->GetName());
					return widgetAnim;
				}
			}
		}

		prop = prop->PropertyLinkNext;
	}

	return nullptr;
}

/*bool AVRCharacter::IsHeadMountedDisplayConnected()
{
	return GEngine->XRSystem.IsValid() && GEngine->XRSystem->GetHMDDevice() && GEngine->XRSystem->GetHMDDevice()->IsHMDConnected();
}*/

void AVRCharacter::StartFade(float DeltaTime)
{
	fadeDelta += DeltaTime;

	if (reverseFadeDir)
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
		SetStartFadeFlag(false);
		if (reverseFadeDir)
		{
			UE_LOG(LogTemp, Error, TEXT("*** Fade OVER!"));
			reverseFadeDir = false;
			Oculus->SetColorScaleAndOffset(FLinearColor(1, 1, 1, 1), FLinearColor(0, 0, 0, 0), true);
		}
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

void AVRCharacter::SetStartFadeFlag(bool state)
{
	startFade = state;
}

void AVRCharacter::ReverseFade()
{
	reverseFadeDir = true;
	SetStartFadeFlag(true);
}