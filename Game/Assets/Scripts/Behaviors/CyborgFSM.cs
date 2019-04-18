using System.Collections;
using System;
using JellyBitEngine;

// https://forum.unity.com/threads/c-proper-state-machine.380612/

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
        GoToAttackDistance
    }
    private GoToGameObjectType stateType;

    // -----

    private GameObject target = null;

    // -----

    private float maxAcceleration = 0.0f;
    private float maxVelocity = 0.0f;

    // --------------------------------------------------

    public GoToGameObject(GameObject target, GoToGameObjectType stateType)
    {
        this.stateType = stateType;

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
            case GoToGameObjectType.GoToDangerDistance:

                Debug.Log("Enter GoToGameObject: GoToDangerDistance");

                /// Activate/Deactivate
                owner.agent.separationData.isActive = false;
                owner.agent.collisionAvoidanceData.isActive = true;

                break;

            case GoToGameObjectType.GoToAttackDistance:

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

                break;
        }

        owner.agent.SetDestination(target.transform.position);
    }

    public override void Execute(CyborgMeleeController owner)
    {
        switch (stateType)
        {
            case GoToGameObjectType.GoToDangerDistance:
                {
                    if (owner.character.life <= owner.character.minLife) // minLife: do I have enough life to attack the target?
                    {
                        // TODO: RUN AWAY
                        owner.fsm.ChangeState(new Wander(Wander.WanderType.Wander));
                        return;
                    }

                    float distanceToTarget = (target.transform.position - owner.transform.position).magnitude;
                    if (distanceToTarget <= owner.character.dangerDistance)
                    {
                        owner.fsm.ChangeState(new GoToGameObject(Alita.Call.gameObject, GoToGameObjectType.GoToAttackDistance));
                        return;
                    }
                }
                break;

            case GoToGameObjectType.GoToAttackDistance:
                {
                    if (owner.character.life <= owner.character.minLife) // minLife: do I have enough life to attack the target?
                    {
                        // TODO: RUN AWAY
                        owner.fsm.ChangeState(new Wander(Wander.WanderType.Wander));
                        return;
                    }

                    float distanceToTarget = (target.transform.position - owner.transform.position).magnitude;
                    if (distanceToTarget <= owner.character.attackDistance)
                    {
                        // Am I allowed to attack?
                        if (Alita.Call.battleCircle.AddAttacker(owner.gameObject))
                        {
                            // Yes! Attack
                            owner.fsm.ChangeState(new Attack(Alita.Call.gameObject));
                            return;
                        }
                        else
                        {
                            // No! Strafe
                            owner.fsm.ChangeState(new Wander(Wander.WanderType.Strafe));
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
            case GoToGameObjectType.GoToDangerDistance:

                Debug.Log("Exit GoToGameObject: GoToDangerDistance");

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

            case GoToGameObjectType.GoToAttackDistance:
                Debug.DrawSphere(owner.character.attackDistance, Color.Green, owner.transform.position, Quaternion.identity, Vector3.one);
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
        switch (stateType)
        {
            case WanderType.Wander:

                Debug.Log("Enter Wander: Wander");

                // Default agent data
                // Default wander data

                /// Activate/Deactivate
                owner.agent.ActivateWander();
                owner.agent.ActivateAvoidance();

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

                /// Activate/Deactivate
                owner.agent.ActivateWander();
                owner.agent.ActivateAvoidance();

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
                    && owner.character.life > owner.character.minLife) // minLife: do I have enough life to attack the target?
                {
                    Debug.Log("From Wander to Danger");
                    owner.fsm.ChangeState(new GoToGameObject(Alita.Call.gameObject, GoToGameObject.GoToGameObjectType.GoToDangerDistance));
                    return;
                }

                break;

            case WanderType.Strafe:

                if (timer >= strafeTime)
                {
                    owner.fsm.ChangeState(new GoToGameObject(Alita.Call.gameObject, GoToGameObject.GoToGameObjectType.GoToDangerDistance));
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

                break;

            case WanderType.Strafe:

                Debug.Log("Exit Wander: Strafe");

                owner.agent.agentData.maxAcceleration = maxAcceleration;
                owner.agent.agentData.maxVelocity = maxVelocity;

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
    private GameObject target = null;

    // -----

    private bool faceDataIsActive = false;
    private bool lookWhereYoureGoingDataIsActive = false;
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

    public Attack(GameObject target)
    {
        this.target = target;
    }

    // --------------------------------------------------

    public override void Enter(CyborgMeleeController owner)
    {
        Debug.Log("Enter Attack");

        faceDataIsActive = owner.agent.alignData.faceData.isActive;
        lookWhereYoureGoingDataIsActive = owner.agent.alignData.lookWhereYoureGoingData.isActive;
        isMovementStopped = owner.agent.isMovementStopped;
        maxAngularAcceleration = owner.agent.agentData.maxAngularAcceleration;

        // -----

        /// Activate/Deactivate
        owner.agent.alignData.faceData.isActive = true;
        owner.agent.alignData.lookWhereYoureGoingData.isActive = false;

        owner.agent.isMovementStopped = true;

        // Align: Face data
        owner.agent.agentData.maxAngularAcceleration = owner.character.trackMaxAngularAcceleration;
        owner.agent.SetFace(target);

        // -----

        actualAttackRate = owner.character.attackRate + (float)MathScript.GetRandomDouble(-1.0, 1.0) * owner.character.attackRateFluctuation;
        lastAttackedTime = -actualAttackRate;
    }

    public override void Execute(CyborgMeleeController owner)
    {
        float distanceToTarget = (target.transform.position - owner.transform.position).magnitude;
        bool contains = Alita.Call.battleCircle.AttackersContains(owner.gameObject);
        if (distanceToTarget > owner.character.attackDistance // attackDistance: has the target moved out of my attack range?
            || owner.character.life <= owner.character.minLife // minLife: do I have enough life to attack the target?
            || !contains) // attackers: am I still an attacker?
        {
            if (contains)
                Alita.Call.battleCircle.RemoveAttacker(owner.gameObject);

            // TODO: RUN AWAY
            owner.fsm.ChangeState(new Wander(Wander.WanderType.Wander));
            return;
        }

        // When attack cooldown is 0.0f...
        if (AttackCooldown <= 0.0f)
        {
            // Am I allowed to hit?
            if (Alita.Call.battleCircle.AddSimultaneousAttacker(owner.gameObject))
            {
                // Yes! Hit
                owner.fsm.ChangeState(new Hit(Alita.Call.gameObject));
                return;
            }
        }

        timer += Time.deltaTime;
    }

    public override void Exit(CyborgMeleeController owner)
    {
        Debug.Log("Exit Attack");

        owner.agent.alignData.faceData.isActive = faceDataIsActive;
        owner.agent.alignData.lookWhereYoureGoingData.isActive = lookWhereYoureGoingDataIsActive;
        owner.agent.isMovementStopped = isMovementStopped;
        owner.agent.agentData.maxAngularAcceleration = maxAngularAcceleration;
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
    private GameObject target = null;

    // -----

    private bool faceDataIsActive = false;
    private bool lookWhereYoureGoingDataIsActive = false;
    private bool isMovementStopped = false;
    private float maxAngularAcceleration = 0.0f;

    // -----

    private float timer = 0.0f;

    // --------------------------------------------------

    public Hit(GameObject target)
    {
        this.target = target;
    }

    // --------------------------------------------------

    public override void Enter(CyborgMeleeController owner)
    {
        Debug.Log("Enter Hit");

        faceDataIsActive = owner.agent.alignData.faceData.isActive;
        lookWhereYoureGoingDataIsActive = owner.agent.alignData.lookWhereYoureGoingData.isActive;
        isMovementStopped = owner.agent.isMovementStopped;
        maxAngularAcceleration = owner.agent.agentData.maxAngularAcceleration;

        // -----

        /// Activate/Deactivate
        owner.agent.alignData.faceData.isActive = true;
        owner.agent.alignData.lookWhereYoureGoingData.isActive = false;

        owner.agent.isMovementStopped = true;

        // Align: Face data
        owner.agent.agentData.maxAngularAcceleration = owner.character.trackMaxAngularAcceleration;
        owner.agent.SetFace(target);
    }

    public override void Execute(CyborgMeleeController owner)
    {
        if (timer >= 3.0f)
        {
            Debug.Log("HIT!");
            owner.fsm.ChangeState(new Attack(Alita.Call.gameObject));
            return;
        }

        timer += Time.deltaTime;
    }

    public override void Exit(CyborgMeleeController owner)
    {
        Debug.Log("Exit Hit");

        owner.agent.alignData.faceData.isActive = faceDataIsActive;
        owner.agent.alignData.lookWhereYoureGoingData.isActive = lookWhereYoureGoingDataIsActive;
        owner.agent.isMovementStopped = isMovementStopped;
        owner.agent.agentData.maxAngularAcceleration = maxAngularAcceleration;

        // -----

        Alita.Call.battleCircle.RemoveSimultaneousAttacker(owner.gameObject);
    }

    public override void DrawGizmos(CyborgMeleeController owner)
    {
        
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