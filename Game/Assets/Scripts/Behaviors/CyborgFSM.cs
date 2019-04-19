using System.Collections;
using System;
using JellyBitEngine;

// https://forum.unity.com/threads/c-proper-state-machine.380612/
// >, <=

public abstract class ICyborgMeleeState
{
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

#region GO_TO_GAMEOBJECT
// ----------------------------------------------------------------------------------------------------
// GoToGameObject
// ----------------------------------------------------------------------------------------------------

public class GoToGameObject : ICyborgMeleeState
{
    public enum GoToGameObjectType
    {
        GoToDangerDistance,
        GoToAttackDistance,
        Runaway
    }
    private GoToGameObjectType stateType;

    // -----

    private float maxAcceleration = 0.0f;
    private float maxVelocity = 0.0f;

    // --------------------------------------------------

    public GoToGameObject(GameObject target, GoToGameObjectType stateType)
    {
        this.stateType = stateType;
    }

    // --------------------------------------------------

    public override void Enter(CyborgMeleeController owner)
    {
        switch (stateType)
        {
            case GoToGameObjectType.GoToDangerDistance: // Seek, default max acceleration and velocity

                Debug.Log("Enter GoToGameObject: GoToDangerDistance");

                /// Activate/Deactivate
                owner.agent.separationData.isActive = false;
                owner.agent.collisionAvoidanceData.isActive = true;
                owner.agent.ActivateSeek();

                break;

            case GoToGameObjectType.Runaway: // Flee, 1.5f max acceleration and velocity

                Debug.Log("Enter GoToGameObject: Runaway");

                maxAcceleration = owner.agent.agentData.maxAcceleration;
                maxVelocity = owner.agent.agentData.maxVelocity;

                // -----

                // Agent data
                owner.agent.agentData.maxAcceleration *= 1.5f;
                owner.agent.agentData.maxVelocity *= 1.5f;

                /// Activate/Deactivate
                owner.agent.separationData.isActive = false;
                owner.agent.collisionAvoidanceData.isActive = true;
                owner.agent.ActivateFlee();

                break;

            case GoToGameObjectType.GoToAttackDistance: // Seek, half max acceleration and velocity

                Debug.Log("Enter GoToGameObject: GoToAttackDistance");

                maxAcceleration = owner.agent.agentData.maxAcceleration;
                maxVelocity = owner.agent.agentData.maxVelocity;

                // -----

                // Agent data
                owner.agent.agentData.maxAcceleration /= 2.0f;
                owner.agent.agentData.maxVelocity /= 2.0f;

                /// Activate/Deactivate
                owner.agent.separationData.isActive = true;
                owner.agent.collisionAvoidanceData.isActive = false;
                owner.agent.ActivateSeek();

                break;
        }

        if (!owner.agent.SetDestination(Alita.Call.transform.position))
            owner.fsm.ChangeState(new Wander(Wander.WanderType.Wander));
    }

    public override void Execute(CyborgMeleeController owner)
    {
        switch (stateType)
        {
            case GoToGameObjectType.GoToDangerDistance:
                {
                    float distanceToTarget = (Alita.Call.transform.position - owner.transform.position).magnitude;
                    if (distanceToTarget <= owner.character.dangerDistance) // dangerDistance: am I INSIDE my DANGER range?
                    {
                        owner.fsm.ChangeState(new GoToGameObject(Alita.Call.gameObject, GoToGameObjectType.GoToAttackDistance));
                        return;
                    }
                    else if (owner.agent.HasArrived) // HasArrived: has my target run away?
                    {
                        owner.fsm.ChangeState(new Wander(Wander.WanderType.Wander));
                        return;
                    }
                }
                break;

            case GoToGameObjectType.Runaway:
                {
                    float distanceToTarget = (Alita.Call.transform.position - owner.transform.position).magnitude;
                    if (distanceToTarget > owner.character.dangerDistance) // attackDistance: am I OUTSIDE my DANGER range?
                    {
                        owner.fsm.ChangeState(new Wander(Wander.WanderType.Wander));
                        return;
                    }
                }
                break;

            case GoToGameObjectType.GoToAttackDistance:
                {
                    float distanceToTarget = (Alita.Call.transform.position - owner.transform.position).magnitude;
                    if (distanceToTarget <= owner.character.attackDistance) // attackDistance: am I INSIDE my ATTACK range?
                    {
                        // Am I allowed to attack?
                        if (Alita.Call.battleCircle.AddAttacker(owner.gameObject))
                        {
                            // Yes! Attack
                            owner.fsm.ChangeState(new Attack());
                            return;
                        }
                        else
                        {
                            // No! Strafe
                            owner.fsm.ChangeState(new Wander(Wander.WanderType.Strafe));
                            return;
                        }
                    }
                    else if (owner.agent.HasArrived) // HasArrived: has my target run away?
                    {
                        owner.fsm.ChangeState(new Wander(Wander.WanderType.Wander));
                        return;
                    }
                }
                break;
        }
    }

