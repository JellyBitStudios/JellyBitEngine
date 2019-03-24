using System;
using System.Collections;
using JellyBitEngine;

public class Alita : JellyScript
{
    // Unit
    private Unit own_unit = null;

    //Alita propeties
    float speed = 10.0f;
    float acceleration = 30.0f;

    //Abilities
    Dash dash = null;
    AreaAttk areaAttk = null;
    private float ability_anim_timer = 0.0f; 
    public float area_time = 1.0f;

    // Raycast
    public LayerMask terrainMask = new LayerMask();
    public LayerMask enemyMask = new LayerMask();

    //Place to go
    Vector3 placeToGo = new Vector3(0, 0, 0);

    //Agent
    private NavMeshAgent agent = null;

    //Alita states
    private enum Alita_State
    {
        IDLE,
        RUNNING,
        GOING_TO_ATTK,
        ATTK,
        DASHING,
        AREA_ATTK
    }
    Alita_State state = Alita_State.IDLE;

    private enum Attack_State
    {
        FIRST_ATTK,
        SECOND_ATTK,
        THIRD_ATTK
    }
    Attack_State attkState = Attack_State.FIRST_ATTK;

    //Audio
    AlitaAudio soundFX;

    //Enemy
    GameObject enemy = null;
    Unit enemy_unit = null;

    //Variables about attack distance and time
    public float attack_dist = 2.0f;
    public float attk_period = 0.5f;
    private float attk_cool_down = 0.0f;

    //Orienting to enemy
    float minAngle = 0.1f;
    double Rad2Deg = 57.29577;

    //Animations
    public GameObject animManag = null;
    AnimatorScript anim = null;

    // Enemy pressed
    bool enemyPressed = false;
    float enemyPressedTimer = 0.0f;
    bool soundPunch = false;

    public override void Awake()
    {
        agent = gameObject.GetComponent<NavMeshAgent>();
        dash = gameObject.GetComponent<Dash>();
        areaAttk = gameObject.GetComponent<AreaAttk>();
        soundFX = gameObject.GetComponent<AlitaAudio>();

        if (animManag != null)
            anim = animManag.GetComponent<AnimatorScript>();

        own_unit = gameObject.GetComponent<Unit>();
    }

    //Called every frame
    public override void Update()
    {
        if (agent != null && areaAttk != null)
        {
            CheckState();

            if (enemyPressed)
                CoolLeftMouseClickDown();

            if (state != Alita_State.DASHING)
                CheckForMouseClick();

            if (state == Alita_State.IDLE || state == Alita_State.RUNNING)
                CheckForDash();

            if (state != Alita_State.ATTK && state != Alita_State.AREA_ATTK && state != Alita_State.DASHING)
                CheckForSPAttack(); //Only special attacks when no normal attacking
        }
        else
            Debug.Log("AGENT IS NULL");
    }

    //Check for... functions
    //---------------------------------------------------------------------------------------------------------------------------------

    private void CheckState()
    {
        switch (state)
        {
            case Alita_State.IDLE:
                ExecuteIdle();
                break;

            case Alita_State.RUNNING:
                ExecuteRunning();
                break;

            case Alita_State.GOING_TO_ATTK:
                ExecuteGoingToAttack();
                break;

            case Alita_State.ATTK:
                ExecuteAttack();
                break;

            case Alita_State.DASHING:
                ExecuteDashing();
                break;

            case Alita_State.AREA_ATTK:
                ExecuteAreaAttack();
                break;
        }
    }

    private void CheckForMouseClick()
    {
        //Attack
        if (Input.GetMouseButtonDown(MouseKeyCode.MOUSE_LEFT) && !areaAttk.IsAreaActive())
            if(!enemyPressed)
                DecideIfGoToAttack();

        //Move
        if (Input.GetMouseButton(MouseKeyCode.MOUSE_RIGHT))
            Move();

    }


    private void CheckForDash()
    {
        if (Input.GetKeyDown(KeyCode.KEY_SPACE))
        {
            bool dashing = false;
            if (dash != null)
                dashing = dash.DashPush(agent);

            if (dashing)
            {
                if (areaAttk != null)
                    areaAttk.HideArea();

                SwitchStateDash();
            }
        }
    }

