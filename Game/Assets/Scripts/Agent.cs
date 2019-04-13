using System.Collections;
using System;
using JellyBitEngine;

// https://github.com/Unity-Technologies/UnityCsReference/blob/master/Runtime/Export/Math/Vector3.cs

public class AgentConfiguration
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
    // AgentConfiguration
    public float agentMaxVelocity = 1.0f;
    public float agentMaxAngularVelocity = 1.0f;
    public float agentMaxAcceleration = 1.0f;
    public float agentMaxAngularAcceleration = 1.0f;
    public float agentArriveMinDistance = 0.1f;

    // SteeringSeparation
    public LayerMask separationMask = new LayerMask();
    public float separationRadius = 1.0f;
    public float separationThreshold = 1.0f;

    // SteeringAlign
    public float alignMinAngle = 0.01f;
    public float alignSlowAngle = 0.1f;
    public float alignTimeToTarget = 0.1f;
    #endregion

    #region PUBLIC_VARIABLES
    public AgentConfiguration agentConfiguration = new AgentConfiguration();

    public enum MovementState { Stop, GoToPosition, UpdateNextPosition };
    public MovementState movementState = MovementState.Stop;

    public Vector3 velocity = Vector3.zero;
    public float angularVelocity = 0.0f;

    public Vector3 Destination
    {
        get { return pathManager.Destination; }
    }

    // Steerings
    public SteeringSeek seek = new SteeringSeek();
    public SteeringFlee flee = new SteeringFlee();
    public SteeringSeparation separation = new SteeringSeparation();
    public SteeringAlign align = new SteeringAlign();
    #endregion

    #region PRIVATE_VARIABLES
    private PathManager pathManager = new PathManager();

    private Vector3[] velocities = null;
    private float[] angularVelocities = null;
    #endregion

    // ----------------------------------------------------------------------------------------------------

    public override void Start()
    {
        velocities = new Vector3[SteeringConfiguration.maxPriorities];
        angularVelocities = new float[SteeringConfiguration.maxPriorities];

        ResetPriorities();

        // --------------------------------------------------

        // AgentConfiguration
        agentConfiguration.maxVelocity = agentMaxVelocity;
        agentConfiguration.maxAngularVelocity = agentMaxAngularVelocity;
        agentConfiguration.maxAcceleration = agentMaxAcceleration;
        agentConfiguration.maxAngularAcceleration = agentMaxAngularAcceleration;
        agentConfiguration.arriveMinDistance = agentArriveMinDistance;

        // SteeringSeparation
        separation.mask = separationMask;
        separation.radius = separationRadius;
        separation.threshold = separationThreshold;

        // SteeringAlign
        align.minAngle = alignMinAngle;
        align.slowAngle = alignSlowAngle;
        align.timeToTarget = alignTimeToTarget;
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
        //velocities[separation.Priority] += separation.GetSeparation(this);

        // 3. Move
        /// Velocity
        velocities[seek.Priority] += seek.GetSeek(this);
        //velocities[flee.Priority] += flee.GetFlee(this);

        /// Angular velocity
        //angularVelocities[align.Priority] += align.GetAlign(this);

        // Angular velocities
        foreach (float angVel in angularVelocities)
        {
            if (!Approximately(angVel, 0.0f))
            {
                newAngularVelocity = angVel;
                break;
            }
        }

        // Velocities
        foreach (Vector3 vel in velocities)
        {
            if (!Approximately((float)vel.magnitude, 0.0f))
            {
                newVelocity = vel;
                break;
            }
        }

        ResetPriorities();

        velocity += newVelocity;
        angularVelocity += newAngularVelocity;

        // Cap angular velocity
        Mathf.Clamp(angularVelocity, -agentConfiguration.maxAngularVelocity, agentConfiguration.maxAngularVelocity);

        // Cap velocity
        if (velocity.magnitude > agentConfiguration.maxVelocity)
        {
            if (velocity.magnitude > 0.0)
                velocity.Normalize();
            velocity *= agentConfiguration.maxVelocity;
        }

        // Rotate
        transform.rotation *= Quaternion.Rotate(Vector3.up, angularVelocity * Time.deltaTime);

        // Move
        Debug.Log("Velocity " + velocity);
        transform.position += velocity * Time.deltaTime;
    }

    private void ResetPriorities()
    {
        for (uint i = 0; i < SteeringConfiguration.maxPriorities; ++i)
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

                if (pathManager.GetRemainingDistance(this) < agentConfiguration.arriveMinDistance)
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

    // ----------------------------------------------------------------------------------------------------

    // A tiny floating point value (RO)
    public static readonly float Epsilon = float.MinValue;

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