    public override void Exit(CyborgMeleeController owner)
    {
        switch (stateType)
        {
            case GoToGameObjectType.GoToDangerDistance:

                Debug.Log("Exit GoToGameObject: GoToDangerDistance");

                break;

            case GoToGameObjectType.Runaway:

                Debug.Log("Exit GoToGameObject: Runaway");

                owner.agent.agentData.maxAcceleration = maxAcceleration;
                owner.agent.agentData.maxVelocity = maxVelocity;

                owner.agent.separationData.isActive = true;
                owner.agent.collisionAvoidanceData.isActive = true;
                owner.agent.ActivateSeek();

                break;

            case GoToGameObjectType.GoToAttackDistance:

                Debug.Log("Exit GoToGameObject: GoToAttackDistance");

                owner.agent.agentData.maxAcceleration = maxAcceleration;
                owner.agent.agentData.maxVelocity = maxVelocity;

                break;
        }
    }

    public override void DrawGizmos(CyborgMeleeController owner)
    {
        switch (stateType)
        {
            case GoToGameObjectType.GoToDangerDistance:
                Debug.DrawSphere(owner.character.dangerDistance, Color.Blue, owner.transform.position, Quaternion.identity, Vector3.one);
                break;

            case GoToGameObjectType.Runaway:
                Debug.DrawSphere(owner.character.dangerDistance, Color.Blue, owner.transform.position, Quaternion.identity, Vector3.one);
                Debug.DrawSphere(owner.character.attackDistance, Color.Red, owner.transform.position, Quaternion.identity, Vector3.one);
                break;

            case GoToGameObjectType.GoToAttackDistance:
                Debug.DrawSphere(owner.character.attackDistance, Color.Red, owner.transform.position, Quaternion.identity, Vector3.one);
                break;
        }
    }
}
#endregion

#region WANDER
// ----------------------------------------------------------------------------------------------------
// Wander
// ----------------------------------------------------------------------------------------------------

public class Wander : ICyborgMeleeState
{
    public enum WanderType
    {
        Wander,
        Strafe
    }
    private WanderType stateType;

    // -----

    private float maxAcceleration = 0.0f;
    private float maxVelocity = 0.0f;

    // -----

    private float strafeTime = 0.0f;
    private float timer = 0.0f;

    // --------------------------------------------------

    public Wander(WanderType stateType)
    {
        this.stateType = stateType;
    }

    // --------------------------------------------------

    public override void Enter(CyborgMeleeController owner)
    {
        /// Activate/Deactivate
        owner.agent.ActivateWander();
        owner.agent.ActivateAvoidance();

        owner.agent.ClearPath();

        switch (stateType)
        {
            case WanderType.Wander:

                Debug.Log("Enter Wander: Wander");

                // Default agent data

                // Wander data (default)
                owner.agent.wanderData.radius = 1.0f;
                owner.agent.wanderData.offset = 2.0f;

                owner.agent.wanderData.minTime = 1.0f;
                owner.agent.wanderData.maxTime = 2.0f;

                break;

            case WanderType.Strafe:

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

                strafeTime = (float)MathScript.GetRandomDouble(owner.character.strafeMinTime, owner.character.strafeMaxTime);

                break;
        }
    }

