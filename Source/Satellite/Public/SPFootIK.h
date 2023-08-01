// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Satellite.h"
#include "Components/ActorComponent.h"
#include "SPFootIK.generated.h"

//! Line Trace info struct
typedef struct FootTraceInfo
{
	float	fOffset;
	FVector ImpactLocation;
};

//! IK Anim Instance Value struct
USTRUCT(Atomic, BlueprintType)
struct FAnimValue
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float		m_fEffectorLocation_Left;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float		m_fEffectorLocation_Right;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FRotator	m_pFootRotation_Left;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FRotator	m_pFootRotation_Right;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float		m_fHipOffset;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SATELLITE_API USPFootIK : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USPFootIK();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override; //추가한 부분

	//FootTrace & Vars
public:
	FootTraceInfo FootTrace(float fTraceDistance, FName sSocket);
private:
	UPROPERTY()
		class ACharacter* Character;
	float CapsuleHalfHeight;
public:
	// Enable Debug Mode
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Debug")
		bool m_bDebug;
	//! Distance between the floor and the foot
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Values")
		float m_IKAdjustOffset;


private:
	//! Get ground normal
	FRotator NormalToRotator(FVector pVector);
	//! Process IK Foot rotation from ground normal
	void Update_FootRotation(float fDeltaTime, FRotator pTargetValue, FRotator* pFootRotatorValue, float fInterpSpeed);


	// 추가한 변수
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	//! IK Anim Instance Value struct
	FAnimValue m_stIKAnimValuse;
private:

	//! IK Active state
	bool m_bActive = false;

	float m_fDeltaTime = 0.0f;
public:
	//! Default Values

	//! Left foot bone name
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Values Socket")
		FName m_sIKSocketName_LeftFoot;
	//! Right foot bone name
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Values Socket")
		FName m_sIKSocketName_RightFoot;
	//! Max Linetrace distance
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Values")
		float m_fIKTraceDistance;
	//! Hip(pelvis) move speed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Values")
		float m_fIKHipsInterpSpeed;

	//! Leg joing move speed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Values")
		float m_fIKFeetInterpSpeed;

	//추가
	/*
public:
	//! Set foot bone name from skeletalmesh
	void Set_IKSocketName(FString sSocName_LeftFoot, FString sSocName_RightFoot);

	//! Reset IK State
	void IK_ResetVars();

	//! Set IK Active state
	void SetIKActive(bool bActive);

	//! Set IK Debug state
	void SetIKDebug(bool bActive);

private:
	//! IK process update
	void IK_Update(float fDeltaTime);

	//! IK Debug mode update
	void IK_Debug();

	//! process IK Foot,leg Z Offset
	void IK_Update_FootOffset(float fDeltaTime, float fTargetValue, float* fEffectorValue, float fInterpSpeed);

	//! Process IK characters capsule component Z Offset
	void IK_Update_CapsuleHalfHeight(float fDeltaTime, float fHipsShifts, bool bResetDefault);

	//! Check owner character is moving
	bool IsMoving();
	*/
public:
	FORCEINLINE FAnimValue GetIKAnimValue() { return m_stIKAnimValuse; }
	FORCEINLINE bool GetIKDebugState() { return m_bDebug; }
};
