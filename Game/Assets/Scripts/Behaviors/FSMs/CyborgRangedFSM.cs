using System.Collections;
using System;
using JellyBitEngine;

// https://forum.unity.com/threads/c-proper-state-machine.380612/
// >, <=

public abstract class CR_IState
{
    public string name;

    public abstract void Enter(CyborgRangedController owner);
    public abstract void Execute(CyborgRangedController owner);
    public abstract void Exit(CyborgRangedController owner);
    public abstract void DrawGizmos(CyborgRangedController owner);
}

public class CyborgRangedFSM
{
    private CR_IState state = null;
    private CyborgRangedController owner = null;

    public CyborgRangedFSM(CyborgRangedController owner)
    {
        this.owner = owner;
    }

    public void ChangeState(CR_IState state)
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

#region CR_GoToGameObject
// ----------------------------------------------------------------------------------------------------
// CR_GoToGameObject
// ----------------------------------------------------------------------------------------------------

public class CR_GoToGameObject : CR_IState
{
    public CR_GoToGameObject()
    {
        name = "GoToGameObject";
    }

    public override void Enter(CyborgRangedController owner)
    {
        //Debug.Log(owner.character.name + ": " + "ENTER" + " " + name);

        // ----- CR_GoToGameObject -----

        if (!owner.agent.SetDestination(Alita.Call.transform.position))
            owner.fsm.ChangeState(new CR_Wander());
    }

    public override void Execute(CyborgRangedController owner)
    {

    }

    public override void Exit(CyborgRangedController owner)
    {
        //Debug.Log(owner.character.name + ": " + "EXIT" + " " + name);
    }

    public override void DrawGizmos(CyborgRangedController owner)
    {
        
    }
}

public class CR_GoToAttackDistance : CR_GoToGameObject
{
    // ----- Save&Load -----
    private float maxAcceleration = 0.0f;
    private float maxVelocity = 0.0f;

    // --------------------------------------------------

    public CR_GoToAttackDistance()
    {
        name = "GoToAttackDistance";
    }

    public override void Enter(CyborgRangedController owner)
    {
        Debug.Log(owner.character.name + ": " + "ENTER" + " " + name);

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

    public override void Execute(CyborgRangedController owner)
    {
        float distanceToTarget = (Alita.Call.transform.position - owner.transform.position).magnitude;
        if (distanceToTarget <= owner.character.attackDistance) // attackDistance: am I INSIDE my ATTACK range?
        {
            // I am always allowed to attack ;)
            owner.fsm.ChangeState(new CR_Attack());
            return;
        }
        else if (owner.agent.HasArrived) // HasArrived: has my target run away?
        {
            owner.fsm.ChangeState(new CR_Wander());
            return;
        }
    }

    public override void Exit(CyborgRangedController owner)
    {
        Debug.Log(owner.character.name + ": " + "EXIT" + " " + name);

        // ----- Load -----

        owner.agent.agentData.maxAcceleration = maxAcceleration;
        owner.agent.agentData.maxVelocity = maxVelocity;
    }

    public override void DrawGizmos(CyborgRangedController owner)
    {
        Debug.DrawSphere(owner.character.attackDistance, Color.Red, owner.transform.position, Quaternion.identity, Vector3.one);
    }
}

public class CR_GoToSide : CR_GoToGameObject
{
    public CR_GoToSide()
    {
        name = "GoToSide";
    }

    public override void Enter(CyborgRangedController owner)
    {
        Debug.Log(owner.character.name + ": " + "ENTER" + " " + name);

        // ----- Agent -----

        /// Activate/Deactivate
        owner.agent.separationData.isActive = true;
        owner.agent.collisionAvoidanceData.isActive = false;
        owner.agent.ActivateSeek();

        // ----- CR_GoToSide -----

        owner.agent.SetFace(Alita.Call.gameObject);

        float distanceToTarget = (Alita.Call.gameObject.transform.position - owner.transform.position).magnitude;
        Vector3 direction = new Vector3((float)MathScript.GetRandomDouble(-1.0f, 1.0f), 0.0f, (float)MathScript.GetRandomDouble(-1.0f, 1.0f));
        Vector3 position = Alita.Call.transform.position + direction * distanceToTarget;

        if (!owner.agent.SetDestination(position))
            owner.fsm.ChangeState(new CR_Attack());
    }

    public override void Execute(CyborgRangedController owner)
    {
        if (owner.agent.HasArrived)
        {
            owner.fsm.ChangeState(new CR_Attack());
            return;
        }
    }

    public override void Exit(CyborgRangedController owner)
    {
        Debug.Log(owner.character.name + ": " + "EXIT" + " " + name);

        // ----- CR_GoToSide -----

        owner.agent.FinishFace();
    }

