using System.Collections;
using JellyBitEngine;

public class CameraFollow : JellyScript
{
    public Transform followingTransform;
    public float rangeQuadMove_X;
    public float rangeQuadMove_Y;
    public float smothing = 0.5f;

    Vector3 offset;
    Vector3 initialOffset;
    float alitaInitial_X;
    float alitaInitial_Y; 
    bool moving;
    //Use this method for initialization
    public override void Awake()
    {
        initialOffset = transform.position - followingTransform.position;
        alitaInitial_X = followingTransform.position.x;
        alitaInitial_Y = followingTransform.position.z;
        moving = false;
    }

    //Called every frame
    public override void Update()
    {
        if (!moving)
        {
            offset = transform.position - followingTransform.position;
            if(isOutQuad(offset - initialOffset))
                moving = true;
          //  Debug.Log("Stopped");
        }
        else
        {
            initialOffset = transform.position - followingTransform.position;
            if (isInQuad(offset - initialOffset))
                moving = false;
            transform.position = followingTransform.position + offset;
          //  Debug.Log((initialOffset - offset).ToString());
        }
    }

    bool isOutQuad(Vector3 finalOffset)
    {
        return (finalOffset.x > rangeQuadMove_X || finalOffset.x < -rangeQuadMove_X
            || finalOffset.z > rangeQuadMove_Y || finalOffset.z < -rangeQuadMove_Y);
    }

    bool isInQuad(Vector3 finalOffset)
    {
        return (finalOffset.x <= rangeQuadMove_X && finalOffset.x >= -rangeQuadMove_X
            && finalOffset.z <= rangeQuadMove_Y && finalOffset.z >= -rangeQuadMove_Y);
    }

    public override void OnDrawGizmos()
    {
        Debug.DrawLine(new Vector3(rangeQuadMove_X + alitaInitial_X, 0.1f, rangeQuadMove_Y + alitaInitial_Y), new Vector3(alitaInitial_X - rangeQuadMove_X, 0.1f, rangeQuadMove_Y + alitaInitial_Y), Color.Green);
        Debug.DrawLine(new Vector3(rangeQuadMove_X + alitaInitial_X, 0.1f, rangeQuadMove_Y + alitaInitial_Y), new Vector3(alitaInitial_X + rangeQuadMove_X, 0.1f, alitaInitial_Y - rangeQuadMove_Y), Color.Green);
        Debug.DrawLine(new Vector3(alitaInitial_X - rangeQuadMove_X, 0.1f, alitaInitial_Y - rangeQuadMove_Y), new Vector3(alitaInitial_X + rangeQuadMove_X, 0.1f, alitaInitial_Y - rangeQuadMove_Y), Color.Green);
        Debug.DrawLine(new Vector3(alitaInitial_X - rangeQuadMove_X, 0.1f, alitaInitial_Y - rangeQuadMove_Y), new Vector3(alitaInitial_X - rangeQuadMove_X, 0.1f, rangeQuadMove_Y + alitaInitial_Y), Color.Green);
    }
}

