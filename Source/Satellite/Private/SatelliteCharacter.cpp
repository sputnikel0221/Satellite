// Copyright Epic Games, Inc. All Rights Reserved.

#include "SatelliteCharacter.h"
#include "SatelliteAnimInstance.h"

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
	bUseControllerRotationYaw = true; //카메라 방향에 따라 회전하도록
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = false; // Character moves in the direction of input...	 false로 수정
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 850.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

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
}

void ASatelliteCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Add Input Mapping Context
	/*
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
	*/

}

//////////////////////////////////////////////////////////////////////////
// Input

void ASatelliteCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// 새로 만든 입력
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
	//AddMovementInput(GetActorRightVector(), RLValue); // 이건 회전임
	AddMovementInput(FRotationMatrix(GetControlRotation()).GetUnitAxis(EAxis::Y), RLValue);
}

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


void ASatelliteCharacter::LookUp(float MouseValue)
{
	AddControllerPitchInput(MouseValue);
}

void ASatelliteCharacter::Turn(float MouseValue)
{
	AddControllerYawInput(MouseValue);
}




/*
void ASatelliteCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}
*/

/* 쓸지도 모르겠다..
void ASatelliteCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}
*/




