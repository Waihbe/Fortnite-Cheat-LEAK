

/*

Free open SRC thisisrico.de | Discord ThisIsRico#8696

*/


#include "stdafx.h"
#include "renderer.h"
PVOID localPlayerWeapon = 0;

namespace hooks {

	bool NoSpread = true;
	bool IsLocalPlayerInVehicle = false;
	PVOID LocalPlayerCurrentWeapon = nullptr;
	PVOID LocalPlayerPawn = nullptr;
	DWORD_PTR LocalPawn;
	PVOID LocalPlayerController = nullptr;
	PVOID LocalPosition = nullptr;
	PVOID TargetPawn = nullptr;
	PVOID VehicleTargetPawn = nullptr;
	PVOID ClosestVehicle = nullptr;
	PVOID CurrentVehicle = nullptr;
	PVOID CurrentVehicle2 = nullptr;
	PVOID LocalRootComp = nullptr;
	PVOID PlayerCameraManager = nullptr;
	FVector LocalplayerPosition = { 0, 0, 0 };
	FVector ClosestTargetCoord = { 0, 0, 0 };

	static void fire(float xyz)
	{
		uint64_t CurrentWeapon = 0;
		float i = 0;
		float c = 0;
		/*CurrentWeapon = *(uint64_t*)(LocalPlayerPawn + offsets::CurrentWeapon);*/
		CurrentWeapon = *(uint64_t*)(LocalPawn + offsets::CurrentWeapon); //currentweapon
		if (CurrentWeapon) {
			i = *(float*)(CurrentWeapon + offsets::LastFireTime);
			c = *(float*)(CurrentWeapon + offsets::LastFireTimeVerified);
			*(float*)(CurrentWeapon + offsets::LastFireTime) = i + c - xyz;
			return;
		}

	}
	float Distance = 0;
	bool IsSniper = 0;
	PVOID(*ProcessEvent)(PVOID, PVOID, PVOID, PVOID) = nullptr;
	float* (*CalculateShot)(PVOID, PVOID, PVOID) = nullptr;
	VOID(*ReloadOriginal)(PVOID, PVOID) = nullptr;
	PVOID(*GetWeaponStats)(PVOID) = nullptr;
	INT(*GetViewPoint)(PVOID, FMinimalViewInfo*, BYTE) = nullptr;

	float originalReloadTime = 0.0f;

	PVOID(*CalculateSpread)(PVOID, float*, float*) = nullptr;

	PVOID calculateSpreadCaller = nullptr;


	//SPEEDHACK SHIT -_-
#include "detours.h"

#pragma comment(lib, "detours.lib")

//#include "SpeedHack.h"
// TODO: put in another file, and rename to something better
	template<class DataType>
	class SpeedHack {
		DataType time_offset;
		DataType time_last_update;

		double speed_;

	public:
		SpeedHack(DataType currentRealTime, double initialSpeed) {
			time_offset = currentRealTime;
			time_last_update = currentRealTime;

			speed_ = initialSpeed;
		}

		// TODO: put lock around for thread safety
		void setSpeed(DataType currentRealTime, double speed) {
			time_offset = getCurrentTime(currentRealTime);
			time_last_update = currentRealTime;

			speed_ = speed;
		}

		// TODO: put lock around for thread safety
		DataType getCurrentTime(DataType currentRealTime) {
			DataType difference = currentRealTime - time_last_update;

			return (DataType)(speed_ * difference) + time_offset;
		}
	};


	// function signature typedefs
	typedef DWORD(WINAPI* GetTickCountType)(void);
	typedef ULONGLONG(WINAPI* GetTickCount64Type)(void);

	typedef BOOL(WINAPI* QueryPerformanceCounterType)(LARGE_INTEGER* lpPerformanceCount);

	// globals
	GetTickCountType   g_GetTickCountOriginal;
	GetTickCount64Type g_GetTickCount64Original;
	GetTickCountType   g_TimeGetTimeOriginal;    // Same function signature as GetTickCount

	QueryPerformanceCounterType g_QueryPerformanceCounterOriginal;


	const double kInitialSpeed = 1.0; // initial speed hack speed

	//                                  (initialTime,      initialSpeed)
	SpeedHack<DWORD>     g_speedHack(GetTickCount(), kInitialSpeed);
	SpeedHack<ULONGLONG> g_speedHackULL(GetTickCount64(), kInitialSpeed);
	SpeedHack<LONGLONG>  g_speedHackLL(0, kInitialSpeed); // Gets set properly in DllMain

	// function prototypes

