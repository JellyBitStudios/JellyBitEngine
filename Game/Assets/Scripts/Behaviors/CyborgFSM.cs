using System.Collections;
using System;
using JellyBitEngine;

// https://forum.unity.com/threads/c-proper-state-machine.380612/

public enum StateType
{
    None,
    GoTo
}

public abstract class IState
{
    public StateType stateType = StateType.None;

    public abstract void Enter();
    public abstract void Execute();
    public abstract void Exit();
}

public class FSM
{
    private IState state = null;

    public void ChangeState(IState state)
    {
        if (state == null)
            return;

        // Exit
        if (this.state != null)
            this.state.Exit();

        this.state = state;

        // Enter
        this.state.Enter();
    }

    public void UpdateState()
    {
        if (state != null)
            state.Execute();
    }
}

// ----------------------------------------------------------------------------------------------------

public class GoTo : IState
{
    private StateType prevStateType = StateType.None;

    private Agent agent = null;

    public GoTo(Agent agent, StateType prevStateType = StateType.None)
    {
        this.prevStateType = prevStateType;

        this.agent = agent;
    }

    public override void Enter()
    {
        stateType = StateType.GoTo;
        Debug.Log("Enter");
    }
    public override void Execute()
    {
        Debug.Log("Execute");
        if (agent.HasArrived)
        {
            switch (prevStateType)
            {
                case StateType.None:


                    break;
            }
        }
    }
    public override void Exit()
    {
        Debug.Log("Exit");
    }
}