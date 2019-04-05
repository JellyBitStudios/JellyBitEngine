﻿using System;
using JellyBitEngine;

public class NetmanCharacter : Character
{

}

public class NetmanController : JellyScript
{
    #region PUBLIC_VARIABLES
    //public NetmanCharacter stats = new NetmanCharacter();

    public enum NetmanStates { wander, attack };
    public NetmanStates currentState = NetmanStates.wander;

    /////
    public GameObject target = null;

    /// Public to private
    public GameObject goLineOfSight = null;
    public GameObject goWaypointsManager = null;
    #endregion

    #region PRIVATE_VARIABLES
    private enum WanderStates { findRandomPosition, goToPosition }
    private WanderStates wanderState = WanderStates.findRandomPosition;
    bool isWanderActive = false;

    private enum AttackStates { goToWaypoint, hit };
    private AttackStates attackState = AttackStates.goToWaypoint;
    bool isAttackActive = false;

    /////
    private NavMeshAgent agent = null;

    private Waypoint waypoint = null;

    /// Public to private
    private LineOfSight lineOfSight = null;
    private WaypointsManager waypointsManager = null;
    #endregion

    public override void Awake()
    {
        lineOfSight = goLineOfSight.GetComponent<LineOfSight>();
        waypointsManager = goWaypointsManager.GetComponent<WaypointsManager>();
        agent = gameObject.GetComponent<NavMeshAgent>();
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
                    /// Is there any free waypoint?
                    waypoint = waypointsManager.GetClosestWaypoint(transform.position);
                    if (waypoint == null)
                        break;

                    Debug.Log("Closest point found");

                    waypoint.isOccupied = true;
                    agent.SetDestination(waypoint.transform.position);

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
        Debug.Log("Wander: ACTIVATE");
        /////
        isWanderActive = true;
    }

    // Update
    private void UpdateWander()
    {
        switch (wanderState)
        {
            case WanderStates.findRandomPosition:

                break;

            case WanderStates.goToPosition:

                break;
        }
    }

    // Terminate
    private void TerminateWander()
    {
        Debug.Log("Wander: TERMINATE");
        /////
        isWanderActive = false;
    }

    // -------------------- Attack --------------------

    // Activate
    private void ActivateAttack()
    {
        Debug.Log("Attack: ACTIVATE");
        /////
        isAttackActive = true;
    }

    // Update
    private void UpdateAttack()
    {
        switch (attackState)
        {
            case AttackStates.goToWaypoint:

                if (!agent.isWalking())
                {
                    // TODO: play animation
                    Debug.Log("HIT HIT HIT");
                    attackState = AttackStates.hit;
                }

                break;

            case AttackStates.hit:

                float threshold = 0.25f; // TODO: it should be the waypoint radius
                Vector3 diff = waypoint.transform.position - transform.position;
                diff = new Vector3(diff.x, 0.0f, diff.z);
                if (diff.magnitude > threshold)
                {
                    TerminateAttack();
                    currentState = NetmanStates.wander;
                    break;
                }

                // FaceMovement
                float targetDegrees = (float)Math.Atan2(target.transform.forward.x, target.transform.forward.z);
		        float currentDegrees = (float)Math.Atan2(transform.forward.x, transform.forward.z);
                float degrees = targetDegrees - currentDegrees;
                Quaternion orientation = Quaternion.Rotate(Vector3.up, degrees);
                transform.rotation = orientation;

                // If Alita's health is 0... TerminateAttack();

                break;
        }
    }

    // Terminate
    private void TerminateAttack()
    {
        Debug.Log("Attack: TERMINATE");
        waypoint.isOccupied = false;
        waypoint = null;
        /////
        attackState = AttackStates.goToWaypoint;
        isAttackActive = false;
    }
}