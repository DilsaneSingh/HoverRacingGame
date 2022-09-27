// Name - Dilsane Singh

// GAMEDS24.cpp: A program using the TL-Engine

#include <TL-Engine.h>	// TL-Engine include file and namespace
#include <fstream>
#include <vector>
#include <iostream>
#include <sstream>

using namespace tle;
using namespace std;

struct SModelSize {
	const float MIN_X;
	const float MAX_X;
	const float MIN_Y;
	const float MAX_Y;
	const float MIN_Z;
	const float MAX_Z;
};

const float CHECKPOINT_MIN_X = -9.9f;
const float CHECKPOINT_MAX_X = 9.9f;
const float CHECKPOINT_MIN_Y = 0.0f;
const float CHECKPOINT_MAX_Y = 10.6f;
const float CHECKPOINT_MIN_Z = -1.3f;
const float CHECKPOINT_MAX_Z = 1.3f;

SModelSize SCheckpointSize{
	CHECKPOINT_MIN_X,
	CHECKPOINT_MAX_X,
	CHECKPOINT_MIN_Y,
	CHECKPOINT_MAX_Y,
	CHECKPOINT_MIN_Z,
	CHECKPOINT_MAX_Z
};

const float ISLE_MIN_X = -2.7f;
const float ISLE_MAX_X = 2.7f;
const float ISLE_MIN_Y = 0.02f;
const float ISLE_MAX_Y = 5.5f;
const float ISLE_MIN_Z = -3.42f;
const float ISLE_MAX_Z = 3.42f;

SModelSize SIsleSize = {
	ISLE_MIN_X,
	ISLE_MAX_X,
	ISLE_MIN_Y,
	ISLE_MAX_Y,
	ISLE_MIN_Z,
	ISLE_MAX_Z
};

const float WALL_MIN_X = -1.0f;
const float WALL_MAX_X = 1.0f;
const float WALL_MIN_Y = -0.0f;
const float WALL_MAX_Y = 4.5f;
const float WALL_MIN_Z = -4.8f;
const float WALL_MAX_Z = 4.8f;

SModelSize SWallSize = {
	WALL_MIN_X,
	WALL_MAX_X,
	WALL_MIN_Y,
	WALL_MAX_Y,
	WALL_MIN_Z,
	WALL_MAX_Z
};

const float TANKT2_MIN_X = -3.1f;
const float TANKT2_MAX_X = 5.1f;
const float TANKT2_MIN_Y = -0.4f;
const float TANKT2_MAX_Y = 16.5f;
const float TANKT2_MIN_Z = -3.0f;
const float TANKT2_MAX_Z = 4.8f;

SModelSize STankT2Size = {
	TANKT2_MIN_X,
	TANKT2_MAX_X,
	TANKT2_MIN_Y,
	TANKT2_MAX_Y,
	TANKT2_MIN_Z,
	TANKT2_MAX_Z
};

const float FORWARD_SPEED = 50.0f;
//const float BACKWARD_SPEED = -FORWARD_SPEED / 2;

const float DEFAULT_Y_POS = 0;
const float DEFAULT_Y_ROT = 0;

struct SCheckpointPos {
	IModel* model;
	float x;
	float z;
	float rotation = DEFAULT_Y_ROT;
	bool passed = false;
};

struct SModelPos {
	IModel* model;
	float x;
	float z;
	float yRotation = DEFAULT_Y_ROT;
};

float speed = 0.2f;
float dragFactor = -0.0004f;

struct Vector3 {
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
};

int HP = 100;

Vector3 momentum, oldMomentum, thrust, drag;

void HovercarMovements(I3DEngine* myEngine, IModel* hovercar, float frameTime, Vector3 localZ);

void CameraMovements(ICamera* myCamera, I3DEngine* myEngine, float frameTime);

string GameStateText(vector <SCheckpointPos>& VCheckpoints, Vector3& HovercarPos, string& outText);

enum ECollisionAxis { xAxis, zAxis, none };

ECollisionAxis PointBoxCollision(Vector3 HovercarPos, SModelPos ModelPos, SModelSize ModelI, Vector3 oldHovercarPos, int& HP);

bool sphere2SphereCollisionChecker(IModel* model1, float radius1, IModel* model2, float radius2);

