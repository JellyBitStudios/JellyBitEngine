﻿using System.Collections;
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
    public uint seekPriority = 1;

    // SteeringFleeData
    public bool isFleeActive = true;
    public uint fleePriority = 1;

    // SteeringSeparationData
    public bool isSeparationActive = true;
    public uint separationPriority = 1;
    public LayerMask separationMask = new LayerMask();
    public float separationRadius = 1.0f;
    public float separationThreshold = 1.0f;

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
    public SteeringAlignData alignData = new SteeringAlignData();

    // --------------------------------------------------

    public enum MovementState { Stop, GoToPosition, UpdateNextPosition };
    public MovementState movementState = MovementState.Stop;

    public Vector3 velocity = Vector3.zero;
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

        // 1. Collision avoidance
        // TODO

        // 2. Separation
        if (separationData.isActive)
            velocities[separationData.Priority] += SteeringSeparation.GetSeparation(this);

        // 3. Move
        /// Velocity
        if (seekData.isActive)
            velocities[seekData.Priority] += SteeringSeek.GetSeek(this);
        //velocities[flee.Priority] += flee.GetFlee();

        /// Angular velocity
        //angularVelocities[align.Priority] += align.GetAlign(this);

        // Angular velocities
        for (uint i = 0; i < SteeringData.maxPriorities; ++i)
        {
            if (!Approximately(angularVelocities[i], 0.0f))
            {
                newAngularVelocity = angularVelocities[i];
                break;
            }
        }

        // Velocities
        for (uint i = 0; i < SteeringData.maxPriorities; ++i)
        {
            if (!Approximately(velocities[i].magnitude, 0.0f))
            {
                newVelocity = velocities[i];
                break;
            }
        }

        ResetPriorities();

        velocity += newVelocity;
        angularVelocity += newAngularVelocity;

        // Cap angular velocity
        Mathf.Clamp(angularVelocity, -agentData.maxAngularVelocity, agentData.maxAngularVelocity);

        // Cap velocity
        if (velocity.magnitude > agentData.maxVelocity)
        {
            velocity.Normalize();
            velocity *= agentData.maxVelocity;
        }

        // Rotate
        transform.rotation *= Quaternion.Rotate(Vector3.up, angularVelocity * Time.deltaTime);

        // Move
        velocity = new Vector3(velocity.x, 0.0f, velocity.z);
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
        float[] color = { 1.0f, 0.0f, 0.0f, 1.0f };
        Debug.DrawSphere(4.0f, color, gameObject.transform.position, Quaternion.identity, Vector3.one);

        //SteeringSeek.DrawGizmos(this);
        //SteeringFlee.DrawGizmos(this);
        //SteeringSeparation.DrawGizmos(this);
        //SteeringAlign.DrawGizmos(this);
    }

    // ----------------------------------------------------------------------------------------------------

        // A tiny floating point value (RO)
    public static readonly float Epsilon = float.Epsilon;

    // Compares two floating point values if they are similar
    public static bool Approximately(float a, float b)
    {
        // If a or b is zero, compare that the other is less or equal to epsilon.
        // If neither a or b are 0, then find an epsilon that is good for
        // comparing numbers at the maximum magnitude of a and b.
        // Floating points have about 7 significant digits, so
        // 1.000001f can be represented while 1.0000001f is rounded to zero,
        // thus we could use an epsilon of 0.000001f for comparing values close to 1.
        // We multiply this epsilon by the biggest magnitude of a and b.
        return Math.Abs(b - a) < Math.Max(0.000001f * Math.Max(Math.Abs(a), Math.Abs(b)), Epsilon * 8);
    }
}