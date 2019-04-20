using System.Collections;
using System;
using JellyBitEngine;

public class CameraFollow : JellyScript
{
    public Transform followingTransform;
    public float rangeQuadMove_X;
    public float rangeQuadMove_Y;
    public bool debugDraw;
    public float smothing = 0.5f;

    Vector3 offset;
    Vector3 initialOffset;
    Vector3 objetctInitial;
    Vector3 lastFramePos;
    bool movingX;
    bool movingZ;
    bool freeze;
    //Use this method for initialization
    public override void Awake()
    {
        
        initialOffset = transform.position - followingTransform.position;
        objetctInitial = lastFramePos = followingTransform.position;

        movingX = movingZ = freeze = false;
    }

    //Called every frame
    public override void Update()
    {
        Vector3 objectPos = followingTransform.position;
        if (!movingZ && !movingX)
        {
            offset = transform.position - objectPos;
            if(!freeze)
                OutQuad(offset - initialOffset);
            if (freeze)
                freeze = false;
        }
        else
        {
            transform.position = objectPos + offset;
            objetctInitial = transform.position - initialOffset;
            if (isInQuad(objectPos - objetctInitial, lastFramePos - objetctInitial))
                freeze = true;

            lastFramePos = objectPos;
        }
    }

    void OutQuad(Vector3 finalOffset)
    {
        if(finalOffset.x > rangeQuadMove_X || finalOffset.x < -rangeQuadMove_X)
            movingX = true;

        if(finalOffset.z > rangeQuadMove_Y || finalOffset.z < -rangeQuadMove_Y)
            movingZ = true;
    }

    bool isInQuad(Vector3 offsetCurr, Vector3 offsetLast)
    {
        bool ret = false;
        offsetCurr.x = Math.Abs(offsetCurr.x);
        offsetCurr.z = Math.Abs(offsetCurr.z);
        offsetLast.x = Math.Abs(offsetLast.x);
        offsetLast.z = Math.Abs(offsetLast.z);

        if (offsetCurr.x < offsetLast.x)
            movingX = false;
        if (offsetCurr.z < offsetLast.z)
            movingZ = false;
        if (!movingZ && !movingX)
            ret = true;

        return ret;
    }

    public override void OnDrawGizmos()
    {
        if (debugDraw)
        {
            Debug.DrawLine(new Vector3(objetctInitial.x + rangeQuadMove_X, 0.1f, objetctInitial.z + rangeQuadMove_Y), new Vector3(objetctInitial.x - rangeQuadMove_X, 0.1f, objetctInitial.z + rangeQuadMove_Y), Color.Green);
            Debug.DrawLine(new Vector3(objetctInitial.x + rangeQuadMove_X, 0.1f, objetctInitial.z + rangeQuadMove_Y), new Vector3(objetctInitial.x + rangeQuadMove_X, 0.1f, objetctInitial.z - rangeQuadMove_Y), Color.Green);
            Debug.DrawLine(new Vector3(objetctInitial.x - rangeQuadMove_X, 0.1f, objetctInitial.z - rangeQuadMove_Y), new Vector3(objetctInitial.x + rangeQuadMove_X, 0.1f, objetctInitial.z - rangeQuadMove_Y), Color.Green);
            Debug.DrawLine(new Vector3(objetctInitial.x - rangeQuadMove_X, 0.1f, objetctInitial.z - rangeQuadMove_Y), new Vector3(objetctInitial.x - rangeQuadMove_X, 0.1f, objetctInitial.z + rangeQuadMove_Y), Color.Green);
        }
    }
}

