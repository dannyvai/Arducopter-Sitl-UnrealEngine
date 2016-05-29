// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "sixDOF.h"
#include "sixDOFGameMode.h"
#include "sixDOFPawn.h"

AsixDOFGameMode::AsixDOFGameMode()
{
	// set default pawn class to our flying pawn
	DefaultPawnClass = AsixDOFPawn::StaticClass();
}