    public override void Execute(CyborgMeleeController owner)
    {
        switch (stateType)
        {
            case WanderType.Wander:

                if (owner.lineOfSight.IsTargetSeen // IsTargetSeen: have I seen the target?
                    && owner.CurrentLife > owner.character.minLife) // minLife: do I have enough life to attack the target?
                {
                    owner.fsm.ChangeState(new GoToGameObject(Alita.Call.gameObject, GoToGameObject.GoToGameObjectType.GoToDangerDistance));
                    return;
                }

                break;

            case WanderType.Strafe:

                if (timer >= strafeTime)
                {
                    owner.fsm.ChangeState(new GoToGameObject(Alita.Call.gameObject, GoToGameObject.GoToGameObjectType.GoToDangerDistance)); // danger distance just in case I have moved out of my attack range
                    return;
                }

                timer += Time.deltaTime;

                break;
        }
    }

    public override void Exit(CyborgMeleeController owner)
    {
        switch (stateType)
        {
            case WanderType.Wander:

                Debug.Log("Exit Wander: Wander");

                /// Activate/Deactivate
                owner.agent.ActivateSeek();
                // Avoidance is already activated

                break;

            case WanderType.Strafe:

                Debug.Log("Exit Wander: Strafe");

                owner.agent.agentData.maxAcceleration = maxAcceleration;
                owner.agent.agentData.maxVelocity = maxVelocity;

                // -----

                /// Activate/Deactivate
                owner.agent.ActivateSeek();
                // Avoidance is already activated

                break;
        }
    }

    public override void DrawGizmos(CyborgMeleeController owner)
    {

    }
}
#endregion

#region ATTACK
// ----------------------------------------------------------------------------------------------------
// Attack
// ----------------------------------------------------------------------------------------------------

public class Attack : ICyborgMeleeState
{
    private bool isMovementStopped = false;
    private float maxAngularAcceleration = 0.0f;

    // -----

    private float actualAttackRate = 0.0f;
    private float lastAttackedTime = 0.0f;

    private float AttackCooldown
    {
        get { return Mathf.Max(actualAttackRate - (timer - lastAttackedTime), 0.0f); }
    }

    private float timer = 0.0f;

    // --------------------------------------------------

    public Attack()
    {

    }

    // --------------------------------------------------

    public override void Enter(CyborgMeleeController owner)
    {
        Debug.Log("Enter Attack");

        isMovementStopped = owner.agent.isMovementStopped;
        maxAngularAcceleration = owner.agent.agentData.maxAngularAcceleration;

        // -----

        /// Activate/Deactivate
        owner.agent.alignData.faceData.isActive = true;
        owner.agent.alignData.lookWhereYoureGoingData.isActive = false;

        owner.agent.isMovementStopped = true;

        // Align: Face data
        owner.agent.agentData.maxAngularAcceleration = owner.character.trackMaxAngularAcceleration;
        owner.agent.SetFace(Alita.Call.gameObject);

        // -----

        actualAttackRate = owner.character.attackRate + (float)MathScript.GetRandomDouble(-1.0, 1.0) * owner.character.attackRateFluctuation;
        lastAttackedTime = -actualAttackRate;
    }

    public override void Execute(CyborgMeleeController owner)
    {
        float distanceToTarget = (Alita.Call.gameObject.transform.position - owner.transform.position).magnitude;
        bool contains = Alita.Call.battleCircle.AttackersContains(owner.gameObject);
        if (distanceToTarget > owner.character.attackDistance // attackDistance: has the target moved out of my attack range?
            || !contains) // attackers: am I still an attacker?
        {
            if (contains)
                Alita.Call.battleCircle.RemoveAttacker(owner.gameObject);

            owner.fsm.ChangeState(new GoToGameObject(Alita.Call.gameObject, GoToGameObject.GoToGameObjectType.GoToDangerDistance)); // danger distance just in case I have moved out of my attack range
            return;
        }

        // When attack cooldown is 0.0f...
        if (AttackCooldown <= 0.0f)
        {
            // Am I allowed to hit?
            if (Alita.Call.battleCircle.AddSimultaneousAttacker(owner.gameObject))
            {
                // Yes! Hit
                owner.fsm.ChangeState(new Hit());
                return;
            }
        }

        timer += Time.deltaTime;
    }

