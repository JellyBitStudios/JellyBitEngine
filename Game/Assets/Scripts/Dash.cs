using System.Collections;
using JellyBitEngine;
using System;

public class Dash : JellyScript
{
    //Dash propeties
    public float newDashDist = 5.0f;
    public float maxDashSpeed = 200.0f;
    public float maxDashAcc = 200.0f;
    public float coolDownTime = 0.2f;

    public bool coolDown = false;
    private float coolDwnTimr = 0.0f;

    //Place to go
    private Vector3 placeToGo = new Vector3(0, 0, 0);

    // Raycast
    public LayerMask terrainMask = new LayerMask();

    //Cooling down
    private float time_to_stop = 1.5f;
    private float time_increment = 0.0f;


    public override void Awake()
    {

    }

    //DASH
    //-----------------------------------------------------------------------------------------------------------------------

    public bool StartDash(NavMeshAgent agent)
    {
        Ray ray = Physics.ScreenToRay(Input.GetMousePosition(), Camera.main);
        RaycastHit hit;

        if (Physics.Raycast(ray, out hit, float.MaxValue, terrainMask, SceneQueryFlags.Dynamic | SceneQueryFlags.Static))
        {
            agent.maxSpeed = maxDashSpeed;
            agent.maxAcceleration = maxDashAcc;

            Vector3 mouse_pos = hit.point;
            Vector3 direction = (mouse_pos - transform.position).normalized();

            //Go to the place dashing
            agent.RequestMoveVelocity(direction * maxDashSpeed);
            //Place where you are right now
            placeToGo = transform.position;

            return true;
        }
        return false;
    }

    //Dash end
    //-----------------------------------------------------------------------------------------------------------------------

    public bool CheckForDashEnd(NavMeshAgent agent)
    {
        bool dashing = true;
        time_increment += Time.deltaTime;

        float distDash = (float)(transform.position - placeToGo).magnitude;
        Debug.Log(distDash.ToString());
        if (distDash >= newDashDist || time_increment >= time_to_stop)
        {
            if (agent != null)
                agent.ResetMoveTarget();
            coolDown = true;
        }

        if (coolDown)
            dashing = CoolingDown();

        return dashing;
    }

    private bool CoolingDown()
    {
        coolDwnTimr += Time.deltaTime;

        if (coolDwnTimr >= coolDownTime)
        {
            Debug.Log("COOLED");
            coolDwnTimr = 0.0f;
            time_increment = 0.0f;
            coolDown = false;
            return false;
        }

        return true;
    }
}