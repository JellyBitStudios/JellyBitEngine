using System.Collections;
using JellyBitEngine;

public class MoveBurstForward : JellyScript
{
    //Death of the burst
    public float timeUntilDeath = 3.0f;
    private float timeToDie = 0.0f;

    //Burst properties
    public float moveVelocity = 2.0f;
    private Vector3 direction = new Vector3(0,0,0);
    private bool directionGot = false;
    public GameObject dirGameObject;

    //Attack properties
    public LayerMask enemyMask = new LayerMask();
    public float attackRadius = 5.0f;
    public int burstDamage = 20;

    //Hit timer
    public float timeToCheckHit = 0.05f; //20 times x sec
    private float checkHitTimer = 0.0f;


    public override void Awake()
    {
        direction = dirGameObject.transform.forward.normalized();
        directionGot = true;
    }

    public override void Update()
    {
        //Direction check
        if (!directionGot)
        { 
            direction = dirGameObject.transform.forward.normalized();
            directionGot = true;
        }

        //Move
        MoveForward();

        //Hit
        checkHitTimer += Time.deltaTime;
        if(checkHitTimer >= timeToCheckHit)
            CheckIfHit();

        //Die
        CheckIfDeath();

    }

    public void SetDirecction(Vector3 dir)
    {
        direction = dir;
    }

    private void MoveForward()
    {
        transform.position = transform.position + (direction * moveVelocity * Time.deltaTime);
    }

    private void CheckIfHit()
    {
        Debug.Log("AREA ATTACK!!!!!");

        OverlapHit[] hitInfo;
        if (Physics.OverlapSphere(attackRadius, transform.position, out hitInfo, enemyMask, SceneQueryFlags.Dynamic | SceneQueryFlags.Static))
        {
            foreach (OverlapHit hit in hitInfo)
            {
                hit.gameObject.GetComponent<Unit>().Hit(burstDamage); //Not change this
                Debug.Log("HIT ENEMY: " + hit.gameObject.name);
            }
        }

        checkHitTimer = 0.0f;
    }

    private void CheckIfDeath()
    {
        timeToDie += Time.deltaTime;

        if (timeToDie >= timeUntilDeath)
            Destroy(gameObject);
    }

}