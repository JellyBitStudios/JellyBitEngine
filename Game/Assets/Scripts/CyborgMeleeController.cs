using System.Collections;
using System;
using JellyBitEngine;

public class CyborgMeleeCharacter : Character
{

}

public class CyborgMeleeController : JellyScript
{
    #region INSPECTOR_VARIABLES
    public GameObject gameObjectLineOfSight = null;
    public GameObject gameObjectWaypointsManager = null;
    #endregion

    #region PUBLIC_VARIABLES
    //public NetmanCharacter stats = new NetmanCharacter();
    /// <Temporal>
    public LayerMask raycastMask = new LayerMask();
    /// </Temporal>

    public enum NetmanStates { wander, attack };
    public NetmanStates currentState = NetmanStates.wander;

    public GameObject target = null;
    #endregion

    #region PRIVATE_VARIABLES
    private enum WanderStates { findRandomPosition, goToPosition }
    private WanderStates wanderState = WanderStates.findRandomPosition;
    bool isWanderActive = false;

    private enum AttackStates { goToWaypoint, hit };
    private AttackStates attackState = AttackStates.goToWaypoint;
    bool isAttackActive = false;

    private Agent agent = null;
    private LineOfSight lineOfSight = null;
    private WaypointsManager waypointsManager = null;

    private Waypoint waypoint = null;
    #endregion

    // ----------------------------------------------------------------------------------------------------

    public override void Awake()
    {
        if (gameObjectLineOfSight != null)
            lineOfSight = gameObjectLineOfSight.GetComponent<LineOfSight>();
        if (gameObjectWaypointsManager != null)
            waypointsManager = gameObjectWaypointsManager.GetComponent<WaypointsManager>();

        agent = gameObject.GetComponent<Agent>();
    }

    public override void Update()
    {
        UpdateState();
    }

    // ----------------------------------------------------------------------------------------------------

    // Update state
    private void UpdateState()
    {
        switch (currentState)
        {
            case NetmanStates.wander:

                if (!isWanderActive)
                    ActivateWander();

                UpdateWander();

                /*
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
                */

                break;

            case NetmanStates.attack:

                /*
                if (!isAttackActive)
                    ActivateAttack();

                UpdateAttack();
                */

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

                if (Input.GetMouseButtonDown(MouseKeyCode.MOUSE_LEFT))
                {
                    Debug.Log("Mouse left down");

                    Ray ray = Physics.ScreenToRay(Input.GetMousePosition(), Camera.main);
                    RaycastHit hitInfo;
                    if (Physics.Raycast(ray, out hitInfo, float.MaxValue, raycastMask, SceneQueryFlags.Static))
                    {
                        Debug.Log("Hit");
                        if (hitInfo != null)
                        {
                            agent.SetDestination(hitInfo.point);
                            wanderState = WanderStates.goToPosition;
                        }
                    }
                }

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
    /*
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
    */
}