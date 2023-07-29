// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Satellite.h"
#include "Animation/AnimInstance.h"
#include "SatelliteAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class SATELLITE_API USatelliteAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:
	USatelliteAnimInstance();
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Pawn, Meta = (AllowPrivateAccess = true))
		float CurrentPawnSpeed;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Pawn, Meta = (AllowPrivateAccess = true))
		float CurrentPawnDirection;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Pawn, Meta = (AllowPrivateAccess = true))
		bool bIsInAir;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Pawn, Meta = (AllowPrivateAccess = true))
		bool bStartMontage;


protected:
	FTimerHandle InputDisableTimerHandle;
	bool bWasInAir;
	bool bCanReceiveInput = true;

	// �Է��� ��Ȱ��ȭ�ϴ� �Լ�
	void DisableInput();

	// �Է��� �ٽ� Ȱ��ȭ�ϴ� �Լ�
	void EnableInput();

private:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = Attack, Meta = (AllowPrivateAccess = true))
		UAnimMontage* JumpLandMontage;
public:
	void PlayJumpLandMontage();


private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Pawn, Meta = (AllowPrivateAccess = true))
		float CurrentPawnPitch;
};
