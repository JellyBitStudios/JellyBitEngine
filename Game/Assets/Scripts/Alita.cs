using System;
using System.Collections;
using JellyBitEngine;

public class Alita : JellyScript
{
    //Alita propeties
    int damage = 20;
    float speed = 10.0f;
    float acceleration = 30.0f;

    //Habilities
    Dash dash = null;
    AreaAttk areaAttk = null;

    // Raycast
    public LayerMask terrainMask = new LayerMask();
    public LayerMask enemyMask = new LayerMask();

    //Place to go
    Vector3 placeToGo = new Vector3(0, 0, 0);

    //Agent
    private NavMeshAgent agent = null;

    //Particles
    //ParticleEmitter smoke = null;

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

    //Audio Source
    private AudioSource audioSource = null;
    private Alita_State lastState = Alita_State.IDLE;

    //Enemy
    GameObject enemy = null;
    Unit enemy_unit = null;

    //Variables about attack distance and time
    public float attack_dist = 2.0f;
    public float attk_period = 1.0f;
    private float attk_cool_down = 0.0f;

    //Orienting to enemy
    float minAngle = 0.1f;
    double Rad2Deg = 57.29577;

    //Animations
    public GameObject animManag = null;
    AnimatorScript anim = null;

    public override void Awake()
    {
        agent = gameObject.GetComponent<NavMeshAgent>();
        dash = gameObject.GetComponent<Dash>();
        areaAttk = gameObject.GetComponent<AreaAttk>();
        audioSource = gameObject.GetComponent<AudioSource>();

        if (animManag != null)
            anim = animManag.GetComponent<AnimatorScript>();
    }

    //Called every frame
    public override void Update()
    {
        if (agent != null && areaAttk != null)
        {
            CheckState();

            if (state != Alita_State.DASHING)
                CheckForMouseClick();

            if (state == Alita_State.IDLE || state == Alita_State.RUNNING)
                CheckForDash();

            if (state != Alita_State.ATTK && state != Alita_State.AREA_ATTK && state != Alita_State.DASHING)
                if (areaAttk != null)
                    if(!areaAttk.IsAreaCooldown())
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

                if (audioSource != null)
                {
                    if (lastState == Alita_State.RUNNING)
                    {
                        ModuleAudio.Instance.CleanAudio(audioSource);//solo si vienes de correr o dashear
                        Debug.Log("LO ULTIMO KE EE ESHO A SIO CORRER");
                    }
                    //if (lastState == Alita_State.DASHING)
                    //{
                    //    ModuleAudio.Instance.CleanAudio(audioSource);//solo si vienes de correr o dashear
                    //    Debug.Log("LO ULTIMO KE EE ESHO A SIO DASHEAR");
                    //}
                    ModuleAudio.Instance.CanPlayRunning();
                    ModuleAudio.Instance.PlayIdleFX(audioSource);
                }
                lastState = Alita_State.IDLE;
                break;
            case Alita_State.RUNNING:

                float dist = (float)(placeToGo - transform.position).magnitude;
                //Debug.Log(dist.ToString());
                if (dist <= 2.0f)
                {
                    Debug.Log("IDLE BOI");
                    agent.maxSpeed = 0;
                    state = Alita_State.IDLE;

                    //Anim
                    if (anim != null)
                        anim.PlayIdle();
                }

                if (audioSource != null && ModuleAudio.Instance.canPlayRunning)
                {
                    ModuleAudio.Instance.ResetIdleCounter();
                    ModuleAudio.Instance.CleanAudio(audioSource);
                    ModuleAudio.Instance.PlayRunningFX(audioSource);
                    ModuleAudio.Instance.canPlayRunning = false;
                }
                lastState = Alita_State.RUNNING;
                break;
            case Alita_State.GOING_TO_ATTK:

                float diff = (float)(enemy.transform.position - transform.position).magnitude;
                if (diff <= attack_dist + 1.0f)
                {
                    Debug.Log("ARRIVE TO ENEMY");
                    agent.maxSpeed = 0;
                    state = Alita_State.ATTK;

                    //Anim
                    if (anim != null)
                        anim.PlayAttack();
                }

                if (audioSource != null)
                {
                    ModuleAudio.Instance.ResetIdleCounter();
                    ModuleAudio.Instance.CleanAudio(audioSource);
                    ModuleAudio.Instance.CanPlayRunning();
                    ModuleAudio.Instance.PlayRunningFX(audioSource);
                }
                lastState = Alita_State.GOING_TO_ATTK;
                break;
            case Alita_State.ATTK:
                if (enemy != null)
                { 
                    NormalAttack();
                    CheckIfRotateToEnem();
                }
                else
                {
                    enemy = null;
                    enemy_unit = null;
                    state = Alita_State.IDLE;
                    Debug.Log("IDLE BOI");

                    //Anim
                    if (anim != null)
                        anim.PlayIdle();
                }
                lastState = Alita_State.ATTK;
                break;
            case Alita_State.DASHING:
                bool dashing = true;
                dashing = dash.CheckForPushDashEnd(agent);

                if (audioSource != null)
                {
                    if(lastState != Alita_State.DASHING)
                    {
                        ModuleAudio.Instance.CleanAudio(audioSource);
                    }
                    ModuleAudio.Instance.ResetIdleCounter();
                    ModuleAudio.Instance.CanPlayRunning();
                    ModuleAudio.Instance.PlayDashFX(audioSource);
                }

                if (!dashing)
                {
                    ModuleAudio.Instance.dashPlayed = false;
                    agent.obstacleAvoidance = true;
                    state = Alita_State.IDLE;
                    Debug.Log("IDLE BOI");

                    //Anim
                    if (anim != null)
                        anim.PlayIdle();
                }
                lastState = Alita_State.DASHING;
                break;
            case Alita_State.AREA_ATTK:
                agent.SetDestination(transform.position);
                areaAttk.AreaAttack(enemyMask);
                if (audioSource != null)
                {
                    ModuleAudio.Instance.ResetIdleCounter();
                    ModuleAudio.Instance.CanPlayRunning();
                    ModuleAudio.Instance.CleanAudio(audioSource);
                    ModuleAudio.Instance.PlayOnce(audioSource, ModuleAudio.Instance.areaAttackFX);
                    ModuleAudio.Instance.PlayOnce(audioSource, ModuleAudio.Instance.areaAttackScreamFX);
                }
                state = Alita_State.IDLE;
                Debug.Log("IDLE BOI");
                lastState = Alita_State.AREA_ATTK;

                //Anim
                if (anim != null)
                    anim.PlayIdle();
                lastState = Alita_State.AREA_ATTK;
                break;
        }
    }

