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
    Attack
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

public class GoToGameObject : ICyborgMeleeState
{
    private StateType prevStateType = StateType.None;

    // -----

    private GameObject target = null;

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
        Debug.Log("Enter GoToGameObject");

        switch (stateType)
        {
            case StateType.GoToDangerDistance:
                {
                    owner.agent.separationData.isActive = false;
                    owner.agent.collisionAvoidanceData.isActive = true;

                    owner.agent.SetDestination(target.transform.position);
                }
                break;

            case StateType.GoToAttackDistance:
                {
                    owner.agent.separationData.isActive = true;
                    owner.agent.collisionAvoidanceData.isActive = false;
                }
                break;
        }
    }

    public override void Execute(CyborgMeleeController owner)
    {
        Debug.Log("Execute GoToGameObject");

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
                    if (owner.agent.separationData.hasOutput)
                        // Avoid the enemies
                        owner.agent.SetDestination(owner.transform.position);
                    else
                        // Approach the player
                        owner.agent.SetDestination(target.transform.position);

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
                            owner.fsm.ChangeState(new Attack(owner.target, StateType.Attack, stateType));
                            return;
                        }
                    }
                }
                break;
        }
    }

    public override void Exit(CyborgMeleeController owner)
    {
        Debug.Log("Exit GoTo");
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

public class Attack : ICyborgMeleeState
{
    private StateType prevStateType = StateType.None;

    // -----

    private GameObject target = null;

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
    }

    public override void Execute(CyborgMeleeController owner)
    {
        Debug.Log("Execute Attack");      
    }

    public override void Exit(CyborgMeleeController owner)
    {
        Debug.Log("Exit Attack");
    }

    public override void DrawGizmos(CyborgMeleeController owner)
    {
        Debug.DrawSphere(owner.character.attackDistance, Color.Red, owner.transform.position, Quaternion.identity, Vector3.one);
    }
}