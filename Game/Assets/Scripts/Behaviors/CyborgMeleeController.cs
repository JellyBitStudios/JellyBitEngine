using System.Collections;
using System;
using JellyBitEngine;

public class CyborgMeleeCharacter : Character
{
    public uint minLife = 30; // when this number is reached, the character runs away

    // -----

    // GoToGameObject
    /// GoToAttackDistance
    public float attackDistance = 1.0f;
    /// GoToDangerDistance
    public float dangerDistance = 2.0f;

    // Attack
    public float attackRate = 10.0f;
    public float attackRateFluctuation = 0.0f;
    public float trackMaxAngularAcceleration = 90.0f;

    // Wander
    /// Strafe
    public float strafeMinTime = 1.0f;
    public float strafeMaxTime = 3.0f;
}

public class CyborgMeleeController : JellyScript
{
    #region INSPECTOR_VARIABLES
    public GameObject tmp_lineOfSight = null;

    // Character
    public float tmp_attackDistance = 1.0f;
    public float tmp_dangerDistance = 2.0f;              
    public float tmp_attackRate = 10.0f;
    public float tmp_attackRateFluctuation = 0.0f;
    public float tmp_trackMaxAngularAcceleration = 90.0f;
    public float tmp_strafeMinTime = 1.0f;
    public float tmp_strafeMaxTime = 3.0f;
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

    public bool drawGizmosCyborgMelee = true;
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

        // Character
        character.attackDistance = tmp_attackDistance;
        character.dangerDistance = tmp_dangerDistance;
        character.attackRate = tmp_attackRate;
        character.attackRateFluctuation = tmp_attackRateFluctuation;
        character.trackMaxAngularAcceleration = tmp_trackMaxAngularAcceleration;
        character.strafeMinTime = tmp_strafeMinTime;
        character.strafeMaxTime = tmp_strafeMaxTime;
    }

    public override void Start()
    {
        //fsm.ChangeState(new Wander(StateType.Wander));  
        fsm.ChangeState(new GoToGameObject(target, StateType.GoToDangerDistance));
    }

    public override void Update()
    {
        UpdateInspectorVariables();

        // --------------------------------------------------

        //HandleInput();
        fsm.UpdateState();
    }

    public override void OnDrawGizmos()
    {
        if (!drawGizmosCyborgMelee)
            return;

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

    private void UpdateInspectorVariables()
    {
        character.attackDistance = tmp_attackDistance;
        character.dangerDistance = tmp_dangerDistance;
        character.attackRate = tmp_attackRate;
        character.attackRateFluctuation = tmp_attackRateFluctuation;
        character.trackMaxAngularAcceleration = tmp_trackMaxAngularAcceleration;
        character.strafeMinTime = tmp_strafeMinTime;
        character.strafeMaxTime = tmp_strafeMaxTime;
    }
}