void main()
{
	// Create a 3D engine (using TLX engine here) and open a window for it
	I3DEngine* myEngine = New3DEngine(kTLX);
	myEngine->StartWindowed();

	// Add default folder for meshes and other media
	myEngine->AddMediaFolder("C:\\ProgramData\\TL-Engine\\Media");
	myEngine->AddMediaFolder("D:\\GAMEDS24\\Assessment2_Models");

	/**** Set up your scene here ****/

	// Meshes
	IMesh* checkpointMesh = myEngine->LoadMesh("Checkpoint.x");
	IMesh* isleMesh = myEngine->LoadMesh("IsleStraight.x");
	IMesh* wallMesh = myEngine->LoadMesh("Wall.x");
	IMesh* tank1Mesh = myEngine->LoadMesh("TankSmall1.x");
	IMesh* tank2Mesh = myEngine->LoadMesh("TankSmall2.x");
	IMesh* crossMesh = myEngine->LoadMesh("Cross.x");
	IMesh* groundMesh = myEngine->LoadMesh("Ground.x");
	IMesh* skyboxMesh = myEngine->LoadMesh("Skybox 07.x");
	IMesh* hovercarMesh = myEngine->LoadMesh("hovercar.x");
	IMesh* dummyMesh = myEngine->LoadMesh("Dummy.x");

	// Models for RaceTrack file.

	const int NUM_CHECKPOINTS = 6;

	IModel* checkpoint[NUM_CHECKPOINTS];
	for (int i = 0; i < NUM_CHECKPOINTS; i++) {
		checkpoint[i] = checkpointMesh->CreateModel();
	}

	const int NUM_ISLES = 18;

	IModel* isle[NUM_ISLES];
	for (int i = 0; i < NUM_ISLES; i++) {
		isle[i] = isleMesh->CreateModel();
	}

	const int NUM_WALLS = 9;

	IModel* wall[NUM_WALLS];
	for (int i = 0; i < NUM_WALLS; i++) {
		wall[i] = wallMesh->CreateModel();
	}

	const int NUM_TANKS2 = 10;

	IModel* tank2[NUM_TANKS2];
	for (int i = 0; i < NUM_TANKS2; i++) {
		tank2[i] = tank2Mesh->CreateModel();
	}

	// Set Position of RaceTrack File Models.
	string filename = "RaceTrack1.txt";

	string objectType;
	float abscissa;
	float applicate;
	float yRotation;

	vector <SCheckpointPos> VCheckpoints;
	int checkpointI = 0;

	vector <SModelPos> VIsles;
	int isleI = 0;

	vector <SModelPos> VWalls;
	int wallI = 0;

	vector <SModelPos> VTanks;
	int tankI = 0;

	ifstream infile;
	infile.open(filename);
	if (!infile) {
		cout << "Error! Unable to open " << filename << endl;
	}
	else {
		while (!infile.eof()) {
			infile >> objectType >> abscissa >> applicate >> yRotation;

			if (objectType == "Checkpoint") {
				VCheckpoints.push_back({ checkpoint[checkpointI], abscissa, applicate, yRotation, false });
				checkpointI++;
			}

			else if (objectType == "Isle") {
				VIsles.push_back({ isle[isleI], abscissa, applicate, yRotation });
				isleI++;
			}

			else if (objectType == "Wall") {
				VWalls.push_back({ wall[wallI], abscissa, applicate, yRotation });
				wallI++;
			}

			else if (objectType == "TankSmall2") {
				VTanks.push_back({ tank2[tankI], abscissa, applicate, yRotation });
				tankI++;
			}

		}

	}

	for (int i = 0; i < VCheckpoints.size(); i++) {
		VCheckpoints[i].model->SetPosition(VCheckpoints[i].x, DEFAULT_Y_POS, VCheckpoints[i].z);
		VCheckpoints[i].model->RotateLocalY(VCheckpoints[i].rotation);
	}

	for (int i = 0; i < VIsles.size(); i++) {
		VIsles[i].model->SetPosition(VIsles[i].x, DEFAULT_Y_POS, VIsles[i].z);
		VIsles[i].model->RotateLocalY(VIsles[i].yRotation);
	}

	for (int i = 0; i < VWalls.size(); i++) {
		VWalls[i].model->SetPosition(VWalls[i].x, DEFAULT_Y_POS, VWalls[i].z);
		VWalls[i].model->RotateLocalY(VWalls[i].yRotation);
	}

	for (int i = 0; i < VTanks.size() - 1; i++) {
		VTanks[i].model->SetPosition(VTanks[i].x, DEFAULT_Y_POS, VTanks[i].z);
		VTanks[i].model->RotateLocalY(VTanks[i].yRotation);
	}
	VTanks[2].model->SetY(-5);
	VTanks[2].model->RotateLocalX(15);
	VTanks[3].model->SetY(-5);
	VTanks[3].model->RotateLocalZ(15);
	VTanks[4].model->SetY(-5);
	VTanks[4].model->RotateLocalZ(15);

	// Other Models

	IModel* ground = groundMesh->CreateModel();

	IModel* skybox = skyboxMesh->CreateModel(0.0f, -960.0f, 0.0f);

	IModel* hovercar = hovercarMesh->CreateModel(0.0f, 0.0f, -25.0f);

	// Camera Dummy Settings

	ICamera* myCamera = myEngine->CreateCamera(kManual);

	IModel* defaultCameraDummy = dummyMesh->CreateModel();
	defaultCameraDummy->SetPosition(myCamera->GetX(), myCamera->GetY(), myCamera->GetZ());
	defaultCameraDummy->AttachToParent(hovercar);

	IModel* FPP_CameraDummy = dummyMesh->CreateModel();
	FPP_CameraDummy->SetPosition(hovercar->GetX(), hovercar->GetY() + 5, hovercar->GetZ() + 30);
	FPP_CameraDummy->AttachToParent(hovercar);

	IModel* RestartDummy = dummyMesh->CreateModel();

	myCamera->AttachToParent(hovercar);

	IModel* Cross[NUM_CHECKPOINTS];
	float CrossTimer[NUM_CHECKPOINTS];

	for (int i = 0; i < NUM_CHECKPOINTS; i++) {
		Cross[i] = crossMesh->CreateModel(0, -100, 0);
		CrossTimer[i] = 0.0f;
	}

	IMesh* sphereMesh = myEngine->LoadMesh("sphere.x");
	IModel* sphere[2 * NUM_CHECKPOINTS];
	for (int i = 0; i < 2 * NUM_CHECKPOINTS; i++) {
		sphere[i] = sphereMesh->CreateModel();
		sphere[i]->Scale(0.01);
	}

	vector <SModelPos> VSpheres;
	for (int i = 0; i < VCheckpoints.size(); i++) {
		VSpheres.push_back({ sphere[i + i], VCheckpoints[i].x + CHECKPOINT_MAX_X, VCheckpoints[i].z, VCheckpoints[i].rotation });
		VSpheres.push_back({ sphere[2 * i + 1], VCheckpoints[i].x + CHECKPOINT_MIN_X, VCheckpoints[i].z, VCheckpoints[i].rotation });
	}

	for (int i = 0; i < VSpheres.size(); i++) {
		VSpheres[i].model->SetPosition(VSpheres[i].x, 0.0f, VSpheres[i].z);
	}

	VSpheres[2].model->SetPosition(-19.9921, 0.0, 129.9);
	VSpheres[3].model->SetPosition(-20.0079, 0.0, 110.1);
	VSpheres[8].model->SetPosition(20.0079, 0.0, -80.1);
	VSpheres[9].model->SetPosition(19.9921, 0.0, -99.9);


	// Game States
	enum EGameStates { InitialState, Started, Paused, Playing, GameOver };
	EGameStates GameState = InitialState;

	float time = 0.0f, timePassed = 0.0f;

	// Sprites and Backdrop

	float windowY = myEngine->GetWidth();
	float windowX = myEngine->GetHeight();

	ISprite* backdrop = myEngine->CreateSprite("ui_backdrop.jpg", windowX / 2, windowY / 2);

	IFont* myFont = myEngine->LoadFont("Comic Sans MS", 36);

	stringstream curState;
	string outText = "Hit Space to Start";

	// Hovering Effect
	float hoverTimer = 0.0f;
	float frequency = 4.0f;
	float amplitude = 1.0f;
	float newHovercarY;

	// Old Hovercar Position
	float oldHovercarX, oldHovercarY, oldHovercarZ;

	// Timer
	myEngine->Timer();

	// The main game loop, repeat until engine is stopped
	while (myEngine->IsRunning())
	{
		// Draw the scene
		myEngine->DrawScene();
		float frameTime = myEngine->Timer();

		/**** Update your scene each frame here ****/

		// Camera Settings.
		//Do mouse, camera configurations

		CameraMovements(myCamera, myEngine, frameTime);

		if (myEngine->KeyHit(Key_1)) {
			myCamera->SetPosition(defaultCameraDummy->GetX(), defaultCameraDummy->GetY(), defaultCameraDummy->GetZ());
		}
		else if (myEngine->KeyHit(Key_2)) {
			myCamera->SetPosition(FPP_CameraDummy->GetX(), FPP_CameraDummy->GetY(), FPP_CameraDummy->GetZ());
		}

		// Hovering Effect
		oldHovercarY = hovercar->GetY();
		newHovercarY = oldHovercarY + sin(hoverTimer * frequency) * amplitude * frameTime;
		hovercar->SetLocalY(newHovercarY);
		hoverTimer += frameTime;

		// Hovercar Movement
		oldHovercarX = hovercar->GetX();
		oldHovercarZ = hovercar->GetZ();

		Vector3 oldHoverCarPos = {
			oldHovercarX,
			oldHovercarY,
			oldHovercarZ
		};

		float matrix[4][4];
		hovercar->GetMatrix(&matrix[0][0]);

		Vector3 localZ = { matrix[2][0], matrix[2][1], matrix[2][2] };

		if (GameState == Playing) {
			HovercarMovements(myEngine, hovercar, frameTime, localZ);
		}

		Vector3 HovercarPos{
			hovercar->GetX(),
			hovercar->GetY(),
			hovercar->GetZ()
		};

		// Collision detect

		for (int i = 0; i < VSpheres.size(); i++) {
			if (sphere2SphereCollisionChecker(hovercar, 5.0f, VSpheres[i].model, 2.0f)) {
				hovercar->SetX(oldHovercarX);
				hovercar->SetZ(oldHovercarZ);
				//HP = HP - 1;
				momentum.z = { -1.0 };
				hovercar->Move(momentum.x * frameTime, momentum.y * frameTime, momentum.z * frameTime);
			}
		}

		ECollisionAxis checkAxis = none;
		for (int i = 0; i < VWalls.size(); i++) {
			checkAxis = PointBoxCollision(HovercarPos, VWalls[i], SWallSize, oldHoverCarPos, HP);
			if (checkAxis == xAxis) {
				hovercar->SetX(oldHovercarX);
				hovercar->SetZ(oldHovercarZ);
				momentum.z = { -1.0 };
				hovercar->Move(momentum.x * frameTime, momentum.y * frameTime, momentum.z * frameTime);

			}
			else if (checkAxis == zAxis) {
				hovercar->SetX(oldHovercarX);
				hovercar->SetZ(oldHovercarZ);
				momentum.x = { -1.0 };
				hovercar->Move(momentum.x * frameTime, momentum.y * frameTime, momentum.z * frameTime);

			}
		}

		for (int i = 0; i < VIsles.size(); i++) {
			checkAxis = PointBoxCollision(HovercarPos, VIsles[i], SIsleSize, oldHoverCarPos, HP);
			if (checkAxis == xAxis) {
				hovercar->SetX(oldHovercarX);
				hovercar->SetZ(oldHovercarZ);
				momentum.z = { -1.0 };
				hovercar->Move(momentum.x * frameTime, momentum.y * frameTime, momentum.z * frameTime);

			}
			else if (checkAxis == zAxis) {
				hovercar->SetX(oldHovercarX);
				hovercar->SetZ(oldHovercarZ);
				momentum.x = { -1.0 };
				hovercar->Move(momentum.x * frameTime, momentum.y * frameTime, momentum.z * frameTime);

			}
		}

		/*for (int i = 0; i < VTanks.size(); i++) {
			checkAxis = PointBoxCollision(HovercarPos, VTanks[i], SIsleSize, oldHoverCarPos, HP);
			if (checkAxis == xAxis) {
				hovercar->SetX(oldHovercarX);
				hovercar->SetZ(oldHovercarZ);
				momentum.z = { -1.0 };
				hovercar->Move(momentum.x * frameTime, momentum.y * frameTime, momentum.z * frameTime);

			}
			else if (checkAxis == zAxis) {
				hovercar->SetX(oldHovercarX);
				hovercar->SetZ(oldHovercarZ);
				momentum.x = { -1.0 };
				hovercar->Move(momentum.x * frameTime, momentum.y * frameTime, momentum.z * frameTime);

			}
		}*/

		for (int i = 0; i < VTanks.size() - 1; i++) {
			if (sphere2SphereCollisionChecker(hovercar, 5.0f, VTanks[i].model, 3.5f)) {
				checkAxis = PointBoxCollision(HovercarPos, VTanks[i], STankT2Size, oldHoverCarPos, HP);
				if (checkAxis == xAxis) {
					hovercar->SetX(oldHovercarX);
					hovercar->SetZ(oldHovercarZ);
					momentum.z = { -1.0 };
					hovercar->Move(momentum.x * frameTime, momentum.y * frameTime, momentum.z * frameTime);

				}
				else if (checkAxis == zAxis) {
					hovercar->SetX(oldHovercarX);
					hovercar->SetZ(oldHovercarZ);
					momentum.x = { -1.0 };
					hovercar->Move(momentum.x * frameTime, momentum.y * frameTime, momentum.z * frameTime);

				}
			}
		}

		// Countdown

		if (myEngine->KeyHit(Key_Space)) {
			GameState = Started;
		}

		if (GameState == Started) {
			time = myEngine->Timer();
			timePassed += time;
		}

		if (timePassed >= 0.004) {
			outText = "Go ";
			GameState = Playing;
		}

		else if (timePassed >= 0.003) {
			outText = "1 ";
		}

		else if (timePassed >= 0.002) {
			outText = "2 ";
		}

		else if (timePassed >= 0.001) {
			outText = "3 ";
		}

		oldMomentum = momentum;

		//
		outText = GameStateText(VCheckpoints, HovercarPos, outText);

		for (int i = 0; i < VCheckpoints.size(); i++) {
			if (VCheckpoints[i].passed) {
				if (CrossTimer[i] == 0) {
					Cross[i]->SetPosition(VCheckpoints[i].x, 10, VCheckpoints[i].z);
					Cross[i]->RotateLocalY(VCheckpoints[i].rotation);

				}
				CrossTimer[i] += frameTime;

			}
		}

		for (int i = 0; i < 6; i++) {
			if (CrossTimer[i] > 5) {
				Cross[i]->SetPosition(0, -100, 0);
			}
		}

		if (VCheckpoints[VCheckpoints.size() - 1].passed) {
			outText = "Race Completed ";
		}

		if (HP <= 0) {
			GameState = GameOver;
		}

		if (GameState == GameOver) {
			HP = 0;
			outText = " Game Over - Press Esc to Quit or Press R to Restart";
			if (myEngine->KeyHit(Key_Escape)) {
				myEngine->Stop();
			}
			else if (myEngine->KeyHit(Key_R)) {
				HP = 100;
				hovercar->SetPosition(0, 0, -25);
				timePassed = 0.0f;
				momentum.x = 0;
				momentum.y = 0;
				oldMomentum.x = 0;
				oldMomentum.y = 0;
				thrust.x = 0;
				thrust.y = 0;
				drag.x = 0;
				drag.y = 0;
				for (int i = 0; i < VCheckpoints.size(); i++) {
					VCheckpoints[i].passed = false;
				}
				for (int i = 0; i < 6; i++) {
					CrossTimer[i] = 0.0f;
				}
				hovercar->LookAt(RestartDummy);
				outText = "Hit Space to Start";

				GameState = InitialState;
			}
		}

		float speedX = momentum.x * momentum.x;
		float speedZ = momentum.z * momentum.z;
		float speedP = speedX + speedZ;
		speedP = sqrt(speedP);

		// Text
		curState << outText << " HP = " << HP << " Speed = " << speedP << endl;
		myFont->Draw(curState.str(), windowX / 2, windowY / 2);
		curState.str(""); // Clear myStream

	}

	// Delete the 3D engine now we are finished with it
	myEngine->Delete();
}