    public override void DrawGizmos(CyborgRangedController owner)
    {
        Debug.DrawSphere(owner.character.attackDistance, Color.Red, owner.transform.position, Quaternion.identity, Vector3.one);
    }
}
#endregion

#region CR_Wander
// ----------------------------------------------------------------------------------------------------
// CR_Wander
// ----------------------------------------------------------------------------------------------------

public class CR_Wander : CR_IState
{
    // ----- Save&Load -----
    private float maxAcceleration = 0.0f;
    private float maxVelocity = 0.0f;

    // ----- CR_WANDER -----
    private float wanderTime = 0.0f;
    private float timer = 0.0f;

    // --------------------------------------------------

    public CR_Wander()
    {
        name = "Wander";
    }

    public override void Enter(CyborgRangedController owner)
    {
        Debug.Log(owner.character.name + ": " + "ENTER" + " " + name);

        // ----- Save -----

        maxAcceleration = owner.agent.agentData.maxAcceleration;
        maxVelocity = owner.agent.agentData.maxVelocity;

        // ----- Agent -----

        owner.agent.ClearPath();

        /// Activate/Deactivate
        owner.agent.ActivateWander();
        owner.agent.ActivateAvoidance();

        // Agent data
        owner.agent.agentData.maxAcceleration /= 2.0f;
        owner.agent.agentData.maxVelocity /= 2.0f;

        // Wander data
        owner.agent.wanderData.radius = 1.0f;
        owner.agent.wanderData.offset = 2.0f;

        owner.agent.wanderData.minTime = 0.3f;
        owner.agent.wanderData.maxTime = 0.7f;

        // ----- CR_Wander -----

        wanderTime = (float)MathScript.GetRandomDouble(owner.character.wanderMinTime, owner.character.wanderMaxTime);
    }

    public override void Execute(CyborgRangedController owner)
    {
        if (owner.sight.IsTargetSeen // IsTargetSeen: have I seen the target?
            && owner.CurrentLife > owner.character.minLife) // minLife: do I have enough life to attack the target?
        {
            //owner.fsm.ChangeState(new CM_GoToGameObject(CM_GoToGameObject.GoToGameObjectType.GoToDangerDistance));
            return;
        }
        else if (timer >= wanderTime)
        {
            owner.fsm.ChangeState(new CR_LookAround());
            return;
        }

        timer += Time.deltaTime;
    }

    public override void Exit(CyborgRangedController owner)
    {
        Debug.Log(owner.character.name + ": " + "EXIT" + " " + name);

        // ----- Load -----

        owner.agent.agentData.maxAcceleration = maxAcceleration;
        owner.agent.agentData.maxVelocity = maxVelocity;

        // ----- Agent -----

        /// Activate/Deactivate
        owner.agent.ActivateSeek();
        // Avoidance is already activated
    }

    public override void DrawGizmos(CyborgRangedController owner)
    {

    }
}
#endregion

#region CR_LookAround
// ----------------------------------------------------------------------------------------------------
// CR_LookAround
// ----------------------------------------------------------------------------------------------------

public class CR_LookAround : CR_IState
{
    // ----- CR_LookAround -----
    private uint lookAroundTimes = 0;
    private float lookAroundTime = 0.0f;

    private uint count = 0;
    private float timer = 0.0f;

    // --------------------------------------------------

    public CR_LookAround()
    {
        name = "LookAround";
    }

    public override void Enter(CyborgRangedController owner)
    {
        Debug.Log(owner.character.name + ": " + "ENTER" + " " + name);

        // ----- Agent -----

        owner.agent.isMovementStopped = true;

        // ----- CR_LookAround -----

        lookAroundTimes = (uint)MathScript.GetRandomInteger((int)owner.character.lookAroundMinTimes, (int)owner.character.lookAroundMaxTimes);
        RecalculateTime(owner);

        owner.agent.SetFace(GetDirection(owner));
    }

    public override void Execute(CyborgRangedController owner)
    {
        if (owner.sight.IsTargetSeen // IsTargetSeen: have I seen the target?
            && owner.CurrentLife > owner.character.minLife) // minLife: do I have enough life to attack the target?
        {
            owner.fsm.ChangeState(new CR_GoToAttackDistance());
            return;
        }
        else if (count == lookAroundTimes)
        {
            owner.fsm.ChangeState(new CR_Wander());
            return;
        }
        else if (owner.agent.HasFaced &&
            timer >= lookAroundTime)
        {
            timer = 0.0f;
            ++count;

            RecalculateTime(owner);

            owner.agent.SetFace(GetDirection(owner));
        }

        timer += Time.deltaTime;
    }

