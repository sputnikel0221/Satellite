// Fill out your copyright notice in the Description page of Project Settings.


#include "SatelliteAnimInstance.h"
#include "SatelliteCharacter.h"

USatelliteAnimInstance::USatelliteAnimInstance()
{
	CurrentPawnSpeed = 0.0f;
	CurrentPawnDirection = 0.0f;
	IsInAir = false;
}

void USatelliteAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
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
		IsInAir = Character->GetMovementComponent()->IsFalling();
		CurrentPawnDirection = Character->GetDirection();
		D2_LOG(Warning, TEXT("Angle is : %f"), CurrentPawnDirection);
	}


}
