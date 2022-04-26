


/*

Free open SRC thisisrico.de | Discord ThisIsRico#8696

*/



#pragma once

namespace addresses
{
	extern PVOID* uWorld;
	extern PVOID GetPlayerName;
	extern PVOID SetPawnVisibility;
	extern PVOID ClientSetRotation;
	extern PVOID ClientSetLocation;
	extern PVOID IsInVehicle;
	extern PVOID K2_TeleportTo;
	extern PVOID SetActorRelativeScale3D;
	extern PVOID AddYawInput;
	extern PVOID AddPitchInput;
	extern PVOID GetVehicleActor;
	extern PVOID GetVehicle;
	extern PVOID K2_AttachToActor;
	extern PVOID SetForcedLodModel;
	extern PVOID LaunchCharacter;
	extern PVOID SetActorLocation;
	extern PVOID GetActorEnableCollision;
	extern PVOID SetActorEnableCollision;
	extern PVOID ECollisionEnabled;
	extern PVOID SetCollisionEnabled;
	extern PVOID SetControlRotation;
	extern PVOID SetFirstPersonCamera;
	extern PVOID K2_SetActorRelativeLocation;
	extern PVOID K2_SetActorLocationAndRotation;
	extern PVOID K2_AddRelativeLocation;
	extern PVOID K2_SetRelativeLocation;
	extern PVOID CreativeToggleFly;
	extern PVOID ServerCreativeToggleFly;
	extern PVOID K2_AttachTo;
	extern PVOID K2_SetWorldLocation;
	extern PVOID LaunchPawn;
	extern PVOID OnLaunchPawn;
	extern PVOID TeleportVehicle;
	extern PVOID TeleportPlayerPawn;
	extern PVOID K2_SetActorRotation;
	extern PVOID K2_SetActorRelativeRotation;
	extern PVOID K2_SetWorldRotation;
	extern PVOID ReloadTime;
	extern PVOID ClientSetRotation;

}



namespace offsets
{
	enum Main : uint64_t
	{
		UWorld = 0x9BAA2D0, 
	};

	enum World : uint64_t
	{
		OwningGameInstance = 0x180,
		Levels = 0x138,
	};

	enum Level : uint64_t
	{
		AActors = 0x98,
	};

	enum GameInstance : uint64_t
	{
		LocalPlayers = 0x38,
	};

	enum Player : uint64_t
	{
		PlayerController = 0x30,
	};

	enum Controller : uint64_t
	{
		ControlRotation = 0x288,

	};

	enum PlayerController : uint64_t
	{
		AcknowledgedPawn = 0x2A0,
	};

	enum Pawn : uint64_t
	{
		PlayerState = 0x240,
	};

	enum Actor : uint64_t
	{
		RootComponent = 0x130,
		CustomTimeDilation = 0x98,
	};

	enum Character : uint64_t
	{
		Mesh = 0x280,
	};

	enum SceneComponent : uint64_t
	{
		RelativeLocation = 0x11C,
		ComponentVelocity = 0x140,
	};

	enum StaticMeshComponent : uint64_t
	{
		ComponentToWorld = 0x1C0,
		StaticMesh = 0x480, 
	};

	enum SkinnedMeshComponent : uint64_t
	{
		CachedWorldSpaceBounds = 0x600,
	};

	enum FortPawn : uint64_t
	{
		bIsDBNO = 0x552,
		bIsDying = 0x538,
		CurrentWeapon = 0x5D0,
		LastFireTimeVerified = 0x900,
		LastFireTime = 0x8FC,
	};

	enum FortPickup : uint64_t
	{
		PrimaryPickupItemEntry = 0x2A8,
	};

	enum FortItemEntry : uint64_t
	{
		ItemDefinition = 0x18,
	};

	enum FortItemDefinition : uint64_t
	{
		DisplayName = 0x80,
		Tier = 0x64,
	};

	enum FortPlayerStateAthena : uint64_t
	{
		TeamIndex = 0x0EC0,
	};

	enum FortWeapon : uint64_t
	{
		WeaponData = 0x378,
		AmmoCount = 0x974,
	};

	enum FortWeaponItemDefinition : uint64_t
	{
		WeaponStatHandle = 0x820,
	};

	enum FortProjectileAthena : uint64_t
	{
		FireStartLoc = 0x878,
	};

	enum FortBaseWeaponStats : uint64_t
	{
		ReloadTime = 0xFC,
	};
	enum BuildingContainer : uint64_t
	{
		bAlreadySearched = 0xC61,
	};


	enum BuildingWeakSpot : uint64_t
	{
		bActive = 0x238,
	};


	enum MeatballVehicleConfigs : uint64_t
	{
		BoostMinPushForce = 0x678,
		BoostTopSpeedForceMultiplier = 0x67C,
		BoostTopSpeedMultiplier = 0x680,
		LandTopSpeedMultiplier = 0x688,
		LandPushForceMultiplier = 0x68C,
		BoostSteeringMultiplier = 0x6CC,
		LandSteeringMultiplier = 0x6D4,
		LandMinSpeedSteeringAngle = 0x6D8,
		LandMaxSpeedSteeringAngle = 0x6DC,
	};

	enum FortRangedWeaponStats : uint64_t
	{
		Spread = 0x15C,
		SpreadDownsights = 0x160,
		StandingStillSpreadMultiplier = 0x164,
		AthenaJumpingFallingSpreadMultiplier = 0x16C,
		AthenaCrouchingSpreadMultiplier = 0x168,
		AthenaSprintingSpreadMultiplier = 0x21C,
		MinSpeedForSpreadMultiplier = 0x174,
		MaxSpeedForSpreadMultiplier = 0x178,
		RecoilHoriz = 0x200,
		RecoilVert = 0x1F0,
		RecoilDownsightsMultiplier = 0x21C,
	};

	BOOLEAN Initialize();
}


