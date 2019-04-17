﻿using System.Collections;
using System;
using JellyBitEngine;

public class AgentData
{
    public float maxVelocity = 5.0f;
    public float maxAngularVelocity = 360.0f;
    public float maxAcceleration = 10.0f;
    public float maxAngularAcceleration = 360.0f;
}

public class Agent : JellyScript
{
    #region INSPECTOR_VARIABLES    
    // AgentData
    public float tmp_agentMaxVelocity = 5.0f;
    public float tmp_agentMaxAngularVelocity = 360.0f;
    public float tmp_agentMaxAcceleration = 10.0f;
    public float tmp_agentMaxAngularAcceleration = 360.0f;

    // SteeringSeekData
    public bool tmp_seekIsActive = true;
    public uint tmp_seekPriority = 2;
    public float tmp_arriveMinDistance = 0.6f;

    // SteeringFleeData
    public bool tmp_fleeIsActive = false;
    public uint tmp_fleePriority = 2;

    // SteeringWanderData
    public bool tmp_wanderIsActive = false;
    public uint tmp_wanderPriority = 2;
    public float tmp_radius = 1.0f;
    public float tmp_offset = 1.0f;
    public float tmp_minTime = 0.5f;
    public float tmp_maxTime = 3.0f;

    // SteeringSeparationData
    public bool tmp_separationIsActive = true;
    public uint tmp_separationPriority = 1;
    public LayerMask tmp_separationMask = new LayerMask();
    public float tmp_separationRadius = 5.0f;
    public float tmp_separationThreshold = 3.0f;

    // SteeringCollisionAvoidance
    public bool tmp_collisionAvoidanceIsActive = true;
    public uint tmp_collisionAvoidancePriority = 0;
    public LayerMask tmp_collisionAvoidanceMask = new LayerMask();
    public float tmp_collisionAvoidanceRadius = 5.0f;
    public float tmp_collisionAvoidanceConeHalfAngle = 45.0f;

    // SteeringObstacleAvoidance
    public bool tmp_obstacleAvoidanceIsActive = true;
    public uint tmp_obstacleAvoidancePriority = 0;
    public LayerMask tmp_obstacleAvoidanceMask = new LayerMask();
    public float tmp_obstacleAvoidanceAvoidDistance = 5.0f;

    // SteeringAlignData
    public bool tmp_alignIsActive = true;
    public uint tmp_alignPriority = 0;
    public float tmp_alignMinAngle = 5.0f;
    public float tmp_alignSlowAngle = 15.0f;
    public float tmp_alignTimeToTarget = 0.1f;
    public bool tmp_alignIsLookWhereYoureGoingActive = true;
    public bool tmp_alignIsFaceToActive = false;
    public GameObject tmp_alignFaceToTarget = null;
    #endregion

    #region PUBLIC_VARIABLES
    // AgentData
    public AgentData agentData = new AgentData();

    // SteeringsData
    public SteeringSeekData seekData = new SteeringSeekData();
    public SteeringFleeData fleeData = new SteeringFleeData();
    public SteeringWanderData wanderData = new SteeringWanderData();
    public SteeringSeparationData separationData = new SteeringSeparationData();
    public SteeringCollisionAvoidanceData collisionAvoidanceData = new SteeringCollisionAvoidanceData();
    public SteeringObstacleAvoidanceData obstacleAvoidanceData = new SteeringObstacleAvoidanceData();
    public SteeringAlignData alignData = new SteeringAlignData();

    // --------------------------------------------------

    public Vector3 Destination
    {
        get { return pathManager.Destination; }
    }
    public Vector3 NextPosition
    {
        get { return pathManager.GetNextPosition(this); }
    }
    public bool HasArrived
    {
        get { return pathManager.IsLastPosition && movementState == MovementState.Stop; }
    }

    public bool isMovementStopped = false;
    public bool isRotationStopped = false;
    [HideInInspector]
    public Vector3 velocity = Vector3.zero;
    [HideInInspector]
    public float angularVelocity = 0.0f;
    public bool HasFaced
    {
        get { return hasFaced; }
    }

    public bool drawGizmos = true;
    #endregion

    #region PRIVATE_VARIABLES
    private PathManager pathManager = new PathManager();

    private Vector3[] velocities = null;
    private float[] angularVelocities = null;

    private bool hasFaced = false;

    private enum MovementState { Stop, GoToPosition, UpdateNextPosition };
    private MovementState movementState = MovementState.Stop;
    #endregion

