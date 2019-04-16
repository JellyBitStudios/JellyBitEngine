using System.Collections;
using System;
using JellyBitEngine;

public class CyborgMeleeCharacter : Character
{
    // Battle Circle
    public float attackDistance = 1.0f;
    public float dangerDistance = 2.0f;

    public float trackSpeed = 0.1f;

    public float attackRate = 10.0f;
    public float attackRateFluctuation = 0.0f;

    public float strafeMinTime = 1.0f;
    public float strafeMaxTime = 3.0f;
}

public class CyborgMeleeController : JellyScript
{
    #region INSPECTOR_VARIABLES
    public GameObject tmp_lineOfSight = null;

    // Battle Circle
    public float tmp_attackDistance = 1.0f;
    public float tmp_dangerDistance = 2.0f;
                 
    public float tmp_trackSpeed = 0.1f;
                 
    public float tmp_attackRate = 10.0f;
    public float tmp_attackRateFluctuation = 0.0f;

    public float tmp_strafeMinTime = 0.0f;
    public float tmp_strafeMaxTime = 0.0f;
    #endregion

    #region PUBLIC_VARIABLES
    /// <Temporal>
    public LayerMask raycastMask = new LayerMask();
    /// </Temporal>

    public CyborgMeleeCharacter character = new CyborgMeleeCharacter();

    public GameObject target = null; // Alita

    public CyborgMeleeFSM fsm = null;
    public Agent agent = null;
    public LineOfSight lineOfSight = null;
    public BattleCircle battleCircle = null;
    #endregion

    #region PRIVATE_VARIABLES
    // Battle Circle
    private float actualAttackRate = 0.0f;
    private float thinkPeriod = 1.5f;
    private float reactPeriod = 0.4f;

    private float lastAttackedTime = 0.0f;
    private float lastThought = 0.0f;
    private float lastReact = 0.0f;
    #endregion

    // ----------------------------------------------------------------------------------------------------

    public override void Awake()
    {
        fsm = new CyborgMeleeFSM(this);

        agent = gameObject.GetComponent<Agent>();

        // --------------------------------------------------

        if (tmp_lineOfSight != null)
            lineOfSight = tmp_lineOfSight.GetComponent<LineOfSight>();

        if (target != null)
            battleCircle = target.GetComponent<BattleCircle>();

        // Battle Circle
        character.attackDistance = tmp_attackDistance;
        character.dangerDistance = tmp_dangerDistance;

        character.trackSpeed = tmp_trackSpeed;

        character.attackRate = tmp_attackRate;
        character.attackRateFluctuation = tmp_attackRateFluctuation;

        character.strafeMinTime = tmp_strafeMinTime;
        character.strafeMaxTime = tmp_strafeMaxTime;
    }

    public override void Start()
    {
        fsm.ChangeState(new GoToGameObject(target, StateType.GoToDangerDistance));

        actualAttackRate = character.attackRate + (float)MathScript.GetRandomDouble(-1.0, 1.0) * character.attackRateFluctuation;      
    }

    public override void Update()
    {
        //HandleInput();
        fsm.UpdateState();
    }

    public override void OnDrawGizmos()
    {
        fsm.DrawGizmos();
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