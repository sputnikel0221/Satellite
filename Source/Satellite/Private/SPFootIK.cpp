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

	m_IKAdjustOffset = 5.0f; //���� ��ġ�� ������ �����ϴ� � threshold��
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

// sSocket�� �������� linetrace�� �����ϰ�, fTraceDistance��ŭ ���־� ���� ���� ��ġ�� ���ϴ� �ڵ�
FootTraceInfo USPFootIK::FootTrace(float fTraceDistance, FName sSocket)
{
	FootTraceInfo thisTraceInfo;

	//! 1 - Set Linetraces startpoint and end point
	//!	Linetrace�� ���� �������� ������ �����ϴ� ���
	//! �Ű������� ���� sSocket�� ��ġ(���̴� ĸ��)�� start�� �ΰ�, sSocket�� ��ġ�κ��� end�� ���Ѵ�.
	FVector pSocketLocation = Character->GetMesh()->GetSocketLocation(sSocket);
	FVector pLine_Start = FVector(pSocketLocation.X, pSocketLocation.Y, Character->GetActorLocation().Z);
	FVector pLine_End = FVector(pSocketLocation.X, pSocketLocation.Y
		, (Character->GetActorLocation().Z - CapsuleHalfHeight) - fTraceDistance);

	//! 2 - Process Line Trace
	//! ���� Linetrace�� ����

	//LineTrace�� ������ ���ͷ� �÷��̾�ĳ���� ����
	FHitResult pHitResult;
	TArray<AActor*> pIgnore;
	pIgnore.Add(GetOwner());

	// m_bDebug == true���߸� ����׸�� ����
	EDrawDebugTrace::Type eDebug = EDrawDebugTrace::None;
	if (m_bDebug == true) eDebug = EDrawDebugTrace::ForOneFrame;

	// Linetrace
	bool bResult = UKismetSystemLibrary::LineTraceSingle(GetWorld(), pLine_Start, pLine_End,
		UEngineTypes::ConvertToTraceType(ECC_Visibility), true, pIgnore, eDebug, pHitResult, true);

	//! 3 - Set ImpactNormal and Offset from HitResult
	if (pHitResult.IsValidBlockingHit() == true)
	{
		// ImpactNormal�� ���� �浹�� ������ �������͸� ������
		thisTraceInfo.ImpactLocation = pHitResult.ImpactNormal; //���� �ۿ� �����־���

		// ImpactPoint�� ���� �浹 ������ ��������, TraceEnd��� linetrace���������� �� �Ÿ��� ����
		float fImpactLegth = (pHitResult.ImpactPoint - pHitResult.TraceEnd).Size();

		// fOffset�̶�� ���� Offset�� = ���� + ���� ��ġ - ĸ�������� ������ ���� ��
		thisTraceInfo.fOffset = m_IKAdjustOffset + (fImpactLegth - fTraceDistance);
	}
	else
	{
		thisTraceInfo.fOffset = 0.0f;
	}

	return thisTraceInfo;
}

// Vector�� Rotator�� ��ȯ
FRotator USPFootIK::NormalToRotator(FVector pVector)
{
	float fAtan2_1 = UKismetMathLibrary::DegAtan2(pVector.Y, pVector.Z);
	float fAtan2_2 = UKismetMathLibrary::DegAtan2(pVector.X, pVector.Z);
	fAtan2_2 *= -1.0f;
	FRotator pResult = FRotator(fAtan2_2, 0.0f, fAtan2_1);

	return pResult;
}

// RInterpTo�� ���� ������ ȸ��
void USPFootIK::Update_FootRotation(float fDeltaTime, FRotator pTargetValue, FRotator* pFootRotatorValue, float fInterpSpeed)
{
	//! Set Foot Rotation value with FInterpTo
	FRotator pInterpRotator = UKismetMathLibrary::RInterpTo(*pFootRotatorValue, pTargetValue, fDeltaTime, fInterpSpeed);
	*pFootRotatorValue = pInterpRotator;
}

