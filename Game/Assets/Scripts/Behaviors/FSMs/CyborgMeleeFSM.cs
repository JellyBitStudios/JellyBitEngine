using JellyBitEngine;

// https://forum.unity.com/threads/c-proper-state-machine.380612/
// >, <=

public abstract class CM_IState
{
    public string name;

    public abstract void Enter(CyborgMeleeController owner);
    public abstract void Execute(CyborgMeleeController owner);
    public abstract void Exit(CyborgMeleeController owner);
    public abstract void DrawGizmos(CyborgMeleeController owner);
}

public class CyborgMeleeFSM
{
    private CM_IState state = null;
    private CyborgMeleeController owner = null;

    public CyborgMeleeFSM(CyborgMeleeController owner)
    {
        this.owner = owner;
    }

    public CM_IState GetState()
    {
        return state;
    }

    public void ChangeState(CM_IState state)
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

#region CM_GoToGameObject
// ----------------------------------------------------------------------------------------------------
// GoToGameObject
// ----------------------------------------------------------------------------------------------------

public class CM_GoToGameObject : CM_IState
{
    public CM_GoToGameObject()
    {
        name = "GoToGameObject";
    }

    public override void Enter(CyborgMeleeController owner)
    {
        //Debug.Log(owner.character.name + ": " + "ENTER" + " " + name);

        // ----- Agent -----

        owner.agent.isMovementStopped = false;
        owner.agent.isRotationStopped = false;

        // ----- CM_GoToGameObject -----

        if (!owner.agent.SetDestination(Alita.Call.transform.position))
        {
            owner.fsm.ChangeState(new CM_WanderDefault());
            return;
        }

        owner.animator.PlayAnimation("melee_run_cyborg_animation");
        owner.animator.SetAnimationLoop(true);
    }

    public override void Execute(CyborgMeleeController owner) { }

    public override void Exit(CyborgMeleeController owner)
    {
        //Debug.Log(owner.character.name + ": " + "EXIT" + " " + name);
    }

    public override void DrawGizmos(CyborgMeleeController owner) { }
}

public class CM_GoToDangerDistance : CM_GoToGameObject
{
    // ----- Save&Load -----
    private float maxAcceleration = 0.0f;
    private float maxVelocity = 0.0f;

    // --------------------------------------------------

    public CM_GoToDangerDistance()
    {
        name = "GoToDangerDistance";
    }

    public override void Enter(CyborgMeleeController owner)
    {
        Debug.Log(owner.cbg_Entity.name + ": " + "ENTER" + " " + name);

        // ----- Save -----

        maxAcceleration = owner.agent.agentData.maxAcceleration;
        maxVelocity = owner.agent.agentData.maxVelocity;

        // ----- Agent -----

        /// Activate/Deactivate
        owner.agent.separationData.isActive = false;
        owner.agent.collisionAvoidanceData.isActive = true;
        owner.agent.ActivateSeek();

        // Agent data
        owner.agent.agentData.maxAcceleration *= 2.0f;
        owner.agent.agentData.maxVelocity *= 2.0f;

        // ----- Base -----

        base.Enter(owner);
    }

    public override void Execute(CyborgMeleeController owner)
    {
        float distanceToTarget = (Alita.Call.transform.position - owner.transform.position).magnitude;

        if (!owner.sight.IsTargetSeen // IsTargetSeen: have I seen my target?
            || owner.agent.HasArrived) // HasArrived: has my target run away?
        {
            owner.fsm.ChangeState(new CM_Wander());
            return;
        }
        else if (distanceToTarget <= owner.cbg_Entity.dangerDistance) // dangerDistance: am I INSIDE my DANGER range?
        {
            owner.fsm.ChangeState(new CM_GoToAttackDistance());
            return;
        }
    }

    public override void Exit(CyborgMeleeController owner)
    {
        Debug.Log(owner.cbg_Entity.name + ": " + "EXIT" + " " + name);

        // ----- Load -----

        owner.agent.agentData.maxAcceleration = maxAcceleration;
        owner.agent.agentData.maxVelocity = maxVelocity;
    }

    public override void DrawGizmos(CyborgMeleeController owner)
    {
        Debug.DrawSphere(owner.cbg_Entity.dangerDistance, Color.Blue, owner.transform.position, Quaternion.identity, Vector3.one);
    }
}

public class CM_GoToAttackDistance : CM_GoToGameObject
{
    public CM_GoToAttackDistance()
    {
        name = "GoToAttackDistance";
    }

