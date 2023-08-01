// Fill out your copyright notice in the Description page of Project Settings.


#include "SatelliteAnimInstance.h"
#include "SatelliteCharacter.h"
#include "SPController.h"
#include "TimerManager.h"
#include "Kismet/KismetMathLibrary.h"

USatelliteAnimInstance::USatelliteAnimInstance()
{
	CurrentPawnSpeed = 0.0f;
	CurrentPawnDirection = 0.0f;
	bIsInAir = false;
	bWasInAir = false;
	bStartMontage = false;
	CurrentPawnPitch = 0.0f;

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
	ASatelliteCharacter* SCharacter = Cast<ASatelliteCharacter>(Pawn);
	if (SCharacter)
	{
		CurrentPawnPitch = SCharacter->GetPitch();
		bIsInAir = SCharacter->GetMovementComponent()->IsFalling();
		CurrentPawnDirection = SCharacter->GetDirection();
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

	// Foot IK 실행/  DeltaTime값 대입
	FootIK(1.0f);
}

/// JumpLandMontage 1배속으로 실행
void USatelliteAnimInstance::PlayJumpLandMontage()
{
	Montage_Play(JumpLandMontage, 1.0f);
	D2_LOG(Warning, TEXT("LandAnim Start"));
	bStartMontage = false;
}

/// Foot IK를 수행 - BP용 변수 RRot / RIK 등을 지속적으로 계산
void USatelliteAnimInstance::FootIK(float DeltaTime)
{
	Character = Cast<ACharacter>(TryGetPawnOwner());

	// 점프중인 경우가 아니라면,
	if (Character && !Character->GetCharacterMovement()->IsFalling())
	{
		// Linetrace의 무시할 목록에 Character를  집어넣음
		IgnoreActors.Emplace(Character);

		// Foot 소켓의 이름을 각각 대입, 반환값은 <충돌했는지, 시작점과 충돌한 객체와의 거리>
		TTuple<bool, float> Foot_R = CapsuleDistance("ik_foot_r", Character);
		TTuple<bool, float> Foot_L = CapsuleDistance("ik_foot_l", Character);

		// 결과값중에서 '충돌했는지'에 대한 부분이 한쪽 발이라도 참이라면,
		if (Foot_L.Get<0>() || Foot_R.Get<0>())
		{
			// ? : 와 같은 문장. 다른점은 조건이 가장 뒤에 나온다. 따라서 L의거리 >= R의거리라면, L의 거리를 Selectfloat로 둔다.
			const float Selectfloat = UKismetMathLibrary::SelectFloat(Foot_L.Get<1>(), Foot_R.Get<1>(), Foot_L.Get<1>() >= Foot_R.Get<1>());
			// 현재값, 추가계산한 목표값, (델타타임, 속도) = 둘다 보간속도에 관련
			Displacement = FMath::FInterpTo(Displacement, (Selectfloat - 98.f) * -1.f, DeltaTime, IKInterpSpeed);

			// 반환값은 <충돌이 발생했는지 / 충돌한객체와의 거리 / 충돌한 표면의 법선벡터>
			TTuple<bool, float, FVector> FootTrace_R = FootLineTrace("ik_foot_r", Character);
			TTuple<bool, float, FVector> FootTrace_L = FootLineTrace("ik_foot_l", Character);

			// 오른발의 충돌한객체와의 거리 / 충돌한 표면의 법선벡터 값을 받아온다. 그 값에 따른 회전을 생성한다.
			const float Distance_R = FootTrace_R.Get<1>();
			const FVector FootRVector(FootTrace_R.Get<2>());
			const FRotator MakeRRot(UKismetMathLibrary::DegAtan2(FootRVector.X, FootRVector.Z) * -1.f, 0.f, UKismetMathLibrary::DegAtan2(FootRVector.Y, FootRVector.Z));

			// 오른발의 Rotation / 실수를 보간한다. 해당 값은 나중에 BP에서 쓰인다.
			RRot = FMath::RInterpTo(RRot, MakeRRot, DeltaTime, IKInterpSpeed);
			RIK = FMath::FInterpTo(RIK, (Distance_R - 110.f) / -45.f, DeltaTime, IKInterpSpeed);

			// 왼발의 ~ 회전을 생성한다.
			const float Distance_L = FootTrace_L.Get<1>();
			const FVector FootLVector(FootTrace_L.Get<2>());
			const FRotator MakeLRot(UKismetMathLibrary::DegAtan2(FootLVector.X, FootLVector.Z) * -1.f, 0.f, UKismetMathLibrary::DegAtan2(FootLVector.Y, FootLVector.Z));

			// 왼발의 ~ BP에서 쓰인다.
			LRot = FMath::RInterpTo(LRot, MakeLRot, DeltaTime, IKInterpSpeed);
			LIK = FMath::FInterpTo(LIK, (Distance_L - 110.f) / -45.f, DeltaTime, IKInterpSpeed);
		}
	}
	// 점프중이라면, 원래 위치와 회전으로 돌아감. (그냥 else로 퉁치면 안될 것 같은데..)
	else
	{
		LRot = FMath::RInterpTo(LRot, FRotator::ZeroRotator, DeltaTime, IKInterpSpeed);
		LIK = FMath::FInterpTo(LIK, 0.f, DeltaTime, IKInterpSpeed);

		RRot = FMath::RInterpTo(RRot, FRotator::ZeroRotator, DeltaTime, IKInterpSpeed);
		RIK = FMath::FInterpTo(RIK, 0.f, DeltaTime, IKInterpSpeed);
	}
}

/// SocketName을 바탕으로 소켓을 찾아 Linetrace를 진행, 해당 결과를 리턴. <충돌했는지, 시작점과 충돌한 객체와의 거리>
TTuple<bool, float> USatelliteAnimInstance::CapsuleDistance(FName SocketName, ACharacter* Char)
{
	// 캐릭터 매시 위치 // 캡슐의 위치로 수정해도 될듯
	const FVector WorldLocation{ Char->GetMesh()->GetComponentLocation() };
	
	// 소켓의 위치
	const FVector SocketLocation{ Char->GetMesh()->GetSocketLocation(SocketName) };

	// 소켓의 위치에 BreakVector값을 z로 둠 / Linetrace의 Start설정
	const FVector BreakVector{ WorldLocation + FVector(0.f,0.f,98.f) };
	const FVector Start{ SocketLocation.X,SocketLocation.Y,BreakVector.Z };

	// Linetrace의 End설정.  
	const FVector End{ Start - FVector(0.f,0.f,151.f) };

	// Linetrace 진행, 디버그도 진행
	FHitResult HitResult;
	UKismetSystemLibrary::LineTraceSingle(
		this,
		Start,
		End,
		ETraceTypeQuery::TraceTypeQuery1,
		false,
		IgnoreActors,
		EDrawDebugTrace::ForOneFrame,
		HitResult,
		false);

	// HitResult의 bBlockingHit으로 충돌이 발생했는지 파악하고, Result에 해당 값 대입
	const bool Result(HitResult.bBlockingHit);

	// Distance와 같이 Tuple을 만들어 리턴
	return MakeTuple(Result, HitResult.Distance);
}

/// SocketName을 바탕으로 Linetrace를 진행, <충돌이 발생했는지 / 충돌한객체와의 거리 / 충돌한 표면의 법선벡터>
TTuple<bool, float, FVector> USatelliteAnimInstance::FootLineTrace(FName SocketName, ACharacter* Char)
{
	// 소켓과 Root의 위치를 가져온다.
	const FVector SocketLocation{ Char->GetMesh()->GetSocketLocation(SocketName) };
	const FVector RootLocation(Char->GetMesh()->GetSocketLocation("root"));

	// 소켓의 위치는 X, Y / Root의 위치는 Z를 반영하여 Start를 생성한다.
	const FVector Start{ SocketLocation.X,SocketLocation.Y,RootLocation.Z };

	// Start점을 기준으로 +-105를 Linetrace한다. 디버그는 진행X
	FHitResult HitResult;
	UKismetSystemLibrary::LineTraceSingle(
		this,
		Start + FVector(0.f, 0.f, 105.f),
		Start + FVector(0.f, 0.f, -105.f),
		ETraceTypeQuery::TraceTypeQuery1,
		false,
		IgnoreActors,
		EDrawDebugTrace::None,
		HitResult,
		false);

	// 충돌이 발생했는지 확인 후 대입
	const bool Result(HitResult.bBlockingHit);

	// 충돌이 발생했다면, <충돌이 발생함 / 충돌한객체와의 거리 / 충돌한 표면의 법선벡터>
	if (HitResult.bBlockingHit)
	{
		return MakeTuple(Result, HitResult.Distance, HitResult.Normal);
	}
	// 발생하지 않았다면, <충돌이 발생 안함 / 999라는 의미없는 거리 / 의미없는 제로벡터>
	else
	{
		return MakeTuple(Result, 999.f, FVector::ZeroVector);
	}
}
