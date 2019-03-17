using System.Collections;
using JellyBitEngine;

public class CameraMove : JellyScript
{
    public GameObject alita;
    Vector3 startPos;
    Vector3 alitaPos = new Vector3(0, 0, 0);

    public override void Awake()
    {
        startPos = gameObject.transform.position;
        Debug.Log(startPos.ToString());
    }

    public override void Update()
    {
        if(startPos == null)
            startPos = gameObject.transform.position;

        alitaPos = alita.transform.position;
        Vector3 currentPos = new Vector3(startPos.x + alitaPos.x, startPos.y, startPos.z + alitaPos.z);

        //Update pos
        transform.position = currentPos;
    }

}