    public override void Enter(CyborgMeleeController owner)
    {
        Debug.Log(owner.cbg_Entity.name + ": " + "ENTER" + " " + name);

        // ----- Agent -----

        /// Activate/Deactivate
        owner.agent.separationData.isActive = true;
        owner.agent.collisionAvoidanceData.isActive = false;
        owner.agent.ActivateSeek();

        // ----- Base -----

        base.Enter(owner);
    }

    public override void Execute(CyborgMeleeController owner)
    {
        float distanceToTarget = (Alita.Call.transform.position - owner.transform.position).magnitude;

        if (!owner.sight.IsTargetSeen // IsTargetSeen: have I seen my target?
            || owner.agent.HasArrived) // HasArrived: has my target run away?
        {
            owner.fsm.ChangeState(new CM_WanderDefault());
            return;
        }
        else if (distanceToTarget <= owner.cbg_Entity.attackDistance) // attackDistance: am I INSIDE my ATTACK range?
        {
            // Am I allowed to attack?
            if (Alita.Call.battleCircle.AddAttacker(owner.gameObject))
            {
                // Yes! Attack
                owner.fsm.ChangeState(new CM_Attack());
                return;
            }
            else
            {
                // No! Strafe
                owner.fsm.ChangeState(new CM_WanderStrafe());
                return;
            }
        }
    }

    public override void Exit(CyborgMeleeController owner)
    {
        Debug.Log(owner.cbg_Entity.name + ": " + "EXIT" + " " + name);
    }

    public override void DrawGizmos(CyborgMeleeController owner)
    {
        Debug.DrawSphere(owner.cbg_Entity.attackDistance, Color.Red, owner.transform.position, Quaternion.identity, Vector3.one);
    }
}
#endregion

#region CM_Wander
// ----------------------------------------------------------------------------------------------------
// Wander
// ----------------------------------------------------------------------------------------------------

public class CM_Wander : CM_IState
{
    public CM_Wander()
    {
        name = "Wander";
    }

    public override void Enter(CyborgMeleeController owner)
    {
        //Debug.Log(owner.character.name + ": " + "ENTER" + " " + name);

        // ----- Agent -----

        owner.agent.ClearPath();

        /// Activate/Deactivate
        owner.agent.ActivateWander();
        owner.agent.ActivateAvoidance();

        // ----- CM_Wander -----

        owner.animator.PlayAnimation("melee_run_cyborg_animation");
        owner.animator.SetAnimationLoop(true);
    }

    public override void Execute(CyborgMeleeController owner) { }

    public override void Exit(CyborgMeleeController owner)
    {
        //Debug.Log(owner.character.name + ": " + "ENTER" + " " + name);

        // ----- Agent -----

        /// Activate/Deactivate
        owner.agent.ActivateSeek();
        // Avoidance is already activated
    }

    public override void DrawGizmos(CyborgMeleeController owner) { }
}

public class CM_WanderDefault : CM_Wander
{
    public CM_WanderDefault()
    {
        name = "WanderDefault";
    }

    public override void Enter(CyborgMeleeController owner)
    {
        Debug.Log(owner.cbg_Entity.name + ": " + "ENTER" + " " + name);

        // ----- Agent -----

        // Wander data
        owner.agent.wanderData.radius = 1.0f;
        owner.agent.wanderData.offset = 1.0f;

        owner.agent.wanderData.minTime = 0.3f;
        owner.agent.wanderData.maxTime = 0.7f;

        // ----- Base -----

        base.Enter(owner);
    }

    public override void Execute(CyborgMeleeController owner)
    {
        if (owner.sight.IsTargetSeen) // IsTargetSeen: have I seen my target?
        {
            owner.fsm.ChangeState(new CM_GoToDangerDistance());
            return;
        }
    }

    public override void Exit(CyborgMeleeController owner)
    {
        Debug.Log(owner.cbg_Entity.name + ": " + "EXIT" + " " + name);

        // ----- Base -----

        base.Exit(owner);
    }

    public override void DrawGizmos(CyborgMeleeController owner) { }
}

public class CM_WanderStrafe : CM_Wander
{
    // ----- Save&Load -----
    private float maxAcceleration = 0.0f;
    private float maxVelocity = 0.0f;

    // ----- CM_WanderStrafe -----
    private float actualStrafeTime = 0.0f;
    private float timer = 0.0f;

