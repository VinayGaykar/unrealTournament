// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.
#include "UnrealTournament.h"
#include "UTCTFFlagBase.h"
#include "Net/UnrealNetwork.h"

AUTCTFFlagBase::AUTCTFFlagBase(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh> FlagBaseMesh (TEXT("StaticMesh'/Game/RestrictedAssets/Proto/UT3_Pickups/Flag/S_Pickups_Base_Flag.S_Pickups_Base_Flag'"));

	Mesh = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("FlagBase"));
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->SetStaticMesh(FlagBaseMesh.Object);
	Mesh->AttachParent = RootComponent;

	Capsule = ObjectInitializer.CreateDefaultSubobject<UCapsuleComponent>(this, TEXT("Capsule"));
	// overlap Pawns, no other collision
	Capsule->SetCollisionResponseToAllChannels(ECR_Ignore);
	Capsule->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	Capsule->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Capsule->InitCapsuleSize(92.f, 134.0f);
	Capsule->OnComponentBeginOverlap.AddDynamic(this, &AUTCTFFlagBase::OnOverlapBegin);
	Capsule->RelativeLocation = FVector(0.0f, 0.0f, 134.0f);
	Capsule->AttachParent = RootComponent;
}

void AUTCTFFlagBase::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AUTCTFFlagBase, MyFlag);
}

void AUTCTFFlagBase::OnOverlapBegin(AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AUTCharacter* Character = Cast<AUTCharacter>(OtherActor);
	if (Character != NULL)
	{
		AUTCTFFlag* CharFlag = Cast<AUTCTFFlag>(Character->GetCarriedObject());
		if ( CharFlag != NULL && CharFlag != CarriedObject && CarriedObject->ObjectState == CarriedObjectState::Home && CharFlag->GetTeamNum() != GetTeamNum() &&
			!GetWorld()->LineTraceTestByChannel(OtherActor->GetActorLocation(), Capsule->GetComponentLocation(), ECC_Pawn, FCollisionQueryParams(), WorldResponseParams) )
		{
			AUTGameState* GS = GetWorld()->GetGameState<AUTGameState>();
			if (GS == NULL || (GS->IsMatchInProgress() && !GS->IsMatchAtHalftime()))
			{
				CharFlag->Score(FName(TEXT("FlagCapture")), CharFlag->HoldingPawn, CharFlag->Holder);
				CharFlag->PlayCaptureEffect();
			}
		}
	}
}

void AUTCTFFlagBase::CreateCarriedObject()
{
	if (TeamFlagTypes.IsValidIndex(TeamNum) && TeamFlagTypes[TeamNum] != NULL)
	{
		CarriedObjectClass = TeamFlagTypes[TeamNum];
	}
	Super::CreateCarriedObject();
	MyFlag = Cast<AUTCTFFlag>(CarriedObject);
	if (MyFlag && MyFlag->GetMesh())
	{
		MyFlag->GetMesh()->ClothBlendWeight = MyFlag->ClothBlendHome;
	}
}

FName AUTCTFFlagBase::GetFlagState()
{
	if (MyFlag != NULL)
	{
		return MyFlag->ObjectState;
	}

	return NAME_None;
}

void AUTCTFFlagBase::RecallFlag()
{
	if (MyFlag != NULL && MyFlag->ObjectState != FName(TEXT("Home")) )
	{
		MyFlag->SendHome();
	}
}

void AUTCTFFlagBase::ObjectWasPickedUp(AUTCharacter* NewHolder, bool bWasHome)
{
	Super::ObjectWasPickedUp(NewHolder, bWasHome);

	if (bWasHome)
	{
		if (!EnemyFlagTakenSound)
		{
			EnemyFlagTakenSound = FlagTakenSound;
		}
		for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
		{
			AUTPlayerController* PC = Cast<AUTPlayerController>(*Iterator);
			if (PC && ((PC->PlayerState && PC->PlayerState->bOnlySpectator) || (PC->GetTeamNum() == GetTeamNum())))
			{
				PC->ClientPlaySound(FlagTakenSound, 1.f);
			}
			else if (PC)
			{
				PC->HearSound(EnemyFlagTakenSound, this, GetActorLocation(), true, false);
			}
		}
	}
}

void AUTCTFFlagBase::ObjectReturnedHome(AUTCharacter* Returner)
{
	Super::ObjectReturnedHome(Returner);

	UUTGameplayStatics::UTPlaySound(GetWorld(), FlagReturnedSound, this);
}
