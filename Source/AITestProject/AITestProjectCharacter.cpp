// Copyright Epic Games, Inc. All Rights Reserved.

#include "AITestProjectCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Blueprint/UserWidget.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "AITestProject.h"
#include "CustomizingMotion/CustomizingMotionComponent.h"
#include "CustomizingMotion/CustomizingMotionWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

AAITestProjectCharacter::AAITestProjectCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// Create CustomizingMotion component
	CustomizingMotionComp = CreateDefaultSubobject<UCustomizingMotionComponent>(TEXT("CustomizingMotion"));

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

void AAITestProjectCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AAITestProjectCharacter::Move);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &AAITestProjectCharacter::Look);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AAITestProjectCharacter::Look);
	}
	else
	{
		UE_LOG(LogAITestProject, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}

	// CustomizingMotion key bindings (T: UI toggle, M: motion loop toggle)
	PlayerInputComponent->BindKey(EKeys::T, IE_Pressed, this, &AAITestProjectCharacter::ToggleMotionUI);
	PlayerInputComponent->BindKey(EKeys::M, IE_Pressed, this, &AAITestProjectCharacter::ToggleMotionLoop);
}

void AAITestProjectCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	// route the input
	DoMove(MovementVector.X, MovementVector.Y);
}

void AAITestProjectCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// route the input
	DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void AAITestProjectCharacter::DoMove(float Right, float Forward)
{
	if (GetController() != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = GetController()->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, Forward);
		AddMovementInput(RightDirection, Right);
	}
}

void AAITestProjectCharacter::DoLook(float Yaw, float Pitch)
{
	if (GetController() != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void AAITestProjectCharacter::DoJumpStart()
{
	// signal the character to jump
	Jump();
}

void AAITestProjectCharacter::DoJumpEnd()
{
	// signal the character to stop jumping
	StopJumping();
}

void AAITestProjectCharacter::ToggleMotionUI()
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (PC == nullptr)
	{
		return;
	}

	// 이미 생성된 위젯이 있으면 토글
	if (MotionWidget != nullptr)
	{
		if (MotionWidget->IsInViewport())
		{
			MotionWidget->RemoveFromParent();
			PC->bShowMouseCursor = false;
			PC->SetInputMode(FInputModeGameOnly());
		}
		else
		{
			MotionWidget->AddToViewport();
			PC->bShowMouseCursor = true;
			PC->SetInputMode(FInputModeGameAndUI());
		}
		return;
	}

	// WBP 클래스 동적 로드 (UUserWidget 기반 — Parent Class 설정 여부 무관)
	TSubclassOf<UUserWidget> WClass = LoadClass<UUserWidget>(
		nullptr,
		TEXT("/Game/UI/Customizing/WBP_CustomizingMotionWidget.WBP_CustomizingMotionWidget_C"));

	if (WClass == nullptr)
	{
		UE_LOG(LogTemplateCharacter, Warning, TEXT("ToggleMotionUI: WBP_CustomizingMotionWidget 로드 실패"));
		return;
	}

	UUserWidget* NewWidget = CreateWidget<UUserWidget>(PC, WClass);
	if (NewWidget == nullptr)
	{
		UE_LOG(LogTemplateCharacter, Warning, TEXT("ToggleMotionUI: CreateWidget 실패"));
		return;
	}

	MotionWidget = NewWidget;
	MotionWidget->AddToViewport();

	// C++ Parent Class 설정 시 InitWidget 호출
	UCustomizingMotionWidget* TypedWidget = Cast<UCustomizingMotionWidget>(MotionWidget);
	if (TypedWidget != nullptr)
	{
		TypedWidget->InitWidget(CustomizingMotionComp);
	}

	PC->bShowMouseCursor = true;
	PC->SetInputMode(FInputModeGameAndUI());
}

void AAITestProjectCharacter::ToggleMotionLoop()
{
	if (!CustomizingMotionComp) return;
	if (CustomizingMotionComp->IsPlaying())
	{
		CustomizingMotionComp->StopMotionLoop();
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow, TEXT("[M] Motion Loop Stopped"));
	}
	else
	{
		CustomizingMotionComp->StartMotionLoop();
		if (CustomizingMotionComp->IsPlaying())
		{
			if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, TEXT("[M] Motion Loop Started"));
		}
		else
		{
			if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT("[M] Cannot play"));
		}
	}
}