    // --------------------------------------------------

    public CM_WanderStrafe()
    {
        name = "WanderStrafe";
    }

    public override void Enter(CyborgMeleeController owner)
    {
        Debug.Log(owner.cbg_Entity.name + ": " + "ENTER" + " " + name);

        // ----- Save -----

        maxAcceleration = owner.agent.agentData.maxAcceleration;
        maxVelocity = owner.agent.agentData.maxVelocity;

        // ----- Agent -----

        // Agent data
        owner.agent.agentData.maxAcceleration /= 2.0f;
        owner.agent.agentData.maxVelocity /= 2.0f;

        // Wander data
        owner.agent.wanderData.radius = 1.0f;
        owner.agent.wanderData.offset = 0.5f;

        owner.agent.wanderData.minTime = 0.3f;
        owner.agent.wanderData.maxTime = 0.7f;

        // ----- CM_WanderStrafe -----

        actualStrafeTime = (float)MathScript.GetRandomDouble(owner.cbg_Entity.strafeMinTime, owner.cbg_Entity.strafeMaxTime);

        // ----- Base -----

        base.Enter(owner);
    }

    public override void Execute(CyborgMeleeController owner)
    {
        if (timer >= actualStrafeTime
            || owner.isBeingAttacked)
        {
            owner.fsm.ChangeState(new CM_GoToDangerDistance()); // danger distance just in case I have moved out of my attack range
            return;
        }

        timer += Time.deltaTime;
    }

    public override void Exit(CyborgMeleeController owner)
    {
        Debug.Log(owner.cbg_Entity.name + ": " + "EXIT" + " " + name);

        // ----- Load -----

        owner.agent.agentData.maxAcceleration = maxAcceleration;
        owner.agent.agentData.maxVelocity = maxVelocity;

        // ----- Base -----

        base.Exit(owner);
    }

    public override void DrawGizmos(CyborgMeleeController owner) { }
}
#endregion

#region CM_Attack
// ----------------------------------------------------------------------------------------------------
// Attack
// ----------------------------------------------------------------------------------------------------

public class CM_Attack : CM_IState
{
    // ----- Save&Load -----
    private float maxAngularAcceleration = 0.0f;
    private float maxAngularVelocity = 0.0f;

    // ----- CM_Attack -----
    private float actualAttackRate = 0.0f;
    private float lastAttackedTime = 0.0f;
    private float timer = 0.0f;

    private float AttackCooldown
    {
        get { return Mathf.Max(actualAttackRate - (timer - lastAttackedTime), 0.0f); }
    }

    // --------------------------------------------------

    public CM_Attack()
    {
        name = "Attack";
    }

    public override void Enter(CyborgMeleeController owner)
    {
        Debug.Log(owner.cbg_Entity.name + ": " + "ENTER" + " " + name);

        // ----- Save -----

        maxAngularAcceleration = owner.agent.agentData.maxAngularAcceleration;
        maxAngularVelocity = owner.agent.agentData.maxAngularVelocity;

        // ----- Agent -----

        /// Activate/Deactivate
        owner.agent.isMovementStopped = true;
        owner.agent.isRotationStopped = false;

        // Align: Face data
        owner.agent.agentData.maxAngularAcceleration /= 2.0f;
        owner.agent.agentData.maxAngularVelocity /= 2.0f;

        // ----- CM_Attack -----

        owner.agent.SetFace(Alita.Call.gameObject);

        actualAttackRate = owner.cbg_Entity.attackRate + (float)MathScript.GetRandomDouble(-1.0, 1.0) * owner.cbg_Entity.attackRateFluctuation;
        lastAttackedTime = -actualAttackRate;

        owner.animator.PlayAnimation("melee_iddle_attack_cyborg_animation");
        owner.animator.SetAnimationLoop(true);
    }

    public override void Execute(CyborgMeleeController owner)
    {
        float distanceToTarget = (Alita.Call.gameObject.transform.position - owner.transform.position).magnitude;
        bool contains = Alita.Call.battleCircle.AttackersContains(owner.gameObject);
        if (distanceToTarget > owner.cbg_Entity.attackDistance // attackDistance: has the target moved out of my attack range?
            || !contains) // Am I still an attacker?
        {
            if (Alita.Call.battleCircle.SimultaneousAttackersContains(owner.gameObject))
                Alita.Call.battleCircle.RemoveSimultaneousAttacker(owner.gameObject);
            if (contains)
                Alita.Call.battleCircle.RemoveAttacker(owner.gameObject);

            owner.fsm.ChangeState(new CM_GoToDangerDistance());
            return;
        }

        if (owner.agent.HasFaced
           && AttackCooldown <= 0.0f)
        {
            // Am I allowed to hit?
            if (Alita.Call.battleCircle.AddSimultaneousAttacker(owner.gameObject))
            {
                // Yes! Hit
                owner.fsm.ChangeState(new CM_Hit());
                return;
            }
        }

        timer += Time.deltaTime;
    }