	DWORD     WINAPI GetTickCountHacked(void);
	ULONGLONG WINAPI GetTickCount64Hacked(void);

	BOOL      WINAPI QueryPerformanceCounterHacked(LARGE_INTEGER* lpPerformanceCount);

	DWORD     WINAPI KeysThread(LPVOID lpThreadParameter);

	// functions

	void MainGay()
	{
		// TODO: split up this function for readability.

		HMODULE kernel32 = GetModuleHandleA(xorstr("Kernel32.dll"));
		HMODULE winmm = GetModuleHandleA(xorstr("Winmm.dll"));

		// TODO: check if the modules are even loaded.

		// Get all the original addresses of target functions
		g_GetTickCountOriginal = (GetTickCountType)GetProcAddress(kernel32, xorstr("GetTickCount"));
		g_GetTickCount64Original = (GetTickCount64Type)GetProcAddress(kernel32, xorstr("GetTickCount64"));

		g_TimeGetTimeOriginal = (GetTickCountType)GetProcAddress(winmm, xorstr("timeGetTime"));

		g_QueryPerformanceCounterOriginal = (QueryPerformanceCounterType)GetProcAddress(kernel32, xorstr("QueryPerformanceCounter"));

		// Setup the speed hack object for the Performance Counter
		LARGE_INTEGER performanceCounter;
		g_QueryPerformanceCounterOriginal(&performanceCounter);

		g_speedHackLL = SpeedHack<LONGLONG>(performanceCounter.QuadPart, kInitialSpeed);

		// Detour functions
		DetourTransactionBegin();

		DetourAttach((PVOID*)&g_GetTickCountOriginal, (PVOID)GetTickCountHacked);
		DetourAttach((PVOID*)&g_GetTickCount64Original, (PVOID)GetTickCount64Hacked);

		// Detour timeGetTime to the hacked GetTickCount (same signature)
		DetourAttach((PVOID*)&g_TimeGetTimeOriginal, (PVOID)GetTickCountHacked);

		DetourAttach((PVOID*)&g_QueryPerformanceCounterOriginal, (PVOID)QueryPerformanceCounterHacked);

		DetourTransactionCommit();
	}

	void setAllToSpeed(double speed) {
		g_speedHack.setSpeed(g_GetTickCountOriginal(), speed);

		g_speedHackULL.setSpeed(g_GetTickCount64Original(), speed);

		LARGE_INTEGER performanceCounter;
		g_QueryPerformanceCounterOriginal(&performanceCounter);

		g_speedHackLL.setSpeed(performanceCounter.QuadPart, speed);
	}

	DWORD WINAPI GetTickCountHacked(void) {
		return g_speedHack.getCurrentTime(g_GetTickCountOriginal());
	}

	ULONGLONG WINAPI GetTickCount64Hacked(void) {
		return g_speedHackULL.getCurrentTime(g_GetTickCount64Original());
	}

	BOOL WINAPI QueryPerformanceCounterHacked(LARGE_INTEGER* lpPerformanceCount) {
		LARGE_INTEGER performanceCounter;

		BOOL result = g_QueryPerformanceCounterOriginal(&performanceCounter);

		lpPerformanceCount->QuadPart = g_speedHackLL.getCurrentTime(performanceCounter.QuadPart);

		return result;
	}

	BOOLEAN GetTargetHead(FVector& out) {
		if (!hooks::TargetPawn) {
			return FALSE;
		}

		auto mesh = ReadPointer(hooks::TargetPawn, offsets::Character::Mesh);
		if (!mesh) {
			return FALSE;
		}

		auto bones = ReadPointer(mesh, offsets::StaticMeshComponent::StaticMesh);
		if (!bones) bones = ReadPointer(mesh, offsets::StaticMeshComponent::StaticMesh + 0x10);
		if (!bones) {
			return FALSE;
		}

		float compMatrix[4][4] = { 0 };
		Util::ToMatrixWithScale(reinterpret_cast<float*>(reinterpret_cast<PBYTE>(mesh) + offsets::StaticMeshComponent::ComponentToWorld), compMatrix);

		Util::GetBoneLocation(compMatrix, bones, BONE_CHEST_ID, &out.X); //BONE_HEAD_ID
		return TRUE;
	}
	template<typename T>
	T WriteMemory(DWORD_PTR address, T value)
	{
		return *(T*)address = value;
	}
	template<typename T>
	T ReadMemory(DWORD_PTR address, const T& def = T())
	{
		return *(T*)address;
	}

