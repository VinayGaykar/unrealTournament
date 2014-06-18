// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "UTWeapon.generated.h"

USTRUCT(BlueprintType)
struct FInstantHitDamageInfo
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DamageInfo")
	int32 Damage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DamageInfo")
	TSubclassOf<UDamageType> DamageType;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DamageInfo")
	float Momentum;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DamageInfo")
	float TraceRange;

	FInstantHitDamageInfo()
		: Damage(10), TraceRange(10000.0f)
	{}
};

UCLASS(Blueprintable, Abstract, NotPlaceable)
class AUTWeapon : public AUTInventory
{
	GENERATED_UCLASS_BODY()

	friend class UUTWeaponState;
	friend class UUTWeaponStateInactive;
	friend class UUTWeaponStateActive;
	friend class UUTWeaponStateEquipping;
	friend class UUTWeaponStateUnequipping;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TSubclassOf<class AUTWeaponAttachment> AttachmentType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, ReplicatedUsing=OnRep_Ammo, Category = "Weapon")
	int32 Ammo;
	/** handles weapon switch when out of ammo, etc
	 * NOTE: called on server if owner is locally controlled, on client only when owner is remote
	 */
	UFUNCTION()
	virtual void OnRep_Ammo();
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Weapon")
	int32 MaxAmmo;
	/** ammo cost for one shot of each fire mode */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TArray<int32> AmmoCost;
	/** projectile class for fire mode (if applicable) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TArray< TSubclassOf<AUTProjectile> > ProjClass;
	/** instant hit data for fire mode (if applicable) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TArray<FInstantHitDamageInfo> InstantHitInfo;
	/** firing state for mode, contains core firing sequence and directs to appropriate global firing functions */
	UPROPERTY(Instanced, EditAnywhere, EditFixedSize, BlueprintReadWrite, Category = "Weapon")
	TArray<class UUTWeaponStateFiring*> FiringState;
#if WITH_EDITORONLY_DATA
protected:
	/** class of firing state to use (workaround for editor limitations - editinlinenew doesn't work) */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TArray< TSubclassOf<class UUTWeaponStateFiring> > FiringStateType;
public:
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) OVERRIDE;
	virtual void PreSave() OVERRIDE
	{
		Super::PreSave();
		ValidateFiringStates();
	}
	virtual void PostLoad() OVERRIDE
	{
		Super::PostLoad();
		ValidateFiringStates();
	}
	void ValidateFiringStates();
