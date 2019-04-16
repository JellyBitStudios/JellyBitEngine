using System.Collections;
using System;
using JellyBitEngine;

// https://forum.unity.com/threads/c-proper-state-machine.380612/

public enum StateType
{
    None,

    // GoTo
    GoToDangerDistance,
    GoToAttackDistance,

    // Attack
    Attack,

    // Wander
    Strafe
}

public abstract class ICyborgMeleeState
{
    public StateType stateType = StateType.None;

    public abstract void Enter(CyborgMeleeController owner);
    public abstract void Execute(CyborgMeleeController owner);
    public abstract void Exit(CyborgMeleeController owner);
    public abstract void DrawGizmos(CyborgMeleeController owner);
}

public class CyborgMeleeFSM
{
    private ICyborgMeleeState state = null;
    private CyborgMeleeController owner = null;

    public CyborgMeleeFSM(CyborgMeleeController owner)
    {
        this.owner = owner;
    }

    public void ChangeState(ICyborgMeleeState state)
    {
        if (state == null)
            return;

        // Exit
        if (this.state != null)
            this.state.Exit(owner);

        this.state = state;

        // Enter
        this.state.Enter(owner);
    }

    public void UpdateState()
    {
        if (state != null)
            state.Execute(owner);
    }

    public void DrawGizmos()
    {
        if (state != null)
            state.DrawGizmos(owner);
    }
}

// ----------------------------------------------------------------------------------------------------
// GoToGameObject
// ----------------------------------------------------------------------------------------------------

public class GoToGameObject : ICyborgMeleeState
{
    private StateType prevStateType = StateType.None;

    // -----

    private GameObject target = null;

    // -----

    private float maxAcceleration = 0.0f;
    private float maxVelocity = 0.0f;

    // --------------------------------------------------

    public GoToGameObject(GameObject target, StateType stateType, StateType prevStateType = StateType.None)
    {
        this.stateType = stateType;
        this.prevStateType = prevStateType;

        // -----

        this.target = target;
    }

    // --------------------------------------------------

    public override void Enter(CyborgMeleeController owner)
    {
        /// Activate/Deactivate
        owner.agent.ActivateSeek();

        switch (stateType)
        {
            case StateType.GoToDangerDistance:
                {
                    Debug.Log("Enter GoToGameObject: GoToDangerDistance");

                    owner.agent.separationData.isActive = false;
                    owner.agent.collisionAvoidanceData.isActive = true;
                }
                break;

            case StateType.GoToAttackDistance:
                {
                    Debug.Log("Enter GoToGameObject: GoToAttackDistance");

                    maxAcceleration = owner.agent.agentData.maxAcceleration;
                    maxVelocity = owner.agent.agentData.maxVelocity;

                    // -----

                    owner.agent.agentData.maxAcceleration /= 2.0f;
                    owner.agent.agentData.maxVelocity /= 2.0f;

                    owner.agent.separationData.isActive = true;
                    owner.agent.collisionAvoidanceData.isActive = false;
                }
                break;
        }

        owner.agent.SetDestination(target.transform.position);
    }

    public override void Execute(CyborgMeleeController owner)
    {
        switch (stateType)
        {
            case StateType.GoToDangerDistance:
                {
                    float distanceToTarget = (target.transform.position - owner.transform.position).magnitude;
                    if (distanceToTarget <= owner.character.dangerDistance)
                    {
                        owner.fsm.ChangeState(new GoToGameObject(owner.target, StateType.GoToAttackDistance, stateType));
                        return;
                    }
                }
                break;

            case StateType.GoToAttackDistance:
                {
                    float distanceToTarget = (target.transform.position - owner.transform.position).magnitude;
                    if (distanceToTarget <= owner.character.attackDistance)
                    {
                        // Am I allowed to attack?
                        if (owner.battleCircle.AddAttacker(owner.gameObject))
                        {
                            // Yes! Attack
                            owner.fsm.ChangeState(new Attack(owner.target, StateType.Attack, stateType));
                            return;
                        }
                        else
                        {
                            // No! Strafe
                            owner.fsm.ChangeState(new Wander(StateType.Strafe, stateType));
                            return;
                        }
                    }
                }
                break;
        }
    }

    public override void Exit(CyborgMeleeController owner)
    {
        switch (stateType)
        {
            case StateType.GoToDangerDistance:

                Debug.Log("Exit GoToGameObject: GoToDangerDistance");

                break;

            case StateType.GoToAttackDistance:

                Debug.Log("Exit GoToGameObject: GoToAttackDistance");

                owner.agent.agentData.maxAcceleration = maxAcceleration;
                owner.agent.agentData.maxVelocity = maxVelocity;

                // -----

                break;
        }
    }