    // ----------------------------------------------------------------------------------------------------

    public override void Start()
    {
        velocities = new Vector3[SteeringData.maxPriorities];
        angularVelocities = new float[SteeringData.maxPriorities];

        ResetPriorities();

        // --------------------------------------------------

        // AgentData
        agentData.maxVelocity = tmp_agentMaxVelocity;
        agentData.maxAngularVelocity = tmp_agentMaxAngularVelocity;
        agentData.maxAcceleration = tmp_agentMaxAcceleration;
        agentData.maxAngularAcceleration = tmp_agentMaxAngularAcceleration;

        // SteeringSeekData
        seekData.isActive = tmp_seekIsActive;
        seekData.Priority = tmp_seekPriority;
        seekData.arriveMinDistance = tmp_arriveMinDistance;

        // SteeringFleeData
        fleeData.isActive = tmp_fleeIsActive;
        fleeData.Priority = tmp_fleePriority;

        // SteeringWanderData
        wanderData.isActive = tmp_wanderIsActive;
        wanderData.Priority = tmp_wanderPriority;
        wanderData.radius = tmp_radius;
        wanderData.offset = tmp_offset;
        wanderData.minTime = tmp_minTime;
        wanderData.maxTime = tmp_maxTime;

        // SteeringSeparationData
        separationData.isActive = tmp_separationIsActive;
        separationData.Priority = tmp_separationPriority;
        separationData.mask = tmp_separationMask;
        separationData.radius = tmp_separationRadius;
        separationData.threshold = tmp_separationThreshold;

        // SteeringCollisionAvoidance
        collisionAvoidanceData.isActive = tmp_collisionAvoidanceIsActive;
        collisionAvoidanceData.Priority = tmp_collisionAvoidancePriority;
        collisionAvoidanceData.mask = tmp_collisionAvoidanceMask;
        collisionAvoidanceData.radius = tmp_collisionAvoidanceRadius;
        collisionAvoidanceData.coneHalfAngle = tmp_collisionAvoidanceConeHalfAngle;

        // SteeringObstacleAvoidance
        obstacleAvoidanceData.isActive = tmp_obstacleAvoidanceIsActive;
        obstacleAvoidanceData.Priority = tmp_obstacleAvoidancePriority;
        obstacleAvoidanceData.mask = tmp_obstacleAvoidanceMask;
        obstacleAvoidanceData.avoidDistance = tmp_obstacleAvoidanceAvoidDistance;
        obstacleAvoidanceData.rays = new SteeringRay[3];
        for (uint i = 0; i < obstacleAvoidanceData.rays.Length; ++i)
            obstacleAvoidanceData.rays[i] = new SteeringRay();
        obstacleAvoidanceData.rays[0].length = 3.0f;
        obstacleAvoidanceData.rays[1].direction = new Vector3(-1.0f, 0.0f, 1.0f);
        obstacleAvoidanceData.rays[1].length = 1.5f;
        obstacleAvoidanceData.rays[2].direction = new Vector3(1.0f, 0.0f, 1.0f);
        obstacleAvoidanceData.rays[2].length = 1.5f;

        // SteeringAlignData
        alignData.isActive = tmp_alignIsActive;
        alignData.Priority = tmp_alignPriority;
        alignData.minAngle = tmp_alignMinAngle;
        alignData.slowAngle = tmp_alignSlowAngle;
        alignData.timeToTarget = tmp_alignTimeToTarget;
        alignData.lookWhereYoureGoingData.isActive = tmp_alignIsLookWhereYoureGoingActive;
        alignData.faceData.isActive = tmp_alignIsFaceToActive;
        alignData.faceData.target = tmp_alignFaceToTarget;
    }

