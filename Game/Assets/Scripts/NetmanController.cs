using System;
using JellyBitEngine;

public class NetmanCharacter : Character
{

}

public class NetmanController : JellyScript
{
    #region PUBLIC_VARIABLES
    public NetmanCharacter stats = new NetmanCharacter();

    public enum NetmanStates { wander, attack };
    public NetmanStates currentState = NetmanStates.wander;

    /////
    public LineOfSight lineOfSight = null;
    public GameObject target = null;
    #endregion

    #region PRIVATE_VARIABLES
    private enum WanderStates { goToWaypoint }
    private WanderStates wanderState = WanderStates.goToWaypoint;
    bool isWanderActive = false;

    private enum AttackStates { findWaypoint, goToWaypoint, hit };
    private AttackStates attackState = AttackStates.findWaypoint;
    bool isAttackActive = false;

    /////
    private NavMeshAgent agent = null;
    private Vector3 destination = new Vector3(0.0f, 0.0f, 0.0f);
    #endregion

    public override void Awake()
    {
        agent = gameObject.GetComponent<NavMeshAgent>();
    }

    public override void Start()
    {
        destination = transform.position;
    }

    // Called every frame
    public override void Update()
    {
        UpdateState();
    }

    // Update state
    private void UpdateState()
    {
        switch (currentState)
        {
            case NetmanStates.wander:

                if (!isWanderActive)
                    ActivateWander();

                UpdateWander();

                /// Is target seen?
                if (lineOfSight.IsTargetSeen)
                {
                    TerminateWander();
                    currentState = NetmanStates.attack;
                }

                break;

            case NetmanStates.attack:

                if (!isAttackActive)
                    ActivateAttack();

                UpdateAttack();

                break;
        }
    }

    // -------------------- Wander --------------------
    // Activate
    private void ActivateWander()
    {

        /////
        isWanderActive = true;
    }

    // Update
    private void UpdateWander()
    {
        switch (wanderState)
        {
            case WanderStates.goToWaypoint:

                break;
        }
    }

    // Terminate
    private void TerminateWander()
    {

        /////
        isWanderActive = false;
    }

    // -------------------- Attack --------------------
    // Activate
    private void ActivateAttack()
    {
        /////
        isAttackActive = true;
    }

    // Update
    private void UpdateAttack()
    {
        switch (attackState)
        {
            case AttackStates.findWaypoint:

                destination = lineOfSight.LastPositionSeen;
                agent.SetDestination(destination);

                attackState = AttackStates.goToWaypoint;

                break;

            case AttackStates.goToWaypoint:

                if (!agent.isWalking())
                {
                    if (destination == lineOfSight.LastPositionSeen)
                    {
                        // Play animation

                        attackState = AttackStates.hit;
                    }
                    else
                        attackState = AttackStates.findWaypoint;
                }              

                break;

            case AttackStates.hit:

                // FaceMovement
                float targetDegrees = (float)Math.Atan2(target.transform.forward.x, target.transform.forward.z);
		        float currentDegrees = (float)Math.Atan2(transform.forward.x, transform.forward.z);
                float degrees = targetDegrees - currentDegrees;
                Quaternion orientation = Quaternion.Rotate(Vector3.up, degrees);
                transform.rotation = orientation;

                break;
        }
    }

    // Terminate
    private void TerminateAttack()
    {
        /////
        attackState = AttackStates.findWaypoint;
        isAttackActive = false;
    }
}