    public override void Exit(CyborgMeleeController owner)
    {
        Debug.Log(owner.cbg_Entity.name + ": " + "EXIT" + " " + name);

        // ----- Load -----

        owner.agent.agentData.maxAngularAcceleration = maxAngularAcceleration;
        owner.agent.agentData.maxAngularVelocity = maxAngularVelocity;

        // ----- Agent -----

        /// Activate/Deactivate
        owner.agent.isMovementStopped = false;

        // ----- CM_Attack -----

        owner.agent.FinishFace();
    }

    public override void DrawGizmos(CyborgMeleeController owner)
    {
        Debug.DrawSphere(owner.cbg_Entity.attackDistance, Color.Red, owner.transform.position, Quaternion.identity, Vector3.one);
    }
}
#endregion

#region CM_Hit
// ----------------------------------------------------------------------------------------------------
// Hit
// ----------------------------------------------------------------------------------------------------

public class CM_Hit : CM_IState
{
    // ----- CM_Hit -----
    private bool animationHit = false;

    // --------------------------------------------------

    public CM_Hit()
    {
        name = "Hit";
    }

    public override void Enter(CyborgMeleeController owner)
    {
        Debug.Log(owner.cbg_Entity.name + ": " + "ENTER" + " " + name);

        // ----- Agent -----

        /// Activate/Deactivate
        owner.agent.Stop();

        // ----- CM_Hit -----

        owner.animator.PlayAnimation("melee_attack_cyborg_animation");
        owner.animator.SetAnimationLoop(false);
    }

    public override void Execute(CyborgMeleeController owner)
    {
        if (owner.animator.GetCurrentFrame() >= 25
            && !animationHit)
        {
            Alita.Call.character.currentLife -= (int)owner.cbg_Entity.dmg;

            animationHit = true;
        }
        else if (owner.animator.AnimationFinished())
        {
            owner.fsm.ChangeState(new CM_Attack());
            return;
        }
    }

    public override void Exit(CyborgMeleeController owner)
    {
        Debug.Log(owner.cbg_Entity.name + ": " + "EXIT" + " " + name);

        // ----- Agent -----

        /// Activate/Deactivate
        owner.agent.Resume();

        // ----- CM_Hit -----

        if (Alita.Call.battleCircle.SimultaneousAttackersContains(owner.gameObject)
            && Alita.Call.currentTarget != owner.gameObject) // Am I the current target?
            Alita.Call.battleCircle.RemoveSimultaneousAttacker(owner.gameObject);
    }

    public override void DrawGizmos(CyborgMeleeController owner)
    {
        Debug.DrawSphere(owner.cbg_Entity.attackDistance, Color.Red, owner.transform.position, Quaternion.identity, Vector3.one);
    }
}
#endregion

#region CM_Stun
// ----------------------------------------------------------------------------------------------------
// Stun
// ----------------------------------------------------------------------------------------------------

public class CM_Stun : CM_IState
{
    // ----- CM_Stun -----
    public CM_IState lastState = null;

    // --------------------------------------------------

    public CM_Stun(CM_IState lastState)
    {
        name = "Stun";

        // -----

        this.lastState = lastState;
    }

    public override void Enter(CyborgMeleeController owner)
    {
        Debug.Log(owner.cbg_Entity.name + ": " + "ENTER" + " " + name);

        // ----- CM_Stun -----

        owner.animator.PlayAnimation("melee_hurt_cyborg_animation");
        owner.animator.SetAnimationLoop(false);

        owner.isStunned = true;
    }

    public override void Execute(CyborgMeleeController owner) { }

    public override void Exit(CyborgMeleeController owner)
    {
        Debug.Log(owner.cbg_Entity.name + ": " + "EXIT" + " " + name);

        // ----- CM_Stun -----

        owner.isStunned = false;
    }