	void WriteAngles(float TargetX, float TargetY)
	{
		if (hooks::TargetPawn && hooks::LocalPlayerPawn && hooks::LocalPlayerController)
		{
			float X = TargetX / PI; //6.666666666666667f
			float Y = TargetY / PI;
			Y = -(Y);

			*reinterpret_cast<float*>(reinterpret_cast<PBYTE>(LocalPlayerController) + 0x418) = Y;
			*reinterpret_cast<float*>(reinterpret_cast<PBYTE>(LocalPlayerController) + 0x418 + 0x4) = X;
		}
	}

	FVector Subtract(FVector point1, FVector point2)
	{
		FVector vector{ 0, 0, 0 };
		vector.X = point1.X - point2.X;
		vector.Y = point1.Y - point2.Y;
		vector.Z = point1.Z - point2.Z;
		return vector;
	}

	float GetDistance(FVector x, FVector y)
	{
		auto z = Subtract(x, y);
		return sqrt(z.X * z.X + z.Y * z.Y + z.Z * z.Z);
	}

	void SetPlayerVisibility(int VisibilityValue)
	{
		if (hooks::LocalPlayerPawn && hooks::LocalPlayerController)
		{
			hooks::ProcessEvent(hooks::LocalPlayerPawn, addresses::SetPawnVisibility, &VisibilityValue, 0);
		}
	}

	void Teleport(PVOID Pawn, FVector Coords)
	{

	}

	void Teleport2(FVector Coords)
	{
		if (hooks::LocalPlayerPawn && hooks::LocalPlayerController)
		{
			ProcessEvent(hooks::LocalPlayerController, addresses::ClientSetLocation, &Coords, 0);
		}
	}

	uint64_t BaseAddr = (uint64_t)GetModuleHandleA(NULL);
	int hitbox;
	PVOID CalculateSpreadHook(PVOID arg0, float* arg1, float* arg2) {

		if (hooks::NoSpread && Util::SpoofCall(GetAsyncKeyState, 0x02)) {
			return 0;
		}
		if (hooks::NoSpread && Util::SpoofCall(GetAsyncKeyState, 0x01)) {
			return 0;
		}
		return CalculateSpread(arg0, arg1, arg2);
	}








	BOOLEAN GetTarget(FVector& out) {
		if (!hooks::TargetPawn) {
			return FALSE;
		}

		auto mesh = ReadPointer(hooks::TargetPawn, 0x278);
		if (!mesh) {
			return FALSE;
		}

		auto bones = ReadPointer(mesh, 0x420);
		if (!bones) bones = ReadPointer(mesh, 0x420 + 0x10);
		if (!bones) {
			return FALSE;
		}

		float compMatrix[4][4] = { 0 };
		Util::ToMatrixWithScale(reinterpret_cast<float*>(reinterpret_cast<PBYTE>(mesh) + 0x1C0), compMatrix);

		float AimPointer;
		if (Settings.HitBoxPos == 0) {
			AimPointer = BONE_HEAD_ID;
		}
		else if (Settings.HitBoxPos == 1) {
			AimPointer = BONE_NECK_ID;
		}
		else if (Settings.HitBoxPos == 2) {
			AimPointer = BONE_CHEST_ID;
		}
		else if (Settings.HitBoxPos == 3) {
			AimPointer = BONE_PELVIS_ID;
		}
		else if (Settings.HitBoxPos == 4) {
			AimPointer = BONE_RIGHTELBOW_ID;
		}
		else if (Settings.HitBoxPos == 5) {
			AimPointer = BONE_LEFTELBOW_ID;
		}
		else if (Settings.HitBoxPos == 6) {
			AimPointer = BONE_RIGHTTHIGH_ID;
		}
		else if (Settings.HitBoxPos == 7) {
			AimPointer = BONE_LEFTTHIGH_ID;
		}
		else if (Settings.HitBoxPos == 8) {
			AimPointer = BONE_PELVIS_ID;
		}
		Util::GetBoneLocation(compMatrix, bones, AimPointer, &out.X);

		return TRUE;
	}
	
