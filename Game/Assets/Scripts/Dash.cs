using System.Collections;
using JellyBitEngine;
using System;

public class Dash : JellyScript
{
    //Dash propeties
    public float dashLength = 7.0f;
    public float stopRadius = 3.0f;
    public float maxDashSpeed = 200.0f;
    public float maxDashAcc = 200.0f;
    public float coolDownTime = 0.2f;

    public bool coolDown = false;
    private float coolDwnTimr = 0.0f;

    //Place to go
    private Vector3 placeToGo = new Vector3(0, 0, 0);

    // Raycast
    public LayerMask terrainMask = new LayerMask();
    public LayerMask wallMask = new LayerMask();

    //Bad fix
    private float time_to_stop = 1.5f;
    private float time_increment = 0.0f;

    //New dash!!!
    public float newDashDist = 5.0f;

    public override void Awake()
    {

    }

    //DASH
    //-----------------------------------------------------------------------------------------------------------------------

    public bool DashPush(NavMeshAgent agent)
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



    public bool ExecuteDash(NavMeshAgent agent)
    {
        Ray ray = Physics.ScreenToRay(Input.GetMousePosition(), Camera.main);
        RaycastHit hit;
        if (agent != null)
        {
            if (Physics.Raycast(ray, out hit, float.MaxValue, terrainMask, SceneQueryFlags.Dynamic | SceneQueryFlags.Static))
            {
                agent.maxSpeed = maxDashSpeed;
                agent.maxAcceleration = maxDashAcc;
                agent.obstacleAvoidance = false;

                Vector3 mouse_pos = hit.point;
                Debug.Log((mouse_pos - transform.position).magnitude.ToString());
                Vector3 direction = (mouse_pos - transform.position).normalized();

                //Determine where to stop dashing
                if ((mouse_pos - transform.position).magnitude < stopRadius)
                {
                    //If moving and mouse near Gally, dash
                    if (Input.GetMouseButton(MouseKeyCode.MOUSE_RIGHT))
                        direction = transform.forward.normalized();
                    else
                    {
                        //If the agent is not moving and the mouse is near Gally don't dash
                        agent.maxSpeed = 0.0f;
                        return false;
                    }
                }
                Vector3 dash_pos = transform.position + direction * dashLength;

                //Check if there's a wall in front of Gally
                bool iWallInFront = CheckIfWallInFront(agent, direction);

                if (iWallInFront)
                    return false;

                //Go to the place dashing
                agent.SetDestination(dash_pos);
                placeToGo = dash_pos;

                return true;
            }
        }
        return false;
    }

    //Check wall
    //-----------------------------------------------------------------------------------------------------------------------

    public bool CheckIfWallInFront(NavMeshAgent agent, Vector3 direction)
    {
        Ray rayWall = new Ray();
        rayWall.position = gameObject.transform.position;
        rayWall.direction = direction;
        rayWall.length = dashLength;

        RaycastHit hitWall;
        if (Physics.Raycast(rayWall, out hitWall, float.MaxValue, wallMask, SceneQueryFlags.Dynamic | SceneQueryFlags.Static))
        {
            Debug.Log("Cannot Go There F");
            agent.maxSpeed = 0.0f;
            return true;
        }
        return false;
    }

    //Dash end
    //-----------------------------------------------------------------------------------------------------------------------

    public bool CheckForPushDashEnd(NavMeshAgent agent)
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

    public bool CheckForDashEnd(NavMeshAgent agent)
    {
        bool dashing = true;
        time_increment += Time.deltaTime;

        float distDash = (float)(placeToGo - transform.position).magnitude;
        if (distDash <= stopRadius || time_increment >= time_to_stop)
        {
            if (agent != null)
                agent.maxSpeed = 0.0f;
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