    private void CheckForMouseClick()
    {
        //Attack
        if (Input.GetMouseButtonDown(MouseKeyCode.MOUSE_LEFT) && !areaAttk.IsAreaActive())
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
                state = Alita_State.DASHING;
                Debug.Log("DASH");
                if (areaAttk != null)
                    areaAttk.HideArea();

                //Anim
                if (anim != null)
                    anim.PlayDash();
            }
        }
    }

    private void CheckForSPAttack()
    {
        if (Input.GetKeyDown(KeyCode.KEY_Q))
        {
            areaAttk.ToggleAreaVisibility();
        }

        if (Input.GetMouseButton(MouseKeyCode.MOUSE_LEFT))
        {
            bool areaActive = false;
            areaActive = areaAttk.HideArea();

            if (areaActive)
                state = Alita_State.AREA_ATTK;

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
            state = Alita_State.RUNNING;
            agent.maxSpeed = speed;
            agent.maxAcceleration = acceleration;

            //Anim
            if (anim != null)
                anim.PlayRunning();

            if (agent != null)
            {
                Debug.Log("GOING TO SPOT");
                agent.SetDestination(hit.point);
                placeToGo = hit.point;
            }
            else
                Debug.Log("AGENT IS NULL");
        }
    }

    private void DecideIfGoToAttack()
    {
        Ray ray = Physics.ScreenToRay(Input.GetMousePosition(), Camera.main);
        RaycastHit hit;
        if (Physics.Raycast(ray, out hit, float.MaxValue, enemyMask, SceneQueryFlags.Dynamic | SceneQueryFlags.Static))
        {
            Debug.Log("TE PARTO LA CARA PAVO");
            //Go to attack
            enemy = hit.gameObject;
            if (enemy != null)
            {
                state = Alita_State.GOING_TO_ATTK;
                agent.maxSpeed = speed;
                agent.maxAcceleration = acceleration;

                //Anim
                if (anim != null)
                    anim.PlayRunning();

                //Determine a place a little further than enemy position
                Vector3 enemy_fwrd_vec = (transform.position - enemy.transform.position).normalized();
                Vector3 enemy_pos = enemy.transform.position + enemy_fwrd_vec * attack_dist;

                agent.SetDestination(enemy_pos);
                Debug.Log(enemy_pos.ToString());
                enemy_unit = enemy.GetComponent<Unit>();

                Debug.Log("GOING TO ENEMY");
            }
            else
                Debug.Log("ENEMY IS NULL");
        }
    }

    private void CheckIfRotateToEnem()
    {
        Vector3 faceDirection = (enemy.transform.position - transform.position).normalized();
        double targetDegrees = Math.Atan2(faceDirection.x, faceDirection.z) * Rad2Deg;

        if (targetDegrees > minAngle)
            transform.rotation = Quaternion.Rotate(Vector3.up, (float)(targetDegrees));
    }


    //Attacks
    //---------------------------------------------------------------------------------------------------------------------------------

    private void NormalAttack()
    {
        attk_cool_down += Time.deltaTime;

        //if (audioSource != null)
        //    ModuleAudio.Instance.CleanAudio(audioSource);

        //Attack every second
        if (attk_cool_down >= attk_period)
        {
            if (enemy != null && enemy_unit != null)
            {
                enemy_unit.Hit(damage);
                if (audioSource != null)
                {
                    ModuleAudio.Instance.ResetIdleCounter();
                    ModuleAudio.Instance.CanPlayRunning();
                    ModuleAudio.Instance.CleanAudio(audioSource);
                    ModuleAudio.Instance.PlayRandomeSound(audioSource, ModuleAudio.Instance.hitScream_1,
                                                                      ModuleAudio.Instance.hitScream_2,
                                                                      ModuleAudio.Instance.hitScream_3);
                }
                Debug.Log("ENEMY HIT");
            }

            attk_cool_down = 0.0f;

            if (!Input.GetMouseButton(MouseKeyCode.MOUSE_LEFT))
            {
                state = Alita_State.IDLE;

                //Anim
                if (anim != null)
                    anim.PlayIdle();
            }
        }
    }
}
