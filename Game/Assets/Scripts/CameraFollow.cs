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
    bool moving;
    bool freeze;
    //Use this method for initialization
    public override void Awake()
    {
        
        initialOffset = transform.position - followingTransform.position;
        objetctInitial = lastFramePos = followingTransform.position;

        moving = false;
        freeze = false;
    }

    //Called every frame
    public override void Update()
    {
        //scenealeix
        Vector3 objectPos = followingTransform.position;
        if (!moving)
        {
            offset = transform.position - objectPos;
            Debug.Log("Stoped: " + (offset - initialOffset).ToString());
            if(isOutQuad(offset - initialOffset) && !freeze)
                moving = true;
            if (freeze)
                freeze = false;
        }
        else
        {
            transform.position = objectPos + offset;
            objetctInitial = transform.position - initialOffset;
            Debug.Log("--------------------");
            if (isInQuad(objectPos - objetctInitial, lastFramePos - objetctInitial))
            {
                moving = false;
                freeze = true;
            }
            lastFramePos = objectPos;
        }
    }

    bool isOutQuad(Vector3 finalOffset)
    {
        return (finalOffset.x > rangeQuadMove_X || finalOffset.x < -rangeQuadMove_X
            || finalOffset.z > rangeQuadMove_Y || finalOffset.z < -rangeQuadMove_Y);
    }

    bool isInQuad(Vector3 offsetCurr, Vector3 offsetLast)
    {
        bool ret = false;
        offsetCurr.x = Math.Abs(offsetCurr.x);
        offsetCurr.z = Math.Abs(offsetCurr.z);
        offsetLast.x = Math.Abs(offsetLast.x);
        offsetLast.z = Math.Abs(offsetLast.z);

        Debug.Log(offsetCurr.ToString());
        Debug.Log(offsetLast.ToString());
        if(offsetCurr.x < offsetLast.x && offsetCurr.z < offsetLast.z)
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