    public override void Exit(CyborgRangedController owner)
    {
        Debug.Log(owner.character.name + ": " + "EXIT" + " " + name);

        // ----- Agent -----

        owner.agent.isMovementStopped = false;

        // ----- CR_LookAround -----

        owner.agent.FinishFace();
    }

    public override void DrawGizmos(CyborgRangedController owner)
    {

    }

    // --------------------------------------------------

    private void RecalculateTime(CyborgRangedController owner)
    {
        lookAroundTime = (float)MathScript.GetRandomDouble(owner.character.lookAroundMinTime, owner.character.lookAroundMaxTime);
    }

    private Vector3 GetDirection(CyborgRangedController owner)
    {      
        float angle = MathScript.GetRandomSign() * (float)MathScript.GetRandomDouble(owner.character.lookAroundMinAngle, owner.character.lookAroundMaxAngle);
        return Quaternion.Rotate(Vector3.up, angle) * owner.transform.forward;
    }
}
#endregion

#region CR_Attack
// ----------------------------------------------------------------------------------------------------
// CR_Attack
// ----------------------------------------------------------------------------------------------------

public class CR_Attack : CR_IState
{
    // ----- Save&Load -----
    private float maxAngularAcceleration = 0.0f;
    private float maxAngularVelocity = 0.0f;

    // ----- CR_Attack -----
    float[] color = Color.Green;    

    // --------------------------------------------------

    public CR_Attack()
    {
        name = "Attack";
    }

    public override void Enter(CyborgRangedController owner)
    {
        Debug.Log(owner.character.name + ": " + "ENTER" + " " + name);

        // ----- Save -----

        maxAngularAcceleration = owner.agent.agentData.maxAngularAcceleration;
        maxAngularVelocity = owner.agent.agentData.maxAngularVelocity;

        // ----- Agent -----

        owner.agent.isMovementStopped = true;

        // Align: Face data
        owner.agent.agentData.maxAngularAcceleration = owner.character.trackMaxAngularAcceleration;
        owner.agent.agentData.maxAngularVelocity = owner.character.trackMaxAngularVelocity;

        // ----- CR_Attack -----

        owner.agent.SetFace(Alita.Call.gameObject);
    }

    public override void Execute(CyborgRangedController owner)
    {
        float distanceToTarget = (Alita.Call.gameObject.transform.position - owner.transform.position).magnitude;
        if (distanceToTarget > owner.character.attackDistance) // attackDistance: has the target moved out of my attack range?
        {
            owner.fsm.ChangeState(new CR_Wander());
            return;
        }
        else if (MathScript.GetRandomDouble(0.0f, 1.0f) == 1)
        {
            color = Color.Red;

            owner.fsm.ChangeState(new CR_GoToSide());
            return;
        }
        else
        {
            // I am always allowed to hit ;)
            owner.fsm.ChangeState(new CR_Hit());
            return;
        }
    }

    public override void Exit(CyborgRangedController owner)
    {
        Debug.Log(owner.character.name + ": " + "EXIT" + " " + name);

        // ----- Load -----

        owner.agent.agentData.maxAngularAcceleration = maxAngularAcceleration;
        owner.agent.agentData.maxAngularVelocity = maxAngularVelocity;

        // ----- Agent -----

        owner.agent.isMovementStopped = false;

        // ----- CR_Attack -----

        owner.agent.FinishFace();
    }

    public override void DrawGizmos(CyborgRangedController owner)
    {
        Debug.DrawLine(owner.transform.position, Alita.Call.transform.position, color);
    }
}
#endregion

#region CR_Hit
// ----------------------------------------------------------------------------------------------------
// CR_Hit
// ----------------------------------------------------------------------------------------------------

public class CR_Hit : CR_IState
{
    // ----- Save&Load -----
    private float maxAngularAcceleration = 0.0f;
    private float maxAngularVelocity = 0.0f;

    // ----- CR_Hit -----
    private uint hitTimes = 0;
    private float hitRate = 0.0f;
    private float lastHitTime = 0.0f;

    public uint count = 0;
    private float timer = 0.0f;

    private float HitCooldown
    {
        get { return Mathf.Max(hitRate - (timer - lastHitTime), 0.0f); }
    }

    // --------------------------------------------------

    public CR_Hit()
    {
        name = "Hit";
    }

    public override void Enter(CyborgRangedController owner)
    {
        Debug.Log(owner.character.name + ": " + "ENTER" + " " + name);

        // ----- Save -----

        maxAngularAcceleration = owner.agent.agentData.maxAngularAcceleration;
        maxAngularVelocity = owner.agent.agentData.maxAngularVelocity;

        // ----- Agent -----

        owner.agent.isMovementStopped = true;

        // Align: Face data
        owner.agent.agentData.maxAngularAcceleration = owner.character.trackMaxAngularAcceleration * 2.0f;
        owner.agent.agentData.maxAngularVelocity = owner.character.trackMaxAngularVelocity * 2.0f;

        // ----- CR_Hit -----

        owner.agent.SetFace(Alita.Call.gameObject);

        RecalculateHitRate(owner);
        lastHitTime = -hitRate;
    }

