// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "sixDOF.h"
#include "sixDOFPawn.h"

/*
	Start UDP Includes
*/
#include<stdio.h> //printf
#include<string.h> //memset
#include<stdlib.h> //exit(0);
#include<arpa/inet.h>
#include<sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

#include <errno.h>
 
#define BUFLEN 512  //Max length of buffer
#define PORT 8888   //The port on which to listen for incoming data

/*
	End UDP Includes
*/

int AsixDOFPawn::m_socket = -1;

int recvtimeout(int s, char *buf, int len, int timeout,struct sockaddr * si_other,socklen_t * slen)
{
    fd_set fds;
    int n;
    struct timeval tv;

    // set up the file descriptor set
    FD_ZERO(&fds);
    FD_SET(s, &fds);

    // set up the struct timeval for the timeout
    tv.tv_sec = 0;
    tv.tv_usec = timeout;

    // wait until timeout or data received
    n = select(s+1, &fds, NULL, NULL, &tv);
    if (n == 0) return -2; // timeout!
    if (n == -1) return -1; // error

    // data must be here, so do a normal recv()

    return recvfrom(s, buf, len, 0,si_other, slen);
}


AsixDOFPawn::AsixDOFPawn()
{
	// Structure to hold one-time initialization
	struct FConstructorStatics
	{
		ConstructorHelpers::FObjectFinderOptional<UStaticMesh> PlaneMesh;
		FConstructorStatics()
			: PlaneMesh(TEXT("/Game/Flying/Meshes/UFO.UFO"))
		{
		}
	};
	static FConstructorStatics ConstructorStatics;

	// Create static mesh component
	PlaneMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlaneMesh0"));
	PlaneMesh->SetStaticMesh(ConstructorStatics.PlaneMesh.Get());
	RootComponent = PlaneMesh;

	// Create a spring arm component
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm0"));
	SpringArm->AttachTo(RootComponent);
	SpringArm->TargetArmLength = 160.0f; // The camera follows at this distance behind the character	
	SpringArm->SocketOffset = FVector(0.f,0.f,60.f);
	SpringArm->bEnableCameraLag = false;
	SpringArm->CameraLagSpeed = 15.f;

	// Create camera component 
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera0"));
	Camera->AttachTo(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false; // Don't rotate camera with controller

	// Set handling parameters
	Acceleration = 500.f;
	TurnSpeed = 50.f;
	MaxSpeed = 4000.f;
	MinSpeed = 500.f;
	CurrentForwardSpeed = 500.f;


/*odod*/

	deltaTime = 0;
	struct sockaddr_in si_me;


    if (AsixDOFPawn::m_socket == -1 )
    {
	    //create a UDP socket
	    if ((AsixDOFPawn::m_socket=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
	    {
            printf("SOCKET ::error is : %s\n", strerror(errno));
            return;
	    }
        printf("SOCKET is OK! \n");

	    // zero out the structure
	    memset((char *) &si_me, 0, sizeof(si_me));

	    si_me.sin_family = AF_INET;
	    si_me.sin_port = htons(PORT);
	    si_me.sin_addr.s_addr = htonl(INADDR_ANY);

	    //bind socket to port
	    if( bind(AsixDOFPawn::m_socket , (struct sockaddr*)&si_me, sizeof(si_me) ) == -1)
	    {
            printf("BIND :: error is : %s\n", strerror(errno));
            AsixDOFPawn::m_socket = -1;
            return;
	    }
        else
        {
            printf("BIND :: OK\n");
        }
	    fcntl(AsixDOFPawn::m_socket,O_NONBLOCK);
    }
/*odod*/
}

AsixDOFPawn::~AsixDOFPawn()
{
    printf("dying! dying! dying! dying! dying! dying! dying! dying! dying! dying! dying! dying! dying! \n");
    if (AsixDOFPawn::m_socket != -1)
    {
        close(AsixDOFPawn::m_socket);
    }
}


void AsixDOFPawn::OurTick(float DeltaSeconds) 
{ 

    char buf[BUFLEN];
	struct sockaddr_in si_other;
	int slen = sizeof(si_other) , recv_len = 0;
	char temp_str[256];
    double *telem;

    if (m_socket == -1)
    {
        return;
    }
	if ((recv_len = recvtimeout(AsixDOFPawn::m_socket, buf, BUFLEN, 10 ,(struct sockaddr *) &si_other, (socklen_t *)&slen)) == -1)
	{   
        printf("error is : %s\n", strerror(errno));
	}

	if (recv_len > 0)
	{
        printf("Recieved message (len %d)\n",recv_len );
    }
}

void AsixDOFPawn::Tick(float DeltaSeconds)
{
    OurTick(DeltaSeconds);
    //printf("running %f\n",DeltaSeconds);
	const FVector LocalMove = FVector(CurrentForwardSpeed * DeltaSeconds, 0.f, 0.f);

	// Move plan forwards (with sweep so we stop when we collide with things)
	AddActorLocalOffset(LocalMove, true);

	// Calculate change in rotation this frame
	FRotator DeltaRotation(0,0,0);
	DeltaRotation.Pitch = CurrentPitchSpeed * DeltaSeconds;
	DeltaRotation.Yaw = CurrentYawSpeed * DeltaSeconds;
	DeltaRotation.Roll = CurrentRollSpeed * DeltaSeconds;

	// Rotate plane
	AddActorLocalRotation(DeltaRotation);

	// Call any parent class Tick implementation
	Super::Tick(DeltaSeconds);
}

void AsixDOFPawn::NotifyHit(class UPrimitiveComponent* MyComp, class AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);

	// Set velocity to zero upon collision
	CurrentForwardSpeed = 0.f;
}


void AsixDOFPawn::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	check(InputComponent);

	// Bind our control axis' to callback functions
	InputComponent->BindAxis("Thrust", this, &AsixDOFPawn::ThrustInput);
	InputComponent->BindAxis("MoveUp", this, &AsixDOFPawn::MoveUpInput);
	InputComponent->BindAxis("MoveRight", this, &AsixDOFPawn::MoveRightInput);
}

void AsixDOFPawn::ThrustInput(float Val)
{
	// Is there no input?
	bool bHasInput = !FMath::IsNearlyEqual(Val, 0.f);
	// If input is not held down, reduce speed
	float CurrentAcc = bHasInput ? (Val * Acceleration) : (-0.5f * Acceleration);
	// Calculate new speed
	float NewForwardSpeed = CurrentForwardSpeed + (GetWorld()->GetDeltaSeconds() * CurrentAcc);
	// Clamp between MinSpeed and MaxSpeed
	CurrentForwardSpeed = FMath::Clamp(NewForwardSpeed, MinSpeed, MaxSpeed);
}

void AsixDOFPawn::MoveUpInput(float Val)
{
	// Target pitch speed is based in input
	float TargetPitchSpeed = (Val * TurnSpeed * -1.f);

	// When steering, we decrease pitch slightly
	TargetPitchSpeed += (FMath::Abs(CurrentYawSpeed) * -0.2f);

	// Smoothly interpolate to target pitch speed
	CurrentPitchSpeed = FMath::FInterpTo(CurrentPitchSpeed, TargetPitchSpeed, GetWorld()->GetDeltaSeconds(), 2.f);
}

void AsixDOFPawn::MoveRightInput(float Val)
{
	// Target yaw speed is based on input
	float TargetYawSpeed = (Val * TurnSpeed);

	// Smoothly interpolate to target yaw speed
	CurrentYawSpeed = FMath::FInterpTo(CurrentYawSpeed, TargetYawSpeed, GetWorld()->GetDeltaSeconds(), 2.f);

	// Is there any left/right input?
	const bool bIsTurning = FMath::Abs(Val) > 0.2f;

	// If turning, yaw value is used to influence roll
	// If not turning, roll to reverse current roll value
	float TargetRollSpeed = bIsTurning ? (CurrentYawSpeed * 0.5f) : (GetActorRotation().Roll * -2.f);

	// Smoothly interpolate roll speed
	CurrentRollSpeed = FMath::FInterpTo(CurrentRollSpeed, TargetRollSpeed, GetWorld()->GetDeltaSeconds(), 2.f);
}
