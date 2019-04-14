using System.Collections;
using System;
using JellyBitEngine;

public class AgentData
{
    public float maxVelocity = 1.0f;
    public float maxAngularVelocity = 1.0f;
    public float maxAcceleration = 1.0f;
    public float maxAngularAcceleration = 1.0f;

    // Arrive
    public float arriveMinDistance = 0.1f;
}

public class Agent : JellyScript
{
    #region INSPECTOR_VARIABLES    
    // AgentData
    public float agentMaxVelocity = 1.0f;
    public float agentMaxAngularVelocity = 1.0f;
    public float agentMaxAcceleration = 1.0f;
    public float agentMaxAngularAcceleration = 1.0f;
    public float agentArriveMinDistance = 0.1f;

    // SteeringSeekData
    public bool isSeekActive = true;
    public uint seekPriority = 2;

    // SteeringFleeData
    public bool isFleeActive = false;
    public uint fleePriority = 2;

    // SteeringSeparationData
    public bool isSeparationActive = true;
    public uint separationPriority = 1;
    public LayerMask separationMask = new LayerMask();
    public float separationRadius = 1.0f;
    public float separationThreshold = 1.0f;

    // SteeringObstacleAvoidance
    public bool isObstacleAvoidanceActive = true;
    public uint obstacleAvoidancePriority = 0;
    public LayerMask obstacleAvoidanceMask = new LayerMask();
    public float obstacleAvoidanceAvoidDistance = 1.0f;

    // SteeringAlignData
    public bool isAlignActive = true;
    public uint alignPriority = 1;
    public float alignMinAngle = 0.01f;
    public float alignSlowAngle = 0.1f;
    public float alignTimeToTarget = 0.1f;
    #endregion

    #region PUBLIC_VARIABLES
    // AgentData
    public AgentData agentData = new AgentData();

    // SteeringsData
    public SteeringSeekData seekData = new SteeringSeekData();
    public SteeringFleeData fleeData = new SteeringFleeData();
    public SteeringSeparationData separationData = new SteeringSeparationData();
    public SteeringObstacleAvoidanceData obstacleAvoidanceData = new SteeringObstacleAvoidanceData();
    public SteeringAlignData alignData = new SteeringAlignData();

    // --------------------------------------------------

    public enum MovementState { Stop, GoToPosition, UpdateNextPosition };
    public MovementState movementState = MovementState.Stop;

    [HideInInspector]
    public Vector3 velocity = Vector3.zero;
    [HideInInspector]
    public float angularVelocity = 0.0f;

    public Vector3 Destination
    {
        get { return pathManager.Destination; }
    }
    public Vector3 NextPosition
    {
        get { return pathManager.NextPosition; }
    }
    #endregion

    #region PRIVATE_VARIABLES
    private PathManager pathManager = new PathManager();

    private Vector3[] velocities = null;
    private float[] angularVelocities = null;
    #endregion

    // ----------------------------------------------------------------------------------------------------