    public override void DrawGizmos(CyborgMeleeController owner)
    {
        switch (stateType)
        {
            case StateType.GoToDangerDistance:
                Debug.DrawSphere(owner.character.dangerDistance, Color.Blue, owner.transform.position, Quaternion.identity, Vector3.one);
                break;

            case StateType.GoToAttackDistance:
                Debug.DrawSphere(owner.character.attackDistance, Color.Red, owner.transform.position, Quaternion.identity, Vector3.one);
                break;
        }
    }
}

// ----------------------------------------------------------------------------------------------------
// Attack
// ----------------------------------------------------------------------------------------------------

public class Attack : ICyborgMeleeState
{
    private StateType prevStateType = StateType.None;

    // -----

    private GameObject target = null;

    // -----

    public bool faceDataIsActive = false;
    public bool lookWhereYoureGoingDataIsActive = false;
    public bool isMovementStopped = false;

    // --------------------------------------------------

    public Attack(GameObject target, StateType stateType, StateType prevStateType = StateType.None)
    {
        this.stateType = stateType;
        this.prevStateType = prevStateType;

        // -----

        this.target = target;
    }

    // --------------------------------------------------

    public override void Enter(CyborgMeleeController owner)
    {
        Debug.Log("Enter Attack");

        faceDataIsActive = owner.agent.alignData.faceData.isActive;
        lookWhereYoureGoingDataIsActive = owner.agent.alignData.lookWhereYoureGoingData.isActive;
        isMovementStopped = owner.agent.isMovementStopped;

        // -----

        /// Activate/Deactivate
        owner.agent.alignData.faceData.isActive = true;
        owner.agent.alignData.lookWhereYoureGoingData.isActive = false;

        owner.agent.isMovementStopped = true;

        owner.agent.SetFace(target);
    }

    public override void Execute(CyborgMeleeController owner)
    {
        // TODO: keep hitting Alita after x seconds
    }

    public override void Exit(CyborgMeleeController owner)
    {
        Debug.Log("Exit Attack");

        owner.agent.alignData.faceData.isActive = faceDataIsActive;
        owner.agent.alignData.lookWhereYoureGoingData.isActive = lookWhereYoureGoingDataIsActive;
        owner.agent.isMovementStopped = isMovementStopped;

        // -----
    }

    public override void DrawGizmos(CyborgMeleeController owner)
    {
        Debug.DrawSphere(owner.character.attackDistance, Color.Red, owner.transform.position, Quaternion.identity, Vector3.one);
    }
}

// ----------------------------------------------------------------------------------------------------
// Wander
// ----------------------------------------------------------------------------------------------------

public class Wander : ICyborgMeleeState
{
    private StateType prevStateType = StateType.None;

    // -----

    private float strafeTime = 0.0f;
    private float timer = 0.0f;

    // -----

    private float maxAcceleration = 0.0f;
    private float maxVelocity = 0.0f;

    // --------------------------------------------------

    public Wander(StateType stateType, StateType prevStateType = StateType.None)
    {
        this.stateType = stateType;
        this.prevStateType = prevStateType;
    }

    // --------------------------------------------------

    public override void Enter(CyborgMeleeController owner)
    {
        switch (stateType)
        {
            case StateType.Strafe:
                {
                    Debug.Log("Enter Wander: Strafe");

                    maxAcceleration = owner.agent.agentData.maxAcceleration;
                    maxVelocity = owner.agent.agentData.maxVelocity;

                    // -----

                    // Agent data
                    owner.agent.agentData.maxAcceleration /= 2.0f;
                    owner.agent.agentData.maxVelocity /= 2.0f;

                    // Wander data
                    owner.agent.wanderData.radius = 1.0f;
                    owner.agent.wanderData.offset = 2.0f;

                    owner.agent.wanderData.minTime = 0.3f;
                    owner.agent.wanderData.maxTime = 0.7f;

                    /// Activate/Deactivate
                    owner.agent.ActivateWander();
                    owner.agent.ActivateAvoidance();

                    strafeTime = (float)MathScript.GetRandomDouble(owner.character.strafeMinTime, owner.character.strafeMaxTime);
                }
                break;
        }
    }

    public override void Execute(CyborgMeleeController owner)
    {
        switch (stateType)
        {
            case StateType.Strafe:
                {
                    if (timer >= strafeTime)
                    {
                        owner.fsm.ChangeState(new GoToGameObject(owner.target, StateType.GoToDangerDistance, stateType));
                        break;
                    }

                    timer += Time.deltaTime;
                }
                break;
        }
    }

    public override void Exit(CyborgMeleeController owner)
    {
        switch (stateType)
        {
            case StateType.Strafe:

                Debug.Log("Exit Wander: Strafe");

                owner.agent.agentData.maxAcceleration = maxAcceleration;
                owner.agent.agentData.maxVelocity = maxVelocity;

                // -----

                break;
        }
    }

    public override void DrawGizmos(CyborgMeleeController owner)
    {
        
    }
}