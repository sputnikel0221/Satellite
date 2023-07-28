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

	// Mesh�� AnimInstance�� ����� Pawn���κ��� ���� ������
	CurrentPawnSpeed = Pawn->GetVelocity().Size();


	// IsInAir�� Movement���� ������ �� �ִ�.
	// GetDirection���� angle�� �����´�.
	ASatelliteCharacter* Character = Cast<ASatelliteCharacter>(Pawn);
	if (Character)
	{
		IsInAir = Character->GetMovementComponent()->IsFalling();
		CurrentPawnDirection = Character->GetDirection();
		D2_LOG(Warning, TEXT("Angle is : %f"), CurrentPawnDirection);
	}


}
