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
    // Character
    public int tmp_life = 100;
    public uint tmp_minLife = 30;
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

    public CyborgMeleeFSM fsm;

    public Agent agent;
    public LineOfSight lineOfSight;

    public int Life
    {
        get { return character.life; }
        set
        {
            character.life += value;
            if (character.life <= 0)
            {
                character.life = 0;
                fsm.ChangeState(new Die(StateType.GoToDangerDistance));
            }
        }
    }

    public bool drawGizmosCyborgMelee = true;
    #endregion

    // ----------------------------------------------------------------------------------------------------

    public override void Awake()
    {
        fsm = new CyborgMeleeFSM(this);

        agent = gameObject.GetComponent<Agent>();
        lineOfSight = gameObject.childs[0].GetComponent<LineOfSight>();

        // --------------------------------------------------

        // Character
        character.life = tmp_life;
        character.minLife = tmp_minLife;
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
        fsm.ChangeState(new GoToGameObject(Alita.Call.gameObject, StateType.GoToDangerDistance));
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
        tmp_life = character.life;
        tmp_minLife = character.minLife;
        tmp_attackDistance = character.attackDistance;
        tmp_dangerDistance = character.dangerDistance;
        tmp_attackRate = character.attackRate;
        tmp_attackRateFluctuation = character.attackRateFluctuation;
        tmp_trackMaxAngularAcceleration = character.trackMaxAngularAcceleration;
        tmp_strafeMinTime = character.strafeMinTime;
        tmp_strafeMaxTime = character.strafeMaxTime;
    }
}