    public override void Start()
    {
        velocities = new Vector3[SteeringData.maxPriorities];
        angularVelocities = new float[SteeringData.maxPriorities];

        ResetPriorities();

        // --------------------------------------------------

        // AgentData
        agentData.maxVelocity = agentMaxVelocity;
        agentData.maxAngularVelocity = agentMaxAngularVelocity;
        agentData.maxAcceleration = agentMaxAcceleration;
        agentData.maxAngularAcceleration = agentMaxAngularAcceleration;
        agentData.arriveMinDistance = agentArriveMinDistance;

        // SteeringSeekData
        seekData.isActive = isSeekActive;
        seekData.Priority = seekPriority;

        // SteeringFleeData
        fleeData.isActive = isFleeActive;
        fleeData.Priority = fleePriority;

        // SteeringSeparationData
        separationData.isActive = isSeparationActive;
        separationData.Priority = separationPriority;
        separationData.mask = separationMask;
        separationData.radius = separationRadius;
        separationData.threshold = separationThreshold;

        // SteeringObstacleAvoidance
        obstacleAvoidanceData.isActive = isObstacleAvoidanceActive;
        obstacleAvoidanceData.Priority = obstacleAvoidancePriority;
        obstacleAvoidanceData.mask = obstacleAvoidanceMask;
        obstacleAvoidanceData.avoidDistance = obstacleAvoidanceAvoidDistance;
        obstacleAvoidanceData.rays = new SteeringRay[3];
        for (uint i = 0; i < obstacleAvoidanceData.rays.Length; ++i)
            obstacleAvoidanceData.rays[i] = new SteeringRay();
        obstacleAvoidanceData.rays[0].length = 3.0f;
        obstacleAvoidanceData.rays[1].direction = new Vector3(-1.0f, 0.0f, 1.0f);
        obstacleAvoidanceData.rays[2].direction = new Vector3(1.0f, 0.0f, 1.0f);

        // SteeringAlignData
        alignData.isActive = isAlignActive;
        alignData.Priority = alignPriority;
        alignData.minAngle = alignMinAngle;
        alignData.slowAngle = alignSlowAngle;
        alignData.timeToTarget = alignTimeToTarget;
    }

    public override void FixedUpdate()
    {
        Move();

        // --------------------------------------------------

        Vector3 newVelocity = Vector3.zero;
        float newAngularVelocity = 0.0f;

        // 1. Obstacle avoidance
        if (obstacleAvoidanceData.isActive)
            velocities[obstacleAvoidanceData.Priority] += SteeringObstacleAvoidance.GetObstacleAvoidance(this);

        // 2. Separation
        if (separationData.isActive)
            velocities[separationData.Priority] += SteeringSeparation.GetSeparation(this);

        // 3. Move
        /// Velocity
        if (seekData.isActive)
            velocities[seekData.Priority] += SteeringSeek.GetSeek(this);
        if (fleeData.isActive)
            velocities[fleeData.Priority] += SteeringFlee.GetFlee(this);

        /// Angular velocity
        if (alignData.isActive)
            angularVelocities[alignData.Priority] += SteeringAlign.GetAlign(this);

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

        angularVelocity += newAngularVelocity;
        velocity += newVelocity;

        // Cap angular velocity
        angularVelocity = Mathf.Clamp(angularVelocity, -agentData.maxAngularVelocity, agentData.maxAngularVelocity);
  
        // Cap velocity
        if (velocity.magnitude > agentData.maxVelocity)
        {
            velocity.Normalize();
            velocity *= agentData.maxVelocity;
        }

        // Rotate
        transform.rotation *= Quaternion.Rotate(Vector3.up, angularVelocity * Time.deltaTime);

        // Move
        transform.position += velocity * Time.deltaTime;
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
    
    public void SetDestination(Vector3 destination)
    {
        if (pathManager.GetPath(transform.position, destination))
            movementState = MovementState.GoToPosition;
        else
            movementState = MovementState.Stop;
    }

    public void Move()
    {
        switch (movementState)
        {
            case MovementState.GoToPosition:

                if (pathManager.GetRemainingDistance(this) < agentData.arriveMinDistance)
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

    public override void OnDrawGizmos()
    {
        float[] colorAngularVelocity = { 0.0f, 0.0f, 0.0f, 1.0f };
        Debug.DrawLine(transform.position, transform.position + Quaternion.Rotate(Vector3.up, angularVelocity) * transform.forward * 2.0f, colorAngularVelocity);

        float[] colorVelocity = { 1.0f, 1.0f, 1.0f, 1.0f };
        Debug.DrawLine(transform.position, transform.position + velocity, colorVelocity);

        SteeringSeek.DrawGizmos(this);
        SteeringFlee.DrawGizmos(this);
        SteeringSeparation.DrawGizmos(this);
        SteeringObstacleAvoidance.DrawGizmos(this);
        SteeringAlign.DrawGizmos(this);
    }
}