void CameraMovements(ICamera* myCamera, I3DEngine* myEngine, float frameTime) {

	if (myEngine->KeyHeld(Key_Up)) {
		myCamera->MoveLocalZ(FORWARD_SPEED * frameTime);
	}

	else if (myEngine->KeyHeld(Key_Down)) {
		myCamera->MoveLocalZ(-FORWARD_SPEED * frameTime);
	}

	if (myEngine->KeyHeld(Key_Right)) {
		myCamera->MoveLocalX(FORWARD_SPEED * frameTime);
	}

	else if (myEngine->KeyHeld(Key_Left)) {
		myCamera->MoveLocalX(-FORWARD_SPEED * frameTime);
	}
}

void HovercarMovements(I3DEngine* myEngine, IModel* hovercar, float frameTime, Vector3 localZ) {

	if (myEngine->KeyHeld(Key_W)) {
		//hovercar->MoveLocalZ(FORWARD_SPEED * frameTime);
		thrust = { localZ.x * speed, localZ.y * speed, localZ.z * speed };
	}

	else if (myEngine->KeyHeld(Key_S)) {
		//hovercar->MoveLocalZ(BACKWARD_SPEED * frameTime);
		thrust = { localZ.x * -speed, localZ.y * -speed, localZ.z * -speed };
	}
	else {
		thrust = { 0,0,0 };
	}

	if (myEngine->KeyHeld(Key_D)) {
		hovercar->RotateLocalY(FORWARD_SPEED * frameTime);
	}

	else if (myEngine->KeyHeld(Key_A)) {
		hovercar->RotateLocalY(-FORWARD_SPEED * frameTime);
	}

	drag = {
		oldMomentum.x * dragFactor,
		oldMomentum.y * dragFactor,
		oldMomentum.z * dragFactor
	};

	momentum = { thrust.x + oldMomentum.x + drag.x,
	thrust.y + oldMomentum.y + drag.y ,
	thrust.z + oldMomentum.z + drag.z };

	//oldMomentum = momentum;

	hovercar->Move(momentum.x * frameTime, momentum.y * frameTime, momentum.z * frameTime);
}