	PVOID ProcessEventHook(UObject* object, UObject* func, PVOID params, PVOID result) {
		if (object && func) {
			auto objectName = Util::GetObjectFirstName(object);
			auto funcName = Util::GetObjectFirstName(func);

			do {
				if (hooks::TargetPawn && hooks::LocalPlayerController)
				{
					if (wcsstr(objectName.c_str(), L"B_Prj_Bullet_Sniper") && funcName == L"OnRep_FireStart") {
						FVector CurrentAimPointer = { 0 };
						if (!GetTarget(CurrentAimPointer)) {
							break;
						}

						*reinterpret_cast<FVector*>(reinterpret_cast<PBYTE>(object) + offsets::FireStartLoc) = CurrentAimPointer;

						auto root = reinterpret_cast<PBYTE>(ReadPointer(object, offsets::RootComponent));
						*reinterpret_cast<FVector*>(root + offsets::RelativeLocation) = CurrentAimPointer;
						memset(root + offsets::ComponentVelocity, 0, sizeof(FVector));
					}
					else if (!Settings.SilentAimbot && wcsstr(funcName.c_str(), L"Tick")) {
						FVector CurrentAimPointer;
						if (!GetTarget(CurrentAimPointer)) { //head
							break;
						}

						float angles[2] = { 0 };
						Util::CalcAngle(&Util::GetViewInfo().Location.X, &CurrentAimPointer.X, angles); //head instead of neck.X

						if (Settings.AimbotSlow <= 0.0f) {
							if (Settings.triggerbot) {
								FRotator args = { 0 };
								args.Pitch = angles[0];
								args.Yaw = angles[1];
								ProcessEvent(hooks::LocalPlayerController, addresses::ClientSetRotation, &args, 0);
								mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
								mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
								int x = 0;
								while (x < Settings.triggerspeed * 10) {
									x++;
								}
								x = 0;
							}
							else {
								auto scale = Settings.AimbotSlow + 1.0f;
								auto currentRotation = Util::GetViewInfo().Rotation;

								FRotator args = { 0 };
								args.Pitch = (angles[0] - currentRotation.Pitch) / scale + currentRotation.Pitch;
								args.Yaw = (angles[1] - currentRotation.Yaw) / scale + currentRotation.Yaw;
								ProcessEvent(hooks::LocalPlayerController, addresses::ClientSetRotation, &args, 0);
							}
						}
						else {
							if (Settings.triggerbot) {
								auto scale = Settings.AimbotSlow + 1.0f;
								auto currentRotation = Util::GetViewInfo().Rotation;

								FRotator args = { 0 };
								args.Pitch = (angles[0] - currentRotation.Pitch) / scale + currentRotation.Pitch;
								args.Yaw = (angles[1] - currentRotation.Yaw) / scale + currentRotation.Yaw;
								ProcessEvent(hooks::LocalPlayerController, addresses::ClientSetRotation, &args, 0);
								mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
								mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
								int v = 0;
								while (v < Settings.triggerspeed * 10) {
									v++;
								}
								v = 0;
							}
							else {
								auto scale = Settings.AimbotSlow + 1.0f;
								auto currentRotation = Util::GetViewInfo().Rotation;

								FRotator args = { 0 };
								args.Pitch = (angles[0] - currentRotation.Pitch) / scale + currentRotation.Pitch;
								args.Yaw = (angles[1] - currentRotation.Yaw) / scale + currentRotation.Yaw;
								ProcessEvent(hooks::LocalPlayerController, addresses::ClientSetRotation, &args, 0);
							}

						}
					}
				}
				if (Settings.TeleportToEnemies) {
					if (hooks::LocalPlayerPawn && hooks::LocalPlayerController) {
						if (Util::SpoofCall(GetAsyncKeyState, VK_F3)) {
							if (IsLocalPlayerInVehicle) {
								/*auto bones = offsets::StaticMeshComponent::StaticMesh;
								if (!bones) continue;
								auto mesh = offsets::Character::Mesh;
								float compMatrix[4][4] = { 0 };
								Util::ToMatrixWithScale(reinterpret_cast<float*>(reinterpret_cast<PBYTE>(mesh) + offsets::StaticMeshComponent::ComponentToWorld), compMatrix);
								float root[3] = { 0 };*/
								Sleep(150);
								//ProcessEvent(hooks::LocalPlayerController, addresses::ClientSetLocation, root, 0);
								//ProcessEvent(hooks::LocalPlayerController, addresses::ClientSetLocation, compMatrix, 0);

							}
						}
					}
				}
				if (Settings.FastReload)
				{
					//if (hooks::LocalPlayerPawn && hooks::LocalPlayerController)
					//{

					if (Util::SpoofCall(GetAsyncKeyState, 0x52))
					{
						setAllToSpeed(Settings.CustomSpeedHack);
					}
					else
					{
						setAllToSpeed(1.0);
					}
					//}
				}
				/*if (Settings.killaura)
				{
					//if (hooks::LocalPlayerPawn && hooks::LocalPlayerController)
					//{
					if (hooks::LocalPlayerPawn && hooks::LocalPlayerController)
					{
						if (Util::SpoofCall(GetAsyncKeyState, 0x01))
						{
							setAllToSpeed(6.0f);
						}
						else
						{
							setAllToSpeed(1.0);
						}
					}
					//}
				}*/

				



				bool keystateA = true;
				bool keystateB = false;
				if (keystateA)
				{
					auto clicked = Util::SpoofCall(GetAsyncKeyState, VK_F2);
					if (keystateB) {
					}
					else if (clicked && Settings.NoClip == true)
					{
						Settings.NoClip = false;
					}
				}
				









				if (Settings.Spinbot)
				{
					//fire(float(3));
				}


				if (Settings.AirStuck)
				{
					if (hooks::LocalPlayerPawn && hooks::LocalPlayerController)
					{
						if (Util::SpoofCall(GetAsyncKeyState, 0x14))
						{
							*reinterpret_cast<float*>(reinterpret_cast<PBYTE>(hooks::LocalPlayerPawn) + offsets::Actor::CustomTimeDilation) = 0;
						}
						else
						{
							*reinterpret_cast<float*>(reinterpret_cast<PBYTE>(hooks::LocalPlayerPawn) + offsets::Actor::CustomTimeDilation) = 1;
						}
					}
				}

				if (Settings.slowmo)
				{
					if (hooks::LocalPlayerPawn && hooks::LocalPlayerController)
					{
						if (Util::SpoofCall(GetAsyncKeyState, Settings.slowmokey))
						{
							*reinterpret_cast<float*>(reinterpret_cast<PBYTE>(hooks::LocalPlayerPawn) + offsets::Actor::CustomTimeDilation) = 0.5;
						}
						else
						{
							*reinterpret_cast<float*>(reinterpret_cast<PBYTE>(hooks::LocalPlayerPawn) + offsets::Actor::CustomTimeDilation) = 1;
						}
					}
				}


				if (Settings.CustomSpeedHack)
				{
					if (Util::SpoofCall(GetAsyncKeyState, Settings.CustomSpeedKeybind))
					{
						setAllToSpeed(Settings.CustomSpeedValue);
					}
					else
					{
						setAllToSpeed(1.0);
					}
				}



				if (Settings.VehicleBoost)
				{
					if (hooks::LocalPlayerPawn && hooks::LocalPlayerController)
					{
						if (Util::SpoofCall(GetAsyncKeyState, VK_LBUTTON))
						{
							setAllToSpeed(3.0);
						}
						else
						{
							setAllToSpeed(1.0);
						}
					}
				}

				//FortniteGame.FortPlayerController.SetRotatePawnToCamera
				//FortniteGame.FortPlayerController.SetFirstPersonCamera
				//bPlayerOutlinesEnabled
				//FortniteGame.FortClientSettingsRecord.SetPlayerOutlinesEnabled

				if (hooks::LocalPlayerPawn && hooks::TargetPawn && hooks::LocalPlayerController)
				{


					if (!Settings.AimbotModePos == 1 && wcsstr(funcName.c_str(), L"Tick")) {
						FVector head;
						if (!GetTargetHead(head)) {
							break;
						}



						float angles[2] = { 0 };
						Util::CalcAngle(&Util::GetViewInfo().Location.X, &head.X, angles);

						if (Settings.AimbotSlow <= 20.0f) {
							FRotator args = { 0 };
							args.Pitch = angles[0];
							args.Yaw = angles[1];
							ProcessEvent(hooks::LocalPlayerController, addresses::SetControlRotation, &args, 0);
						}


					}
				}


			} while (FALSE);

		}

		return ProcessEvent(object, func, params, result);
	}