    public override void FixedUpdate()
    {
        UpdateInspectorVariables();

        // --------------------------------------------------

        Move();

        // --------------------------------------------------

        Vector3 newVelocity = Vector3.zero;
        float newAngularVelocity = 0.0f;

        // 1. Obstacle avoidance
        if (obstacleAvoidanceData.isActive)
            velocities[obstacleAvoidanceData.Priority] += SteeringObstacleAvoidance.GetObstacleAvoidance(this);
        if (collisionAvoidanceData.isActive)
            velocities[collisionAvoidanceData.Priority] += SteeringCollisionAvoidance.GetCollisionAvoidance(this);

        // 2. Separation
        if (separationData.isActive)
            velocities[separationData.Priority] += SteeringSeparation.GetSeparation(this);

        // 3. Move
        /// Velocity      
        if (seekData.isActive)
            velocities[seekData.Priority] += SteeringSeek.GetSeek(NextPosition, this);
        if (fleeData.isActive)
            velocities[fleeData.Priority] += SteeringFlee.GetFlee(NextPosition, this);
        if (wanderData.isActive)
            velocities[wanderData.Priority] += SteeringWander.GetWander(this);

        /// Angular velocity
        hasFaced = false;
        if (alignData.isActive)
        {
            if (alignData.lookWhereYoureGoingData.isActive)
            {
                if (velocity.magnitude != 0.0f)
                    angularVelocities[alignData.Priority] += SteeringAlign.GetLookWhereYoureGoing(this);
            }

            if (alignData.faceData.isActive)
            {
                float align = SteeringAlign.GetFace(this);
                angularVelocities[alignData.Priority] += align;
                hasFaced = align == 0.0f;
            }
        }

        // Angular velocities
        for (uint i = 0; i < SteeringData.maxPriorities; ++i)
        {
            if (!MathScript.Approximately(angularVelocities[i], 0.0f))
            {
                newAngularVelocity = angularVelocities[i];
                break;
            }
        }

        // Velocities
        for (uint i = 0; i < SteeringData.maxPriorities; ++i)
        {
            if (!MathScript.Approximately(velocities[i].magnitude, 0.0f))
            {
                newVelocity = velocities[i];
                break;
            }
        }

        ResetPriorities();

        if (newAngularVelocity != 0.0f)
            angularVelocity += newAngularVelocity;
        else
            angularVelocity = newAngularVelocity;
        if (newVelocity.magnitude != 0.0f)
            velocity += newVelocity;
        else
            velocity = Vector3.zero;

        // Cap angular velocity
        angularVelocity = Mathf.Clamp(angularVelocity, -agentData.maxAngularVelocity, agentData.maxAngularVelocity);
  
        // Cap velocity
        if (velocity.magnitude > agentData.maxVelocity)
        {
            velocity.Normalize();
            velocity *= agentData.maxVelocity;
        }

        if (!isRotationStopped)
            // Rotate
            transform.rotation *= Quaternion.Rotate(Vector3.up, angularVelocity * Time.deltaTime);

        if (!isMovementStopped)
            // Move
            transform.position += velocity * Time.deltaTime;
    }

    public override void OnDrawGizmos()
    {
        if (!drawGizmos)
            return;

        Debug.DrawLine(transform.position, transform.position + Quaternion.Rotate(Vector3.up, angularVelocity) * transform.forward * 3.0f, Color.Black);
        Debug.DrawLine(transform.position, transform.position + velocity.normalized() * 3.0f, Color.White);

        if (seekData.isActive)
            SteeringSeek.DrawGizmos(this);
        if (fleeData.isActive)
            SteeringFlee.DrawGizmos(this);
        if (wanderData.isActive)
            SteeringWander.DrawGizmos(this);
        if (separationData.isActive)
            SteeringSeparation.DrawGizmos(this);
        if (collisionAvoidanceData.isActive)
            SteeringCollisionAvoidance.DrawGizmos(this);
        if (obstacleAvoidanceData.isActive)
            SteeringObstacleAvoidance.DrawGizmos(this);
        if (alignData.isActive)
            SteeringAlign.DrawGizmos(this);

        pathManager.DrawGizmos();
    }

    // ----------------------------------------------------------------------------------------------------
    
    public bool SetDestination(Vector3 destination)
    {
        bool hasPath = pathManager.GetPath(transform.position, destination);

        if (hasPath)
            movementState = MovementState.GoToPosition;
        else
            movementState = MovementState.Stop;

        return hasPath;
    }

    public bool SetFace(GameObject gameObject)
    {
        if (gameObject == null)
            return false;

        alignData.lookWhereYoureGoingData.isActive = false;
        alignData.faceData.isActive = true;
        alignData.faceData.target = gameObject;

        return true;
    }

    public void Stop()
    {
        isMovementStopped = true;
        isRotationStopped = true;
    }

    public void Reset()
    {
        isMovementStopped = false;
        isRotationStopped = false;
    }

    public void ClearPath()
    {
        pathManager.ClearPath();
    }