    public override void DrawGizmos(CyborgMeleeController owner) { }
}

public class CM_StunBasic : CM_Stun
{
    public CM_StunBasic(CM_IState lastState) : base(lastState)
    {
        name = "StunBasic";
    }

    public override void Enter(CyborgMeleeController owner)
    {
        Debug.Log(owner.cbg_Entity.name + ": " + "ENTER" + " " + name);

        // ----- Agent -----

        /// Activate/Deactivate
        owner.agent.Stop();

        // ----- Base -----

        base.Enter(owner);
    }

    public override void Execute(CyborgMeleeController owner)
    {
        if (owner.animator.AnimationFinished())
        {
            owner.fsm.ChangeState(lastState);
            return;
        }
    }

    public override void Exit(CyborgMeleeController owner)
    {
        Debug.Log(owner.cbg_Entity.name + ": " + "EXIT" + " " + name);

        // ----- Agent -----

        /// Activate/Deactivate
        owner.agent.Resume();

        // ----- Base -----

        base.Exit(owner);
    }

    public override void DrawGizmos(CyborgMeleeController owner) { }
}

public class CM_StunForce : CM_Stun
{
    // ----- Save&Load -----
    private float maxAcceleration = 0.0f;
    private float maxVelocity = 0.0f;

    // ----- CM_StunForce -----
    private float timer = 0.0f;

    // --------------------------------------------------

    public CM_StunForce(CM_IState lastState) : base(lastState)
    {
        name = "StunForce";
    }

    public override void Enter(CyborgMeleeController owner)
    {
        Debug.Log(owner.cbg_Entity.name + ": " + "ENTER" + " " + name);

        // ----- Save -----

        maxAcceleration = owner.agent.agentData.maxAcceleration;
        maxVelocity = owner.agent.agentData.maxVelocity;

        // ----- Agent -----

        /// Activate/Deactivate
        owner.agent.isMovementStopped = false;
        owner.agent.isRotationStopped = true;

        owner.agent.agentData.maxAcceleration *= 4.0f;
        owner.agent.agentData.maxVelocity *= 4.0f;

        // ----- CM_StunForce -----

        owner.agent.direction = (owner.transform.position - Alita.Call.transform.position).normalized();
        owner.agent.useDirection = true;

        // ----- Base -----

        base.Enter(owner);
    }

    public override void Execute(CyborgMeleeController owner)
    {
        if (timer >= owner.cbg_Entity.stunTime)
        {
            owner.agent.isMovementStopped = true;

            if (owner.animator.AnimationFinished())
            {
                owner.fsm.ChangeState(lastState);
                return;
            }
        }

        timer += Time.deltaTime;
    }

    public override void Exit(CyborgMeleeController owner)
    {
        Debug.Log(owner.cbg_Entity.name + ": " + "EXIT" + " " + name);

        // ----- Load -----

        owner.agent.agentData.maxAcceleration = maxAcceleration;
        owner.agent.agentData.maxVelocity = maxVelocity;

        // ----- Agent -----

        /// Activate/Deactivate
        owner.agent.isMovementStopped = false;
        owner.agent.isRotationStopped = false;

        // ----- CM_StunForce -----

        owner.agent.useDirection = false;

        // ----- Base -----

        base.Exit(owner);
    }

    public override void DrawGizmos(CyborgMeleeController owner) { }
}
#endregion

#region CM_Die
// ----------------------------------------------------------------------------------------------------
// Die
// ----------------------------------------------------------------------------------------------------

public class CM_Die : CM_IState
{
    public CM_Die()
    {
        name = "Die";
    }

    public override void Enter(CyborgMeleeController owner)
    {
        Debug.Log(owner.cbg_Entity.name + ": " + "ENTER" + " " + name);

        // ----- Agent -----

        /// Activate/Deactivate
        owner.agent.Stop();
        owner.agent.ClearPath();
        owner.agent.ClearMovementAndRotation();
    }

    public override void Execute(CyborgMeleeController owner)
    {
        EnemyEvent myEvent = new EnemyEvent();
        myEvent.type = Event_Type.EnemyDie;
        myEvent.gameObject = owner.gameObject;
        EventsManager.Call.PushEvent(myEvent);
    }

    public override void Exit(CyborgMeleeController owner)
    {
        Debug.Log(owner.cbg_Entity.name + ": " + "EXIT" + " " + name);
    }

    public override void DrawGizmos(CyborgMeleeController owner) { }
}
#endregion