    private void CheckForSPAttack()
    {
        if (!areaAttk.IsAreaCooldown())
        {
            if (Input.GetKeyDown(KeyCode.KEY_Q))
            {
                areaAttk.ToggleAreaVisibility();
            }

            if (Input.GetKeyUp(KeyCode.KEY_Q))
            {
                bool areaActive = false;
                areaActive = areaAttk.HideArea();

                if (areaActive)
                {
                    agent.maxSpeed = 0;

                    SwitchStateAreaAttack();
                    soundFX.CleanAlitaAudio();
                }
            }
        }
    }

    //Basic Actions
    //---------------------------------------------------------------------------------------------------------------------------------

    private void Move()
    {
        Ray ray = Physics.ScreenToRay(Input.GetMousePosition(), Camera.main);
        RaycastHit hit;
        if (Physics.Raycast(ray, out hit, float.MaxValue, terrainMask, SceneQueryFlags.Dynamic | SceneQueryFlags.Static))
        {
            SwitchStateRunning();

            //Go to place
            agent.SetDestination(hit.point);
            placeToGo = hit.point;

        }
    }

    private void DecideIfGoToAttack()
    {
        Ray ray = Physics.ScreenToRay(Input.GetMousePosition(), Camera.main);
        RaycastHit hit;
        if (Physics.Raycast(ray, out hit, float.MaxValue, enemyMask, SceneQueryFlags.Dynamic | SceneQueryFlags.Static))
        {
            Debug.Log("GOING TO HIT ENEMY");
            //Go to attack
            enemy = hit.gameObject;
            if (enemy != null)
            {
                SwitchStateGoToEnemy();

                //Determine a place a little further than enemy position
                Vector3 enemy_fwrd_vec = (transform.position - enemy.transform.position).normalized();
                Vector3 enemy_pos = enemy.transform.position + enemy_fwrd_vec * attack_dist;

                //Go to enemy
                agent.SetDestination(enemy_pos);
                Debug.Log(enemy_pos.ToString());
                enemy_unit = enemy.GetComponent<Unit>();

                enemyPressed = true;
            }
        }
    }

    void CoolLeftMouseClickDown()
    {
        enemyPressedTimer += Time.deltaTime;

        if (enemyPressedTimer >= 1.2f)
        {
            enemyPressed = false;
            enemyPressedTimer = 0.0f;
        }

    }

    private void CheckIfRotateToEnem()
    {
        Vector3 faceDirection = (enemy.transform.position - transform.position).normalized();
        double targetDegrees = Math.Atan2(faceDirection.x, faceDirection.z) * Rad2Deg;

        //if (targetDegrees > minAngle)
            transform.rotation = Quaternion.Rotate(Vector3.up, (float)(targetDegrees));
    }


    //Attacks
    //---------------------------------------------------------------------------------------------------------------------------------

    private void NormalAttack()
    {
        attk_cool_down += Time.deltaTime;

        //if (audioSource != null)
        //    ModuleAudio.Instance.CleanAudio(audioSource);

        if(attk_cool_down >= 0.5f && !soundPunch)
        {
            Debug.Log("ENEMY HIT");
            if (enemy != null && enemy_unit != null)
            {
                switch (attkState)
                {
                    case Attack_State.FIRST_ATTK:
                        enemy_unit.Hit((int)own_unit.damage);
                        break;
                    case Attack_State.SECOND_ATTK:
                        enemy_unit.Hit((int)(own_unit.damage * 1.2));
                        break;
                    case Attack_State.THIRD_ATTK:
                        enemy_unit.Hit((int)(own_unit.damage * 1.5));
                        break;
                }

                soundFX.PlayHit();
                soundPunch = true;
            }
        }

        //Attack every second
        if (attk_cool_down >= attk_period)
        {
          attk_cool_down = 0.0f;
          soundPunch = false;

            if (!Input.GetMouseButton(MouseKeyCode.MOUSE_LEFT))
            {
                SwitchStateIdle();
                attkState = Attack_State.FIRST_ATTK;
            }
            else
                FollowComboAttack();

        }
    }

    public void FollowComboAttack()
    {
        switch (attkState)
        {
            case Attack_State.FIRST_ATTK:
                anim.PlayAttack();
                attkState = Attack_State.SECOND_ATTK;
                break;
            case Attack_State.SECOND_ATTK:
                anim.PlayAttack();
                attkState = Attack_State.THIRD_ATTK;
                break;
            case Attack_State.THIRD_ATTK:
                anim.PlayAttack();
                attkState = Attack_State.FIRST_ATTK;
                break;
        }
    }


