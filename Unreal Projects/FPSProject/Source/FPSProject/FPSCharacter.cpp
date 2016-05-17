// Fill out your copyright notice in the Description page of Project Settings.

#include "FPSProject.h"
#include "FPSCharacter.h"

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
/*
	End UDP Includes
*/

#include <errno.h>
 
#define BUFLEN 512  //Max length of buffer
#define PORT 8888   //The port on which to listen for incoming data

int errno;  

// Sets default values
AFPSCharacter::AFPSCharacter():
m_pitch(0), m_roll(0), m_yaw(0),m_utm_x(0), m_utm_y(0), m_alt(0)
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    
}

AFPSCharacter::~AFPSCharacter()
{
    close(m_socket);
}

// Called when the game starts or when spawned
void AFPSCharacter::BeginPlay()
{
   	char temp_str[256];
	Super::BeginPlay();
	if (GEngine)
	{
	    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("We are using FPSCharacter!"));
	}
	
	deltaTime = 0;


	struct sockaddr_in si_me;

	//create a UDP socket
	if ((m_socket=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
	{
        sprintf(temp_str,"SOCKET ::error is : %s\n", strerror(errno));
		if (GEngine)
		{
		    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, temp_str);
		}
        return;
	}

	// zero out the structure
	memset((char *) &si_me, 0, sizeof(si_me));

	si_me.sin_family = AF_INET;
	si_me.sin_port = htons(PORT);
	si_me.sin_addr.s_addr = htonl(INADDR_ANY);

	//bind socket to port
	if( bind(m_socket , (struct sockaddr*)&si_me, sizeof(si_me) ) == -1)
	{
        sprintf(temp_str,"BIND :: error is : %s\n", strerror(errno));
		if (GEngine)
		{
		    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, temp_str);
		}
        m_socket = -1;
        return;
	}
	fcntl(m_socket,O_NONBLOCK);
}


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


// Called every frame
void AFPSCharacter::Tick( float DeltaTime )
{
	char buf[BUFLEN];
	struct sockaddr_in si_other;
	int slen = sizeof(si_other) , recv_len = 0;
	char temp_str[256];
    double *telem;
	Super::Tick( DeltaTime );
	deltaTime += DeltaTime;
    if (m_socket == -1)
    {
        return;
    }
	if ((recv_len = recvtimeout(m_socket, buf, BUFLEN, 10 ,(struct sockaddr *) &si_other, (socklen_t *)&slen)) == -1)
	{   
        sprintf(temp_str,"error is : %s\n", strerror(errno));
		if (GEngine)
		{
		    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, temp_str);
		}
	}

	if (recv_len > 0)
	{

		//print details of the client/peer and the data received
		sprintf(temp_str,"Received packet from %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
		if (0) //(GEngine)
		{
		    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, temp_str);
		}
        
        telem = (double*) buf;
        double utm_x = telem[0];
        double utm_y = telem[1];
        double alt = telem[2];
        double roll = telem[3];
        double pitch = telem[4];
        double yaw = telem[5];


		sprintf(temp_str,"lat:%f,lon:%f,alt:%f,roll:%f,pitch:%f,yaw:%f" ,utm_x,utm_y,alt,roll,pitch,yaw);

        if(m_yaw != yaw) 
        {
            float difYaw = yaw - m_yaw;
            this->AddControllerYawInput(difYaw);
            m_yaw = yaw;
        }
        if(m_pitch != pitch) 
        {
            float dif = pitch - m_pitch;
            this->AddControllerPitchInput(dif);
            m_pitch = pitch;
        }

        if(m_roll != roll) 
        {
            float dif = roll - m_roll;
            this->AddControllerRollInput(dif);
            m_roll = roll;
        }

        if(m_utm_x != utm_x)
        {
            if ((int)m_utm_x != 0)
            {
                float dif = utm_x - m_utm_x;
	            const FRotator Rotation = Controller->GetControlRotation();
	            const FVector Direction = FRotationMatrix(Rotation).GetScaledAxis(EAxis::Y);
	            // add movement in that direction
	            AddMovementInput(Direction, dif);
	            if (GEngine)
	            {
                    sprintf(temp_str,"m_utm_x diff : %f",dif);
	                GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, temp_str);

	            }
            }
            m_utm_x = utm_x;
        }

        if(m_alt != alt)
        {
            if ((int)m_alt != 0)
            {
                float dif = alt - m_alt;

	            const FRotator Rotation = Controller->GetControlRotation();
	            const FVector Direction = FRotationMatrix(Rotation).GetScaledAxis(EAxis::Z);
	            // add movement in that direction
	            AddMovementInput(GetActorUpVector() , dif);
	            if (GEngine)
	            {
                    sprintf(temp_str,"m_utm_z diff : %f",dif);
	                GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, temp_str);

	            }
            }
            m_alt = alt;
        }

        if(m_utm_y != utm_y)
        {
            if ((int)m_utm_y != 0)
            {

                float dif = utm_y - m_utm_y;
	            // find out which way is forward
	            FRotator Rotation = Controller->GetControlRotation();
	            const FVector Direction = FRotationMatrix(Rotation).GetScaledAxis(EAxis::X);
	            AddMovementInput(Direction, dif);
	            if (GEngine)
	            {
                    sprintf(temp_str,"m_utm_y diff : %f",dif);
	                GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, temp_str);
	            }
            }
            m_utm_y = utm_y;
        }
        //FRotator rot(30, 45, 5);
        //this->SetActorRotation(rot);
        
		if (0) //(GEngine)
		{
		    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, temp_str);
		}
	}	
}

// Called to bind functionality to input
void AFPSCharacter::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	Super::SetupPlayerInputComponent(InputComponent);
        // set up gameplay key bindings
        InputComponent->BindAxis("MoveForward", this, &AFPSCharacter::MoveForward);
        InputComponent->BindAxis("MoveRight", this, &AFPSCharacter::MoveRight);
        InputComponent->BindAction("Jump", IE_Pressed, this, &AFPSCharacter::OnStartJump);
        InputComponent->BindAction("Jump", IE_Released, this, &AFPSCharacter::OnStopJump);
}

void AFPSCharacter::MoveForward(float Value)
{
    
	if ( (Controller != NULL) && (Value != 0.0f) )
	{
	    // find out which way is forward
	    FRotator Rotation = Controller->GetControlRotation();
	    // Limit pitch when walking or falling
	    if (GetCharacterMovement()->IsMovingOnGround() || GetCharacterMovement()->IsFalling() )
	    {
		Rotation.Pitch = 0.0f;
	    }
	    // add movement in that direction
	    const FVector Direction = FRotationMatrix(Rotation).GetScaledAxis(EAxis::X);
	    AddMovementInput(Direction, Value);
	}
}

void AFPSCharacter::MoveRight(float Value)
{

	if ( (Controller != NULL) && (Value != 0.0f) )
	{
	    // find out which way is right
	    const FRotator Rotation = Controller->GetControlRotation();
	    const FVector Direction = FRotationMatrix(Rotation).GetScaledAxis(EAxis::Y);
	    // add movement in that direction
	    AddMovementInput(Direction, Value);
	}
}

void AFPSCharacter::OnStartJump()
{
   bPressedJump = true;
}
void AFPSCharacter::OnStopJump()
{
   bPressedJump = false;
}