    public void ClearMovementAndRotation()
    {
        velocity = Vector3.zero;
        angularVelocity = 0.0f;
    }

    public void ActivateSeek()
    {
        seekData.isActive = true;
        fleeData.isActive = false;
        wanderData.isActive = false;
    }

    public void ActivateFlee()
    {
        seekData.isActive = false;
        fleeData.isActive = true;
        wanderData.isActive = false;
    }

    public void ActivateWander()
    {
        wanderData.isActive = true;
        seekData.isActive = false;
        fleeData.isActive = false;
    }

    public void ActivateAvoidance()
    {
        separationData.isActive = true;
        collisionAvoidanceData.isActive = true;
        obstacleAvoidanceData.isActive = true;
    }

    public void DeactivateAvoidance()
    {
        separationData.isActive = false;
        collisionAvoidanceData.isActive = false;
        obstacleAvoidanceData.isActive = false;
    }

    private void Move()
    {
        switch (movementState)
        {
            case MovementState.GoToPosition:

                if (pathManager.GetRemainingDistance(this) < seekData.arriveMinDistance)
                    movementState = MovementState.UpdateNextPosition;

                break;

            case MovementState.UpdateNextPosition:

                if (pathManager.UpdateNextPosition())
                    movementState = MovementState.GoToPosition;
                else
                    movementState = MovementState.Stop;

                break;
        }
    }

    private void ResetPriorities()
    {
        for (uint i = 0; i < SteeringData.maxPriorities; ++i)
        {
            velocities[i] = Vector3.zero;
            angularVelocities[i] = 0.0f;
        }
    }

    // ----------------------------------------------------------------------------------------------------

    private void UpdateInspectorVariables()
    {
        // AgentData
        tmp_agentMaxVelocity = agentData.maxVelocity;
        tmp_agentMaxAngularVelocity = agentData.maxAngularVelocity;
        tmp_agentMaxAcceleration = agentData.maxAcceleration;
        tmp_agentMaxAngularAcceleration = agentData.maxAngularAcceleration;

        // SteeringSeekData
        tmp_seekIsActive = seekData.isActive;
        tmp_seekPriority = seekData.Priority;
        tmp_arriveMinDistance = seekData.arriveMinDistance;

        // SteeringFleeData
        tmp_fleeIsActive = fleeData.isActive;
        tmp_fleePriority = fleeData.Priority;

        // SteeringWanderData
        tmp_wanderIsActive = wanderData.isActive;
        tmp_wanderPriority = wanderData.Priority;
        tmp_radius = wanderData.radius;
        tmp_offset = wanderData.offset;
        tmp_minTime = wanderData.minTime;
        tmp_maxTime = wanderData.maxTime;

        // SteeringSeparationData
        tmp_separationIsActive = separationData.isActive;
        tmp_separationPriority = separationData.Priority;
        tmp_separationMask = separationData.mask;
        tmp_separationRadius = separationData.radius;
        tmp_separationThreshold = separationData.threshold;

        // SteeringCollisionAvoidance
        tmp_collisionAvoidanceIsActive = collisionAvoidanceData.isActive;
        tmp_collisionAvoidancePriority = collisionAvoidanceData.Priority;
        tmp_collisionAvoidanceMask = collisionAvoidanceData.mask;
        tmp_collisionAvoidanceRadius = collisionAvoidanceData.radius;
        tmp_collisionAvoidanceConeHalfAngle = collisionAvoidanceData.coneHalfAngle;

        // SteeringObstacleAvoidance
        tmp_obstacleAvoidanceIsActive = obstacleAvoidanceData.isActive;
        tmp_obstacleAvoidancePriority = obstacleAvoidanceData.Priority;
        tmp_obstacleAvoidanceMask = obstacleAvoidanceData.mask;
        tmp_obstacleAvoidanceAvoidDistance = obstacleAvoidanceData.avoidDistance;

        // SteeringAlignData
        tmp_alignIsActive = alignData.isActive;
        tmp_alignPriority = alignData.Priority;
        tmp_alignMinAngle = alignData.minAngle;
        tmp_alignSlowAngle = alignData.slowAngle;
        tmp_alignTimeToTarget = alignData.timeToTarget;
        tmp_alignIsLookWhereYoureGoingActive = alignData.lookWhereYoureGoingData.isActive;
        tmp_alignIsFaceToActive = alignData.faceData.isActive;
        tmp_alignFaceToTarget = alignData.faceData.target;
    }
}