#endif

	/** time between shots, trigger checks, etc */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon", meta = (ClampMin = "0.1"))
	TArray<float> FireInterval;
	/** firing spread (random angle added to shots) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TArray<float> Spread;
	/** Sound to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TArray<USoundBase*> FireSound;
	/** looping (ambient) sound to set on owner while firing */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TArray<USoundBase*> FireLoopingSound;
	/** AnimMontage to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TArray<UAnimMontage*> FireAnimation;
	/** particle component for muzzle flash */
	UPROPERTY(EditAnywhere, Category = "Weapon")
	TArray<UParticleSystemComponent*> MuzzleFlash;
	/** particle system for firing effects (instant hit beam and such)
	 * particles will be sourced at FireOffset and a parameter HitLocation will be set for the target, if applicable
	 */
	UPROPERTY(EditAnywhere, Category = "Weapon")
	TArray<UParticleSystem*> FireEffect;
	
	/** first person mesh */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TSubobjectPtr<USkeletalMeshComponent> Mesh;

	/** causes weapons fire to originate from the center of the player's view when in first person mode (and human controlled)
	 * in other cases the fire start point defaults to the weapon's world position
	 */
	UPROPERTY(EditAnywhere, Category = "Weapon")
	bool bFPFireFromCenter;
	/** Firing offset from weapon for weapons fire. If bFPFireFromCenter is true and it's a player in first person mode, this is from the camera center */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	FVector FireOffset;

	/** time to bring up the weapon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	float BringUpTime;
	/** time to put down the weapon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	float PutDownTime;

	/** equip anims */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	UAnimMontage* BringUpAnim;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	UAnimMontage* PutDownAnim;

	/** weapon group - NextWeapon() picks the next highest group, PrevWeapon() the next lowest, etc
	 * generally, the corresponding number key is bound to access the weapons in that group
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	int32 Group;
	/** if the player acquires more than one weapon in a group, we assign a unique GroupSlot to keep a consistent order
	 * this value is only set on clients
	 */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Weapon")
	int32 GroupSlot;

	/** whether this weapon stays around by default when someone picks it up (i.e. multiple people can pick up from the same spot without waiting for respawn time) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	bool bWeaponStay;

	virtual void BeginPlay() OVERRIDE;
	virtual void RegisterAllComponents() OVERRIDE
	{
		// don't register in game by default for perf, we'll manually call Super from AttachToOwner()
		if (GetWorld()->WorldType == EWorldType::Editor || GetWorld()->WorldType == EWorldType::Preview)
		{
			Super::RegisterAllComponents();
		}
		else
		{
			RootComponent = NULL; // this was set for the editor view, but we don't want it
		}
	}

	virtual UMeshComponent* GetPickupMeshTemplate_Implementation(FVector& OverrideScale) const OVERRIDE;

	void GotoState(class UUTWeaponState* NewState);

	/** firing entry point */
	virtual void StartFire(uint8 FireModeNum);
	virtual void StopFire(uint8 FireModeNum);
	UFUNCTION(Server, Reliable, WithValidation)
	virtual void ServerStartFire(uint8 FireModeNum);
	UFUNCTION(Server, Reliable, WithValidation)
	virtual void ServerStopFire(uint8 FireModeNum);

	virtual void BeginFiringSequence(uint8 FireModeNum);
	virtual void EndFiringSequence(uint8 FireModeNum);

	/** hook when the firing state starts; called on both client and server */
	UFUNCTION(BlueprintNativeEvent)
	void OnStartedFiring();
	/** hook for the return to active state (was firing, refire timer expired, trigger released and/or out of ammo)  */
	UFUNCTION(BlueprintNativeEvent)
	void OnStoppedFiring();
	/** hook for when the weapon has fired, the refire delay passes, and the user still wants to fire (trigger still down) so the firing loop will repeat */
	UFUNCTION(BlueprintNativeEvent)
	void OnContinuedFiring();
	/** blueprint hook for pressing one fire mode while another is currently firing (e.g. hold alt, press primary)
	 * CurrentFireMode == current, OtherFireMode == one just pressed
	 */
	UFUNCTION(BlueprintNativeEvent)
	void OnMultiPress(uint8 OtherFireMode);

	/** activates the weapon as part of a weapon switch
	 * (this weapon has already been set to Pawn->Weapon)
	 */
	virtual void BringUp();
	/** puts the weapon away as part of a weapon switch
	 * return false to prevent weapon switch (must keep this weapon equipped)
	 */
	virtual bool PutDown();
	/** allows blueprint to prevent the weapon from being switched away from */
	UFUNCTION(BlueprintImplementableEvent)
	bool eventPreventPutDown();

	/** attach the visuals to Owner's first person view */
	UFUNCTION(BlueprintNativeEvent)
	void AttachToOwner();
	/** detach the visuals from the Owner's first person view */
	UFUNCTION(BlueprintNativeEvent)
	void DetachFromOwner();

	/** return number of fire modes */
	virtual uint8 GetNumFireModes()
	{
		return FMath::Min3(255, FiringState.Num(), FireInterval.Num());
	}

	virtual void GivenTo(AUTCharacter* NewOwner, bool bAutoActivate) OVERRIDE;
	virtual void ClientGivenTo_Internal(bool bAutoActivate) OVERRIDE;

	virtual void Removed() OVERRIDE;
	virtual void ClientRemoved_Implementation() OVERRIDE;

	/** fires a shot and consumes ammo */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual void FireShot();
	/** blueprint override for firing
	 * NOTE: do an authority check before spawning projectiles, etc as this function is called on both sides
	 */
	UFUNCTION(BlueprintImplementableEvent)
	bool FireShotOverride();

	/** play firing effects not associated with the shot's results (e.g. muzzle flash but generally NOT emitter to target) */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual void PlayFiringEffects();
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual void StopFiringEffects();

	/** play effects associated with the shot's impact given the impact point
	 * called only if FlashLocation has been set (instant hit weapon)
	 */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual void PlayImpactEffects(const FVector& TargetLoc);

	/** return start point for weapons fire */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Weapon")
	virtual FVector GetFireStartLoc();
	/** return base fire direction for weapons fire (i.e. direction player's weapon is pointing) */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Weapon")
	virtual FRotator GetBaseFireRotation();
	/** return adjusted fire rotation after accounting for spread, aim help, and any other secondary factors affecting aim direction (may include randomized components) */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, BlueprintPure, Category = "Weapon")
	FRotator GetAdjustedAim(FVector StartFireLoc);

	/** add (or remove via negative number) the ammo held by the weapon */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Weapon")
	virtual void AddAmmo(int32 Amount);

	/** use up AmmoCost units of ammo for the current fire mode
	 * also handles triggering auto weapon switch if out of ammo
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Weapon")
	virtual void ConsumeAmmo(uint8 FireModeNum);
	
	virtual void FireInstantHit(bool bDealDamage = true, FHitResult* OutHit = NULL);
	UFUNCTION(BlueprintCallable, Category = Firing)
	void K2_FireInstantHit(bool bDealDamage, FHitResult& OutHit);
	UFUNCTION(BlueprintCallable, Category = Firing)
	virtual AUTProjectile* FireProjectile();

	/** returns whether we can meet AmmoCost for the given fire mode */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual bool HasAmmo(uint8 FireModeNum);
	/** returns whether we have ammo for any fire mode */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual bool HasAnyAmmo();

	/** get interval between shots, including any fire rate modifiers */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual float GetRefireTime(uint8 FireModeNum);

	inline uint8 GetCurrentFireMode()
	{
		return CurrentFireMode;
	}

	inline void GotoActiveState()
	{
		GotoState(ActiveState);
	}

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	bool IsFiring() const;

	virtual bool StackPickup_Implementation(AUTInventory* ContainedInv) OVERRIDE;

	/** update any timers and such due to a weapon timing change; for example, a powerup that changes firing speed */
	virtual void UpdateTiming();

	virtual void Tick(float DeltaTime) OVERRIDE;

	virtual void Destroyed() OVERRIDE;

	/** we added an editor tool to allow the user to set the MuzzleFlash entries to a component created in the blueprint components view,
	 * but the resulting instances won't be automatically set...
	 * so we need to manually hook it all up
	 * static so we can share with UTWeaponAttachment
	 */
	static void InstanceMuzzleFlashArray(AActor* Weap, TArray<UParticleSystemComponent*>& MFArray);

	inline UUTWeaponState* GetCurrentState()
	{
		return CurrentState;
	}

	UFUNCTION(BlueprintNativeEvent)
	void DrawWeaponInfo(UUTHUDWidget* WeaponHudWidget, AUTCharacter* UTInstigator, float RenderDelta);

	UFUNCTION(BlueprintNativeEvent)
	void DrawWeaponCrosshair(UUTHUDWidget* WeaponHudWidget, AUTCharacter* UTInstigator, float RenderDelta);

	/** helper for shared overlay code between UTWeapon and UTWeaponAttachment
	 * NOTE: can called on default object!
	 */
	virtual void UpdateOverlaysShared(AActor* WeaponActor, AUTCharacter* InOwner, USkeletalMeshComponent* InMesh, USkeletalMeshComponent*& InOverlayMesh) const;
	/** read WeaponOverlayFlags from owner and apply the appropriate overlay material (if any) */
	virtual void UpdateOverlays();

	/** set main skin override for the weapon, NULL to restore to default */
	virtual void SetSkin(UMaterialInterface* NewSkin);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
	UUTWeaponState* CurrentState;
	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
	uint8 CurrentFireMode;

	UPROPERTY(Instanced, BlueprintReadOnly, Category = "States")
	TSubobjectPtr<UUTWeaponState> ActiveState;
	UPROPERTY(Instanced, BlueprintReadOnly, Category = "States")
	TSubobjectPtr<UUTWeaponState> EquippingState;
	UPROPERTY(Instanced, BlueprintReadOnly,  Category = "States")
	TSubobjectPtr<UUTWeaponState> UnequippingState;
	UPROPERTY(Instanced, BlueprintReadOnly, Category = "States")
	TSubobjectPtr<UUTWeaponState> InactiveState;

	/** overlay mesh for overlay effects */
	UPROPERTY()
	USkeletalMeshComponent* OverlayMesh;
};