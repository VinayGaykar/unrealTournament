// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.
#pragma once 

/**
 *
 **/

#include "UTHUDWidget_CTFSpectator.generated.h"

UCLASS()
class UUTHUDWidget_CTFSpectator : public UUTHUDWidget
{
	GENERATED_UCLASS_BODY()

public:

	virtual void Draw_Implementation(float DeltaTime);
	virtual void DrawSimpleMessage(FText SimpleMessage, float DeltaTime);
protected:

private:
};
