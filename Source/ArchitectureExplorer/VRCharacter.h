// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

//#include "Math/Vector.h"
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
//#include "Templates/UniquePtr.h"
//#include "Engine/GameEngine.h"
#include "VRCharacter.generated.h"

UCLASS()
class ARCHITECTUREEXPLORER_API AVRCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AVRCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	void UpdateDestinationMarker();
	void MoveForward(float throttle);
	void MoveRight(float throttle);
	void BeginTeleport();
	void EndTeleport();
	//bool IsHeadMountedDisplayConnected();
	void SetLocationToMarker();
	class UWidgetAnimation* GetWidgetAnim();
	void StartFade(float DeltaTime);
	void Fade(FLinearColor start, FLinearColor end);
	void SetStartFadeFlag(bool state);
	void ReverseFade();
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
	class UCameraComponent* Camera;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	class USceneComponent* VRRoot;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UStaticMeshComponent* DestinationMarker;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	class UStereoLayerComponent* StereoLayer;
	UPROPERTY(VisibleAnywhere)
	class ULevelSequencePlayer* SequencePlayer;
	//class UMovieSceneSequencePlayer* SequencePlayer;
	//class ALevelSequenceActor* MovieActor;
	UPROPERTY(EditAnywhere)
	//class ULevelSequence* SequenceAsset;
	class ULevelSequence* SequenceAsset;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	class UWidgetComponent* Widget;

private:
	UPROPERTY(EditAnywhere)
	float maxTeleportDistance = 1000;

	UPROPERTY(EditAnywhere)
	float teleportFadeTime = 1; // seconds

	class UOculusFunctionLibrary* Oculus = nullptr;;

	//UPROPERTY(meta = (BindWidgetAnim))
	class UWidgetAnimation* FadeAnim = nullptr;
	class UMaterialInterface* WidgetMat = nullptr;

	bool init = false;
	float fadeDelta;
	bool startFade = false;
	bool reverseFadeDir = false;
	float fadeScreenTime = 0.5f;

/*	const FVector* Transparent = nullptr;
	const FVector* Opaque = nullptr;*/
	struct FLinearColor Transparent;
	struct FLinearColor Opaque;
	//TUniquePtr<FVector> Transparent = nullptr;
	//TUniquePtr<FVector> Opaque = nullptr;
};
