// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "FPSCharacter.h"
#include "GameFramework/GameMode.h"
#include "FPSGameMode.generated.h"

/**
 * 
 */
UCLASS()
class FPSPROJECT_API AFPSGameMode : public AGameMode
{
	GENERATED_BODY()
	AFPSGameMode(const FObjectInitializer& ObjectInitializer);
	virtual void StartPlay() override; // Note that engine version 4.3 changed this method's name to StartPlay(), because of this engine versions before 4.3, or older tutorials, use BeginPlay()
	
	
	
};
