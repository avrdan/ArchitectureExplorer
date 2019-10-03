// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
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
	void SetLocationToMarker();

	void ContinueFade(float DeltaTime);
	void Fade(FLinearColor start, FLinearColor end);
	void ReverseFade();
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
	class UCameraComponent* Camera;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	class USceneComponent* VRRoot;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UStaticMeshComponent* DestinationMarker;

private:
	UPROPERTY(EditAnywhere)
	float maxTeleportDistance = 1000;

	UPROPERTY(EditAnywhere)
	float teleportFadeTime = 1; // seconds

	class UOculusFunctionLibrary* Oculus = nullptr;;

	float fadeDelta = 0;
	float fadeScreenTime = 0.5f;

	struct FLinearColor Transparent;
	struct FLinearColor Opaque;

	enum class EFade { OFF, START, REVERSE};
	EFade fadeState = EFade::OFF;
};