string GameStateText(vector <SCheckpointPos>& VCheckpoints, Vector3& HovercarPos, string& outText) {

	for (int i = 0; i < VCheckpoints.size(); i++) {
		if (HovercarPos.x > VCheckpoints[i].x + CHECKPOINT_MIN_X) {
			if (HovercarPos.x < VCheckpoints[i].x + CHECKPOINT_MAX_X) {

				if (HovercarPos.z > VCheckpoints[i].z + CHECKPOINT_MIN_X) {
					if (HovercarPos.z < VCheckpoints[i].z + CHECKPOINT_MIN_Z) {
						VCheckpoints[i].passed = true;
					}
				}

			}
		}
	}

	int i = 0;
	for (i = 0; i < VCheckpoints.size(); i++) {
		if (VCheckpoints[i].passed == true) {
			bool check = true;
			for (int j = 0; j < i; j++) {
				if (VCheckpoints[j].passed == false) {
					check = false;
				}
			}
			VCheckpoints[i].passed = check;
		}
	}

	string str;
	for (int k = 0; k < VCheckpoints.size(); k++) {
		if (VCheckpoints[k].passed == false) {
			if (k == 0) {
				break;
			}
			else if (k < VCheckpoints.size()) {
				str = to_string(k);
				outText = "Stage " + str + " passed";
				break;
			}

		}
	}

	return outText;

}

