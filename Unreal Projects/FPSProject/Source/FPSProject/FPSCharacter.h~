// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Character.h"
#include "FPSCharacter.generated.h"

UCLASS()
class FPSPROJECT_API AFPSCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AFPSCharacter();
    ~AFPSCharacter();
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;

	//handles moving forward/backward
	UFUNCTION()
	void MoveForward(float Val);
	//handles strafing
	UFUNCTION()
	void MoveRight(float Val);

   //sets jump flag when key is pressed
   UFUNCTION()
   void OnStartJump();
   //clears jump flag when key is released
   UFUNCTION()
   void OnStopJump();

protected:
	float deltaTime;
	int m_socket;

    float m_pitch, m_roll, m_yaw;
    float m_utm_x, m_utm_y, m_alt;
};

 
