

#pragma once
typedef struct Fortnite {
bool Aimbot;
bool AutoAimbot;
bool SilentAimbot;
float AimbotFOV;
int HitBoxPos = 0;
bool AirStuck;
int AirstuckKey;
int FreezetargetKey;
int slowmokey;
bool jumptest;
float ChestDist;
float AimbotSlow;
float SniperAimbotSlow;
bool InstantReload;
float LootDist;
bool invis;
bool NoSpreadAimbot;
bool HeadDotSize;
bool realplayertags;
bool NoClip;
float FOV;
float BoatFlySpeed;
bool Crosshair;
bool WaterMark;
bool slowmo;
bool fishingholeesp;
bool botesp;
int watermarkx;
float FreeCamSpeed;
int watermarky;
bool Head;
int triggerspeed;
bool FastReload;
bool HenchMen;
int hitboxes;
bool PlayerFly;
float espdistancee;
bool OutlineText;
bool triggerbot;
bool FastActions;
bool Test;
bool Rainbow;
bool RestoreRotation;
bool Silent;
bool Prediction;
bool VehicleTel;
bool FovChanger;
bool SpinBot2;
int FovValue;
bool TargetLine;
bool MemoryV2;
int AimbotModePos;
bool ColorAdjuster;
bool ColorAdjuster1;
bool ColorAdjuster2;
bool ColorAdjuster3;
bool ColorAdjuster4;
bool RapidFire;
float RFMod;
int CrosshairThickness;
int CrosshairScale;

bool CustomSpeedHack;
float CustomSpeedValue;
int CustomSpeedKeybind;

bool MemesTest;
bool TestKek;
bool Info;
bool BulletTP;
bool IsBulletTeleporting;

bool VisibleCheck;
bool FPS;
bool Spinbot;
int SpinKey;
float SpinSpeed;

bool VehicleTeleport;
bool VehicleTeleporter;
int AimKey;
bool BigPlayers;

bool InActionTeleport;

int SpinbotPitchMode;
int SpinbotYawMode;
int SnaplineStartPoint;

bool Chams;
bool TeleportToEnemies;
int EnemyTeleportKey;

bool VehicleBoost;
int gayy;
bool AnnasMisc;
bool MouseAim;
bool AimHelper;
bool FreeCam;
bool FOVChanger;
bool SniperHits;
bool SniperThroughWalls;
bool SniperNoAimbot;
int AimPos; // head body dick
int Aimkeypos; // the key
int aimkey;
int crosshairScale;
float crosshairNuclear;
bool isRecFov;
bool visCheck;
int OverallDistance;
float SnaplinePower;
bool ShowInfoBar;
bool RadarESP;
float RadarESPRange;
bool WarningEnemiesArea;
bool ClosestEnemyDistance;
bool HelicopterInfiniteBoost;
bool HelicopterSpeed;
bool InvisiblePlayer;
float HelicopterSpeedMultiplier;
bool Dev;
bool BoatBoost;
float BoatBoostMultiplier;
bool BoatTP;
bool BoatFly;
bool PlrTP;
int AirStuckKey;
int BoatTPkey;
bool WeakSpot;

struct {
	bool Distance;
	bool AimbotFOV;
	bool Boxes;
	bool Visuals;
	bool Skeletons;
	bool Skeletons2;
	bool PlayerLines;
	bool PlayerNames;
	bool PlayerWeapons;
	bool PlayerAmmo;
	bool LLamas;

	bool Memes;
	bool Radar;
	bool TestChams;
	int RadarDistance;
	float PlayerNameVisibleColor[4];
	float PlayerNameNotVisibleColor[4];
	float PlayerNameColor[4];
	float BoxVisibleColor[4];
	float BoxNotVisibleColor[4];
	float SnaplineVisibleColor[4];
	float SnaplineNotVisibleColor[4];
	float SkeletonVisibleColor[4];
	float SkeletonNotVisibleColor[4];
	float FovColor[4];
	float TargetLineColor[4];
	bool debug;
	bool debug2;
	bool Ammo;
	bool AmmoBox;
	bool cornerbox;
	bool Containers;
	bool Weapons;
	bool Vehicles;
	bool SupplyDrops;
	INT MinWeaponTier;
	bool Players;
	bool Skeleton;
	bool Lazer;
	float PlayerVisibleColor[3];
	float PlayerNotVisibleColor[3];
	bool Boats;
	bool Helicopters;
	bool Trucks;
	bool Taxi;
	bool Cars;
	bool Chests;
	bool UpgradeBench;
	bool Llama;
	bool SupplyDrop;
	bool Box;
	bool ThreeDBox;
	bool AgentsBoss;
	bool CornerBox;
	bool AimLine;
	bool Chams;
	bool Shark;
	bool RandomAim;
	bool XPCoin;
	}ESP;
} SETTINGS;

extern SETTINGS Settings;

namespace SettingsHelper
{
	VOID LoadSavedConfig();
	VOID SaveConfig();
	VOID LoadDefaultConfig();
	VOID LoadTheme();
}