    public override void Exit(CyborgMeleeController owner)
    {
        Debug.Log("Exit Attack");

        owner.agent.isMovementStopped = isMovementStopped;
        owner.agent.agentData.maxAngularAcceleration = maxAngularAcceleration;

        // -----

        /// Activate/Deactivate
        owner.agent.alignData.faceData.isActive = false;
        owner.agent.alignData.lookWhereYoureGoingData.isActive = true;
    }

    public override void DrawGizmos(CyborgMeleeController owner)
    {
        Debug.DrawSphere(owner.character.attackDistance, Color.Red, owner.transform.position, Quaternion.identity, Vector3.one);
    }
}
#endregion

#region HIT
// ----------------------------------------------------------------------------------------------------
// Hit
// ----------------------------------------------------------------------------------------------------

public class Hit : ICyborgMeleeState
{
    private bool isMovementStopped = false;
    private float maxAngularAcceleration = 0.0f;

    // -----

    private float timer = 0.0f;

    // --------------------------------------------------

    public Hit()
    {

    }

    // --------------------------------------------------

    public override void Enter(CyborgMeleeController owner)
    {
        Debug.Log("Enter Hit");

        isMovementStopped = owner.agent.isMovementStopped;
        maxAngularAcceleration = owner.agent.agentData.maxAngularAcceleration;

        // -----

        /// Activate/Deactivate
        owner.agent.alignData.faceData.isActive = true;
        owner.agent.alignData.lookWhereYoureGoingData.isActive = false;

        owner.agent.isMovementStopped = true;

        // Align: Face data
        owner.agent.agentData.maxAngularAcceleration = owner.character.trackMaxAngularAcceleration;
        owner.agent.SetFace(Alita.Call.gameObject);
    }

    public override void Execute(CyborgMeleeController owner)
    {
        if (timer >= 3.0f)
        {
            Debug.Log("HIT!");
            Alita.Call.character.currentLife -= owner.character.dmg;

            owner.fsm.ChangeState(new Attack());
            return;
        }

        timer += Time.deltaTime;
    }

    public override void Exit(CyborgMeleeController owner)
    {
        Debug.Log("Exit Hit");

        owner.agent.isMovementStopped = isMovementStopped;
        owner.agent.agentData.maxAngularAcceleration = maxAngularAcceleration;

        // -----

        /// Activate/Deactivate
        owner.agent.alignData.faceData.isActive = false;
        owner.agent.alignData.lookWhereYoureGoingData.isActive = true;

        Alita.Call.battleCircle.RemoveSimultaneousAttacker(owner.gameObject);
    }

    public override void DrawGizmos(CyborgMeleeController owner)
    {
        Debug.DrawSphere(owner.character.attackDistance, Color.Red, owner.transform.position, Quaternion.identity, Vector3.one);
    }
}
#endregion

#region DIE
// ----------------------------------------------------------------------------------------------------
// Die
// ----------------------------------------------------------------------------------------------------

public class Die : ICyborgMeleeState
{
    public Die()
    {

    }

    // --------------------------------------------------

    public override void Enter(CyborgMeleeController owner)
    {
        Debug.Log("Enter Die");
    }

    public override void Execute(CyborgMeleeController owner)
    {
        Debug.Log("Die");
        // if animation has finished...
    }

    public override void Exit(CyborgMeleeController owner)
    {
        Debug.Log("Exit Die");
    }

    public override void DrawGizmos(CyborgMeleeController owner)
    {

    }
}
#endregion