ECollisionAxis PointBoxCollision(Vector3 HovercarPos, SModelPos ModelPos, SModelSize ModelI, Vector3 oldHovercarPos, int& HP) {

	ECollisionAxis CollisionAxis = none;
	if (HovercarPos.x<(ModelPos.x + ModelI.MAX_X) && HovercarPos.x>(ModelPos.x + ModelI.MIN_X)) {
		if (HovercarPos.z<(ModelPos.z + ModelI.MAX_Z) && HovercarPos.z>(ModelPos.z + ModelI.MIN_Z)) {
			HP = HP - 1;

			if (oldHovercarPos.x<(ModelPos.x + ModelI.MAX_X) && oldHovercarPos.x>(ModelPos.x + ModelI.MIN_X)) {
				CollisionAxis = xAxis;
			}

			else if (oldHovercarPos.z<(ModelPos.z + ModelI.MAX_Z) && oldHovercarPos.z>(ModelPos.z + ModelI.MIN_Z)) {
				CollisionAxis = zAxis;
			}

		}
	}

	return CollisionAxis;

}

bool sphere2SphereCollisionChecker(IModel* model1, float radius1, IModel* model2, float radius2)
{
	Vector3 model1Pos = { model1->GetX(), model1->GetY(), model1->GetZ() };
	Vector3 model2Pos = { model2->GetX(), model2->GetY(), model2->GetZ() };
	Vector3 vectorBetweenModels = { model2Pos.x - model1Pos.x, model2Pos.y - model1Pos.y, model2Pos.z - model1Pos.z };
	float DistanceBetweenModels = sqrt(vectorBetweenModels.x * vectorBetweenModels.x + vectorBetweenModels.y * vectorBetweenModels.y + vectorBetweenModels.z * vectorBetweenModels.z);

	if (DistanceBetweenModels < (radius1 + radius2))
	{
		return true;
	}
	return false;
}
