#ifndef __NM_SUPPLIER_H__
#define __NM_SUPPLIER_H__

#include "Module.h"

#include "Recast&Detour/Recast/Include/Recast.h"
#include "Recast&Detour/Detour/Include/DetourNavMeshQuery.h"
#include "NMBuildContext.h"

#include <vector>
#include "MathGeoLib/include/Math/float3.h"

constexpr int max_Agents = 128;
#define N_EPSILON 0.00050f

class ModuleNavigation : public Module
{
public:

	ModuleNavigation(bool start_enabled = true);
	virtual ~ModuleNavigation();
	bool Init(JSON_Object* jObject);
	update_status Update();
	bool CleanUp();

	void OnSystemEvent(System_Event e);
	void InitDetour();
	void Draw() const;
	void AddComponent(class ComponentNavAgent*);
	void EraseComponent(class ComponentNavAgent*);
	void SetInputGeom(class NMInputGeom& inputGeom);
	bool FindPath(float* start, float* end, std::vector<math::float3>& finalPath, const math::float3& extents = math::float3(1.2f, 0.9f, 1.2f)) const;
	bool ProjectPoint(float* point, math::float3& projectedPoint, const math::float3& extents = math::float3(1.2f, 0.9f, 1.2f)) const;
	bool ProjectPointPolyBoundary(float* point, math::float3& projectedPoint, const math::float3& extents = math::float3(1.2f, 0.9f, 1.2f)) const;
	int  AddAgent(const float* p, float radius, float height, float maxAcc, float maxSpeed,
				  float collQueryRange, float pathOptimRange, unsigned char updateFlags,
				  unsigned char obstacleAvoidanceType, float stopAtLength) const;
	bool UpdateAgentParams(int indx, float radius, float height, float maxAcc, float maxSpeed,
						   float collQueryRange, float pathOptimRange, unsigned char updateFlags,
						   unsigned char obstacleAvoidanceType, float stopAtLength) const;
	void RemoveAgent(int indx) const;
	void SetDestination(const float* p, int indx) const;
	bool IsWalking(int index) const;
	void RequestMoveVelocity(int index, const float* vel);
	void ResetMoveTarget(int index);

	static void calcVel(float* vel, const float* pos, const float* tgt, const float speed);

	bool IsCrowInitialized() const { return m_crowd ? true :  false; };

	bool HandleBuild();

	int  GetNavMeshSerialitzationBytes() const;
	void SaveNavmesh(char*& cursor);
	void LoadNavmesh(char*& cursor);

private:

	void cleanup();
	void InitCrowd();

public:

	bool drawNavmesh = false;

protected:

	class dtNavMesh* m_navMesh = 0;
	class dtNavMeshQuery* m_navQuery = 0;
	class dtCrowd* m_crowd = 0;
	dtQueryFilter m_filter;

	NMBuildContext* m_ctx = 0;

	// Navmesh Cration
	class NMInputGeom* m_geom = 0;
	unsigned char* m_triareas = 0;
	rcHeightfield* m_solid = 0;
	rcCompactHeightfield* m_chf = 0;
	rcContourSet* m_cset = 0;
	rcPolyMesh* m_pmesh = 0;
	rcConfig m_cfg;
	rcPolyMeshDetail* m_dmesh = 0;
	
	std::vector<class ComponentNavAgent*> c_agents;
};

#endif