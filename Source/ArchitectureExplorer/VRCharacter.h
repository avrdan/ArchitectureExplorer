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
	void MouseLookX(float throttle);
	void MouseLookY(float throttle);
	void BeginTeleport();
	void EndTeleport();
	void SetLocationToMarker();
	bool GetMarkerPos(const FVector& start, FVector& end, FVector& newPos);
	bool GetProjectedHitLocation(const FVector& hitLoc, FNavLocation& projHit);

	void ContinueFade(float DeltaTime);
	void Fade(FLinearColor start, FLinearColor end);
	void ReverseFade();

	void UpdateBlinkers();
	FVector2D GetBlinkerCenter();
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
	class UCameraComponent* Camera;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	class USceneComponent* VRRoot;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UStaticMeshComponent* DestinationMarker;
	UPROPERTY(VisibleAnywhere)
	class UPostProcessComponent* PostProcessComponent;
	UPROPERTY(EditAnywhere)
	class UMaterialInterface* BlinkerMaterialBase;
	UPROPERTY()
	UMaterialInstanceDynamic* BlinkerMaterialInstance;
	UPROPERTY(EditAnywhere)
	class UCurveFloat* RadiusVsVelocity;
private:
	UPROPERTY(EditAnywhere)
	float maxTeleportDistance = 1000;

	UPROPERTY(EditAnywhere)
	float teleportFadeTime = 1; // seconds

	UPROPERTY(EditAnywhere)
	FVector teleportProjectionExtent {100, 100, 100 };

	class UOculusFunctionLibrary* Oculus = nullptr;

	float fadeDelta = 0;
	float fadeScreenTime = 0.5f;

	struct FLinearColor Transparent;
	struct FLinearColor Opaque;

	enum class EFade { OFF, START, REVERSE};
	EFade fadeState = EFade::OFF;

	bool init = true;
};
