// Copyright Epic Games, Inc. All Rights Reserved.

#include "SatelliteGameMode.h"
#include "SatelliteCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "SPController.h"

ASatelliteGameMode::ASatelliteGameMode()
{
	// set default pawn class to our Blueprinted character
	DefaultPawnClass = ASatelliteCharacter::StaticClass();
	PlayerControllerClass = ASPController::StaticClass();
}
