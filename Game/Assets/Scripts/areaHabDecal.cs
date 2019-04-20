using System.Collections;
using JellyBitEngine;

public class areaHabDecal : JellyScript
{
    public enum AreaHab
    {
        Q_area,
        W_area
    };

    AreaHab currHab = AreaHab.Q_area;
    Projector projector;
    //Use this method for initialization
    public override void Awake()
    {
        projector = gameObject.GetComponent<Projector>();
    }

    //Called every frame
    public override void Update()
    {
        Debug.Log("Working");
        if (Input.GetKeyDown(KeyCode.KEY_Q))
        {
            Debug.Log("in");
            if (currHab != AreaHab.Q_area)
            {
                transform.localPosition = new Vector3(0, 21, 0);
                projector.FOV = 140;
                projector.SetResource("Q_areaDecal");
                currHab = AreaHab.Q_area;
            }
            gameObject.active = true;
        }
        else if (Input.GetKeyDown(KeyCode.KEY_W))
        {
            if (currHab != AreaHab.W_area)
            {
                transform.localPosition = new Vector3(0, 21, 10.5f);
                projector.FOV = 160;
                projector.SetResource("W_areaDecal");

                currHab = AreaHab.W_area;
            }
            gameObject.active = true;
        }

        if (Input.GetKeyUp(KeyCode.KEY_Q) || Input.GetKeyUp(KeyCode.KEY_W))
            gameObject.active = false;
    }
}

