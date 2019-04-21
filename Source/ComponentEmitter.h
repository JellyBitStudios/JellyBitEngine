#ifndef __Emitter_H__
#define __Emitter_H__

#include "Component.h"
#include "GameObject.h"

#include "Timer.h"
#include "GameTimer.h"

#include "MathGeoLib/include/Math/float2.h"
#include"MathGeoLib/include/Math/float4.h"
#include "MathGeoLib/include/Geometry/Sphere.h"
#include "MathGeoLib/include/Geometry/Circle.h"

#include "ImGui\imgui.h"

#include <list>
#include <queue>

class ComponentMaterial;
class Particle;
class ResourceMesh;

struct Vertex;

enum ShapeType {
	ShapeType_BOX,
	ShapeType_SPHERE,
	ShapeType_SPHERE_CENTER,
	ShapeType_SPHERE_BORDER,
	ShapeType_CONE,
	ShapeType_MESH
};

enum SimulatedGame
{
	SimulatedGame_NONE,
	SimulatedGame_PLAY,
	SimulatedGame_PAUSE,
	SimulatedGame_STOP,
};

struct MeshShape
{
	uint uuid = 0u;
	uint meshVertexCont = 0u;
	Vertex* meshVertex;
	std::vector<math::float3> uniqueVertex;
	uint* indices = nullptr;
	uint indicesSize = 0;
};

struct ColorTime
{
	math::float4 color = math::float4::one;
	float position = 0.0f;
	std::string name = " ";
	//open window for change particle color
	bool changingColor = false;

	bool operator<(const ColorTime &color) const
	{
		return position < color.position;
	}

	uint GetColorListSerializationBytes();
	void OnInternalSave(char *& cursor);
	void OnInternalLoad(char *& cursor);

};

struct ParticleAnimation
{
	bool isParticleAnimated = false;
	int  textureRows = 1;
	int  textureColumns = 1;
	float textureRowsNorm = 1.0f;
	float textureColumnsNorm = 1.0f;
	float animationSpeed = 0.1f;
	bool randAnim = false;

	uint GetPartAnimationSerializationBytes()
	{ return sizeof(bool) * 2 + sizeof(float) * 3 + sizeof(int) * 2; }
	void OnInternalSave(char *& cursor);
	void OnInternalLoad(char *& cursor);
};

struct StartValues
{
	// Start values
	math::float2 life = math::float2(5.0f, 5.0f);
	math::float2 speed = math::float2(3.0f, 3.0f);
	math::float3 gravity = math::float3::zero;
	math::float2 acceleration = math::float2::zero;
	math::float2 sizeOverTime = math::float2::zero;
	math::float2 size = math::float2(1.0f, 1.0f);
	math::float2 rotation = math::float2::zero;
	math::float2 angularAcceleration = math::float2::zero;
	math::float2 angularVelocity = math::float2::zero;

	std::list<ColorTime> color;
	bool timeColor = false;

	math::float3 particleDirection = math::float3::unitY;

	bool subEmitterActive = false;
	StartValues()
	{
		ColorTime colorTime;
		colorTime.name = "Start Color";
		color.push_back(colorTime);
	}

	void operator=(StartValues startValue);
	void OnInternalSave(char *& cursor);
	void OnInternalLoad(char *& cursor);
};

class ComponentEmitter : public Component
{
public:
	ComponentEmitter(GameObject* gameObject, bool include = true);
	ComponentEmitter(const ComponentEmitter & componentEmitter, GameObject* parent, bool include = true);
	~ComponentEmitter();

	void StartEmitter();
	void ChangeGameState(SimulatedGame state);

	void Update();

	void OnUniqueEditor();
	void ParticleTexture();
	void ParticleAABB();
	void ParticleSubEmitter();
	void ParticleBurst();
	void SetBurstText();
	void ParticleColor();
	void ParticleValues();
	void ParticleShape();
	void ParticleSpace();
	
