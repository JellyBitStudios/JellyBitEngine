using System.Collections;
using JellyBitEngine;

public class ArrayReferencer : JellyScript
{
    private Vector3[] path;

    private Vector3 destination;

    private bool walking = false;

    public float speed = 1f;

    //Use this method for initialization
    public override void Awake()
    {
        
    }

    //Called every frame
    public override void Update()
    {
        Debug.ClearConsole();

        if (path != null)
        {
            Debug.Log(path.Length.ToString());

            foreach (Vector3 point in path)
            {
                Debug.Log(point.ToString());
            }
        }

        if(Input.GetMouseButton(MouseKeyCode.MOUSE_RIGHT))
        {
            Ray ray = Physics.ScreenToRay(Input.GetMousePosition(), Camera.main);
            RaycastHit hit;
            if(Physics.Raycast(ray, out hit, float.MaxValue, 0, SceneQueryFlags.Dynamic | SceneQueryFlags.Static))
            {
                destination = hit.point;
                destination.y = 1f;

                if (Navigation.GetPath(gameObject.transform.position, destination, out path))
                {
                    Debug.Log("Start walking");
                    walking = true;
                }
            }
        } 

        if(gameObject.transform.position == destination)
        {
            Debug.Log("Stop walking");
            walking = false;
        }

        if(walking)
        {
            transform.position += (destination - transform.position).normalized() * speed * Time.deltaTime;
        }

    }
}

