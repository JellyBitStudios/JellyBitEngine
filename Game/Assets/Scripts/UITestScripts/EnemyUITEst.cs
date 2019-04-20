using System.Collections;
using JellyBitEngine;

public class EnemyUITEst : JellyScript
{

    private GameObject barlife_position = null;

    //Use this method for initialization
    public override void Awake()
    {
        foreach (GameObject go in gameObject.childs)
        {
            if (go.name == "BarLifePosition")
                barlife_position = go;
        }
    }

    //Called every frame
    public override void Update()
    {

    }

    public Vector3 GetLifePosition()
    {
        Vector3 ret = Vector3.zero;
        if(barlife_position != null)
            ret = barlife_position.transform.position;
        return ret;
    }
}

