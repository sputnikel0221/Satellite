// Fill out your copyright notice in the Description page of Project Settings.


#include "SatelliteAnimInstance.h"
#include "SatelliteCharacter.h"
#include "SPController.h"
#include "TimerManager.h"

USatelliteAnimInstance::USatelliteAnimInstance()
{
	CurrentPawnSpeed = 0.0f;
	CurrentPawnDirection = 0.0f;
	bIsInAir = false;
	bWasInAir = false;
	bStartMontage = false;
	CurrentPawnPitch =  0.0f;

	static ConstructorHelpers::FObjectFinder<UAnimMontage> JUMPLAND_MONTAGE(TEXT("/Script/Engine.AnimMontage'/Game/Animation/BP_JumpLandMont.BP_JumpLandMont'"));
	if (JUMPLAND_MONTAGE.Succeeded())
	{
		JumpLandMontage = JUMPLAND_MONTAGE.Object;
	}
}

void USatelliteAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{

	/// Animation에 사용될 변수들 초기화
	APawn* Pawn = TryGetPawnOwner();
	if (!IsValid(Pawn))
	{
		return;
	}

	// Mesh에 AnimInstance가 적용된 Pawn으로부터 값을 가져옴
	CurrentPawnSpeed = Pawn->GetVelocity().Size();
	//CurrentPawnPitch = Pawn->GetControlRotation().Pitch;


	// IsInAir는 Movement에서 가져올 수 있다.
	// GetDirection으로 angle을 가져온다.
	ASatelliteCharacter* Character = Cast<ASatelliteCharacter>(Pawn);
	if (Character)
	{
		CurrentPawnPitch = Character->GetPitch();
		bIsInAir = Character->GetMovementComponent()->IsFalling();
		CurrentPawnDirection = Character->GetDirection();
		D2_LOG(Warning, TEXT("Angle is : %f"), CurrentPawnPitch);

		/// 점프후 착지시, 멈춤 구현
		// bIsInAir 값이 true에서 false가 될 때 입력을 비활성화하도록 처리
		if (bWasInAir && !bIsInAir)
		{
			D2_LOG(Error, TEXT("Jump Landing"));
			bStartMontage = true;
			PlayJumpLandMontage();
		}

		bWasInAir = bIsInAir;
	}
}

/// JumpLandMontage 1배속으로 실행
void USatelliteAnimInstance::PlayJumpLandMontage()
{
	Montage_Play(JumpLandMontage, 1.0f); 
	D2_LOG(Warning, TEXT("LandAnim Start"));
	bStartMontage = false;
}