	float* CalculateShotHook(PVOID arg0, PVOID arg1, PVOID arg2, UObject* object, UObject* func) {
		auto ret = CalculateShot(arg0, arg1, arg2);
		if (ret && Settings.SilentAimbot && hooks::TargetPawn && hooks::LocalPlayerPawn)
		{
			if (Settings.HitBoxPos == 0) //head
			{
				hitbox = 66;
			}
			else if (Settings.HitBoxPos == 1)
			{
				hitbox = 65;
			}
			else if (Settings.HitBoxPos == 2)
			{
				hitbox = 5;
			}
			else if (Settings.HitBoxPos == 3)
			{
				hitbox = 0;
			}
			else if (Settings.HitBoxPos == 4)
			{
				hitbox = 2;
			}
			auto mesh = ReadPointer(hooks::TargetPawn, offsets::Character::Mesh);
			if (!mesh) return ret;

			auto bones = ReadPointer(mesh, offsets::StaticMeshComponent::StaticMesh);
			if (!bones) bones = ReadPointer(mesh, offsets::StaticMeshComponent::StaticMesh + 0x10);
			if (!bones) return ret;

			float compMatrix[4][4] = { 0 };
			Util::ToMatrixWithScale(reinterpret_cast<float*>(reinterpret_cast<PBYTE>(mesh) + offsets::StaticMeshComponent::ComponentToWorld), compMatrix);

			FVector head = { 0 };
			Util::GetBoneLocation(compMatrix, bones, 66, &head.X);

			auto rootPtr = Util::GetPawnRootLocation(hooks::LocalPlayerPawn);
			if (!rootPtr) return ret;
			auto root = *rootPtr;

			auto dx = head.X - root.X;
			auto dy = head.Y - root.Y;
			auto dz = head.Z - root.Z;
			if (dx * dx + dy * dy + dz * dz < 125000.0f) {
				ret[4] = head.X;
				ret[5] = head.Y;
				ret[6] = head.Z;
			}
			else {
				head.Z -= 16.0f;
				root.Z += 45.0f;

				auto y = atan2f(head.Y - root.Y, head.X - root.X);

				root.X += cosf(y + 1.5708f) * 32.0f;
				root.Y += sinf(y + 1.5708f) * 32.0f;

				auto length = Util::SpoofCall(sqrtf, powf(head.X - root.X, 2) + powf(head.Y - root.Y, 2));
				auto x = -atan2f(head.Z - root.Z, length);
				y = atan2f(head.Y - root.Y, head.X - root.X);

				x /= 2.0f;
				y /= 2.0f;

				ret[0] = -(sinf(x) * sinf(y));
				ret[1] = sinf(x) * cosf(y);
				ret[2] = cosf(x) * sinf(y);
				ret[3] = cosf(x) * cosf(y);
			}
			
		}
		if (ret && Settings.Silent && hooks::TargetPawn && hooks::LocalPlayerPawn)
		{
			if (Settings.HitBoxPos == 0) //head
			{
				hitbox = 66;
			}
			else if (Settings.HitBoxPos == 1)
			{
				hitbox = 65;
			}
			else if (Settings.HitBoxPos == 2)
			{
				hitbox = 5;
			}
			else if (Settings.HitBoxPos == 3)
			{
				hitbox = 0;
			}
			else if (Settings.HitBoxPos == 4)
			{
				hitbox = 2;
			}
			auto mesh = ReadPointer(hooks::TargetPawn, offsets::Character::Mesh);
			if (!mesh) return ret;

			auto bones = ReadPointer(mesh, offsets::StaticMeshComponent::StaticMesh);
			if (!bones) bones = ReadPointer(mesh, offsets::StaticMeshComponent::StaticMesh + 0x10);
			if (!bones) return ret;

			float compMatrix[4][4] = { 0 };
			Util::ToMatrixWithScale(reinterpret_cast<float*>(reinterpret_cast<PBYTE>(mesh) + offsets::StaticMeshComponent::ComponentToWorld), compMatrix);

			FVector head = { 0 };
			Util::GetBoneLocation(compMatrix, bones, hitbox, &head.X);

			auto rootPtr = Util::GetPawnRootLocation(hooks::LocalPlayerPawn);
			if (!rootPtr) return ret;
			auto root = *rootPtr;

			auto dx = head.X - root.X;
			auto dy = head.Y - root.Y;
			auto dz = head.Z - root.Z;
			if (dx * dx + dy * dy + dz * dz < 125000.0f) {
				ret[4] = head.X;
				ret[5] = head.Y;
				ret[6] = head.Z;
			}
			else {
				head.Z -= 16.0f;
				root.Z += 45.0f;

				auto y = atan2f(head.Y - root.Y, head.X - root.X);

				root.X += cosf(y + 1.5708f) * 32.0f;
				root.Y += sinf(y + 1.5708f) * 32.0f;

				auto length = Util::SpoofCall(sqrtf, powf(head.X - root.X, 2) + powf(head.Y - root.Y, 2));
				auto x = -atan2f(head.Z - root.Z, length);
				y = atan2f(head.Y - root.Y, head.X - root.X);

				x /= 2.0f;
				y /= 2.0f;

				ret[0] = -(sinf(x) * sinf(y));
				ret[1] = sinf(x) * cosf(y);
				ret[2] = cosf(x) * sinf(y);
				ret[3] = cosf(x) * cosf(y);
			}

		}
		if (ret && Settings.NoClip/*Settings.AimbotModePos == 2*/ && hooks::TargetPawn && hooks::LocalPlayerPawn)
		{
			if (Settings.HitBoxPos == 0) //head
			{
				hitbox = 66;
			}
			else if (Settings.HitBoxPos == 1)
			{
				hitbox = 65;
			}
			else if (Settings.HitBoxPos == 2)
			{
				hitbox = 5;
			}

			else if (Settings.HitBoxPos == 3)
			{
				hitbox = 2;
			}
			auto mesh = ReadPointer(hooks::TargetPawn, offsets::Character::Mesh);
			if (!mesh) return ret;

			auto bones = ReadPointer(mesh, offsets::StaticMeshComponent::StaticMesh);
			if (!bones) bones = ReadPointer(mesh, offsets::StaticMeshComponent::StaticMesh + 0x10);
			if (!bones) return ret;

			float compMatrix[4][4] = { 0 };
			Util::ToMatrixWithScale(reinterpret_cast<float*>(reinterpret_cast<PBYTE>(mesh) + offsets::StaticMeshComponent::ComponentToWorld), compMatrix);

			FVector head = { 0 };
			Util::GetBoneLocation(compMatrix, bones, hitbox, &head.X);

			auto rootPtr = Util::GetPawnRootLocation(hooks::LocalPlayerPawn);
			if (!rootPtr) return ret;
			auto root = *rootPtr;

			auto dx = head.X - root.X;
			auto dy = head.Y - root.Y;
			auto dz = head.Z - root.Z;


			ret[4] = head.X;
			ret[5] = head.Y;
			ret[6] = head.Z;

			head.Z -= 15.0f;

			head.X -= 15.0f;
			head.Y -= 15.0f;

			root.Z += 45.0f;

			auto y = atan2f(head.Y - root.Y, head.X - root.X);

			root.X += cosf(y + 1.5708f) * 32.0f;
			root.Y += sinf(y + 1.5708f) * 32.0f;


			ret[4] = head.X;
			ret[5] = head.Y;
			ret[6] = head.Z;

			auto length = Util::SpoofCall(sqrtf, powf(head.X - root.X, 2) + powf(head.Y - root.Y, 2));
			auto x = -atan2f(head.Z - root.Z, length);
			y = atan2f(head.Y - root.Y, head.X - root.X);

			x /= 2.0f;
			y /= 2.0f;

			ret[0] = -(sinf(x) * sinf(y));
			ret[1] = sinf(x) * cosf(y);
			ret[2] = cosf(x) * sinf(y);
			ret[3] = cosf(x) * cosf(y);
			}

			
		
		return ret;
	}
	static float OldPitch = Util::GetViewInfo().Rotation.Pitch;

