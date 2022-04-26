

/*

Free open SRC thisisrico.de | Discord ThisIsRico#8696

*/



#pragma once



// BODY
#define BONE_HEAD_ID (66)
#define BONE_NECK_ID (65)
#define BONE_CHEST_ID (36)
#define BONE_PELVIS_ID (2)

// ARMS
#define BONE_LEFTSHOULDER_ID (9)
#define BONE_RIGHTSHOULDER_ID (62)
#define BONE_LEFTELBOW_ID (10)
#define BONE_RIGHTELBOW_ID (38)
#define BONE_LEFTHAND_ID (11)
#define BONE_RIGHTHAND_ID (39)

// LEGS
#define BONE_LEFTLEG_ID (67)
#define BONE_RIGHTLEG_ID (74)
#define BONE_LEFTTHIGH_ID (73)
#define BONE_RIGHTTHIGH_ID (80)
#define BONE_LEFTFOOT_ID (68)
#define BONE_RIGHTFOOT_ID (75)
#define BONE_LEFTFEET_ID (71)
#define BONE_RIGHTFEET_ID (78)
#define BONE_LEFTFEETFINGER_ID (72)
#define BONE_RIGHTFEETFINGER_ID (79)

namespace hooks {
	extern bool NoSpread;
	extern bool IsLocalPlayerInVehicle;
	extern PVOID LocalPlayerPawn;
	extern PVOID LocalPlayerCurrentWeapon;
	extern PVOID LocalPlayerController;
	extern PVOID LocalRootComp;
	extern PVOID CurrentVehicle;
	extern PVOID CurrentVehicle2;
	extern PVOID PlayerCameraManager;
	extern FVector LocalplayerPosition;
	extern FVector ClosestTargetCoord;
	extern float Distance;
	extern bool IsSniper;
	extern PVOID TargetPawn;
	extern PVOID VehicleTargetPawn;
	extern PVOID ClosestVehicle;

	extern PVOID(*ProcessEvent)(PVOID, PVOID, PVOID, PVOID);
	BOOLEAN Initialize();
	void SetPlayerVisibility(int VisibilityValue);
	void WriteAngles(float TargetX, float TargetY);
	void Teleport(PVOID Pawn, FVector Coords);
	void Teleport2(FVector Coords);
}