	void UpdateTransform();
	void SetNewAnimation();
	void CreateParticles(int particlesToCreate, ShapeType shapeType, const math::float3 & pos, bool isBurst = false);
	math::float3 RandPos(ShapeType shapeType, bool isBurst = false);
	void ShowFloatValue(math::float2 & value, bool checkBox, const char * name, float v_speed, float v_min, float v_max);
	void EqualsMinMaxValues(math::float2 & value);
	void CheckMinMax(math::float2 & value);
	void ClearEmitter();
	void ConnectSubEmitter();
	void SoftClearEmitter();
	bool EditColor(ColorTime & colorTime, uint pos = 0u);

	math::float3 GetPos();

	void SetMaterialRes(uint materialUuid);
	void SetMeshParticleRes(uint res_uuid);
	void SetBurstMeshParticleRes(uint res_uuid);
	void SetMeshInfo(ResourceMesh* resource, MeshShape& shape);
	uint GetMaterialRes() const;

#ifndef GAMEMODE
	ImVec4 EqualsFloat4(const math::float4 float4D);
#endif
	int GetEmition() const;

	uint GetInternalSerializationBytes() override;
	virtual void OnInternalSave(char*& cursor);
	virtual void OnInternalLoad(char*& cursor);
public:
	GameTimer timer;
	GameTimer burstTime;

	bool drawAABB = false;
	bool drawShape = false;

	//Posibility space where particle is created
	math::AABB boxCreation = math::AABB(math::float3(-0.5f, -0.5f, -0.5f), math::float3(0.5f, 0.5f, 0.5f));
	math::Sphere sphereCreation = math::Sphere(math::float3::zero, 1.0f);
	math::Circle circleCreation = math::Circle(math::float3::unitY, math::float3::unitY, 1.0f);
	float coneHeight = 1.0f;

	// Emitter particles
	std::list<Particle*> particles;

	SimulatedGame simulatedGame = SimulatedGame_NONE;
	GameTimer timeSimulating;

	bool dieOnAnimation = false;

	GameObject* subEmitter = nullptr;
	uint uuidSubEmitter = 0u;
	ShapeType normalShapeType = ShapeType_BOX;

	std::list<math::float3> newPositions;

	StartValues startValues;

	//Create other particle when he death
	bool isSubEmitter = false;

	// Material
	uint materialRes = 0u;

	bool startOnPlay = false;

	float colorAverage = 0.0f;

	MeshShape burstMesh;
	MeshShape shapeMesh;

	bool burst = false;
	ShapeType burstType = ShapeType_BOX;

	math::AABB boundingBox;
private:
	// General info
	//---------------------------------------
	// Duration of the particle emitter
	float duration = 1.0f;

	//Check box Randomize values
	bool checkLife = false;
	bool checkSpeed = false;
	bool checkAcceleration = false;
	bool checkSizeOverTime = false;
	bool checkSize = false;
	bool checkRotation = false;
	bool checkAngularAcceleration = false;
	bool checkAngularVelocity = false;

	// Loop the particle (if true the particle emitter will never stop)
	bool loop = true;
	bool burstLoop = true;
	GameTimer loopTimer;
	// Warm up the particle emitter (if true the particle emitter will be already started at play-time)
	bool preWarm = true;

	ParticleAnimation particleAnim;

	//Burst options
	int minPart = 0;
	int maxPart = 10;
	float repeatTime = 1.0f;

	std::string burstTypeName = "Box Burst";

	int nextPos = 100;
	math::float4 nextColor = math::float4(0.0f, 0.0f, 0.0f, 1.0f);

	//---------------------------------------

	// Emission info
	//---------------------------------------
	// Number of particles created per second
	int rateOverTime = 10;
	float timeToParticle = 0.0f;
	uint nameLenght;
	//---------------------------------------

	//Particle always in GO pos or particle ignore their GO when spawned
	bool localSpace = false;

	bool isPlaying = false;
};
#endif // !__Emitter_H__