    //Things of UI
    //---------------------------------------------------------------------------------------------------------------------------------


    public float GetLifePercent()
    {
        float ret = 1.0f;

        if (own_unit != null)
        {
            ret = own_unit.max_life / own_unit.max_life;
        }

        return ret;
    }

    public float GetFocusUnitLifePercent()
    {
        return enemy_unit != null ? enemy_unit.current_life / enemy_unit.max_life : -1.0f;
    }

    public float GetCDR_Q()
    {
        float ret = 1.0f;

        if (areaAttk != null && areaAttk.cooling < areaAttk.timeCoolDown)
            ret = areaAttk.cooling / areaAttk.timeCoolDown;

        return ret;
    }

    //Switching state methods
    //---------------------------------------------------------------------------------------------------------------------------------
    void SwitchStateIdle()
    {
        state = Alita_State.IDLE;

        //Anim
        if (anim != null)
            anim.PlayIdle();

        //Audio
        soundFX.CheckToPlayIdle();

        Debug.Log("IDLE BOI");
    }

    void SwitchStateRunning()
    {
        state = Alita_State.RUNNING;
        agent.maxSpeed = speed;
        agent.maxAcceleration = acceleration;

        //Anim
        if (anim != null)
            anim.PlayRunning();

        //Audio
        soundFX.PlayRunning();

        Debug.Log("RUNNING");
    }

    void SwitchStateGoToEnemy()
    {
        state = Alita_State.GOING_TO_ATTK;
        agent.maxSpeed = speed;
        agent.maxAcceleration = acceleration;

        //Anim
        if (anim != null)
            anim.PlayRunning();

        //Audio
        soundFX.PlayRunningGoToEnem();

        Debug.Log("GOING TO ENEMY");
    }

    void SwitchStateAttack()
    {
        state = Alita_State.ATTK;

        //Anim
        if (anim != null)
            anim.PlayAttack();

        //Audio
        soundFX.SetLastStateAttack();
    }

    void SwitchStateDash()
    {
        state = Alita_State.DASHING;

        //Anim
        //if (anim != null)
        //    anim.PlayDash();
        
        //Audio
        soundFX.PlayDash();

        Debug.Log("START DASH");
    }

    void SwitchStateAreaAttack()
    {
        state = Alita_State.AREA_ATTK;
        if (anim != null)
            anim.PlayAreaAttack();

        Debug.Log("AREA ATTACK!!");
    }
    //Executing state methods
    //---------------------------------------------------------------------------------------------------------------------------------
    void ExecuteIdle()
    {
        //Audio
        soundFX.PlayIdle();
    }

    void ExecuteRunning()
    {
        float dist = (float)(placeToGo - transform.position).magnitude;
        if (dist <= 2.0f)
        {
            agent.maxSpeed = 0;
            SwitchStateIdle();
        }
    }

    void ExecuteGoingToAttack()
    {
        float diff = (float)(enemy.transform.position - transform.position).magnitude;
        if (diff <= attack_dist + 1.0f)
        {
            Debug.Log("ARRIVE TO ENEMY");
            agent.maxSpeed = 0;
            SwitchStateAttack();
        }
    }

    void ExecuteAttack()
    {
        if (enemy != null)
        {
            NormalAttack();
            CheckIfRotateToEnem();
        }
        else
        {
            enemy = null;
            enemy_unit = null;
            attkState = Attack_State.FIRST_ATTK;
            SwitchStateIdle();
        }
    }

    void ExecuteDashing()
    {
        bool dashing = true;
        dashing = dash.CheckForPushDashEnd(agent);

        if (!dashing)
        {
            ModuleAudio.Instance.dashPlayed = false;
            agent.obstacleAvoidance = true;
            state = Alita_State.IDLE;
            Debug.Log("IDLE BOI");

            ////Anim
            //if (anim != null)
            //    anim.PlayIdle();
        }

    }

    void ExecuteAreaAttack()
    {
        ability_anim_timer += Time.deltaTime;

        if (ability_anim_timer >= area_time)
        {
            ability_anim_timer = 0.0f;

            areaAttk.AreaAttack(enemyMask);
            SwitchStateIdle();

            soundFX.PlayAreaAttackHit();

        }
    }

}