    public override void Execute(CyborgRangedController owner)
    {
        if (count == hitTimes)
        {
            owner.fsm.ChangeState(new CR_WaitHit());
            return;
        }
        else if (HitCooldown <= 0.0f)
        {
            // Hit!
            Alita.Call.character.currentLife -= owner.character.dmg;

            ++count;

            RecalculateHitRate(owner);
            lastHitTime = timer;
        }

        timer += Time.deltaTime;
    }

    public override void Exit(CyborgRangedController owner)
    {
        Debug.Log(owner.character.name + ": " + "EXIT" + " " + name);

        // ----- Load -----

        owner.agent.agentData.maxAngularAcceleration = maxAngularAcceleration;
        owner.agent.agentData.maxAngularVelocity = maxAngularVelocity;

        // ----- Agent -----

        owner.agent.isMovementStopped = false;

        // ----- CR_Hit -----

        owner.agent.FinishFace();
    }

    public override void DrawGizmos(CyborgRangedController owner)
    {

    }

    // --------------------------------------------------

    private void RecalculateHitRate(CyborgRangedController owner)
    {
        hitRate = owner.character.hitRate + (float)MathScript.GetRandomDouble(-1.0, 1.0) * owner.character.hitRateFluctuation;
    }
}
#endregion

#region CR_Wait
// ----------------------------------------------------------------------------------------------------
// CR_Hit
// ----------------------------------------------------------------------------------------------------

public class CR_Wait : CR_IState
{
    // ----- CR_Wait -----
    public float waitTime = 0.0f;
    public float timer = 0.0f;

    // --------------------------------------------------

    public CR_Wait()
    {
        name = "Wait";
    }

    public override void Enter(CyborgRangedController owner)
    {
        Debug.Log(owner.character.name + ": " + "ENTER" + " " + name);

        // ----- Agent -----

        owner.agent.isMovementStopped = true;
        owner.agent.isRotationStopped = true;
    }

    public override void Execute(CyborgRangedController owner)
    {
        timer += Time.deltaTime;
    }

    public override void Exit(CyborgRangedController owner)
    {
        Debug.Log(owner.character.name + ": " + "EXIT" + " " + name);

        // ----- Agent -----

        owner.agent.isMovementStopped = false;
        owner.agent.isRotationStopped = false;
    }

    public override void DrawGizmos(CyborgRangedController owner) { }
}

public class CR_WaitHit : CR_Wait
{
    public CR_WaitHit()
    {
        name = "WaitHit";
    }

    public override void Enter(CyborgRangedController owner)
    {
        Debug.Log(owner.character.name + ": " + "ENTER" + " " + name);

        // ----- CR_WaitHit -----

        waitTime = (float)MathScript.GetRandomDouble(owner.character.waitHitMinTime, owner.character.waitHitMaxTime);

        // ----- Base -----

        base.Enter(owner);
    }

    public override void Execute(CyborgRangedController owner)
    {
        if (timer >= waitTime)
        {
            owner.fsm.ChangeState(new CR_Attack());
            return;
        }

        base.Execute(owner);
    }

    public override void Exit(CyborgRangedController owner)
    {
        Debug.Log(owner.character.name + ": " + "EXIT" + " " + name);

        // ----- Base -----

        base.Exit(owner);
    }
}
#endregion

#region CR_Die
// ----------------------------------------------------------------------------------------------------
// CR_Die
// ----------------------------------------------------------------------------------------------------

public class CR_Die : CR_IState
{
    public CR_Die()
    {
        name = "Die";
    }

    public override void Enter(CyborgRangedController owner)
    {
        Debug.Log(owner.character.name + ": " + "ENTER" + " " + name);

        // ----- Agent -----

        owner.agent.ClearPath();
        owner.agent.ClearMovementAndRotation();
        owner.agent.isMovementStopped = true;
        owner.agent.isRotationStopped = true;
    }

    public override void Execute(CyborgRangedController owner)
    {
        EnemyEvent myEvent = new EnemyEvent();
        myEvent.type = Event_Type.EnemyDie;
        myEvent.gameObject = owner.gameObject;
        EventsManager.Call.PushEvent(myEvent);
    }

    public override void Exit(CyborgRangedController owner)
    {
        Debug.Log(owner.character.name + ": " + "EXIT" + " " + name);
    }

    public override void DrawGizmos(CyborgRangedController owner) { }
}
#endregion