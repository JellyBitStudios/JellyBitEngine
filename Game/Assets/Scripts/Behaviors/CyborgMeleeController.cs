using System.Collections;
using System;
using JellyBitEngine;

public class CyborgMeleeCharacter : Character
{

}

public class CyborgMeleeController : JellyScript
{
    #region INSPECTOR_VARIABLES
    public GameObject tmp_lineOfSight = null;
    #endregion

    #region PUBLIC_VARIABLES
    /// <Temporal>
    public LayerMask raycastMask = new LayerMask();
    /// </Temporal>

    public GameObject target = null;

    // Battle Circle
    public float attackRate = 1.0f;
    public float attackRateFluctuation = 1.0f;
    #endregion

    #region PRIVATE_VARIABLES
    private FSM fsm = new FSM();

    private Agent agent = null;
    private LineOfSight lineOfSight = null;
    #endregion

    // ----------------------------------------------------------------------------------------------------

    public override void Awake()
    {
        agent = gameObject.GetComponent<Agent>();
    }

    public override void Start()
    {
        fsm.ChangeState(new GoTo(agent));
    }

    public override void Update()
    {
        HandleInput();
        fsm.UpdateState();
    }

    // ----------------------------------------------------------------------------------------------------

    private void HandleInput()
    {
        if (Input.GetMouseButton(MouseKeyCode.MOUSE_LEFT))
        {
            Ray ray = Physics.ScreenToRay(Input.GetMousePosition(), Camera.main);
            RaycastHit hitInfo;
            if (Physics.Raycast(ray, out hitInfo, float.MaxValue, raycastMask, SceneQueryFlags.Static))
            {
                agent.SetDestination(hitInfo.point);
                //wanderState = WanderStates.goToPosition;
            }
        }
    }
}