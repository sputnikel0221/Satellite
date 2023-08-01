// Fill out your copyright notice in the Description page of Project Settings.


#include "SPFootIK.h"

#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/CapsuleComponent.h"

// Sets default values for this component's properties
USPFootIK::USPFootIK()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	m_IKAdjustOffset = 5.0f; //발의 위치를 적절히 조정하는 어떤 threshold값
	m_bDebug = false;
}


// Called when the game starts
void USPFootIK::BeginPlay()
{
	Super::BeginPlay();

	D1_LOG(Warning);


	Character = Cast<ACharacter>(GetOwner());
	if (Character == nullptr) return;
	CapsuleHalfHeight = Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
}


// Called every frame
void USPFootIK::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);


	FootTraceInfo pTrace_Left = FootTrace(m_fIKTraceDistance, m_sIKSocketName_LeftFoot);
	FootTraceInfo pTrace_Right = FootTrace(m_fIKTraceDistance, m_sIKSocketName_RightFoot);

	//! Update foot rotation from Impact normal
	Update_FootRotation(m_fDeltaTime, NormalToRotator(pTrace_Left.ImpactLocation), &m_stIKAnimValuse.m_pFootRotation_Left, m_fIKFeetInterpSpeed);
	Update_FootRotation(m_fDeltaTime, NormalToRotator(pTrace_Right.ImpactLocation), &m_stIKAnimValuse.m_pFootRotation_Right, m_fIKFeetInterpSpeed);
}


void USPFootIK::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);


}

// sSocket을 바탕으로 linetrace를 진행하고, fTraceDistance만큼 빼주어 최종 발의 위치를 구하는 코드
FootTraceInfo USPFootIK::FootTrace(float fTraceDistance, FName sSocket)
{
	FootTraceInfo thisTraceInfo;

	//! 1 - Set Linetraces startpoint and end point
	//!	Linetrace를 위한 시작점과 끝점을 지정하는 기능
	//! 매개변수로 넣은 sSocket의 위치(높이는 캡슐)를 start로 두고, sSocket의 위치로부터 end를 구한다.
	FVector pSocketLocation = Character->GetMesh()->GetSocketLocation(sSocket);
	FVector pLine_Start = FVector(pSocketLocation.X, pSocketLocation.Y, Character->GetActorLocation().Z);
	FVector pLine_End = FVector(pSocketLocation.X, pSocketLocation.Y
		, (Character->GetActorLocation().Z - CapsuleHalfHeight) - fTraceDistance);

	//! 2 - Process Line Trace
	//! 실제 Linetrace를 진행

	//LineTrace를 무시할 엑터로 플레이어캐릭터 넣음
	FHitResult pHitResult;
	TArray<AActor*> pIgnore;
	pIgnore.Add(GetOwner());

	// m_bDebug == true여야만 디버그모드 동작
	EDrawDebugTrace::Type eDebug = EDrawDebugTrace::None;
	if (m_bDebug == true) eDebug = EDrawDebugTrace::ForOneFrame;

	// Linetrace
	bool bResult = UKismetSystemLibrary::LineTraceSingle(GetWorld(), pLine_Start, pLine_End,
		UEngineTypes::ConvertToTraceType(ECC_Visibility), true, pIgnore, eDebug, pHitResult, true);

	//! 3 - Set ImpactNormal and Offset from HitResult
	if (pHitResult.IsValidBlockingHit() == true)
	{
		// ImpactNormal을 통해 충돌한 액터의 법선벡터를 가져옴
		thisTraceInfo.ImpactLocation = pHitResult.ImpactNormal; //원래 밖에 빠져있었음

		// ImpactPoint를 통해 충돌 지점을 가져오고, TraceEnd라는 linetrace종료지점과 뺀 거리를 구함
		float fImpactLegth = (pHitResult.ImpactPoint - pHitResult.TraceEnd).Size();

		// fOffset이라는 땅의 Offset값 = 보간 + 발의 위치 - 캡슐밖으로 나가기 위한 값
		thisTraceInfo.fOffset = m_IKAdjustOffset + (fImpactLegth - fTraceDistance);
	}
	else
	{
		thisTraceInfo.fOffset = 0.0f;
	}

	return thisTraceInfo;
}

// Vector를 Rotator로 변환
FRotator USPFootIK::NormalToRotator(FVector pVector)
{
	float fAtan2_1 = UKismetMathLibrary::DegAtan2(pVector.Y, pVector.Z);
	float fAtan2_2 = UKismetMathLibrary::DegAtan2(pVector.X, pVector.Z);
	fAtan2_2 *= -1.0f;
	FRotator pResult = FRotator(fAtan2_2, 0.0f, fAtan2_1);

	return pResult;
}

// RInterpTo를 통해 서서히 회전
void USPFootIK::Update_FootRotation(float fDeltaTime, FRotator pTargetValue, FRotator* pFootRotatorValue, float fInterpSpeed)
{
	//! Set Foot Rotation value with FInterpTo
	FRotator pInterpRotator = UKismetMathLibrary::RInterpTo(*pFootRotatorValue, pTargetValue, fDeltaTime, fInterpSpeed);
	*pFootRotatorValue = pInterpRotator;
}

