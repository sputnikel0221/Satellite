// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "EngineMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(Satellite, Log, All);


#define CODE_LINE (FString(__FUNCTION__) + TEXT("(") + FString::FromInt(__LINE__) + TEXT(")"))

#define D1_LOG(Var) UE_LOG(Satellite, Var, TEXT("%s"), *CODE_LINE)
#define D2_LOG(Var, Format, ...) UE_LOG(Satellite, Var, TEXT("%s %s"), *CODE_LINE, *FString::Printf(Format, ##__VA_ARGS__))
#define ABCHECK(Expr, ...) {if(!(Expr)) {d2_LOG(Error, TEXT("ASSERTION : %s"), TEXT("'"#Expr"'")); return __VA_ARGS__;}}
