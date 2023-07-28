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
	
	//bWasInAir = false;
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


	// IsInAir는 Movement에서 가져올 수 있다.
	// GetDirection으로 angle을 가져온다.
	ASatelliteCharacter* Character = Cast<ASatelliteCharacter>(Pawn);
	if (Character)
	{
		bIsInAir = Character->GetMovementComponent()->IsFalling();
		CurrentPawnDirection = Character->GetDirection();
		//D2_LOG(Warning, TEXT("Angle is : %f"), CurrentPawnDirection);

		/// 점프후 착지시, 멈춤 구현
		// bIsInAir 값이 true에서 false가 될 때 입력을 비활성화하도록 처리
		if (bWasInAir && !bIsInAir)
		{
			D2_LOG(Error, TEXT("Jump Landing"));
			//Timer호출
			auto SPController = Cast<ASPController>(Character->GetOwner());
			if (SPController)
			{
				// 먼저 입력을 비활성화
				Character->DisableInput(SPController);

				// Timer를 통해 해당 초 뒤에 enable로 바꿔놓음
				GetWorld()->GetTimerManager().SetTimer(
					InputDisableTimerHandle,
					FTimerDelegate::CreateLambda(
						[this]()->void {
							APawn* Pawn = TryGetPawnOwner();
							ASatelliteCharacter* Character = Cast<ASatelliteCharacter>(Pawn);
							auto SPController = Cast<ASPController>(Character->GetOwner());
							Character->EnableInput(SPController);
						}), 0.18f, false);

				// Timer해제
				InputDisableTimerHandle.Invalidate();
			}
		}

		bWasInAir = bIsInAir;
	}


}

