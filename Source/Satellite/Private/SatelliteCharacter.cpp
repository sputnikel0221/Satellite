// Copyright Epic Games, Inc. All Rights Reserved.

#include "SatelliteCharacter.h"
#include "SatelliteAnimInstance.h"
#include "Kismet/KismetMathLibrary.h"
#include "Camera/PlayerCameraManager.h"

// 기존에 추가되어있던 헤더
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"


//////////////////////////////////////////////////////////////////////////
// ASatelliteCharacter

ASatelliteCharacter::ASatelliteCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true; //카메라 방향에 따라 플레이어가 회전하도록
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = false; // Character moves in the direction of input...	 false로 수정
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.0f;			// 점프속도
	GetCharacterMovement()->GravityScale = 2.0f;				// 착지속도
	GetCharacterMovement()->AirControl = 0.25f;					// 공중제어 - 높을수록 공중에서 자유로움
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 100.f;	// 높을수록 멈출때 확 멈춤

	// Create a camera boom (pulls in towards the player if there is a collision)
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 350.0f; // The camera follows at this distance behind the character	
	SpringArm->bUsePawnControlRotation = true; // Rotate the arm based on the controller 

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(SpringArm, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm


	// 3 - 가속 설정
	GetCharacterMovement()->MaxAcceleration = 700.0f;

	// 1 - 캐릭터 매시 
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> SK_GADGET(TEXT("/Script/Engine.SkeletalMesh'/Game/ParagonGadget/Characters/Heroes/Gadget/Meshes/Gadget.Gadget'"));
	if (SK_GADGET.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(SK_GADGET.Object);
	}
	GetMesh()->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, -97.0f), FRotator(0.0f, -90.0f, 0.0f));


	// 2 - 애니메이션 BP 연결 - BP에셋으로부터 CPP AnimInstance클래스를 Mesh의 AnimInstance로 지정
	GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint); //BP애니메이션을 사용하겠다.
	static ConstructorHelpers::FClassFinder<UAnimInstance> SatelliteAnimInstance(TEXT("/Game/Animation/BP_GadgetAnim.BP_GadgetAnim_C"));
	if (SatelliteAnimInstance.Succeeded())
	{
		GetMesh()->SetAnimInstanceClass(SatelliteAnimInstance.Class);
	}

	PawnDirection = 0.0f;
	PawnPitch = 0.0f;
}

void ASatelliteCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	CameraManagerHandle();


}

//////////////////////////////////////////////////////////////////////////
// Input

void ASatelliteCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	PlayerInputComponent->BindAction(TEXT("Jump"), EInputEvent::IE_Pressed, this, &ACharacter::Jump);

	PlayerInputComponent->BindAxis(TEXT("UpDown"), this, &ASatelliteCharacter::UpDown);
	PlayerInputComponent->BindAxis(TEXT("RightLeft"), this, &ASatelliteCharacter::RightLeft);

	PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &ASatelliteCharacter::LookUp);
	PlayerInputComponent->BindAxis(TEXT("Turn"), this, &ASatelliteCharacter::Turn);
}

void ASatelliteCharacter::UpDown(float UDValue)
{
	//AddMovementInput(GetActorForwardVector(), UDValue);
	AddMovementInput(FRotationMatrix(GetControlRotation()).GetUnitAxis(EAxis::X), UDValue);
}

void ASatelliteCharacter::RightLeft(float RLValue)
{
	//AddMovementInput(GetActorRightVector(), RLValue); // 이건 회전
	AddMovementInput(FRotationMatrix(GetControlRotation()).GetUnitAxis(EAxis::Y), RLValue);
}

// 정면벡터로부터 Player가 이동하는 방향의 각을 구한다.
float ASatelliteCharacter::GetDirection()
{
	// 1 이동하는 방향의 벡터 구하기
	FVector Velocity = GetMovementComponent()->Velocity;
	FVector MoveDirection = Velocity.GetSafeNormal();

	// 2 캐릭터가 보고있는 정면 방향 구하기
	FVector ForwardVector = GetActorForwardVector();
	ForwardVector.Normalize();

	// 3 각도 구하기
	float AngleRadian = FMath::Atan2(MoveDirection.Y, MoveDirection.X) - FMath::Atan2(ForwardVector.Y, ForwardVector.X);
	float AngleDegree = FMath::RadiansToDegrees(AngleRadian);
	if (AngleDegree > 180.0f)
	{
		AngleDegree -= 360.0f;
	}
	return AngleDegree;
}


/// pitch 각도값만 계산하는 함수
float ASatelliteCharacter::GetPitch()
{
	// 1. Character의 Controller 가져오기
	AController* PlayerController = Cast<APlayerController>(GetController());

	if (PlayerController)
	{
		// 2. Controller의 Pitch 값 가져오기
		FRotator ControllerRotation = PlayerController->GetControlRotation();
		PawnPitch = ControllerRotation.Pitch;

		if (PawnPitch >= 270)
		{
			PawnPitch -= 360.0f;
		}

		return PawnPitch;
	}

	// Controller가 없는 경우 0을 반환
	return 0.0f;
}

/// PlayerCameraManager를 통해서 실제 카메라의 pitch범위를 정한다.
void ASatelliteCharacter::CameraManagerHandle()
{
	AController* PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController)
	{
		APlayerCameraManager* CameraManager =
			Cast<APlayerCameraManager>(UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0));
		if (CameraManager)
		{
			CameraManager->ViewPitchMax = 45.0f;
			CameraManager->ViewPitchMin = -45.0f;
		}
	}

}



// 컨트롤러가 위,아래로 보는 입력을 실행
void ASatelliteCharacter::LookUp(float MouseValue)
{
	AddControllerPitchInput(MouseValue);
}

// 컨트롤러가 좌,우로 보는 입력을 실행
void ASatelliteCharacter::Turn(float MouseValue)
{
	AddControllerYawInput(MouseValue);
}