	INT GetViewPointHook(PVOID player, FMinimalViewInfo* viewInfo, BYTE stereoPass)
	{
		int fovval;
		if (Settings.FovChanger)
		{
			fovval = Settings.FOV;
		}
		else
		{

		}
		const float upperFOV = 50.534008f;
		const float lowerFOV = 40.0f;

		auto ret = GetViewPoint(player, viewInfo, stereoPass);

		auto fov = viewInfo->FOV;
		auto desired = (((180.0f - upperFOV) / (180.0f - 80.0f)) * (fovval - 80.0f)) + upperFOV;

		if (fov > upperFOV) {
			fov = desired;
		}
		else if (fov > lowerFOV) {
			fov = (((fov - lowerFOV) / (upperFOV - lowerFOV)) * (desired - lowerFOV)) + lowerFOV;
		}
		viewInfo->FOV = fov;

		return ret;
	}





	
		


















	BOOLEAN Initialize()
	{
		// GetWeaponStats
		auto addr = Util::FindPattern(xorstr("\x48\x83\xEC\x58\x48\x8B\x91\x00\x00\x00\x00\x48\x85\xD2\x0F\x84\x00\x00\x00\x00\xF6\x81\x00\x00\x00\x00\x00\x74\x10\x48\x8B\x81\x00\x00\x00\x00\x48\x85\xC0\x0F\x85\x00\x00\x00\x00\x48\x8B\x8A\x00\x00\x00\x00\x48\x89\x5C\x24\x00\x48\x8D\x9A\x00\x00\x00\x00\x48\x85\xC9"), xorstr("xxxxxxx????xxxxx????xx?????xxxxx????xxxxx????xxx????xxxx?xxx????xxx"));
		GetWeaponStats = reinterpret_cast<decltype(GetWeaponStats)>(addr);
		if (!addr) {
			MessageBox(0, L"GetWeaponStats", L"Failure", MB_OK | MB_ICONERROR);
			return FALSE;
		}
		// ProcessEvent
		addr = Util::FindPattern(xorstr("\x40\x55\x56\x57\x41\x54\x41\x55\x41\x56\x41\x57\x48\x81\xEC\x00\x00\x00\x00\x48\x8D\x6C\x24\x00\x48\x89\x9D\x00\x00\x00\x00\x48\x8B\x05\x00\x00\x00\x00\x48\x33\xC5\x48\x89\x85\x00\x00\x00\x00\x8B\x41\x0C\x45\x33\xF6\x3B\x05\x00\x00\x00\x00\x4D\x8B\xF8\x48\x8B\xF2\x4C\x8B\xE1\x41\xB8\x00\x00\x00\x00\x7D\x2A"), xorstr("xxxxxxxxxxxxxxx????xxxx?xxx????xxx????xxxxxx????xxxxxxxx????xxxxxxxxxxx????xx"));
		MH_CreateHook(addr, ProcessEventHook, (PVOID*)&ProcessEvent);
		MH_EnableHook(addr);

		// CalculateShot
		addr = Util::FindPattern("\x48\x89\x5C\x24\x00\x4C\x89\x4C\x24\x00\x55\x56\x57\x41\x54\x41\x55\x41\x56\x41\x57\x48\x8D\x6C\x24\x00\x48\x81\xEC\x00\x00\x00\x00\x48\x8B\xF9\x4C\x8D\x6C\x24\x00", "xxxx?xxxx?xxxxxxxxxxxxxxx?xxx????xxxxxxx?");
		MH_CreateHook(addr, CalculateShotHook, (PVOID*)&CalculateShot);
		MH_EnableHook(addr);
		 
		//CalculateSpread
		addr = Util::FindPattern("\x83\x79\x78\x00\x4C\x8B\xC9\x75\x0F\x0F\x57\xC0\xC7\x02\x00\x00\x00\x00\xF3\x41\x0F\x11\x00\xC3\x48\x8B\x41\x70\x8B\x48\x04\x89\x0A\x49\x63\x41\x78\x48\x6B\xC8\x1C\x49\x8B\x41\x70\xF3\x0F\x10\x44\x01\x00\xF3\x41\x0F\x11\x00\xC3", "xxxxxxxxxxxxxx????xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx?xxxxxx");
		if (!addr) {
			MessageBox(0, L"CalculateSpread", L"Failure", MB_OK | MB_ICONERROR);
			return FALSE;
		}

		//DISCORD.HookFunction((uintptr_t)addr, (uintptr_t)CalculateSpreadHook, (uintptr_t)&CalculateSpread);
		MH_CreateHook(addr, CalculateSpreadHook, (PVOID*)&CalculateSpread);
		MH_EnableHook(addr);

		// CalculateSpreadCaller
		addr = Util::FindPattern("\x0F\x57\xD2\x48\x8D\x4C\x24\x00\x41\x0F\x28\xCD\xE8\x00\x00\x00\x00\x48\x8B\x4D\xB0\x0F\x28\xF0\x48\x85\xC9\x74\x05\xE8\x00\x00\x00\x00\x48\x8B\x4D\x98\x48\x8D\x05\x00\x00\x00\x00\x48\x89\x44\x24\x00\x48\x85\xC9\x74\x05\xE8\x00\x00\x00\x00\x48\x8B\x4D\x88", "xxxxxxx?xxxxx????xxxxxxxxxxxxx????xxxxxxx????xxxx?xxxxxx????xxxx");
		if (!addr) {
			MessageBox(0, L"Failed To find CalculateSpreadCaller", L"Failure", MB_OK | MB_ICONERROR);
			return FALSE;
		}


		calculateSpreadCaller = addr;




		// GetViewPoint
		addr = Util::FindPattern(xorstr("\x48\x89\x5C\x24\x00\x48\x89\x74\x24\x00\x57\x48\x83\xEC\x20\x48\x8B\xD9\x41\x8B\xF0\x48\x8B\x49\x30\x48\x8B\xFA\xE8\x00\x00\x00\x00\xBA\x00\x00\x00\x00\x48\x8B\xC8"), xorstr("xxxx?xxxx?xxxxxxxxxxxxxxxxxxx????x????xxx"));
		MH_CreateHook(addr, GetViewPointHook, (PVOID*)&GetViewPoint);
		MH_EnableHook(addr);

		// Init speedhack
		MainGay();

		return TRUE;
	}
}
