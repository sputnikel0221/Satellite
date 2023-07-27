// Fill out your copyright notice in the Description page of Project Settings.


#include "SPController.h"

void ASPController::BeginPlay()
{
	FInputModeGameOnly NewInputMode;
	SetInputMode(NewInputMode);
}
